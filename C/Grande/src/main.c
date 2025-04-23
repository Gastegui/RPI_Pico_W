#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/pwm.h"
#include "hardware/i2c.h"
#include "hardware/spi.h"
#include "constantes.h"
#include "core0.h"
#include "core1.h"
#include "rtc.h"


#include "uno.h"

HARDWARE hw;

void gpio_callback(uint gpio, uint32_t events)
{ 
    ENCODER* encoder = NULL;
    uint8_t a = 0;

    BOTON* boton = NULL;

    uint64_t tiempo = 0;

    switch(gpio)
    {
        case PIN_ENCODER_1_A:
        case PIN_ENCODER_1_B:
            encoder = &hw.encoder_1;
            a = PIN_ENCODER_1_A;
            break;
        case PIN_ENCODER_2_A:
        case PIN_ENCODER_2_B:
            encoder = &hw.encoder_2;
            a = PIN_ENCODER_2_A;
            break;
        case PIN_ENCODER_3_A:
        case PIN_ENCODER_3_B:
            encoder = &hw.encoder_3;
            a = PIN_ENCODER_3_A;
            break;

        case PIN_ENCODER_1_BOTON:            
            boton = &hw.encoder_1.boton;
            break;
        case PIN_ENCODER_2_BOTON:
            boton = &hw.encoder_2.boton;
            break;
        case PIN_ENCODER_3_BOTON:
            boton = &hw.encoder_3.boton;
            break;
        default:
            break;
    }

    //Encoder
    if(gpio == a)
    {
        if(events & GPIO_IRQ_EDGE_RISE)
        {
            encoder->valor = false;
            if(encoder->b == false)
                encoder->a = true;
            else
            {
                encoder->derecha = true;
                encoder->valor = true;
            }
        }
        else if(events & GPIO_IRQ_EDGE_FALL)
            encoder->a = false;
    }
    else if(gpio == a+1)
    {
        if(events & GPIO_IRQ_EDGE_RISE)
        {
            encoder->valor = false;
            if(encoder->a == false)
                encoder->b = true;
            else
            {
                encoder->derecha = false;
                encoder->valor = true;
            }
        }
        else if(events & GPIO_IRQ_EDGE_FALL)
            encoder->b = false;
    }

    //Boton
    if(boton != NULL)
    {
        tiempo = time_us_64();
        if(boton->tiempo + BOTON_DEBOUNCE < tiempo)
        {
            if(boton->mantener)
                boton->pulsado = events & GPIO_IRQ_EDGE_RISE;
            else
                boton->pulsado = events & GPIO_IRQ_EDGE_RISE ? 1 : boton->pulsado;

            boton->tiempo = tiempo;
        }
    }

}

void init()
{
    init_gpio_normal(PIN_LED_VERDE, GPIO_OUT);
    init_gpio_normal(PIN_LED_AMARILLO, GPIO_OUT);
    init_gpio_normal(PIN_LED_ROJO, GPIO_OUT);
    init_gpio_normal(PIN_SD_LED, GPIO_OUT);
    init_gpio_normal(PIN_LED_ABORTAR, GPIO_OUT);

    hw.led_verde = false;
    hw.led_amarillo = false;
    hw.led_rojo = false;

    init_encoder(PIN_ENCODER_1_A, PIN_ENCODER_1_B, PIN_ENCODER_1_BOTON, &hw.encoder_1, &gpio_callback);
    init_encoder(PIN_ENCODER_2_A, PIN_ENCODER_2_B, PIN_ENCODER_2_BOTON, &hw.encoder_2, &gpio_callback);
    init_encoder(PIN_ENCODER_3_A, PIN_ENCODER_3_B, PIN_ENCODER_3_BOTON, &hw.encoder_3, &gpio_callback);

    init_pantalla(&hw.pantalla);

    init_sd(&hw.sd, &hw.pantalla);
}



void main()
{
    stdio_init_all();
    
    time_init();

    init();
    char str[128] = "1234567890abc defghijkl mnopqrstuvwxyz.-";
    oled_print(&hw.pantalla, str, strlen(str)+1);
    oled_mandar_pantalla(&hw.pantalla);

    sleep_ms(500);

    memset(hw.pantalla.bufTexto, 0, sizeof(hw.pantalla.bufTexto));
    oled_cargar(&hw.pantalla, &hw.sd, "0.txt");
    oled_mandar_pantalla(&hw.pantalla);

    uint8_t contador = 0;

    while(1)
    {
        if(encoder_valor(&hw.encoder_1))
        {
            char str[10];
            
            if(encoder_derecha(&hw.encoder_1))
            {
                contador++;
            }
            else
            {
                if(contador != 0)
                    contador--;
            }
            
            sprintf(str, "%d.txt", contador);
            printf(str);

            oled_cargar(&hw.pantalla, &hw.sd, str);
            oled_mandar_pantalla(&hw.pantalla);

            encoder_limpiar(&hw.encoder_1);
        }

        if(encoder_boton(&hw.encoder_2))
        {
            deinit_sd(&hw.sd, &hw.pantalla);
            encoder_boton_limpiar(&hw.encoder_2);
        }

    }

    while(1);
}
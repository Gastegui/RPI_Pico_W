//sudo picocom /dev/ttyACM0 --omap crlf
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/pwm.h"

#define LED1 16
#define LED2 17
#define BOTON_A 18
#define BOTON_B 19
#define SERVO 15

int led = LED1;
int32_t tiempo = 0;


typedef struct s_encoder
{
    bool a;
    bool b;
    bool derecha;
    bool valor;
} ENCODER;

ENCODER encoder = {0};

void core1_entry()
{
    printf("Iniciado core 1\n");
    
    char input[100];
    
    while(1)
    {
        fgets(input, 10, stdin);
        printf("Recibido: %s\n", input);
        printf("Caracter a caracter: ");
        for(int i = 0; i < strlen(input); i++)
        {
            printf("[%d]", input[i]);
        }
        printf(" fin\n");
        switch(input[0])
        {
            case 'd':
                encoder.derecha = true;
                encoder.valor = true;
                break;
            case 'i':
                encoder.derecha = false;
                encoder.valor = true;
            default:
                break;
        }
        input[0] = '\0';
    }
}

void gpio_callback(uint gpio, uint32_t events)
{
    switch(gpio)
    {
        case BOTON_A:
            if(events & GPIO_IRQ_EDGE_RISE)
            {
                encoder.valor = false;
                if(encoder.b == false)
                    encoder.a = true;
                else
                {
                    encoder.derecha = true;
                    encoder.valor = true;
                }
            }
            else if(events & GPIO_IRQ_EDGE_FALL)
                encoder.a = false;
            break;
        case BOTON_B:
            if(events & GPIO_IRQ_EDGE_RISE)
            {
                encoder.valor = false;
                if(encoder.a == false)
                    encoder.b = true;
                else
                {
                    encoder.derecha = false;
                    encoder.valor = true;
                }
            }
            else if(events & GPIO_IRQ_EDGE_FALL)
                encoder.b = false;
            break;
        default:
            break;
    }

}

int main()
{
    stdio_init_all();

    gpio_init(LED1);
    gpio_set_dir(LED1, GPIO_OUT);
    gpio_init(LED2);
    gpio_set_dir(LED2, GPIO_OUT);
    gpio_init(BOTON_A);
    gpio_set_dir(BOTON_A, GPIO_IN);
    gpio_init(BOTON_B);
    gpio_set_dir(BOTON_B, GPIO_IN);
    
    sleep_ms(2000);
    multicore_launch_core1(core1_entry);

    gpio_set_irq_enabled_with_callback(BOTON_A, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    gpio_set_irq_enabled_with_callback(BOTON_B, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);

    gpio_set_function(SERVO, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(SERVO);
    uint chann = pwm_gpio_to_channel(SERVO);
    uint16_t period = 2500;
    pwm_set_wrap(slice_num, period);
    pwm_set_chan_level(slice_num, chann, 188);
    pwm_set_enabled(slice_num, true);

    bool encendido = true;

    while(1)
    {
        if(encoder.valor)
        {
            led = encoder.derecha ? LED1 : LED2;
            encoder.valor = false;
        }
        gpio_put(led, encendido = !encendido);
        if(encendido)
            pwm_set_chan_level(slice_num, chann, (uint16_t)125);
        else
            pwm_set_chan_level(slice_num, chann, (uint16_t)250);
        sleep_ms(500);
    }
}
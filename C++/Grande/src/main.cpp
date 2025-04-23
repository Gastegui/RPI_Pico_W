#include <iostream>
#include <map>
#include <pico/stdlib.h>

#include "../headers/Boton.h"
#include "../headers/Buzzer.h"
#include "../headers/Encoder.h"
#include "../headers/ICallback.h"
#include "../headers/Led.h"
#include "../headers/Oled.h"
#include "../headers/CONSTANTES.h"
#include "../headers/core1.h"
#include "pico/cyw43_arch.h"
#include "pico/multicore.h"

std::map<uint, ICallback*> callbacks;
Oled oled{PIN_OLED_SDA, PIN_OLED_SCL};

void callback(const uint gpio, const uint32_t events)
{
    if (events & GPIO_IRQ_EDGE_RISE)
    {
        if (callbacks.contains(gpio) && callbacks[gpio]->getRise())
            callbacks[gpio]->rise(gpio);
    }
    else if (events & GPIO_IRQ_EDGE_FALL)
    {
        if (callbacks.contains(gpio) && callbacks[gpio]->getFall())
            callbacks[gpio]->fall(gpio);
    }
}

void abortar(const char* mensaje)
{
    printf(mensaje);
    oled.print(mensaje);
    oled.mandarPantalla();

    /*
    if(hw.sd.montado)
        deinit_sd(&hw.sd, NULL);
    */

    gpio_init(PIN_LED_ABORTAR);
    gpio_set_dir(PIN_LED_ABORTAR, GPIO_OUT);
    gpio_put(PIN_LED_ABORTAR, true);
    while (true);
}

void encoderCallback(Encoder* encoder)
{
    callbacks.insert({encoder->getPinA(), encoder});
    callbacks.insert({encoder->getPinB(), encoder});
    callbacks.insert({encoder->getBoton()->getPin(), encoder->getBoton()});
}

int main()
{
    stdio_init_all();

    Buzzer buzzer{PIN_BUZZER};

    buzzer.stop_sound();
    sleep_ms(1000);  // Wait 1 second

    buzzer.play_megalovania(100);
    sleep_ms(1000);  // Wait 1 second

    buzzer.play_zelda_theme(100);


    int segs = 0;
    int mins = 0;
    int horas = 0;
    uint8_t izquierdas = 0;
    uint8_t derechas = 0;
    char str[] = "XX:XX:XX";

    Encoder e1(PIN_ENCODER_1_A, PIN_ENCODER_1_B, PIN_ENCODER_1_BOTON, false, &callback);
    Encoder e2(PIN_ENCODER_2_A, PIN_ENCODER_2_B, PIN_ENCODER_2_BOTON, false, &callback);
    Encoder e3(PIN_ENCODER_3_A, PIN_ENCODER_3_B, PIN_ENCODER_3_BOTON, false, &callback);
    encoderCallback(&e1);
    encoderCallback(&e2);
    encoderCallback(&e3);
    printf("AAAA\n");
    while (true)
    {
        sprintf(str, "%d:%d:%d", horas, mins, segs);
        printf("%s\n", str);
        oled.cursor(0,0);
        oled.limpiar();
        oled.print(str);
        oled.mandarPantalla();
        sleep_ms(500);
        sprintf(str, "%d %d %d", horas, mins, segs);
        oled.cursor(0,0);
        oled.limpiar();
        oled.print(str);
        oled.mandarPantalla();

        e1.get(&derechas, &izquierdas);
        segs += derechas - izquierdas;
        e2.get(&derechas, &izquierdas);
        mins += derechas - izquierdas;
        e3.get(&derechas, &izquierdas);
        horas += derechas - izquierdas;

        segs++;
        if(segs >= 60)
        {
            mins += segs/60;
            segs -= 60 * (segs/60);
        }
        if(mins >= 60)
        {
            horas += mins/60;
            mins -= 60 * (mins/60);
        }
        if(horas >= 24)
        {
            horas -= 24 * (horas/24);
        }
        sleep_ms(500);
    }
}

int main2()
{
    stdio_init_all();

    multicore_launch_core1(&core1);

    Led amarillo{PIN_LED_AMARILLO, false};
    Led rojo{PIN_LED_ROJO, false};

    oled.print("Hello World!");
    oled.mandarPantalla();

    Encoder encoderI{PIN_ENCODER_1_A, PIN_ENCODER_1_B, PIN_ENCODER_1_BOTON, true, &callback};
    Encoder encoderC{PIN_ENCODER_2_A, PIN_ENCODER_2_B, PIN_ENCODER_2_BOTON, true, &callback};
    Encoder encoderD{PIN_ENCODER_3_A, PIN_ENCODER_3_B, PIN_ENCODER_3_BOTON, true, &callback};
    encoderCallback(&encoderI);
    encoderCallback(&encoderC);
    encoderCallback(&encoderD);

    while (true)
    {
        if (encoderI.get() == Encoder::Derecha && !amarillo.get())
            amarillo.encender();
        else if (encoderI.get() == Encoder::Izquierda && amarillo.get())
            amarillo.apagar();

        rojo.poner(encoderI.getPulsado());
    }


    while (true);
}

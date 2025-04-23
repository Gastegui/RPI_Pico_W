#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "pico/multicore.h"
#include <stdio.h>

#define BUZZER_PIN 15 // Cambia este pin según tu conexión

#define BOTON_A 16
#define BOTON_B 17

typedef struct s_encoder
{
    bool a;
    bool b;
    bool derecha;
    bool valor;
} ENCODER;

ENCODER encoder = {0};


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



// Configura el pin del buzzer para usar PWM
void setup_buzzer() {
    gpio_set_function(BUZZER_PIN, GPIO_FUNC_PWM);
}

// Genera una señal PWM con la frecuencia y duración especificadas
void play_tone(uint32_t freq, uint32_t duration_ms, float duty_cycle) {
    uint slice_num = pwm_gpio_to_slice_num(BUZZER_PIN);
    uint32_t clock_freq = 125000000; // Frecuencia de reloj por defecto de la Pico
    uint32_t divider = clock_freq / freq / 4096 + 1;
    uint32_t top = clock_freq / freq / divider - 1;

    pwm_set_clkdiv(slice_num, divider);
    pwm_set_wrap(slice_num, top);
    pwm_set_gpio_level(BUZZER_PIN, top * duty_cycle); // Ajusta el ciclo de trabajo
    pwm_set_enabled(slice_num, true);

    sleep_ms(duration_ms);

    pwm_set_enabled(slice_num, false); // Apaga el buzzer
}
int siguiente = 100;
void core1_entry()
{
    gpio_set_irq_enabled_with_callback(BOTON_A, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    gpio_set_irq_enabled_with_callback(BOTON_B, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);

    while(1)
    {

        if(encoder.valor)
        {
            siguiente += encoder.derecha ? 100 : -100;
            encoder.valor = false;
            printf("Encoder derecha = %d\n", encoder.derecha);
        }
    }
}

int main() {
    stdio_init_all();
    setup_buzzer();

    gpio_init(BOTON_A);
    gpio_set_dir(BOTON_A, GPIO_IN);
    gpio_init(BOTON_B);
    gpio_set_dir(BOTON_B, GPIO_IN);

    multicore_launch_core1(core1_entry);

    int tono;

    while (1) 
    {
        if(tono != siguiente)
        {
            printf("Tono nuevo puesto a: %d\n", tono);
            tono = siguiente;
        }
        play_tone(tono, 500, 0.5); // Tono de 1000 Hz durante 500 ms con un ciclo de trabajo del 50%
        sleep_ms(500);             // Espera 500 ms antes de repetir
    }

    return 0;
}

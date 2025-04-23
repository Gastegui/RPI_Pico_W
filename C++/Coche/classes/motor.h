#include "pico/stdlib.h"
#include <hardware/gpio.h>
#include <hardware/pwm.h>

class Motor
{
private:
    uint pinAlante;
    uint pinAtras;
    uint slice1;
    uint slice2;
    bool invertir = false;

    void set(float porcentaje, uint pin, uint slice)
    {
        if (porcentaje < 0.0f)
            porcentaje = 0.0f;
        if (porcentaje > 100.0f)
            porcentaje = 100.0f;

        float interpolatedPercentage = 40.0f + (porcentaje / 100.0f) * 60.0f;
        if(porcentaje == 0)
            interpolatedPercentage = 10;

        uint16_t level = static_cast<uint16_t>((interpolatedPercentage / 100.0f) * 4095);
        pwm_set_chan_level(slice, pwm_gpio_to_channel(pin), level);
        pwm_set_enabled(slice, true);
    }

public:

    Motor(uint _pinAlante, uint _pinAtras):
        pinAlante{_pinAlante}, pinAtras{_pinAtras}
    {
        slice1 = pwm_gpio_to_slice_num(pinAlante);
        slice2 = pwm_gpio_to_slice_num(pinAtras);

        gpio_set_function(pinAlante, GPIO_FUNC_PWM);
        gpio_set_function(pinAtras, GPIO_FUNC_PWM);

        // Calcular divisor
        uint32_t divider16 = 125000000 / (20000 * 4096);
        if (divider16 < 16)
            divider16 = 16; // Valor mínimo permitido
        if (divider16 > 256 * 16)
            divider16 = 256 * 16; // Valor máximo permitido

        pwm_set_clkdiv_int_frac(slice1, divider16 / 16, divider16 & 0xF);
        pwm_set_clkdiv_int_frac(slice2, divider16 / 16, divider16 & 0xF);

        // Configurar el envolvente de 0 a 4095
        pwm_set_wrap(slice1, 4095);
        pwm_set_wrap(slice2, 4095);
    }

    Motor(uint _pinAlante, uint _pinAtras, bool invertido):
        Motor(_pinAlante, _pinAtras)
    {
        invertir = true;
    }

    void parar()
    {
        pwm_set_chan_level(slice1, pwm_gpio_to_channel(pinAlante), 0);
        pwm_set_chan_level(slice2, pwm_gpio_to_channel(pinAtras), 0);
    }

    void alante(float porcentaje) 
    {
        parar();
        invertir ? set(porcentaje, pinAtras, slice2) : set(porcentaje, pinAlante, slice1);
    }

    void atras(float porcentaje) 
    {
        parar();
        invertir ? set(porcentaje, pinAlante, slice1) : set(porcentaje, pinAtras, slice2);
    }


};
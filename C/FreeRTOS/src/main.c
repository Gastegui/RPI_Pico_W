/**
 * Copyright (c) 2022 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>
#include <time.h>

#include "pico/stdlib.h"

#include "hardware/pwm.h"

void mi_pwm_init(int gpio, uint16_t wrap)
{
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    pwm_set_wrap(pwm_gpio_to_slice_num(gpio), wrap);
    pwm_set_chan_level(pwm_gpio_to_slice_num(gpio), pwm_gpio_to_channel(gpio), wrap);
    pwm_set_enabled(pwm_gpio_to_slice_num(gpio), true);
}

void mi_pwm_value(int gpio, uint16_t value)
{
    pwm_set_chan_level(pwm_gpio_to_slice_num(gpio), pwm_gpio_to_channel(gpio), value);
}

int main() 
{
    stdio_init_all();


    gpio_init(19);              //boton
    gpio_set_dir(19, GPIO_IN);  //boton

    for(int i = 13; i != 19; i++)
    {
        mi_pwm_init(i, 31250); 
        mi_pwm_value(i, 5000);
    }

    mi_pwm_init(5, 31250); //Buzzer
    mi_pwm_value(5, 5000);
    

    bool a = false;
    while(1)
    {
        if(gpio_get(19) && !a)
        {
            mi_pwm_value(5, 30000);
            a = true;
            for(int i = 13; i != 19; i++)
            {
                mi_pwm_value(i, 30000);
            }
        }
        else if (!gpio_get(19) && a)
        {
            mi_pwm_value(5, 5000);
            a = false;
            for(int i = 13; i != 19; i++)
            {
                mi_pwm_value(i, 5000);
            }
        }
    }

    return 0;
}
//
// Created by julen on 2/10/24.
//

#ifndef BOTON_H
#define BOTON_H
#include <hardware/gpio.h>

#include "ICallback.h"

class Boton : public ICallback
{
    volatile bool ultimo{false};
    volatile bool leido{false};
    bool mantener{false};

public:
    explicit Boton(const int pin, const bool mantener, const gpio_irq_callback_t callback)
        :ICallback(pin, true, true), mantener(mantener)
    {

        gpio_init(pin);
        gpio_set_dir(pin, GPIO_IN);

        gpio_set_irq_enabled_with_callback(pin, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, callback);
    }

    [[nodiscard]] bool get() { leido = true; return ultimo; }

    void set(const bool value) { ultimo = value; leido = false; }

    void rise(uint8_t pin) override
    {
        if(mantener)
        {
            ultimo = true;
            leido = false;
        }
    }

    void fall(uint8_t pin) override
    {
        if(mantener)
            ultimo = false;
        else
            ultimo = !ultimo;
        leido = false;
    }
};

#endif //BOTON_H

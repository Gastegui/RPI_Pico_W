//
// Created by julen on 2/10/24.
//

#ifndef LED_H
#define LED_H

#include <hardware/gpio.h>

class Led
{
    int pin;
    bool estado;

    void cambiar() const { gpio_put(pin, estado); }

public:
    Led(const int pin, const bool estado)
        : pin(pin), estado(estado)
    {
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_OUT);
    }

    void encender() { estado = true; cambiar(); }
    void apagar() { estado = false; cambiar(); }
    void alternar() { estado = !estado; cambiar(); }
    void poner(const bool estado_) { estado = estado_; cambiar(); }

    [[nodiscard]] bool get() const { return estado; }
};

#endif //LED_H

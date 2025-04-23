//
// Created by julen on 7/10/24.
//

#ifndef ENCODER_H
#define ENCODER_H
#include "Boton.h"
#include "ICallback.h"

class Encoder : public ICallback
{
    uint8_t pinA;
    uint8_t pinB;
    volatile bool a = false;
    volatile bool b = false;
    volatile bool valor = false;
    volatile bool derecha = false;
    volatile uint8_t derechas = 0;
    volatile uint8_t izquierdas = 0;
    Boton boton;

public:

    Encoder(const uint8_t pinA, const uint8_t pinB, const uint8_t pinBoton, const bool mantener, const gpio_irq_callback_t callback)
        : ICallback(0, true, true), pinA(pinA), pinB(pinB), boton(pinBoton, mantener, callback)
    {
        gpio_init(pinA);
        gpio_set_dir(pinA, GPIO_IN);

        gpio_init(pinB);
        gpio_set_dir(pinB, GPIO_IN);

        gpio_set_irq_enabled_with_callback(pinA, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, callback);
        gpio_set_irq_enabled_with_callback(pinB, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, callback);

    }

    enum ENCODER : uint8_t
    {
        Nada,
        Izquierda,
        Derecha
    };

    [[nodiscard]] ENCODER get()
    {
        if(!valor)
            return Nada;

        ENCODER ret = Izquierda;
        if(derecha)
            ret = Derecha;

        valor = false;
        return ret;
    }
    ENCODER get(uint8_t* retDerechas, uint8_t* retIzquierdas)
    {
        *retDerechas = derechas;
        *retIzquierdas = izquierdas;
        derechas = 0;
        izquierdas = 0;

        return get();
    }
    [[nodiscard]] ENCODER getConst() const
    {
        if(!valor)
            return Nada;
        if(derecha)
            return Derecha;
        return Izquierda;
    }
    ENCODER getConst(uint8_t* retDerechas, uint8_t* retIzquierdas) const
    {
        *retDerechas = derechas;
        *retIzquierdas = izquierdas;

        return getConst();
    }
    [[nodiscard]] Boton* getBoton() { return &boton; }
    [[nodiscard]] bool getPulsado() { return boton.get(); }
    [[nodiscard]] uint8_t getPinA() const { return pinA; }
    [[nodiscard]] uint8_t getPinB() const { return pinB; }


    void rise(const uint8_t pin) override
    {
        if(pin == pinA)
        {
            valor = false;
            if(b == false)
                a = true;
            else
            {
                derecha = true;
                valor = true;
                derechas += 1;
            }
        }
        else
        {
            valor = false;
            if(a == false)
                b = true;
            else
            {
                derecha = false;
                valor = true;
                izquierdas += 1;
            }
        }
    }

    void fall(const uint8_t pin) override
    {
        if(pin == pinA)
        {
            a = false;
        }
        else
        {
            b = false;
        }
    }
};

#endif //ENCODER_H

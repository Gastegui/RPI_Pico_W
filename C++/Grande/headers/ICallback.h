//
// Created by julen on 2/10/24.
//

#ifndef ICALLBACK_H
#define ICALLBACK_H

class ICallback
{
protected:
    int m_pin{-1};
    bool m_rise;
    bool m_fall;
public:
    virtual ~ICallback() = default;

    ICallback(const int pin, const bool rise, const bool fall)
        : m_pin(pin), m_rise(rise), m_fall(fall) {}

    [[nodiscard]] int getPin() const { return m_pin; }
    [[nodiscard]] bool getRise() const { return m_rise; }
    [[nodiscard]] bool getFall() const { return m_fall; }

    virtual void rise(uint8_t pin) = 0;
    virtual void fall(uint8_t pin) = 0;
};

#endif //ICALLBACK_H

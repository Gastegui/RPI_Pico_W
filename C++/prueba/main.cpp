#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

int main()
{
    stdio_init_all();
    if (cyw43_arch_init())
    {
        printf("Wi-Fi init failed");
        return -1;
    }
    while (true)
    {
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, true);
        sleep_ms(500);
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, false);
        sleep_ms(500);
    }
}

#include <string.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "lwip/pbuf.h"
#include "lwip/udp.h"

#define UDP_PORT 4444
#define BEACON_MSG_LEN_MAX 200
#define BEACON_TARGET "192.168.1.132"
#define BEACON_INTERVAL_MS 500

uint ledUno = 16;
uint ledDos = 17;
uint ledTres = 18;

void escribir(uint num)
{
    gpio_put(ledUno, num & 1);
    gpio_put(ledDos, num & 2);
    gpio_put(ledTres, num & 4);
}

void escribir_config()
{
    gpio_init(ledUno);
    gpio_set_dir(ledUno, GPIO_OUT);
    gpio_init(ledDos);
    gpio_set_dir(ledDos, GPIO_OUT);
    gpio_init(ledTres);
    gpio_set_dir(ledTres, GPIO_OUT);
    gpio_put(ledUno, 0);
    gpio_put(ledDos, 0);
    gpio_put(ledTres, 0);
}

void run_udp_beacon() 
{
    struct udp_pcb* pcb = udp_new();

    ip_addr_t addr;
    ipaddr_aton(BEACON_TARGET, &addr);

    int counter = 1;
    while (counter < 100) 
    {
        escribir(1);
        struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, BEACON_MSG_LEN_MAX+1, PBUF_RAM);
        escribir(2);
        char *req = (char *)p->payload;
        memset(req, 0, BEACON_MSG_LEN_MAX+1);
        snprintf(req, BEACON_MSG_LEN_MAX, "%d\n", counter);
        escribir(3);
        err_t er = udp_sendto(pcb, p, &addr, UDP_PORT);
        pbuf_free(p);
        if (er != ERR_OK) {
            printf("Failed to send UDP packet! error=%d\n", er);
            escribir(4);
        } else {
            printf("Sent packet %d\n", counter);
            counter += (counter+1)/2;
            escribir(5);
        }
        sleep_ms(BEACON_INTERVAL_MS);
    }
}

int main()
{
    stdio_init_all();
    
    escribir_config();
    escribir(4);

    if (cyw43_arch_init())
    {
        printf("failed to initialise\n");
        return 1;
    }
    
    printf("initialised\n");

    cyw43_arch_enable_sta_mode();

    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 10000))
    {
        printf("failed to connect\n");
        escribir(2);
        return 1;
    }

    printf("connected\n");
    escribir(1);
    sleep_ms(200);
    escribir(2);
    sleep_ms(200);
    escribir(3);
    sleep_ms(200);
    escribir(4);
    sleep_ms(200);
    escribir(5);
    sleep_ms(200);
    escribir(6);
    sleep_ms(200);
    escribir(7);
    sleep_ms(200);

    run_udp_beacon();
    cyw43_arch_deinit();
}
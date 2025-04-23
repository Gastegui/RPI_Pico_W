/**
 * Copyright (c) 2022 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>
#include <time.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "lwip/pbuf.h"
#include "lwip/tcp.h"

#if !defined(TCP_SERVER_IP)
#error TEST_TCP_SERVER_IP not defined
#endif

#define TCP_PORT 4444
#define DEBUG_printf printf
#define BUF_SIZE 2048

#define TEST_ITERATIONS 10
#define POLL_TIME_S 5

#if 0
static void dump_bytes(const uint8_t *bptr, uint32_t len) {
    unsigned int i = 0;

    printf("dump_bytes %d", len);
    for (i = 0; i < len;) {
        if ((i & 0x0f) == 0) {
            printf("\n");
        } else if ((i & 0x07) == 0) {
            printf(" ");
        }
        printf("%02x ", bptr[i++]);
    }
    printf("\n");
}
#define DUMP_BYTES dump_bytes
#else
#define DUMP_BYTES(A,B)
#endif

typedef struct TCP_CLIENT_
{
    struct tcp_pcb *tcp_pcb;
    ip_addr_t remote_addr;
    uint8_t buffer[BUF_SIZE];
    int buffer_len;
    bool connected;
    bool open;
    bool recibido;
}TCP_CLIENT;

void limpiar_buffer(TCP_CLIENT* state)
{
    state->recibido = 0;
    state->buffer_len = 0;
    memset(state->buffer, 0, BUF_SIZE);
}

void mi_gpio_init()
{
    gpio_init(13);
    gpio_init(14);
    gpio_init(15);
    gpio_init(16);
    gpio_init(17);
    gpio_init(18);
    gpio_init(19);
    gpio_set_dir(13, GPIO_OUT);
    gpio_set_dir(14, GPIO_OUT);
    gpio_set_dir(15, GPIO_OUT);
    gpio_set_dir(16, GPIO_OUT);
    gpio_set_dir(17, GPIO_OUT);
    gpio_set_dir(18, GPIO_OUT);
    gpio_set_dir(19, GPIO_IN);
}

void led_set_izq(uint num)
{
    gpio_put(13, num&1);
    gpio_put(14, num&2);
    gpio_put(15, num&4);
}

void led_set_der(uint num)
{
    gpio_put(18, num&1);
    gpio_put(17, num&2);
    gpio_put(16, num&4);
}

bool button_status()
{
    return gpio_get(19);
}

void button_press()
{
    while(!button_status());
    sleep_ms(50);
    while(button_status());
    sleep_ms(10);
}

static err_t tcp_client_close(void *arg) {
    TCP_CLIENT *state = (TCP_CLIENT*)arg;
    err_t err = ERR_OK;
    if (state->tcp_pcb != NULL) {
        tcp_arg(state->tcp_pcb, NULL);
        tcp_poll(state->tcp_pcb, NULL, 0);
        tcp_sent(state->tcp_pcb, NULL);
        tcp_recv(state->tcp_pcb, NULL);
        tcp_err(state->tcp_pcb, NULL);
        err = tcp_close(state->tcp_pcb);
        if (err != ERR_OK) {
            DEBUG_printf("close failed %d, calling abort\n", err);
            tcp_abort(state->tcp_pcb);
            err = ERR_ABRT;
        }
        state->tcp_pcb = NULL;
        state->connected = false;
    }
    return err;
}

static err_t tcp_client_sent(void *arg, struct tcp_pcb *tpcb, u16_t len) {
    TCP_CLIENT *state = (TCP_CLIENT*)arg;
    DEBUG_printf("tcp_client_sent %s (%u bytes)\n", state->buffer, len);
    return ERR_OK;
}

static err_t tcp_client_connected(void *arg, struct tcp_pcb *tpcb, err_t err) {
    TCP_CLIENT *state = (TCP_CLIENT*)arg;
    if (err != ERR_OK) {
        printf("connect failed %d\n", err);
        return tcp_client_close(state);
    }
    state->connected = true;
    DEBUG_printf("Waiting for buffer from server\n");
    return ERR_OK;
}

static err_t tcp_client_poll(void *arg, struct tcp_pcb *tpcb) {
    DEBUG_printf("tcp_client_poll\n");
    return ERR_OK;
}

static void tcp_client_err(void *arg, err_t err) {
    if (err != ERR_ABRT) {
        DEBUG_printf("tcp_client_err %d\n", err);
    }
}

err_t tcp_client_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    TCP_CLIENT *state = (TCP_CLIENT*)arg;
    if (!p) 
    {
        printf("EN TCP_CLIENT_RECV p == null, cerrando...\n");
        tcp_client_close(state);
    }
    // this method is callback from lwIP, so cyw43_arch_lwip_begin is not required, however you
    // can use this method to cause an assertion in debug mode, if this method is called when
    // cyw43_arch_lwip_begin IS needed
    cyw43_arch_lwip_check();
    
    if (p->tot_len > 0 && strncmp(p->payload, "mensaje:", 8) == 0) 
    {
        limpiar_buffer(state);
        state->recibido = 1;
        
        struct pbuf* q = p;
        uint16_t max = 0;
        while (q != 0)
        {
            max = q->len;
            if (max > BUF_SIZE - state->buffer_len)
                max = BUF_SIZE - state->buffer_len;

            memcpy(state->buffer + state->buffer_len, q->payload + (state->buffer_len == 0 ? 8 : 0), max - (state->buffer_len == 0 ? 8 : 0));
            state->buffer_len += max;

            if (state->buffer_len >= BUF_SIZE)
                break;

            q = q->next;
        }
        tcp_recved(tpcb, state->buffer_len); //Indicar lo que se ha recibido
    }

    pbuf_free(p);

    return ERR_OK; 
}

static bool tcp_client_open(void *arg) {
    TCP_CLIENT *state = (TCP_CLIENT*)arg;

    printf("Connecting to %s port %u\n", ip4addr_ntoa(&state->remote_addr), TCP_PORT);

    state->tcp_pcb = tcp_new_ip_type(IP_GET_TYPE(&state->remote_addr));
    
    if (!state->tcp_pcb) {
        DEBUG_printf("failed to create pcb\n");
        return false;
    }

    tcp_arg(state->tcp_pcb, state);
    tcp_sent(state->tcp_pcb, tcp_client_sent);
    tcp_recv(state->tcp_pcb, tcp_client_recv);
    tcp_err(state->tcp_pcb, tcp_client_err);

    state->buffer_len = 0;
    state->open = true;
    // cyw43_arch_lwip_begin/end should be used around calls into lwIP to ensure correct locking.
    // You can omit them if you are in a callback from lwIP. Note that when using pico_cyw_arch_poll
    // these calls are a no-op and can be omitted, but it is a good practice to use them in
    // case you switch the cyw43_arch type later.
    cyw43_arch_lwip_begin();
    err_t err = tcp_connect(state->tcp_pcb, &state->remote_addr, TCP_PORT, tcp_client_connected);
    cyw43_arch_lwip_end();

    return err == ERR_OK;
}

// Perform initialisation
static TCP_CLIENT* tcp_client_init(void) {
    TCP_CLIENT *state = calloc(1, sizeof(TCP_CLIENT));
    if (!state) {
        DEBUG_printf("failed to allocate state\n");
        return NULL;
    }
    printf("Conversion de la ip, %d\n", ip4addr_aton(TCP_SERVER_IP, &state->remote_addr));
    return state;
}



void mandarMensaje(TCP_CLIENT* state, char* mensaje)
{
    limpiar_buffer(state);

    state->buffer_len = strlen(mensaje)+1;

    snprintf(state->buffer, state->buffer_len, mensaje);

    printf("Mandando %s (%d bytes) al servidor\n", state->buffer, state->buffer_len);
    
    err_t err = tcp_write(state->tcp_pcb, state->buffer, state->buffer_len, TCP_WRITE_FLAG_COPY);
    if (err != ERR_OK) {
        DEBUG_printf("Failed to write data %d\n", err);
    }
}




void run_tcp_client_test(void)
{
    TCP_CLIENT *state = tcp_client_init();
    if (!state)
    {
        return;
    }
    if (!tcp_client_open(state))
    {
        printf("RUN TCP_CLIENT_TEST error\n");
        tcp_client_close(state);
        return;
    }
    while(state->open)
    {
        printf("while\n");
        if(gpio_get(19))
        {
            printf("boton pulsado\n");
            mandarMensaje(state, "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb");
        }
        if(state->recibido)
            printf("buffer: %s\n", state->buffer);
        sleep_ms(1000);
    }
    free(state);
}

int main() 
{
    stdio_init_all();

    mi_gpio_init();

    if (cyw43_arch_init()) {
        DEBUG_printf("failed to initialise\n");
        return 1;
    }
    cyw43_arch_enable_sta_mode();

    printf("Connecting to Wi-Fi...\n");
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("failed to connect.\n");
        return 1;
    } else {
        printf("Connected.\n");
    }
    run_tcp_client_test();
    cyw43_arch_deinit();
    return 0;
}
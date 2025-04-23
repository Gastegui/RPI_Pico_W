#include "includes/core1.h"
#include "includes/core0.h"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include <string.h>
#include <stdlib.h>
#include "includes/pagina.h"
#include <pico/multicore.h>

#define TCP_PORT 80
#define MAX_RESPONSE_LENGTH 4096

// Struct to hold connection state
struct http_connection
{
    struct tcp_pcb *pcb;
    struct pbuf *p;
    char *request;
};

// Function to send a complete HTTP response
err_t send_http_response(struct tcp_pcb *pcb, const char *status, const char *content_type, const char *body)
{
    char full_response[MAX_RESPONSE_LENGTH];

    // Ensure body is not NULL
    const char *safe_body = body ? body : "";

    // Format the full HTTP response
    int len = snprintf(full_response, sizeof(full_response),
                       "%s\r\n"
                       "Content-Type: %s\r\n"
                       "Content-Length: %zu\r\n"
                       "Connection: keep-alive\r\n"
                       "\r\n"
                       "%s",
                       status,            // HTTP status line
                       content_type,      // Content type
                       strlen(safe_body), // Content length
                       safe_body          // Response body
    );

    // Check for buffer overflow
    if (len >= sizeof(full_response))
    {
        printf("Response too long, truncated\n");
        return ERR_MEM;
    }

    // Send the response
    err_t err = tcp_write(pcb, full_response, len, TCP_WRITE_FLAG_COPY);
    if (err != ERR_OK)
    {
        printf("Failed to write response: %d\n", err);
        return err;
    }

    // Ensure the data is sent
    err = tcp_output(pcb);
    if (err != ERR_OK)
    {
        printf("Failed to output response: %d\n", err);
        return err;
    }

    return ERR_OK;
}

// Function to find a specific key's value in URL-encoded form data
bool find_form_value(const char *request, const char *key, char *value, size_t max_value_len)
{
    // Find the start of the form data
    const char *body_start = strstr(request, "\r\n\r\n");
    if (!body_start)
        return false;
    body_start += 4; // Skip \r\n\r\n

    // Create a copy to tokenize
    char *body_copy = strdup(body_start);
    if (!body_copy)
        return false;

    // Prepare value buffer
    value[0] = '\0';

    // Token variables
    char *token = strtok(body_copy, "&");
    while (token != NULL)
    {
        // Find the key-value separator
        char *equal_sign = strchr(token, '=');
        if (equal_sign)
        {
            // Null-terminate the key
            *equal_sign = '\0';

            // Check if this is the key we're looking for
            if (strcmp(token, key) == 0)
            {
                // Copy the value
                strncpy(value, equal_sign + 1, max_value_len - 1);
                value[max_value_len - 1] = '\0';

                // URL decode the value
                char *decoded_value = value;
                char *read = equal_sign + 1;
                while (*read)
                {
                    if (*read == '+')
                    {
                        *decoded_value++ = ' ';
                    }
                    else if (*read == '%' && read[1] && read[2])
                    {
                        // Convert hex to character
                        char hex[3] = {read[1], read[2], '\0'};
                        *decoded_value++ = (char)strtol(hex, NULL, 16);
                        read += 2;
                    }
                    else
                    {
                        *decoded_value++ = *read;
                    }
                    read++;
                }
                *decoded_value = '\0';

                free(body_copy);
                return true;
            }
        }

        // Move to next token
        token = strtok(NULL, "&");
    }

    free(body_copy);
    return false;
}

// Function to handle incoming HTTP requests
err_t http_server_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
    if (p == NULL)
    {
        printf("Connection closed by client\n");
        tcp_close(pcb);
        return ERR_OK;
    }

    char *request = (char *)malloc(p->tot_len + 1);
    if (!request)
    {
        printf("Memory allocation failed\n");
        pbuf_free(p);
        return ERR_MEM;
    }

    pbuf_copy_partial(p, request, p->tot_len, 0);
    request[p->tot_len] = '\0';

    printf("Received request:\n%s\n", request);

    const char *status = "HTTP/1.1 200 OK";
    const char *content_type = "text/plain";
    char body[MAX_RESPONSE_LENGTH] = "";

    if (strstr(request, "GET /") != NULL)
    {
        status = "HTTP/1.1 200 OK";
        content_type = "text/html";
        strcpy(body, pagina);
    }
    else if (strstr(request, "POST /") != NULL)
    {
        char dir[10];
        bool has_direction = find_form_value(request, "button", dir, sizeof(dir));

        if (has_direction && (strcmp(dir, "forward") == 0 || strcmp(dir, "backward") == 0 || strcmp(dir, "left") == 0 || strcmp(dir, "right") == 0 || strcmp(dir, "stop") == 0))
        {
            int move = 0;
            if(strcmp(dir, "forward") == 0)
                move = 1;
            else if (strcmp(dir, "backward") == 0)
                move = 2;
            else if (strcmp(dir, "left") == 0)
                move = 3;
            else if (strcmp(dir, "right") == 0)
                move = 4;
            else
                move = 0;
            multicore_fifo_push_blocking(move);

            snprintf(body, sizeof(body), "Direction: %s" ,dir);
        }
        else
        {
            status = "HTTP/1.1 400 Bad Request";
            strcpy(body, "Required fields not found");
        }
    }
    else
    {
        status = "HTTP/1.1 405 Method Not Allowed";
        strcpy(body, "Method Not Supported");
    }

    send_http_response(pcb, status, content_type, body);

    free(request);
    pbuf_free(p);
    tcp_recved(pcb, p->tot_len);

    // Opcional: cerrar conexión si el cliente lo solicita
    if (strstr(request, "Connection: close"))
    {
        tcp_close(pcb);
    }

    return ERR_OK;
}

// Function to handle new connections
err_t http_server_accept(void *arg, struct tcp_pcb *pcb, err_t err)
{
    printf("New connection accepted\n");

    // Set TCP options
    tcp_setprio(pcb, TCP_PRIO_MIN);

    // Set receive callback
    tcp_recv(pcb, http_server_recv);

    return ERR_OK;
}

void core1_main()
{

    if (cyw43_arch_init())
    {
        printf("WiFi init failed\n");
        return;
    }

    // Connect to WiFi
    cyw43_arch_enable_sta_mode();
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD,
                                           CYW43_AUTH_WPA2_AES_PSK, 30000))
    {
        printf("Failed to connect to WiFi\n");
        return;
    }

    // Create TCP PCB (Protocol Control Block)
    struct tcp_pcb *pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
    if (!pcb)
    {
        printf("Failed to create PCB\n");
        return;
    }

    // Bind to port
    err_t err = tcp_bind(pcb, IP_ANY_TYPE, TCP_PORT);
    if (err)
    {
        printf("Bind failed\n");
        return;
    }

    // Listen for connections
    pcb = tcp_listen_with_backlog(pcb, 1);
    tcp_accept(pcb, http_server_accept);

    printf("HTTP Server running on port %d\n", TCP_PORT);

    struct netif *netif = netif_list; // Obtiene la primera interfaz de red
    if (netif != NULL && netif_is_up(netif))
    {                                                                    // Verifica que la interfaz esté activa
        printf("IP Address: %s\n", ip4addr_ntoa(netif_ip4_addr(netif))); // Imprime la IP
    }
    else
    {
        printf("No IP Address assigned or interface is down.\n");
    }
    // Keep the program running
    while (1)
    {
        cyw43_arch_poll();
        sleep_ms(1);
    }

    return;
}

/*

    struct netif *netif = netif_list; // Obtiene la primera interfaz de red
    if (netif != NULL && netif_is_up(netif)) { // Verifica que la interfaz esté activa
        printf("IP Address: %s\n", ip4addr_ntoa(netif_ip4_addr(netif))); // Imprime la IP
    } else {
        printf("No IP Address assigned or interface is down.\n");
    }
    */
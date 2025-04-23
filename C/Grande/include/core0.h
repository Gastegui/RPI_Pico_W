#ifndef CORE_0_H
#define CORE_0_H

#include "ff.h"

#define led_verde_func &hw.led_verde,PIN_LED_VERDE
#define led_amarillo_func &hw.led_amarillo,PIN_LED_AMARILLO
#define led_rojo_func &hw.led_rojo,PIN_LED_ROJO

#define encoder_1_boton &hw.encoder_1,PIN_ENCODER_1_BOTON
#define encoder_2_boton &hw.encoder_2,PIN_ENCODER_2_BOTON
#define encoder_3_boton &hw.encoder_3,PIN_ENCODER_3_BOTON

#define ABORT(pantalla, error) abortar(pantalla, error, sizeof(error))

typedef struct s_boton
{
    volatile bool pulsado;
    bool mantener;
    volatile uint64_t tiempo;
}BOTON;

typedef struct s_encoder
{
    volatile bool a;
    volatile bool b;
    volatile bool valor;
    volatile bool derecha;
    BOTON boton;
}ENCODER;

typedef struct s_pantalla
{
    uint8_t buf[128*8];
    uint8_t contraste;
    uint8_t bufTexto[128*8];
    uint8_t letra_x;
    uint8_t letra_y;
    uint8_t tamaño_espacio;
    uint8_t tamaño_x_extra;
    uint8_t tamaño_interlineado;
    char texto[128];
    uint8_t len;
}PANTALLA;

typedef struct s_buzzer
{

}BUZZER;

typedef struct s_sd
{
    bool montado;
    bool abierto;
    FIL fil;
    FRESULT fr;
    char linea[255];
    BYTE modo;
}SD;

typedef struct s_hardware
{
    ENCODER encoder_1;
    ENCODER encoder_2;
    ENCODER encoder_3;
    bool led_verde;
    bool led_amarillo;
    bool led_rojo;
    PANTALLA pantalla;
    BUZZER buzzer;
    SD sd;
}HARDWARE;


void init_gpio_normal(uint8_t pin, bool dir);
void init_encoder(uint8_t a, uint8_t b, uint8_t boton, ENCODER* encoder, gpio_irq_callback_t callback);
void init_pantalla(PANTALLA* pantalla);
void init_sd(SD* sd, PANTALLA* pantalla);
void deinit_sd(SD* sd, PANTALLA* pantalla);

bool encoder_valor(ENCODER* encoder);
bool encoder_izquierda(ENCODER* encoder);
bool encoder_derecha(ENCODER* encoder);
void encoder_limpiar(ENCODER* encoder);
bool encoder_boton(ENCODER* encoder);
void encoder_boton_mantener(ENCODER* encoder, bool mantener);
bool encoder_boton_mantener_estado(ENCODER* encoder);
void encoder_boton_mantener_alternar(ENCODER* encoder);
void encoder_boton_limpiar(ENCODER* encoder);

void led_poner(bool* led, uint8_t pin, bool encender);
void led_alternar(bool* led, uint8_t pin);
bool led_estado(bool* led, uint8_t pin);


void oled_mandar_cmd(uint8_t cmd);
void oled_mandar_pantalla(PANTALLA* pantalla);
void oled_cambiar_contraste(PANTALLA* pantalla, uint8_t nuevo);
bool oled_contraste(PANTALLA* pantalla);
void oled_cambiar_pixel(uint8_t* buf, uint8_t x, uint8_t y, bool encender);
void oled_borrar(PANTALLA* pantalla);
void oled_cursor(PANTALLA* pantalla, uint8_t x, uint8_t y);
void oled_print(PANTALLA* pantalla, const char* original, uint8_t len);
void oled_actualizar_texto(PANTALLA* pantalla);
void oled_cargar(PANTALLA* pantalla, SD* sd, const char* path);
void oled_invertir(PANTALLA* pantalla, bool fondo);

void abortar(PANTALLA* pantalla, const char* mensaje, uint8_t len);

#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/pwm.h"
#include "hardware/i2c.h"
#include "hardware/spi.h"
#include "constantes.h"
#include "core0.h"
#include "letras.h"
#include "f_util.h"
#include "ff.h"
#include "hw_config.h"

FATFS fs;

void sd_abrir(SD* sd, const char* path, BYTE modo, PANTALLA* pantalla);
void sd_cerrar(SD* sd, PANTALLA* pantalla);
bool sd_leer_linea(SD* sd, PANTALLA* pantalla);

void led_poner(bool* led, uint8_t pin, bool encender)
{
    gpio_put(pin, encender);
    *led = encender;
}

void led_alternar(bool* led, uint8_t pin)
{
    *led = !(*led);
    gpio_put(pin, *led);
}

bool led_estado(bool* led, uint8_t pin)
{
    return *led;
}


void init_gpio_normal(uint8_t pin, bool dir)
{
    gpio_init(pin);
    gpio_set_dir(pin, dir);
    gpio_pull_down(pin);
}

void abortar(PANTALLA* pantalla, const char* mensaje, uint8_t len)
{
    extern HARDWARE hw;
    printf(mensaje);
    if(pantalla != NULL)
    {
        memset(pantalla->buf, 0, sizeof(pantalla->buf));
        oled_cursor(pantalla, 0, 0);
        oled_print(pantalla, mensaje, len);
        oled_mandar_pantalla(pantalla);
        if(hw.sd.montado)
            deinit_sd(&hw.sd, NULL);
    }
    gpio_put(PIN_LED_ABORTAR, 1);
    while(1);
}

//------------------------------------
//MARK:-------------BOTON-------------
//------------------------------------

void init_boton(BOTON* boton, uint8_t pin, gpio_irq_callback_t callback)
{
    init_gpio_normal(pin, GPIO_IN);
    gpio_set_irq_enabled_with_callback(pin, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, callback);
    boton->pulsado = 0;
    boton->mantener = 0;
    boton->tiempo = 0;
}

//------------------------------------
//MARK:------------ENCODER------------
//------------------------------------

bool encoder_valor(ENCODER* encoder)
{
    return encoder->valor;
}

bool encoder_derecha(ENCODER* encoder)
{
    return encoder->valor && encoder->derecha;
}

bool encoder_izquierda(ENCODER* encoder)
{
    return encoder->valor && !encoder->derecha;
}

void encoder_limpiar(ENCODER* encoder)
{
    encoder->derecha = false;
    encoder->valor = false;
}

bool encoder_boton(ENCODER* encoder)
{
    return encoder->boton.pulsado;
}

void encoder_boton_mantener(ENCODER* encoder, bool mantener)
{
    encoder->boton.mantener = mantener;
}

bool encoder_boton_mantener_estado(ENCODER* encoder)
{
    return encoder->boton.mantener;
}

void encoder_boton_mantener_alternar(ENCODER* encoder)
{
    encoder->boton.mantener = !encoder->boton.mantener;
}

void encoder_boton_limpiar(ENCODER* encoder)
{
    encoder->boton.pulsado = 0;
}

void init_encoder(uint8_t a, uint8_t b, uint8_t boton, ENCODER* encoder, gpio_irq_callback_t callback)
{
    init_gpio_normal(a, GPIO_IN);
    init_gpio_normal(b, GPIO_IN);
    init_boton(&encoder->boton, boton, callback);

    gpio_set_irq_enabled_with_callback(a, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, callback);
    gpio_set_irq_enabled_with_callback(b, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, callback);


    encoder->a = false;
    encoder->b = false;
    encoder->derecha = false;
    encoder->valor = false;
}

//------------------------------------
//MARK: -------------OLED-------------
//------------------------------------

void oled_mandar_cmd(uint8_t cmd)
{
    uint8_t mandar[] = {0x80, cmd};

    i2c_write_blocking(OLED_I2C, OLED_ADDR, mandar, 2, false);
}

void oled_mandar_cmds(uint8_t* cmds, int len)
{
    for(int i = 0; i < len; i++)
        oled_mandar_cmd(cmds[i]);
}

void oled_poner_pagina(uint8_t pagina)
{
    oled_mandar_cmd(0xB0+pagina); //Mandar el num de la pagina que se va a cambiar
    oled_mandar_cmd(0x02); //Poner el offset
    oled_mandar_cmd(0x10); //Hasta el final de la pagina
}

void oled_mandar_pagina(uint8_t* pagina, uint8_t len)
{
    uint8_t mandar[len+1];
    mandar[0] = 0x40;
    memcpy(&mandar[1], pagina, len);
    i2c_write_blocking(OLED_I2C, OLED_ADDR, mandar, len+1, 0);
}

void oled_mandar_pantalla(PANTALLA* pantalla)
{
    uint8_t pagina[128];
    uint8_t j;
    for(uint8_t i = 0; i < 8; i++)
    {
        oled_poner_pagina(i);
        for(j = 0; j < 128; j++)
        {
            pagina[j] = pantalla->buf[j+i*128] ^ pantalla->bufTexto[j+i*128];
        }
        oled_mandar_pagina(pagina, 128);
    }
}

void oled_cambiar_contraste(PANTALLA* pantalla, uint8_t nuevo)
{
    if(nuevo > 255)
        nuevo = 255;

    pantalla->contraste = nuevo;

    uint8_t cmds[] = {OLED_CONTRAST, nuevo};

    oled_mandar_cmds(cmds, sizeof(cmds));

    oled_mandar_pantalla(pantalla);
}

bool oled_contraste(PANTALLA* pantalla)
{
    return pantalla->contraste;
}

void oled_limpiar(uint8_t* buffer)
{
    memset(buffer, 0, 128*8);
}

void oled_cambiar_pixel(uint8_t* buffer, uint8_t x, uint8_t y, bool encender)
{
    if(x > OLED_ANCHO || y > OLED_ALTO)
        return;

    uint8_t page = y / 8;
    uint8_t bit = y % 8;

    uint16_t i = x + page * 128;
    if(encender)
        buffer[i] |= 1 << bit;
    else
        buffer[i] &= ~(1 << bit); 
}

void init_pantalla(PANTALLA* pantalla)
{
    i2c_init(OLED_I2C, OLED_CLK * 1000);
    gpio_set_function(PIN_OLED_SCL, GPIO_FUNC_I2C);
    gpio_set_function(PIN_OLED_SDA, GPIO_FUNC_I2C);
    gpio_pull_up(PIN_OLED_SCL);
    gpio_pull_up(PIN_OLED_SDA);

    pantalla->contraste = 0x20;
    pantalla->tamaño_espacio = LETRAS_ESPACIO;
    pantalla->tamaño_x_extra = LETRAS_TAMAÑO_X_EXTRA;
    pantalla->tamaño_interlineado = LETRAS_INTERLINEADO;

    uint8_t cmds[] = 
    {
        OLED_SCREEN_OFF,
        OLED_START_LINE,
        OLED_SEGMENT_REMAP_H,
        OLED_MULTIPLEX_RATIO,
        OLED_MULTIPLEX_DEFAULT,
        OLED_SCAN_DIRECTION_NORMAL,
        OLED_OFFSET,
        0x00, 
        OLED_COMMON_PADS,
        OLED_COMMON_PADS_DEFAULT,
        OLED_CLK_DIVIDE,
        0x80,
        OLED_PRECHARGE,
        OLED_PRECHARGE_DEFAULT,
        OLED_VCOM_DESELECT,
        0x40,
        OLED_CONTRAST,
        pantalla->contraste,
        OLED_SCREEN_RAM,
        OLED_SCREEN_NORMAL,
        OLED_SCREEN_ON
    };

    oled_mandar_cmds(cmds, count_of(cmds));

    memset(pantalla->buf, 0, sizeof(pantalla->buf));
    oled_mandar_pantalla(pantalla);
    oled_cursor(pantalla, 0, 0);
}

void oled_cursor(PANTALLA* pantalla, uint8_t x, uint8_t y)
{
    pantalla->letra_x = x;
    pantalla->letra_y = y;
    
    if(pantalla->letra_x > OLED_ANCHO - LETRAS_TAMAÑO_X)
        pantalla->letra_x = OLED_ANCHO - LETRAS_TAMAÑO_X;
    
    if(pantalla->letra_y > OLED_ALTO - LETRAS_TAMAÑO_Y)
        pantalla->letra_y = OLED_ALTO - LETRAS_TAMAÑO_Y;
}

void oled_poner_letra(PANTALLA* pantalla, uint8_t *letra)
{
    uint8_t x;
    uint8_t y;
    for(y = 0; y < LETRAS_TAMAÑO_Y; y++)
    {
        for(x = 0; x < LETRAS_TAMAÑO_X; x++)
            oled_cambiar_pixel(pantalla->bufTexto, pantalla->letra_x + x, pantalla->letra_y + y, letra[y] & 1<<7-x);
    }
}

void oled_poner_espacio(PANTALLA* pantalla)
{
    uint8_t x;
    uint8_t y;
    for(y = pantalla->letra_y; y < LETRAS_TAMAÑO_Y; y++)
    {
        for(x = pantalla->letra_x; x < pantalla->tamaño_espacio; x++)
            oled_cambiar_pixel(pantalla->bufTexto, x, y, 0);
    }
}

void oled_poner_x_extra(PANTALLA* pantalla)
{
    uint8_t x;
    uint8_t y;
    for(y = pantalla->letra_y; y < LETRAS_TAMAÑO_Y; y++)
    {
        for(x = pantalla->letra_x; x < pantalla->tamaño_x_extra; x++)
            oled_cambiar_pixel(pantalla->bufTexto, x, y, 0);
    }
}

void oled_print(PANTALLA* pantalla, const char* original, uint8_t len)
{
    uint8_t i;
    uint8_t letra;
    len--; //Quitar el \0
    
    if(len != pantalla->len || strncmp(pantalla->texto, original, len) != 0)
    {
        memcpy(pantalla->texto, original, len);
        pantalla->len = len;
    }

    memset(pantalla->bufTexto, 0, sizeof(pantalla->bufTexto));

    for(i = 0; i < len; i++)
    {
        letra = 0;
        if(original[i] >= '0' && original[i] <= '9')
            letra = original[i] - '0' + LETRAS_0;
        else if(original[i] >= 'a' && original[i] <= 'z')
            letra = original[i] - 'a' + LETRAS_A;
        else if(original[i] >= 'A' && original[i] <= 'Z')
            letra = original[i] - 'A' + LETRAS_A;
        else
        {
            switch(original[i])
            {
                case ' ':
                    letra = LETRAS_CHAR_ESPACIO;
                    break;
                case '\n':
                    letra = LETRAS_CHAR_SALTO;
                    break;
                case '-':
                    letra = LETRAS_GUION;
                    break;
                case '.':
                    letra = LETRAS_PUNTO;
                    break;
                case ',':
                    letra = LETRAS_COMA;
                    break;
                case '!':
                    letra = LETRAS_EXCLAMACION;
                    break;
                default:
                    break;
            }
        }
        if(letra)
        {
            if(letra == LETRAS_CHAR_SALTO || pantalla->letra_x + LETRAS_TAMAÑO_X > OLED_ANCHO) //Salto de linea 
            {
                pantalla->letra_x = 0;
                pantalla->letra_y += LETRAS_TAMAÑO_Y +  pantalla->tamaño_interlineado;
                if(pantalla->letra_y + LETRAS_TAMAÑO_Y > OLED_ALTO)
                    pantalla->letra_y = OLED_ALTO - LETRAS_TAMAÑO_Y;
                
                if(letra == LETRAS_CHAR_SALTO) //Volver al inicio del for si es un salto de linea, ya que no hay que hacer nada más
                    continue;
            }
            
            if(letra != LETRAS_CHAR_ESPACIO)
            {
                oled_poner_letra(pantalla, &letras[letra*LETRAS_TAMAÑO_X]);
                pantalla->letra_x += LETRAS_TAMAÑO_X;
            }
            else
            {
                if(pantalla->letra_x != 0)
                {
                    oled_poner_espacio(pantalla);
                    pantalla->letra_x += pantalla->tamaño_espacio;
                }
            }
            if(pantalla->letra_x != 0)
            {
                //No se comprueba si el espacio cabe o no, ya que en cambiar_pixel() se ignora lo que esté fuera de la pantalla
                oled_poner_x_extra(pantalla);
                pantalla->letra_x += pantalla->tamaño_x_extra;
            }
        }
    }
}

void oled_actualizar_texto(PANTALLA* pantalla)
{
    oled_print(pantalla, pantalla->texto, pantalla->len + 1);
}

void oled_cargar(PANTALLA* pantalla, SD* sd, const char* path)
{
    sd_abrir(sd, path, FA_READ | FA_OPEN_EXISTING, pantalla);

    uint i = 0;

    while(sd_leer_linea(sd, pantalla))
        pantalla->buf[i++] = (uint8_t) strtol(sd->linea, NULL, 16);

    sd_cerrar(sd, pantalla);

}

void oled_invertir(PANTALLA* pantalla, bool fondo)
{
    uint8_t* p;
    if(fondo)
        p = pantalla->buf;
    else
        p = pantalla->bufTexto;

    for(int i = 0; i < sizeof(pantalla->buf); i++)
    {
        *p = ~(*p);
        p++;
    }
}

//------------------------------------
//MARK: --------------SD--------------
//------------------------------------

void init_sd(SD* sd, PANTALLA* pantalla)
{

    sd->fr = f_mount(&fs, "", 1);
    
    sd->montado = false;
    sd->abierto = false;

    if(sd->fr == FR_OK)
        sd->montado = true;
    else
        ABORT(pantalla, "ERROR: no se ha conseguido montar la tarjeta\n");
    

    gpio_put(PIN_SD_LED, 1);
}

void deinit_sd(SD* sd, PANTALLA* pantalla)
{
    if(!sd->montado)
        ABORT(pantalla, "ERROR: se ha intentado desmontar la tarjeta sin haberla montado antes\n");

    if(sd->abierto)
        sd_cerrar(sd, pantalla);

    sd->fr = f_unmount("");

    if(sd->fr != FR_OK)
        ABORT(pantalla, "ERROR: no se ha conseguido desmontar la tarjeta\n");

    gpio_put(PIN_SD_LED, 0);
}

void sd_abrir(SD* sd, const char* path, BYTE modo, PANTALLA* pantalla)
{
    if(!sd->montado)
        ABORT(pantalla, "ERROR: Se ha intentado abrir un archivo sin haber montado una tarjeta antes\n");

    if(sd->abierto)
        ABORT(pantalla, "ERROR: Se ha intentado abrir un archivo habiendo otro habierto ya\n");
    
    sd->modo = modo;
    sd->fr = f_open(&sd->fil, path, modo);

    if(sd->fr != FR_OK)
    {
        printf("Codigo: %d\n", sd->fr);
        ABORT(pantalla, "ERROR: No se ha podido abrir el archivo.");
    }

    sd->abierto = true;
}

void sd_cerrar(SD* sd, PANTALLA* pantalla)
{
    if(!sd->montado)
        ABORT(pantalla, "ERROR: Se ha intentado cerrar un archivo sin haber montado una tarjeta antes\n");

    if(!sd->abierto)
        ABORT(pantalla, "ERROR: Se ha intentado cerrar un archivo sin haber abierto uno antes\n");

    sd->fr = f_close(&sd->fil);    

    if(sd->fr == FR_OK)
        sd->abierto = false;

    if(sd->abierto)
        ABORT(pantalla, "ERROR: Se ha intentado cerrar un archivo pero no se ha conseguido\n");

}

bool sd_leer_linea(SD* sd, PANTALLA* pantalla)
{
    if(!sd->montado || !sd->abierto)
        ABORT(pantalla, "ERROR: Se ha intentado leer de un archivo sin haberlo cargado antes o sin haber montado la tarjeta\n");

    if(sd->modo & FA_READ == 0)
        ABORT(pantalla, "ERROR: Se ha intentado leer de un archivo que no está abierto para lectura\n");

    if(f_eof(&sd->fil) != 0)
        return false;

    f_gets(sd->linea, sizeof(sd->linea), &sd->fil);

    return true;
}
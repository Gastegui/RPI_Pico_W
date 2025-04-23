//
// Created by julen on 7/10/24.
//

#ifndef OLED_H
#define OLED_H

#include <cstring>
#include <hardware/gpio.h>

#include "LETRAS.h"
#include "CONSTANTES.h"
#include "hardware/i2c.h"

extern void abortar(const char* mensaje);

class Oled
{
    uint8_t buf[128*8] = {0};
    uint8_t contraste = 0;
    uint8_t bufTexto[128*8] = {0};
    uint8_t letra_x = 0;
    uint8_t letra_y = 0;
    uint8_t tamaño_espacio;
    uint8_t tamaño_x_extra;
    uint8_t tamaño_interlineado;
    char texto[128] = {0};
    uint8_t len = 0;

    static void mandarCmd(const uint8_t cmd)
    {
        const uint8_t data[] = {0x80, cmd};

        i2c_write_blocking(i2c0, OLED_ADDR, data, 2, false);
    }

    // ReSharper disable once CppDFAConstantParameter
    static void mandarCmds(const uint8_t* cmd, const int len)
    {
        for(int i = 0; i < len; i++)
            mandarCmd(cmd[i]);
    }

    static void ponerPagina(const uint8_t pagina)
    {
        mandarCmd(0xB0+pagina); //Mandar el num de la pagina que se va a cambiar
        mandarCmd(0x02); //Poner el offset
        mandarCmd(0x10); //Hasta el final de la pagina
    }

    // ReSharper disable once CppDFAConstantParameter
    static void mandarPagina(const uint8_t* pagina, const int len)
    {
        uint8_t mandar[len+1];
        mandar[0] = 0x40;
        memcpy(&mandar[1], pagina, len);
        i2c_write_blocking(i2c0, OLED_ADDR, mandar, len+1, false);
    }

    void cambiarContraste(uint8_t nuevo)
    {
        if(nuevo > 255)
            nuevo = 255;

        contraste = nuevo;

        const uint8_t cmds[] = {OLED_CONTRAST, nuevo};

        mandarCmds(cmds, sizeof(cmds));

        mandarPantalla();
    }

    void cambiarPixelPrivado(const bool letra, const uint8_t x, const uint8_t y, const bool encender)
    {
        if(x > OLED_ANCHO || y > OLED_ALTO)
            return;

        const uint8_t page = y / 8;
        const uint8_t bit = y % 8;

        const uint16_t i = x + page * 128;
        if(encender)
        {
            if(letra)
                bufTexto[i] |= 1<<bit;
            else
                buf[i] |= 1 << bit;
        }
        else
        {
            if(letra)
                bufTexto[i] &= ~(1<<bit);
            else
                buf[i] &= ~(1 << bit);
        }
    }

    void ponerLetra(const uint8_t *letra)
    {
        for(uint8_t y = 0; y < LETRAS_TAMAÑO_Y; y++)
        {
            for(uint8_t x = 0; x < LETRAS_TAMAÑO_X; x++)
                cambiarPixelPrivado(true, letra_x + x, letra_y + y, letra[y] & 1<<7-x);
        }
    }

    void ponerEspacio()
    {
        for(uint8_t y = letra_y; y < LETRAS_TAMAÑO_Y; y++)
        {
            for(uint8_t x = letra_x; x < tamaño_espacio; x++)
                cambiarPixelPrivado(true, x, y, false);
        }
    }

    void ponerXExtra()
    {
        for(uint8_t y = letra_y; y < LETRAS_TAMAÑO_Y; y++)
        {
            for(uint8_t x = letra_x; x < tamaño_x_extra; x++)
                cambiarPixelPrivado(true, x, y, false);
        }
    }

public:
    // ReSharper disable once CppDFAConstantParameter
    void cursor(const uint8_t x, const uint8_t y)
    {
        letra_x = x;
        letra_y = y;

        if(letra_x > OLED_ANCHO - LETRAS_TAMAÑO_X)
            letra_x = OLED_ANCHO - LETRAS_TAMAÑO_X;

        if(letra_y > OLED_ALTO - LETRAS_TAMAÑO_Y)
            letra_y = OLED_ALTO - LETRAS_TAMAÑO_Y;
    }

    Oled(const int sda, const int scl)
    {
        i2c_init(OLED_I2C, OLED_CLK * 1000);
        gpio_set_function(scl, GPIO_FUNC_I2C);
        gpio_set_function(sda, GPIO_FUNC_I2C);
        gpio_pull_up(scl);
        gpio_pull_up(sda);

        contraste = 0x20;
        tamaño_espacio = LETRAS_ESPACIO;
        tamaño_x_extra = LETRAS_TAMAÑO_X_EXTRA;
        tamaño_interlineado = LETRAS_INTERLINEADO;

        constexpr uint8_t cmds[] =
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
            0x20, //El valor del contraste
            OLED_SCREEN_RAM,
            OLED_SCREEN_NORMAL,
            OLED_SCREEN_ON
        };

        mandarCmds(cmds, std::size(cmds));

        memset(buf, 0, sizeof(buf));
        mandarPantalla();
        cursor(0, 0);
    };


    void limpiar()
    {
        memset(buf, 0, 128*8);
    }

    void print(const char * original) //TODO: Hacer que esto funcione con <<
    {
        const uint8_t len_ = strlen(original);
        //len_--; //Quitar el \0

        if(len_ != len || strncmp(texto, original, len_) != 0)
        {
            memcpy(texto, original, len_);
            len = len_;
        }

        memset(bufTexto, 0, sizeof(bufTexto));

        for(uint8_t i = 0; i < len_; i++)
        {
            uint8_t letra = 0;
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
                    case ':':
                        letra = LETRAS_DOS_PUNTOS;
                        break;
                    default:
                        break;
                }
            }
            if(letra)
            {
                if(letra == LETRAS_CHAR_SALTO || letra_x + LETRAS_TAMAÑO_X > OLED_ANCHO) //Salto de linea
                {
                    //letra_x = 0;
                    //letra_y += LETRAS_TAMAÑO_Y +  tamaño_interlineado;
                    cursor(0, letra_y += LETRAS_TAMAÑO_Y +  tamaño_interlineado);
                    if(letra_y + LETRAS_TAMAÑO_Y > OLED_ALTO)
                        letra_y = OLED_ALTO - LETRAS_TAMAÑO_Y;

                    if(letra == LETRAS_CHAR_SALTO) //Volver al inicio del for si es un salto de linea, ya que no hay que hacer nada más
                        continue;
                }

                if(letra != LETRAS_CHAR_ESPACIO)
                {
                    ponerLetra(&letras[letra*LETRAS_TAMAÑO_X]);
                    letra_x += LETRAS_TAMAÑO_X;
                }
                else
                {
                    if(letra_x != 0)
                    {
                        ponerEspacio();
                        letra_x += tamaño_espacio;
                    }
                }
                if(letra_x != 0)
                {
                    //No se comprueba si el espacio cabe o no, ya que en cambiar_pixel() se ignora lo que esté fuera de la pantalla
                    ponerXExtra();
                    letra_x += tamaño_x_extra;
                }
            }
        }
    }

    void Invertir(const bool fondo)
    {
        uint8_t* p;
        if(fondo)
            p = buf;
        else
            p = bufTexto;

        for(int i = 0; i < sizeof(buf); i++)
        {
            *p = ~(*p);
            p++;
        }
    }

    void cambiarPixel(const uint8_t x, const uint8_t y, const bool encendido)
    {
        cambiarPixelPrivado(false, x, y, encendido);
    }
    void cargar()
    {
        print("Sin implementar");
        printf("Sin implementar\n");
    }


    void mandarPantalla() const
    {
        uint8_t pagina[128];
        for(uint8_t i = 0; i < 8; i++)
        {
            ponerPagina(i);
            for(uint8_t j = 0; j < 128; j++)
            {
                pagina[j] = buf[j+i*128] ^ bufTexto[j+i*128];
            }
            mandarPagina(pagina, 128);
        }
    }


    /*
    void oled_cargar(SD* sd, const char* path)
    {
        sd_abrir(sd, path, FA_READ | FA_OPEN_EXISTING, pantalla);

        uint i = 0;

        while(sd_leer_linea(sd, pantalla))
            pantalla->buf[i++] = (uint8_t) strtol(sd->linea, NULL, 16);

        sd_cerrar(sd, pantalla);

    }
    */
};

#endif //OLED_H

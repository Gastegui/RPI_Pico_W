#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <string.h>

#define I2C_PORT i2c1
#define I2C_SDA_PIN 14
#define I2C_SCL_PIN 15
#define OLED_ADDR 0x3C

const uint8_t init_cmds[] = {
        0xAE,        // Display off
        0xD5, 0x80,  // Set display clock divide ratio/oscillator frequency
        0xA8, 0x3F,  // Set multiplex ratio (1 to 64)
        0xD3, 0x00,  // Set display offset
        0x40,        // Set start line address
        0x8D, 0x14,  // Charge pump setting
        0x20, 0x00,  // Set memory addressing mode: horizontal addressing mode
        0xA1,        // Set segment re-map 0 to 127
        0xC8,        // Set COM output scan direction: remapped mode
        0xDA, 0x12,  // Set COM pins hardware configuration
        0x81, 0xCF,  // Set contrast control
        0xD9, 0xF1,  // Set pre-charge period
        0xDB, 0x40,  // Set VCOMH deselect level
        0xA4,        // Entire display on: resume to RAM content display
        0xA6,        // Set normal display: display normal, non-inverted
        0xAF         // Display on
        
    };//Configuración de la pantalla

uint8_t buffer[128 * 8];

void oled_escribir_cmd(uint8_t data)
{
    uint8_t mandar[2] = {0x80, data}; //Poner el primer byte que se manda con el identificador de config
    i2c_write_blocking(I2C_PORT, OLED_ADDR, mandar, 2, 0);
}

void oled_escribir(uint8_t* data, int len)
{
    uint8_t mandar[len + 1];
    mandar[0] = 0x40; //Byte de control de datos para el display
    memcpy(mandar + 1, data, len);
    i2c_write_blocking(I2C_PORT, OLED_ADDR, mandar, len + 1, 0); //len + 1 por el 0x40
}

void oled_cambiar_pixel(uint8_t x, uint8_t y, bool encender)
{
    uint8_t page = y / 8; //Sacar en que pagina está el pixel deseado. Hay 8 paginas verticales de 8 bits cada una, 
                          //cada bit de cada pagina hace referencia a un pixel de la pagina. 8*8=64
    uint8_t bit = y % 8;  //Sacar que bit de la pagina es el deseado

    uint8_t i = x + page*128; //El array tiene 8 paginas por cada linea vertical

    buffer[i] = encender ? buffer[i] | (1 << bit) : buffer[i] & ~(1 << bit);
}

void oled_actualizar_pantalla()
{
    for(uint8_t i = 0; i < 8; i++)
    {
        oled_escribir_cmd(0x00);
        oled_escribir_cmd(0x10);
        oled_escribir_cmd(0xB0 + i); //Indicar en que pagina se va a escribir
        oled_escribir(&buffer[i * 128], 128); //Mandar la pagina indicada antes
    }
}

void oled_init()
{
    for(int i = 0; i < sizeof(init_cmds); i++)
        oled_escribir_cmd(init_cmds[i]); //Mandar la configuración

    memset(buffer, 0, sizeof(buffer)); //Vaciar el buffer

    oled_actualizar_pantalla();
}

void main()
{
    stdio_init_all();

    i2c_init(I2C_PORT, 100 * 1000); // Initialize I2C at 100kHz
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);
    oled_init();

    bool encendido = true;
    
    while(1)
    {
        for(uint8_t x = 0; x < 100; x++)
        {
            for(uint8_t y = 0; y < 60; y++)
            {
                oled_cambiar_pixel(x, y, encendido);
                oled_actualizar_pantalla();
                sleep_ms(100);
            }
        }
        encendido != encendido;
    }

    return;
}
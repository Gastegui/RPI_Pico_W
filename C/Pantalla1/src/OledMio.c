#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <string.h>

#define SSD1306_128x32 0
#define SSD1306_128x64 1
#define SSH1106_128x64 2

#define OLED_I2C 0
#define OLED_SPI 1

#define OLED_NO_BUFFER 0
#define OLED_BUFFER 1

#define OLED_CLEAR 0
#define OLED_FILL 1
#define OLED_STROKE 2

#define BUF_ADD 0
#define BUF_SUBTRACT 1
#define BUF_REPLACE 2

#define BITMAP_NORMAL 0
#define BITMAP_INVERT 1


#define OLED_PORT i2c1
#define OLED_ADDR 0x30
#define OLED_SDA 4
#define OLED_SCL 5



#define OLED_WIDTH 128
#define OLED_HEIGHT_32 0x02
#define OLED_HEIGHT_64 0x12
#define OLED_64 0x3F
#define OLED_32 0x1F

#define OLED_DISPLAY_OFF 0xAE
#define OLED_DISPLAY_ON 0xAF

#define OLED_COMMAND_MODE 0x00
#define OLED_ONE_COMMAND_MODE 0x80
#define OLED_DATA_MODE 0x40
#define OLED_ONE_DATA_MODE 0xC0

#define OLED_ADDRESSING_MODE 0x20
#define OLED_HORIZONTAL 0x00
#define OLED_VERTICAL 0x01

#define OLED_NORMAL_V 0xC8
#define OLED_FLIP_V 0xC0
#define OLED_NORMAL_H 0xA1
#define OLED_FLIP_H 0xA0

#define OLED_CONTRAST 0x81
#define OLED_SETCOMPINS 0xDA
#define OLED_SETVCOMDETECT 0xDB
#define OLED_CLOCKDIV 0xD5
#define OLED_SETMULTIPLEX 0xA8
#define OLED_COLUMNADDR 0x21
#define OLED_PAGEADDR 0x22
#define OLED_CHARGEPUMP 0x8D

#define OLED_NORMALDISPLAY 0xA6
#define OLED_INVERTDISPLAY 0xA7

#define BUFSIZE_128x64 (128 * 64 / 8)
#define BUFSIZE_128x32 (128 * 32 / 8)

const uint8_t _maxRow = 7;
const uint8_t _maxY = 63;
const uint8_t _maxX = OLED_WIDTH - 1; 

static const uint8_t init_cmds[] = 
{
    OLED_DISPLAY_OFF,
    OLED_CLOCKDIV,
    0x80,
    OLED_CHARGEPUMP,
    0x14,
    OLED_ADDRESSING_MODE,
    OLED_VERTICAL,
    OLED_NORMAL_H,
    OLED_NORMAL_V,
    OLED_CONTRAST,
    0x7F,
    OLED_SETVCOMDETECT,
    0x40,
    OLED_NORMALDISPLAY,
    OLED_DISPLAY_ON
};

uint8_t buffer[128 * 64 / 8];


int OledMandarComandos(const uint8_t* mandar, int len)
{
    return i2c_write_blocking(OLED_PORT, OLED_ADDR, mandar, len, 0);
}

void OledMandarConfig(const uint8_t* mandar, int len)
{
    for(int i = 0; i < len; i++)
    {
        uint8_t data[2] = {0x00, mandar[i]};
        OledMandarComandos(data, 2);
    }
}

void vaciar()
{
    memset(buffer, 0, sizeof(buffer));
}

void OledMandar(uint8_t* buf, int len, uint8_t page)
{
    uint8_t data = 0x40;
    OledMandarComandos(&data, 1);
    data = 0x00;
    OledMandarComandos(&data, 1);
    data = 0x40;
    OledMandarComandos(&data, 1);
    data = 0x10;
    OledMandarComandos(&data, 1);
    data = 0x40;
    OledMandarComandos(&data, 1);
    data = 0xB0 + page;
    OledMandarComandos(&data, 1);
    data = 0x40;
    OledMandarComandos(&data, 1);

    OledMandarComandos(buf, len);
}

void OledMandarBuffer()
{
    //0x00
    //0x10
    //0x40
    //UNA VEZ POR PAGINA
        //0xB0 + num pagina
        //2 & 0xf
        //0x10


    for(int i = 0; i < 8; i++)
    {
        OledMandar(&buffer[i * 128], 128, i); //Mandar la pagina indicada antes
    }
}

void oled_cambiar_pixel(uint8_t x, uint8_t y, bool encender)
{
    uint8_t page = y / 8; //Sacar en que pagina está el pixel deseado. Hay 8 paginas verticales de 8 bits cada una, 
                          //cada bit de cada pagina hace referencia a un pixel de la pagina. 8*8=64
    uint8_t bit = y % 8;  //Sacar que bit de la pagina es el deseado

    uint8_t i = x + page*128; //El array tiene 8 paginas por cada linea vertical

    buffer[i] = encender ? buffer[i] | (1 << bit) : buffer[i] & ~(1 << bit);
}


void init()
{
    OledMandarConfig(init_cmds, sizeof(init_cmds));
    uint8_t extra[] = {OLED_SETCOMPINS, OLED_HEIGHT_64, OLED_SETMULTIPLEX, OLED_64};
    OledMandarConfig(extra, sizeof(extra));
    uint8_t extra2[] = {OLED_COLUMNADDR, _maxX, OLED_PAGEADDR, _maxRow};
    OledMandarConfig(extra2, sizeof(extra2));

    vaciar();
}


void main()
{
    stdio_init_all();

    i2c_init(OLED_PORT, 100 * 1000); // Initialize I2C at 100kHz
    gpio_set_function(OLED_SDA, GPIO_FUNC_I2C);
    gpio_set_function(OLED_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(OLED_SDA);
    gpio_pull_up(OLED_SCL);
    init();

    bool encendido = true;
    
    while(1)
    {
        for(uint8_t x = 0; x < 100; x++)
        {
            for(uint8_t y = 0; y < 60; y++)
            {
                oled_cambiar_pixel(x, y, encendido);
            }
        }
        sleep_ms(100);
        OledMandarBuffer();
        encendido != encendido;
    }


}
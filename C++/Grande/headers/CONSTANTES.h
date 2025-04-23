#ifndef PINES_H
#define PINES_H

#define PIN_BUZZER 0
#define PIN_SD_LED 1
#define PIN_LED_ABORTAR 2
#define PIN_OLED_SDA 4
#define PIN_OLED_SCL 5
#define PIN_ENCODER_1_A 10
#define PIN_ENCODER_1_B 11
#define PIN_ENCODER_2_A 12
#define PIN_ENCODER_2_B 13
#define PIN_ENCODER_3_A 14
#define PIN_ENCODER_3_B 15
#define PIN_SD_RX 16
#define PIN_SD_CS 17
#define PIN_SD_SCK 18
#define PIN_SD_TX 19
#define PIN_ENCODER_3_BOTON 20
#define PIN_ENCODER_2_BOTON 21
#define PIN_ENCODER_1_BOTON 22
#define PIN_LED_VERDE 26
#define PIN_LED_AMARILLO 27
#define PIN_LED_ROJO 28
//Libres: 3, 6, 7, 8, 9

#pragma region OLED

#define OLED_ADDR 0x3C
#define OLED_CLK 400
#define OLED_I2C i2c0
#define OLED_ANCHO 128
#define OLED_ALTO 64

#define OLED_LOW_COL 0x00
#define OLED_HIGH_COL 0x10
#define OLED_PUMP_VALUE 0x30
//Se le puede sumar hasta 63
#define OLED_START_LINE 0x40
//Hay que poner luego un contraste en otra linea
#define OLED_CONTRAST 0x81
#define OLED_SEGMENT_REMAP_L 0xA0
#define OLED_SEGMENT_REMAP_H 0xA1
#define OLED_SCREEN_RAM 0xA4
#define OLED_SCREEN_WHITE 0xA5
#define OLED_SCREEN_NORMAL 0xA6
#define OLED_SCREEN_REVERSE 0xA7
#define OLED_MULTIPLEX_RATIO 0xA8
#define OLED_MULTIPLEX_DEFAULT 0x3F
#define OLED_SCREEN_OFF 0xAE
#define OLED_SCREEN_ON 0xAF
#define OLED_DC_DC 0xAD
#define OLED_DC_DC_ON OLED_SCREEN_ON
#define OLED_CD_CD_OFF OLED_SCREEN_OFF
//Se le puede sumar hasta 7 para cambiar de pagina
#define OLED_SET_PAGE 0xB0
#define OLED_SCAN_DIRECTION_INVERTED 0xC0
#define OLED_SCAN_DIRECTION_NORMAL 0xC8
//Después de este comando hay que poner otro con el offset (hasta 63)
#define OLED_OFFSET 0xD3
#define OLED_CLK_DIVIDE 0xD5
#define OLED_CLK_DIVIDE_DEFAULT 0x50
#define OLED_PRECHARGE 0xD9
#define OLED_PRECHARGE_DEFAULT 0x22
#define OLED_COMMON_PADS 0XDA
#define OLED_COMMON_PADS_DEFAULT 0x12
#define OLED_VCOM_DESELECT 0xDB
#define OLED_VCOM_DESELECT_DEDAULT 0x35

#pragma endregion

#define BOTON_DEBOUNCE 50000

#endif
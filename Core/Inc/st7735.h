#ifndef ST7735_H
#define ST7735_H

#include "main.h"
#include "spi.h"

// Размеры дисплея
#define ST7735_WIDTH  160
#define ST7735_HEIGHT 128

// Цвета в формате RGB565
#define ST7735_BLACK   0x0000
#define ST7735_WHITE   0xFFFF
#define ST7735_RED     0xF800
#define ST7735_GREEN   0x07E0
#define ST7735_BLUE    0x001F
#define ST7735_YELLOW  0xFFE0
#define ST7735_CYAN    0x07FF
#define ST7735_MAGENTA 0xF81F

// ========== ПРАВИЛЬНЫЕ СМЕЩЕНИЯ ДЛЯ WEACT STUDIO ==========
#define ST7735_X_OFFSET 0   // Смещение по горизонтали (col_start)
#define ST7735_Y_OFFSET 0   // Смещение по вертикали (row_start)


// ========== УПРАВЛЯЮЩИЕ ПИНЫ ==========
// CS  - Chip Select   (PE11)
#define ST7735_CS_LOW()   HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, GPIO_PIN_RESET)
#define ST7735_CS_HIGH()  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, GPIO_PIN_SET)

// DC  - Data/Command  (PE13)
#define ST7735_DC_LOW()   HAL_GPIO_WritePin(GPIOE, GPIO_PIN_13, GPIO_PIN_RESET)
#define ST7735_DC_HIGH()  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_13, GPIO_PIN_SET)

// RST - Reset         (PE15)
#define ST7735_RST_LOW()  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_15, GPIO_PIN_RESET)
#define ST7735_RST_HIGH() HAL_GPIO_WritePin(GPIOE, GPIO_PIN_15, GPIO_PIN_SET)

// BL  - Backlight     (PE10) - LOW = ВКЛ
#define ST7735_BL_ON()    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_10, GPIO_PIN_RESET)
#define ST7735_BL_OFF()   HAL_GPIO_WritePin(GPIOE, GPIO_PIN_10, GPIO_PIN_SET)

// ========== ПРОТОТИПЫ ФУНКЦИЙ ==========
void ST7735_Init(void);
void ST7735_SendCommand(uint8_t cmd);
void ST7735_SendData(uint8_t data);
void ST7735_SetAddressWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);
void ST7735_FillScreen(uint16_t color);
void ST7735_DrawPixel(uint8_t x, uint8_t y, uint16_t color);
void ST7735_DisplayChar(uint8_t x, uint8_t y, char ch, uint16_t color, uint16_t bg);
void ST7735_DisplayString(uint8_t x, uint8_t y, char* str, uint16_t color, uint16_t bg);

#endif

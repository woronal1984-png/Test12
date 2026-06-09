#include "st7735.h"
#include <string.h>

extern SPI_HandleTypeDef hspi4;

// ========== ТАБЛИЦА ШРИФТА 5x7 ==========
static const uint8_t Font5x7[][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, // Пробел
    {0x00, 0x00, 0x4F, 0x00, 0x00}, // !
    {0x00, 0x07, 0x00, 0x07, 0x00}, // "
    {0x14, 0x3E, 0x14, 0x3E, 0x14}, // #
    {0x24, 0x2A, 0x7F, 0x2A, 0x12}, // $
    {0x23, 0x13, 0x08, 0x64, 0x62}, // %
    {0x36, 0x49, 0x55, 0x22, 0x50}, // &
    {0x00, 0x05, 0x07, 0x00, 0x00}, // '
    {0x00, 0x1C, 0x22, 0x41, 0x00}, // (
    {0x00, 0x41, 0x22, 0x1C, 0x00}, // )
    {0x14, 0x08, 0x3E, 0x08, 0x14}, // *
    {0x08, 0x08, 0x3E, 0x08, 0x08}, // +
    {0x00, 0x50, 0x30, 0x00, 0x00}, // ,
    {0x08, 0x08, 0x08, 0x08, 0x08}, // -
    {0x00, 0x60, 0x60, 0x00, 0x00}, // .
    {0x20, 0x10, 0x08, 0x04, 0x02}, // /
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, // 0
    {0x00, 0x42, 0x7F, 0x40, 0x00}, // 1
    {0x42, 0x61, 0x51, 0x49, 0x46}, // 2
    {0x21, 0x41, 0x45, 0x4B, 0x31}, // 3
    {0x18, 0x14, 0x12, 0x7F, 0x10}, // 4
    {0x27, 0x45, 0x45, 0x45, 0x39}, // 5
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, // 6
    {0x01, 0x71, 0x09, 0x05, 0x03}, // 7
    {0x36, 0x49, 0x49, 0x49, 0x36}, // 8
    {0x06, 0x49, 0x49, 0x29, 0x1E}, // 9
    {0x00, 0x36, 0x36, 0x00, 0x00}, // :
    {0x00, 0x56, 0x36, 0x00, 0x00}, // ;
    {0x08, 0x14, 0x22, 0x41, 0x00}, // <
    {0x14, 0x14, 0x14, 0x14, 0x14}, // =
    {0x00, 0x41, 0x22, 0x14, 0x08}, // >
    {0x02, 0x01, 0x51, 0x09, 0x06}, // ?
    {0x32, 0x49, 0x79, 0x41, 0x3E},  // @



	// ========== ЗАГЛАВНЫЕ A-Z (0x41-0x5A) ==========
	   {0x7E, 0x11, 0x11, 0x11, 0x7E}, // A
	   {0x7F, 0x49, 0x49, 0x49, 0x36}, // B
	   {0x3E, 0x41, 0x41, 0x41, 0x22}, // C
	   {0x7F, 0x41, 0x41, 0x22, 0x1C}, // D
	   {0x7F, 0x49, 0x49, 0x49, 0x41}, // E
	   {0x7F, 0x09, 0x09, 0x09, 0x01}, // F
	   {0x3E, 0x41, 0x49, 0x49, 0x3A}, // G
	   {0x7F, 0x08, 0x08, 0x08, 0x7F}, // H
	   {0x00, 0x41, 0x7F, 0x41, 0x00}, // I
	   {0x20, 0x40, 0x41, 0x3F, 0x01}, // J
	   {0x7F, 0x08, 0x14, 0x22, 0x41}, // K
	   {0x7F, 0x40, 0x40, 0x40, 0x40}, // L
	   {0x7F, 0x02, 0x0C, 0x02, 0x7F}, // M
	   {0x7F, 0x04, 0x08, 0x10, 0x7F}, // N
	   {0x3E, 0x41, 0x41, 0x41, 0x3E}, // O
	   {0x7F, 0x09, 0x09, 0x09, 0x06}, // P
	   {0x3E, 0x41, 0x51, 0x21, 0x5E}, // Q
	   {0x7F, 0x09, 0x19, 0x29, 0x46}, // R
	   {0x46, 0x49, 0x49, 0x49, 0x31}, // S
	   {0x01, 0x01, 0x7F, 0x01, 0x01}, // T
	   {0x3F, 0x40, 0x40, 0x40, 0x3F}, // U
	   {0x1F, 0x20, 0x40, 0x20, 0x1F}, // V
	   {0x7F, 0x20, 0x10, 0x20, 0x7F}, // W
	   {0x41, 0x22, 0x1C, 0x22, 0x41}, // X
	   {0x07, 0x08, 0x70, 0x08, 0x07}, // Y
	   {0x61, 0x51, 0x49, 0x45, 0x43}, // Z

	   // ========== СТРОЧНЫЕ a-z (0x61-0x7A) ==========
	   {0x20, 0x54, 0x54, 0x54, 0x78}, // a
	   {0x7F, 0x48, 0x44, 0x44, 0x38}, // b
	   {0x38, 0x44, 0x44, 0x44, 0x20}, // c
	   {0x38, 0x44, 0x44, 0x48, 0x7F}, // d
	   {0x38, 0x54, 0x54, 0x54, 0x18}, // e
	   {0x08, 0x7E, 0x09, 0x01, 0x02}, // f
	   {0x0C, 0x52, 0x52, 0x52, 0x3E}, // g
	   {0x7F, 0x08, 0x04, 0x04, 0x78}, // h
	   {0x00, 0x44, 0x7D, 0x40, 0x00}, // i
	   {0x20, 0x40, 0x44, 0x3D, 0x00}, // j
	   {0x7F, 0x10, 0x28, 0x44, 0x00}, // k
	   {0x00, 0x41, 0x7F, 0x40, 0x00}, // l
	   {0x7C, 0x04, 0x18, 0x04, 0x78}, // m
	   {0x7C, 0x08, 0x04, 0x04, 0x78}, // n
	   {0x38, 0x44, 0x44, 0x44, 0x38}, // o
	   {0x7C, 0x14, 0x14, 0x14, 0x08}, // p
	   {0x08, 0x14, 0x14, 0x18, 0x7C}, // q
	   {0x7C, 0x08, 0x04, 0x04, 0x08}, // r
	   {0x48, 0x54, 0x54, 0x54, 0x20}, // s
	   {0x04, 0x3F, 0x44, 0x40, 0x20}, // t
	   {0x3C, 0x40, 0x40, 0x20, 0x7C}, // u
	   {0x1C, 0x20, 0x40, 0x20, 0x1C}, // v
	   {0x3C, 0x40, 0x30, 0x40, 0x3C}, // w
	   {0x44, 0x28, 0x10, 0x28, 0x44}, // x
	   {0x0C, 0x50, 0x50, 0x50, 0x3C}, // y
	   {0x44, 0x64, 0x54, 0x4C, 0x44}  // z

	};

#define FONT_CHARS (sizeof(Font5x7) / sizeof(Font5x7[0]))  // = 59 символов

// ========== БАЗОВЫЕ ФУНКЦИИ SPI ==========

// Отправка команды
void ST7735_SendCommand(uint8_t cmd) {
    ST7735_CS_LOW();
    ST7735_DC_LOW();
    HAL_SPI_Transmit(&hspi4, &cmd, 1, HAL_MAX_DELAY);
    ST7735_CS_HIGH();
}

// Отправка одного байта данных
void ST7735_SendData(uint8_t data) {
    ST7735_CS_LOW();
    ST7735_DC_HIGH();
    HAL_SPI_Transmit(&hspi4, &data, 1, HAL_MAX_DELAY);
    ST7735_CS_HIGH();
}

// Отправка массива данных
static void ST7735_SendDataArray(uint8_t* data, uint16_t len) {
    ST7735_CS_LOW();
    ST7735_DC_HIGH();
    HAL_SPI_Transmit(&hspi4, data, len, HAL_MAX_DELAY);
    ST7735_CS_HIGH();
}

// ========== ИНИЦИАЛИЗАЦИЯ ДИСПЛЕЯ ==========

void ST7735_Init(void) {
    // Аппаратный сброс
    ST7735_RST_HIGH();
    HAL_Delay(50);
    ST7735_RST_LOW();
    HAL_Delay(50);
    ST7735_RST_HIGH();
    HAL_Delay(150);

    // Software Reset
    ST7735_SendCommand(0x01);
    HAL_Delay(150);

    // Sleep Out
    ST7735_SendCommand(0x11);
    HAL_Delay(150);

    // Frame Rate Control
    ST7735_SendCommand(0xB1);
    ST7735_SendData(0x01);
    ST7735_SendData(0x2C);
    ST7735_SendData(0x2D);

    ST7735_SendCommand(0xB2);
    ST7735_SendData(0x01);
    ST7735_SendData(0x2C);
    ST7735_SendData(0x2D);

    ST7735_SendCommand(0xB3);
    ST7735_SendData(0x01);
    ST7735_SendData(0x2C);
    ST7735_SendData(0x2D);
    ST7735_SendData(0x01);
    ST7735_SendData(0x2C);
    ST7735_SendData(0x2D);

    // Display Inversion Control
    ST7735_SendCommand(0xB4);
    ST7735_SendData(0x07);

    // Power Control 1
    ST7735_SendCommand(0xC0);
    ST7735_SendData(0xA2);
    ST7735_SendData(0x02);
    ST7735_SendData(0x84);

    // Power Control 2
    ST7735_SendCommand(0xC1);
    ST7735_SendData(0xC5);

    // Power Control 3
    ST7735_SendCommand(0xC2);
    ST7735_SendData(0x0A);
    ST7735_SendData(0x00);

    // Power Control 4
    ST7735_SendCommand(0xC3);
    ST7735_SendData(0x8A);
    ST7735_SendData(0x2A);

    // Power Control 5
    ST7735_SendCommand(0xC4);
    ST7735_SendData(0x8A);
    ST7735_SendData(0xEE);

    // VCOM Control
    ST7735_SendCommand(0xC5);
    ST7735_SendData(0x14);

    // Memory Access Control (MADCTL)
    // 0x78 = 120 десятичное - правильный параметр для WeAct Studio
    ST7735_SendCommand(0x36);
    ST7735_SendData(0xB8);

    // Pixel Format (RGB565)
    ST7735_SendCommand(0x3A);
    ST7735_SendData(0x05);

    // Positive Gamma Correction
    ST7735_SendCommand(0xE0);
    uint8_t gamma_pos[] = {0x02, 0x1C, 0x07, 0x12, 0x37, 0x32, 0x29, 0x2D,
                           0x29, 0x25, 0x2B, 0x39, 0x00, 0x01, 0x03, 0x10};
    for(int i = 0; i < 16; i++) {
        ST7735_SendData(gamma_pos[i]);
    }

    // Negative Gamma Correction
    ST7735_SendCommand(0xE1);
    uint8_t gamma_neg[] = {0x03, 0x1D, 0x07, 0x06, 0x2E, 0x2C, 0x29, 0x2D,
                           0x2E, 0x2E, 0x37, 0x3F, 0x00, 0x00, 0x02, 0x10};
    for(int i = 0; i < 16; i++) {
        ST7735_SendData(gamma_neg[i]);
    }

    // Column Address Set (установка области отображения)
    ST7735_SendCommand(0x2A);
    ST7735_SendData(0x00);
    ST7735_SendData(0x01);  // Start = 1
    ST7735_SendData(0x00);
    ST7735_SendData(0xA0);  // End = 160

    // Row Address Set
    ST7735_SendCommand(0x2B);
    ST7735_SendData(0x00);
    ST7735_SendData(0x1A);  // Start = 26
    ST7735_SendData(0x00);
    ST7735_SendData(0x69);  // End = 105

    // Display On
    ST7735_SendCommand(0x29);
    HAL_Delay(50);

    // Заливка экрана черным цветом
    ST7735_FillScreen(ST7735_BLACK);
}

// ========== ФУНКЦИИ ОТОБРАЖЕНИЯ ==========

// Установка окна для вывода (с учетом смещений WeAct Studio)
void ST7735_SetAddressWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) {
    // В альбомной ориентации координаты нужно правильно преобразовать
    // В зависимости от MADCTL, может потребоваться "переворот" координат

    uint16_t x_start = x0 + ST7735_X_OFFSET;
    uint16_t x_end = x1 + ST7735_X_OFFSET;
    uint16_t y_start = y0 + ST7735_Y_OFFSET;
    uint16_t y_end = y1 + ST7735_Y_OFFSET;

    ST7735_SendCommand(0x2A);  // Column address
    ST7735_SendData(x_start >> 8);
    ST7735_SendData(x_start & 0xFF);
    ST7735_SendData(x_end >> 8);
    ST7735_SendData(x_end & 0xFF);

    ST7735_SendCommand(0x2B);  // Row address
    ST7735_SendData(y_start >> 8);
    ST7735_SendData(y_start & 0xFF);
    ST7735_SendData(y_end >> 8);
    ST7735_SendData(y_end & 0xFF);

    ST7735_SendCommand(0x2C);  // Memory write
}

// Заливка всего экрана одним цветом
void ST7735_FillScreen(uint16_t color) {
    uint8_t colorBuf[2];
    colorBuf[0] = (color >> 8) & 0xFF;
    colorBuf[1] = color & 0xFF;

    ST7735_SetAddressWindow(0, 0, ST7735_WIDTH - 1, ST7735_HEIGHT - 1);

    ST7735_CS_LOW();
    ST7735_DC_HIGH();

    uint32_t pixels = ST7735_WIDTH * ST7735_HEIGHT;
    for (uint32_t i = 0; i < pixels; i++) {
        HAL_SPI_Transmit(&hspi4, colorBuf, 2, HAL_MAX_DELAY);
    }

    ST7735_CS_HIGH();
}

// Рисование одного пикселя
void ST7735_DrawPixel(uint8_t x, uint8_t y, uint16_t color) {
    // Проверка границ
    if (x >= ST7735_WIDTH || y >= ST7735_HEIGHT) return;

    uint8_t colorBuf[2];
    colorBuf[0] = (color >> 8) & 0xFF;
    colorBuf[1] = color & 0xFF;

    ST7735_SetAddressWindow(x, y, x, y);
    ST7735_SendDataArray(colorBuf, 2);
}

// Вывод одного символа
void ST7735_DisplayChar(uint8_t x, uint8_t y, char ch, uint16_t color, uint16_t bg) {
    // Проверка границ
    if (x + 5 > ST7735_WIDTH || y + 7 > ST7735_HEIGHT) return;

    // Преобразование символа в индекс таблицы
    uint8_t index;

    if (ch >= 0x20 && ch <= 0x5A) {
        // Символы от пробела до Z
        index = ch - 0x20;
    } else if (ch >= 0x61 && ch <= 0x7A) {
        // Строчные буквы a-z преобразуем в заглавные A-Z
        index = (ch - 0x61) + 0x41 - 0x20;
    } else {
        // Неизвестный символ - отображаем пробел
        index = 0;  // Пробел
    }

    // Ограничение индекса размером таблицы
    if (index >= FONT_CHARS) {
        index = 0;
    }

    // Рисуем символ
    for (uint8_t row = 0; row < 7; row++) {
        for (uint8_t col = 0; col < 5; col++) {
            if (Font5x7[index][col] & (1 << row)) {
                ST7735_DrawPixel(x + col, y + row, color);
            } else {
                ST7735_DrawPixel(x + col, y + row, bg);
            }
        }
    }
}

// Вывод строки символов
void ST7735_DisplayString(uint8_t x, uint8_t y, char* str, uint16_t color, uint16_t bg) {
    uint8_t x_pos = x;
    uint8_t y_pos = y;

    while (*str != '\0') {
        // Автоматический перенос строки
        if (x_pos + 6 > ST7735_WIDTH) {
            x_pos = x;
            y_pos += 8;
            if (y_pos + 8 > ST7735_HEIGHT) break;
        }

        ST7735_DisplayChar(x_pos, y_pos, *str, color, bg);
        x_pos += 6;
        str++;
    }
}

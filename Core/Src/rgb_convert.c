/*
 * rgb_convert.c
 *
 *  Created on: 14 окт. 2026 г.
 *      Author: Alex
 */



#include "rgb_convert.h"

uint16_t rgb_buffer[RGB_BUFFER_SIZE] __attribute__((aligned(16)));

/**
 * Упаковка 8-битных RGB в 16-битный RGB565
 * Формат: RRRRRGGG GGGBBBBB
 */

static inline uint16_t pack_rgb565(uint8_t r, uint8_t g, uint8_t b)
{
    // R: 8 бит -> 5 бит, G: 8 бит -> 6 бит, B: 8 бит -> 5 бит
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

/**
 * Демозаикинг RAW8 (BGGR) в RGB565.
 * Обрабатывает блоки 2x2, выходное разрешение вдвое меньше.
 *
 * @param bayer     Указатель на исходный Bayer-буфер (ширина BAYER_WIDTH)
 * @param rgb       Указатель на выходной RGB565-буфер
 * @param src_width Ширина исходного Bayer-изображения
 * @param src_height Высота исходного Bayer-изображения
 */
void raw8_bggr_to_rgb565(const uint8_t *bayer, uint16_t *rgb,
                         uint16_t src_width, uint16_t src_height)
{
    const uint16_t out_width = src_width / 2;
    const uint16_t out_height = src_height / 2;

    for (uint16_t y = 0; y < out_height; y++)
    {
        // Две строки Bayer, соответствующие одной строке выходного изображения
        const uint8_t *row0 = bayer + (2 * y) * src_width;
        const uint8_t *row1 = bayer + (2 * y + 1) * src_width;

        for (uint16_t x = 0; x < out_width; x++)
        {
            // Позиции в блоке 2x2
            // row0[x*2 + 0] = B
            // row0[x*2 + 1] = G
            // row1[x*2 + 0] = G
            // row1[x*2 + 1] = R

            uint8_t b = row0[x * 2 + 0];       // B
            uint8_t g1 = row0[x * 2 + 1];      // G
            uint8_t g2 = row1[x * 2 + 0];      // G
            uint8_t r = row1[x * 2 + 1];       // R

            // Средний зелёный
            uint8_t g = (uint8_t)(((uint16_t)g1 + g2) >> 1);

            // Упаковка в RGB565
            rgb[y * out_width + x] = pack_rgb565(r, g, b);
        }
    }
}

/**
 * Версия с усреднением по 4 соседним пикселям для лучшего качества.
 * Требует, чтобы ширина и высота были кратны 4.
 */
void raw8_bggr_to_rgb565_interpolated(const uint8_t *bayer, uint16_t *rgb,
                                      uint16_t width, uint16_t height)
{
	int div = 4;
    const uint16_t out_width = width / div;
    const uint16_t out_height = height / div;

    for (uint16_t oy = 0; oy < out_height; oy++)
    {
        for (uint16_t ox = 0; ox < out_width; ox++)
        {
            // Индексы в Bayer-матрице
            uint16_t by = oy * div;
            uint16_t bx = ox * div;

            uint32_t r_sum = 0, g_sum = 0, b_sum = 0;
            uint32_t r_cnt = 0, g_cnt = 0, b_cnt = 0;

            // Проходим по окрестности 3x3 вокруг блока 2x2
            // (простой метод: берём только пиксели того же цвета в радиусе)
            for (int8_t dy = -1; dy <= 2; dy++)
            {
                for (int8_t dx = -1; dx <= 2; dx++)
                {
                    int16_t py = (int16_t)by + dy;
                    int16_t px = (int16_t)bx + dx;

                    if (py < 0 || py >= (int16_t)height ||
                        px < 0 || px >= (int16_t)width)
                        continue;

                    uint8_t val = bayer[py * width + px];

                    // Определяем цвет пикселя по паттерну BGGR
                    // Чётная строка: B G B G ...
                    // Нечётная строка: G R G R ...
                    if ((py & 1) == 0)
                    {
                        if ((px & 1) == 0) { b_sum += val; b_cnt++; }
                        else                { g_sum += val; g_cnt++; }
                    }
                    else
                    {
                        if ((px & 1) == 0) { g_sum += val; g_cnt++; }
                        else                { r_sum += val; r_cnt++; }
                    }
                }
            }

            uint8_t r = (r_cnt > 0) ? (uint8_t)(r_sum / r_cnt) : 0;
            uint8_t g = (g_cnt > 0) ? (uint8_t)(g_sum / g_cnt) : 0;
            uint8_t b = (b_cnt > 0) ? (uint8_t)(b_sum / b_cnt) : 0;

            rgb[oy * out_width + ox] = pack_rgb565(r, g, b);
        }
    }
}

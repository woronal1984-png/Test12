/*
 * rgb_convert.h
 *
 *  Created on: 14 окт. 2026 г.
 *      Author: Alex
 */
#include <stdint.h>

#ifndef INC_RGB_CONVERT_H_
#define INC_RGB_CONVERT_H_

// Размеры исходного Bayer-изображения
#define BAYER_WIDTH   320
#define BAYER_HEIGHT  240

// Размеры выходного RGB565 (вдвое меньше)
#define RGB_WIDTH     (BAYER_WIDTH / 2)
#define RGB_HEIGHT    (BAYER_HEIGHT / 2)
#define RGB_BUFFER_SIZE (RGB_WIDTH * RGB_HEIGHT)

// Буфер RGB565
extern uint16_t rgb_buffer[RGB_BUFFER_SIZE] __attribute__((aligned(32)));


/**
 * Демозаикинг RAW8 (BGGR) в RGB565.
 * Обрабатывает блоки 2x2, выходное разрешение вдвое меньше.
 *
 * @param bayer     Указатель на исходный Bayer-буфер (ширина BAYER_WIDTH)
 * @param rgb       Указатель на выходной RGB565-буфер
 * @param src_width Ширина исходного Bayer-изображения
 * @param src_height Высота исходного Bayer-изображения
 */
void raw8_bggr_to_rgb565(const uint8_t *bayer, uint16_t *rgb, uint16_t src_width, uint16_t src_height);

/**
 * Версия с усреднением по 4 соседним пикселям для лучшего качества.
 * Требует, чтобы ширина и высота были кратны 4.
 */
void raw8_bggr_to_rgb565_interpolated(const uint8_t *bayer, uint16_t *rgb, uint16_t width, uint16_t height);





#endif /* INC_RGB_CONVERT_H_ */

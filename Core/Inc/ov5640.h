// ov5640.h
#ifndef OV5640_H
#define OV5640_H

#include "main.h"
#include <stdint.h>
#include "ov5640_regs.h"

// Адрес камеры на шине I2C (0x78 для записи / 0x79 для чтения)
#define OV5640_ADDR    0x78


// Размеры кадра (VGA)
#define IMAGE_WIDTH 320
#define IMAGE_HEIGHT 240

// Количество пикселей
#define IMAGE_PIXELS  (IMAGE_WIDTH * IMAGE_HEIGHT)  // 76800 pixel

// Размер буфера в байтах (RAW = 1 байт на пиксель)
#define IMAGE_BUFFER_BYTES  (IMAGE_PIXELS * 1)  // 76800 байт

// Размер для DMA в 32-битных словах
#define IMAGE_DMA_WORDS  (IMAGE_BUFFER_BYTES / 4)  // 19200 слов


// Массив uint16_t (каждый элемент = 1 пиксель)
extern uint32_t  g_camera_frame[IMAGE_DMA_WORDS] __attribute__((section(".camera_buffer"), aligned(32)));



//extern volatile uint8_t g_frame_capture_complete;
//extern const uint8_t ov5640_default_regs[][3];


// Функции драйвера
uint8_t OV5640_Init(void);
uint8_t OV5640_ReadID(uint16_t *DeviceID);
void OV5640_StartCapture(void);
void OV5640_StopCapture(void);

uint8_t OV5640_WriteReg(uint16_t reg, uint8_t data);
uint8_t OV5640_ReadReg(uint16_t reg);
uint16_t OV5640_ReadID1(void);


#endif

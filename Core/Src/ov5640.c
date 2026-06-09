/*
 * ov5640.c
 *
 *  Created on: 7 июн. 2026 г.
 *      Author: Alex
 */


// ov5640.c
#include "ov5640.h"
#include "dcmi.h"   // HAL DCMI драйвер
#include "i2c.h"    // HAL I2C драйвер

// Глобальные переменные
uint8_t g_camera_frame[IMAGE_FRAME_SIZE] __attribute__((section(".dtcmram")));
volatile uint8_t g_frame_capture_complete = 0;

// Прототипы
static uint8_t OV5640_WriteRegister(uint16_t reg, uint8_t data);
static uint8_t OV5640_ReadRegister(uint16_t reg, uint8_t *data);
static void OV5640_Config_JPEG(void); // Для продвинутого режима
static void OV5640_Config_RGB565(void); // Наш целевой режим

// Функция записи в регистр (эмуляция SCCB через I2C)
static uint8_t OV5640_WriteRegister(uint16_t reg, uint8_t data) {
    uint8_t buf[3];
    buf[0] = (reg >> 8) & 0xFF;
    buf[1] = reg & 0xFF;
    buf[2] = data;
    // HAL_I2C_Master_Transmit(handle, slave_addr, data, size, timeout)
    // Сдвигаем адрес влево на 1, т.к. HAL ждет 7-битный адрес без бита RW
    if (HAL_I2C_Master_Transmit(&hi2c1, OV5640_ADDR >> 1, buf, 3, HAL_MAX_DELAY) != HAL_OK)
        return 1;
    return 0;
}

// Чтение регистра
static uint8_t OV5640_ReadRegister(uint16_t reg, uint8_t *data) {
    uint8_t buf[2];
    buf[0] = (reg >> 8) & 0xFF;
    buf[1] = reg & 0xFF;
    if (HAL_I2C_Master_Transmit(&hi2c1, OV5640_ADDR >> 1, buf, 2, HAL_MAX_DELAY) != HAL_OK)
        return 1;
    if (HAL_I2C_Master_Receive(&hi2c1, OV5640_ADDR >> 1, data, 1, HAL_MAX_DELAY) != HAL_OK)
        return 1;
    return 0;
}

// Проверка ID (главная проверка, что камера жива)
uint8_t OV5640_ReadID(uint16_t *DeviceID) {
    uint8_t high = 0, low = 0;
    if (OV5640_ReadRegister(0x300A, &high)) return 1;
    if (OV5640_ReadRegister(0x300B, &low)) return 1;
    *DeviceID = (high << 8) | low;
    return 0;
}

// Сброс камеры аппаратно
static void OV5640_HardReset(void) {
    HAL_GPIO_WritePin(OV5640_RST_GPIO_Port, OV5640_RST_Pin, GPIO_PIN_RESET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(OV5640_RST_GPIO_Port, OV5640_RST_Pin, GPIO_PIN_SET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(OV5640_PWDN_GPIO_Port, OV5640_PWDN_Pin, GPIO_PIN_RESET); // PWDN=0 Работаем
    HAL_Delay(50);
}

// Базовая инициализация (заливка регистров)
uint8_t OV5640_Init(void) {
    uint16_t id = 0;

    OV5640_HardReset();

    // Задержка для стабилизации питания
    HAL_Delay(20);

    // Проверяем ID. Если чтение не удалось (HAL_BUSY или ошибка) — камера не отвечает.
    if (OV5640_ReadID(&id) != 0) {
        return 1; // Ошибка I2C
    }

    if (id != 0x5640) {
        return 2; // Неверный ID (возможно, другая камера)
    }

    // 1. Программный сброс через регистр
    OV5640_WriteRegister(0x3008, 0x82);
    HAL_Delay(100);

    // 2. Настройка тактирования (включение PLL для работы от 24 МГц)
    OV5640_WriteRegister(0x3034, 0x1A); // PLL config
    OV5640_WriteRegister(0x3035, 0x21); // PLL config
    OV5640_WriteRegister(0x3036, 0x46); // PLL config
    OV5640_WriteRegister(0x3037, 0x03); // PLL config

    // 3. Настройка разрешения (VGA) и формата (RGB565) - критические настройки!
    //    Обратите внимание: OV5640 имеет огромное количество регистров.
    //    Полный массив инициализации (около 100-200 строк) опущен для краткости.
    //    Однако ключевые:
    OV5640_WriteRegister(0x3035, 0x41); // PLL multiply
    OV5640_WriteRegister(0x3C07, 0x08); // Light mode = 60Hz

    // Выбор формата вывода: RGB565
    OV5640_WriteRegister(0x4300, 0x61); // Output format control 1 (RGB565)
    OV5640_WriteRegister(0x501F, 0x01); // ISP: RGB format

    // Настройка окна вывода (640x480)
    OV5640_WriteRegister(0x3800, 0x00); OV5640_WriteRegister(0x3801, 0x00); // X start
    OV5640_WriteRegister(0x3802, 0x00); OV5640_WriteRegister(0x3803, 0x00); // Y start
    OV5640_WriteRegister(0x3804, 0x02); OV5640_WriteRegister(0x3805, 0x7F); // X end (639)
    OV5640_WriteRegister(0x3806, 0x01); OV5640_WriteRegister(0x3807, 0xDF); // Y end (479)

    // Настройка DMA/DCMI на стороне микроконтроллера выполняются в основном файле,
    // здесь мы лишь говорим камере "начать вещание".

    return 0;
}

// Запуск захвата DMA через HAL
void OV5640_StartCapture(void) {
    g_frame_capture_complete = 0;
    // Запуск DCMI в режиме одиночного снимка (Snapshot)
    // Адрес буфера, размер в байтах (преобразуем кол-во пикселов -> байты)
    HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_SNAPSHOT, (uint32_t)g_camera_frame, IMAGE_FRAME_SIZE / 4);
    // Примечание: размер считается в 32-битных словах (раз / 4), т.к. Word = 4 байта.
}

void OV5640_StopCapture(void) {
    HAL_DCMI_Stop(&hdcmi);
}

// Пример конфигурации для работы с JPEG (если понадобится)
static void OV5640_Config_JPEG(void) {
    // Переключить камеру в JPEG режим: 0x3500, 0x4600 и т.д.
    // Не используется в текущем базовом примере
}

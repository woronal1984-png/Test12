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
uint16_t g_camera_frame[IMAGE_PIXELS] __attribute__((aligned(32)));

// Прототипы
static uint8_t OV5640_WriteRegister(uint16_t reg, uint8_t data);
static uint8_t OV5640_ReadRegister(uint16_t reg, uint8_t *data);
static void OV5640_Config_JPEG(void); // Для продвинутого режима
static void OV5640_Config_RGB565(void); // Наш целевой режим


extern I2C_HandleTypeDef hi2c1;


//#define OV5640_ADDR 0x3C


uint8_t OV5640_WriteReg(uint16_t reg, uint8_t data)
{
    uint8_t buffer[3];
    buffer[0] = (reg >> 8) & 0xFF;  // Старший байт адреса
    buffer[1] = reg & 0xFF;         // Младший байт адреса
    buffer[2] = data;               // Данные

    HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(&hi2c1, OV5640_ADDR , buffer, 3, 100);

    if (status != HAL_OK)
    {
        // Обработка ошибки
        return 1;
    }
    return 0;
}

/**
  * @brief  Чтение байта из регистра OV5640
  * @param  reg: 16-битный адрес регистра
  * @retval Прочитанное значение
  */
uint8_t OV5640_ReadReg(uint16_t reg)
{
    uint8_t reg_addr[2];
    uint8_t data = 0;

    reg_addr[0] = (reg >> 8) & 0xFF;
    reg_addr[1] = reg & 0xFF;

    // Отправляем адрес регистра
    HAL_I2C_Master_Transmit(&hi2c1, OV5640_ADDR, reg_addr, 2, 100);
    // Читаем данные
    HAL_I2C_Master_Receive(&hi2c1, OV5640_ADDR, &data, 1, 100);

    return data;
}

/**
  * @brief  Чтение ID камеры
  * @retval ID камеры (должно быть 0x5640)
  */
uint16_t OV5640_ReadID1(void)
{
    uint16_t pid = 0;
    pid = OV5640_ReadReg(0x300A);
    pid = (pid << 8) | OV5640_ReadReg(0x300B);
    return pid;
}






// Функция записи в регистр (эмуляция SCCB через I2C)
static uint8_t OV5640_WriteRegister(uint16_t reg, uint8_t data) {
    uint8_t buf[3];
    buf[0] = (reg >> 8) & 0xFF;
    buf[1] = reg & 0xFF;
    buf[2] = data;
    // HAL_I2C_Master_Transmit(handle, slave_addr, data, size, timeout)
    // Сдвигаем адрес влево на 1, т.к. HAL ждет 7-битный адрес без бита RW
    if (HAL_I2C_Master_Transmit(&hi2c1, OV5640_ADDR, buf, 3, HAL_MAX_DELAY) != HAL_OK)
        return 1;
    return 0;
}

// Чтение регистра
static uint8_t OV5640_ReadRegister(uint16_t reg, uint8_t *data) {
    uint8_t buf[2];
    buf[0] = (reg >> 8) & 0xFF;
    buf[1] = reg & 0xFF;
    if (HAL_I2C_Master_Transmit(&hi2c1, OV5640_ADDR , buf, 2, HAL_MAX_DELAY) != HAL_OK)
        return 1;
    if (HAL_I2C_Master_Receive(&hi2c1, OV5640_ADDR , data, 1, HAL_MAX_DELAY) != HAL_OK)
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

 // 2. Проверка ID камеры (должно быть 0x5640)
    uint16_t pid = 0;
    pid = OV5640_ReadReg(0x300A); // Читаем старший байт PID
    pid = (pid << 8) | OV5640_ReadReg(0x300B);
    if (pid != 0x5640) return 1; // Ошибка: камера не отвечает

    // Задержка для стабилизации питания
    HAL_Delay(20);


    uint32_t i;
    for ( i = 0; i < ov5640_default_regs_size; i++)
        {
        	OV5640_WriteReg(ov5640_default_regs[i][0] << 8 | ov5640_default_regs[i][1],
                        ov5640_default_regs[i][2]);
            // Можно добавить небольшую задержку для некоторых регистров
        	HAL_Delay(2);
        }


    HAL_Delay(100);

    // Проверяем ID. Если чтение не удалось (HAL_BUSY или ошибка) — камера не отвечает.
    if (OV5640_ReadID(&id) != 0) {
        return 1; // Ошибка I2C
    }

    if (id != 0x5640) {
        return 2; // Неверный ID (возможно, другая камера)
    }


    // Должно быть 0x61 для RGB565 little-endian
	if ( OV5640_ReadReg(0x4300) != 0x61) {
		return 3; //Wrong format!
	}

    return 0;
}

// Запуск захвата DMA через HAL
void OV5640_StartCapture(void) {
    g_frame_capture_complete = 0;
    // Запуск DCMI в режиме одиночного снимка (Snapshot) DCMI_MODE_SNAPSHOT   DCMI_MODE_CONTINUOUS
    // Адрес буфера, размер в байтах (преобразуем кол-во пикселов -> байты)
    HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_SNAPSHOT, (uint32_t)g_camera_frame, IMAGE_DMA_WORDS);

}

void OV5640_StopCapture(void) {
    HAL_DCMI_Stop(&hdcmi);
}

// Пример конфигурации для работы с JPEG (если понадобится)
static void OV5640_Config_JPEG(void) {
    // Переключить камеру в JPEG режим: 0x3500, 0x4600 и т.д.
    // Не используется в текущем базовом примере
}

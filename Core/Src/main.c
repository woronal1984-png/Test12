/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dcmi.h"
#include "dma.h"
#include "i2c.h"
#include "rtc.h"
#include "sdmmc.h"
#include "spi.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "ov5640.h"
#include <stdio.h>
#include <string.h>
#include "st7735.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

volatile uint8_t g_frame_capture_complete = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


static uint16_t line_count = 0;
static uint32_t frame_count = 0;
/**
  * @brief Обработчик завершения DMA для DCMI
  * HAL вызовет эту функцию, когда передача данных закончится
  */
void HAL_DCMI_FrameEventCallback(DCMI_HandleTypeDef *hdcmi) {
    // Устанавливаем флаг, что кадр готов к обработке
    g_frame_capture_complete = 1;
    // Останавливаем захват, если он был в режиме Snapshot (для экономии энергии)
     HAL_DCMI_Stop(hdcmi);

    // Здесь можно обновить дисплей
      // ST7735_DrawImage(0, 0, 320, 240, camera_buffer);


    frame_count++;

}



/**
  * @brief Обработчик ошибок DCMI
  */
void HAL_DCMI_ErrorCallback(DCMI_HandleTypeDef *hdcmi) {
    // Можно зажечь светодиод ошибки или вывести сообщение в UART
    // Error handler
    //g_frame_capture_complete = 0xFF; // Специальное значение для ошибки
}





uint32_t checksum_prev = 0;

void Check_Buffer_Data(void) {
    // Проверяем первые 100 пикселей на изменения
    uint32_t sum = 0;
    for (int i = 0; i < 100; i++) {
        sum += g_camera_frame[i];
    }

    if (sum != checksum_prev) {
        // Данные изменились - камера что-то передает
        ST7735_DisplayString(10, 40, "DATA CHANGED!",  ST7735_WHITE, ST7735_BLACK);
        checksum_prev = sum;
    } else {
        // Данные не меняются
        ST7735_DisplayString(10, 40, "DATA STABLE",  ST7735_WHITE, ST7735_BLACK);
    }
}


void Check_DMA_Activity(void) {
    // NDTR - количество оставшихся слов для передачи
    uint32_t ndtr = DMA2_Stream0->NDTR;

    // Статус DMA (включен ли)
    uint32_t dma_en = (DMA2_Stream0->CR & DMA_SxCR_EN) ? 1 : 0;

    char msg[40];
    sprintf(msg, "NDTR: %lu, EN: %d", ndtr, dma_en);
    ST7735_DisplayString(10, 50, msg, ST7735_WHITE, ST7735_BLACK);

    // Если NDTR меняется - данные идут
    static uint32_t prev_ndtr = 0;
    if (ndtr != prev_ndtr) {
        ST7735_DisplayString(10, 70, "DMA IS TRANSFERRING", ST7735_WHITE, ST7735_BLACK);
        prev_ndtr = ndtr;
    } else {
        ST7735_DisplayString(10, 70, "DMA NOT TRANSFERRING", ST7735_WHITE, ST7735_BLACK);
    }
}

void Check_DMA_Error(void) {
    // Проверяем флаги ошибок DMA2 Stream0
    uint32_t lisr = DMA2->LISR;  // Low Interrupt Status Register
    uint32_t hisr = DMA2->HISR;  // High Interrupt Status Register

    // Для Stream0 используются биты в LISR
    if (lisr & DMA_LISR_TEIF0) {
        ST7735_DisplayString(10, 70, "DMA TRANSFER ERROR!", ST7735_WHITE, ST7735_BLACK);
    }
    if (lisr & DMA_LISR_FEIF0) {
        ST7735_DisplayString(10, 70, "DMA FIFO ERROR!", ST7735_WHITE, ST7735_BLACK);
    }
    if (lisr & DMA_LISR_DMEIF0) {
        ST7735_DisplayString(10, 70, "DMA DIRECT MODE ERROR!", ST7735_WHITE, ST7735_BLACK);
    }
}


// Добавим DMA callback для отладки
void HAL_DCMI_LineEventCallback (DCMI_HandleTypeDef *hdcmi) {
    // Этот коллбэк вызывается на каждую строку

	line_count ++;
	Check_DMA_Error();
    // Каждые 10 строк показываем прогресс
    if (line_count % 10 == 0) {
       // ST7735_DisplayString(10, 30, "Test",  ST7735_WHITE, ST7735_BLACK);

        // Если счетчик дошел до 1080 - это кадр
        if (line_count >= 1080) {
            //ST7735_DisplayString(10, 50, "FRAME COMPLETE!",  ST7735_WHITE, ST7735_BLACK);
            line_count = 0;  // Сброс для следующего кадра
        }
    }






}


void HAL_DCMI_VsyncEventCallback(DCMI_HandleTypeDef *hdcmi) {
    // Этот колбэк должен вызываться на каждый VSYNC
    static uint32_t vsync_count = 0;
    vsync_count++;

    // Мигаем LED или переключаем пин для осциллографа
    //HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_3); // Если есть свободный пин
}



/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* Configure the peripherals common clocks */
  PeriphCommonClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_DCMI_Init();
  MX_RTC_Init();
  MX_I2C1_Init();
  MX_SPI4_Init();
  MX_SDMMC1_SD_Init();
  /* USER CODE BEGIN 2 */

  // 2. Инициализация дисплея
      ST7735_BL_ON();
      ST7735_Init();
      ST7735_FillScreen(ST7735_BLACK);
      ST7735_DisplayString(10, 10, "Init display...", ST7735_WHITE, ST7735_BLACK);


      // 4. Инициализация камеры
      ST7735_DisplayString(10, 30, "Init camera...", ST7735_WHITE, ST7735_BLACK);
      if (OV5640_Init() != 0) {
          ST7735_DisplayString(10, 50, "Camera FAIL!", ST7735_WHITE, ST7735_BLACK);
          Error_Handler();
      }
      // 5. Запуск захвата
      ST7735_DisplayString(10, 70, "Start capture...", ST7735_WHITE, ST7735_BLACK);



      //Start_Camera_Capture();
      OV5640_StartCapture();



  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

	    if (g_frame_capture_complete) {
	        g_frame_capture_complete = 0;

	        // Обновить дисплей
	       // ST7735_DrawImage(10, 10, 50, 50, g_camera_frame);

	        // Небольшая задержка для визуального восприятия
	        HAL_Delay(50);

	        // Перезапустить захват, если он остановился
	       // OV5640_StartCapture();
	    }


  }








  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 5;
  RCC_OscInitStruct.PLL.PLLN = 192;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 8;
  RCC_OscInitStruct.PLL.PLLR = 4;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
  HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_HSE, RCC_MCODIV_1);
}

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Initializes the peripherals clock
  */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_TIM;
  PeriphClkInitStruct.TIMPresSelection = RCC_TIMPRES_ACTIVATED;

  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

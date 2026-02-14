/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : DHT11 Sensor to USB CDC
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"
#include <stdio.h>
#include <string.h>

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim2;

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
#define DHT11_PORT GPIOA
#define DHT11_PIN GPIO_PIN_9

uint8_t RHI, RHD, TCI, TCD, SUM;
float tCelsius = 0;
float RH = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);

/* USER CODE BEGIN 0 */
void microDelay(uint16_t delay) {
    __HAL_TIM_SET_COUNTER(&htim2, 0);
    while (__HAL_TIM_GET_COUNTER(&htim2) < delay);
}

uint8_t DHT11_Start(void) {
    uint8_t Response = 0;
    GPIO_InitTypeDef GPIO_InitStructPrivate = {0};

    // Set Pin as Output
    GPIO_InitStructPrivate.Pin = DHT11_PIN;
    GPIO_InitStructPrivate.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructPrivate.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStructPrivate.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(DHT11_PORT, &GPIO_InitStructPrivate);

    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, 0);
    HAL_Delay(18);
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, 1);
    microDelay(30);

    // Set Pin as Input
    GPIO_InitStructPrivate.Mode = GPIO_MODE_INPUT;
    GPIO_InitStructPrivate.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(DHT11_PORT, &GPIO_InitStructPrivate);

    microDelay(40);
    if (!(HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN))) {
        microDelay(80);
        if ((HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN))) Response = 1;
    }

    uint32_t timeout = HAL_GetTick();
    while ((HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN)) && (HAL_GetTick() - timeout < 2));

    return Response;
}

uint8_t DHT11_Read(void) {
    uint8_t a, b = 0;
    for (a = 0; a < 8; a++) {
        uint32_t timeout = HAL_GetTick();
        while (!(HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN)) && (HAL_GetTick() - timeout < 2));
        microDelay(40);
        if (!(HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN)))
            b &= ~(1 << (7 - a));
        else
            b |= (1 << (7 - a));
        timeout = HAL_GetTick();
        while ((HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN)) && (HAL_GetTick() - timeout < 2));
    }
    return b;
}
/* USER CODE END 0 */

int main(void) {
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_USB_DEVICE_Init();
  MX_TIM2_Init();

  HAL_TIM_Base_Start(&htim2);
  HAL_Delay(2000);

  while (1) {
    if(DHT11_Start()) {
        RHI = DHT11_Read();
        RHD = DHT11_Read();
        TCI = DHT11_Read();
        TCD = DHT11_Read();
        SUM = DHT11_Read();

        if (RHI + RHD + TCI + TCD == SUM) {
            tCelsius = (float)TCI + (float)TCD / 10.0;
            RH = (float)RHI + (float)RHD / 10.0;

            char buf[64];
            int len = sprintf(buf, "Temp: %.1f C | Hum: %.1f %%\r\n", tCelsius, RH);
            CDC_Transmit_FS((uint8_t*)buf, len);
        }
    }
    HAL_Delay(10000);
  }
}

/* --- THE MISSING FUNCTIONS THAT CAUSED YOUR ERROR --- */

static void MX_TIM2_Init(void) {
  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 71; // Assumes 72MHz Clock
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 65535;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  HAL_TIM_Base_Init(&htim2);

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig);
  HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig);
}

static void MX_GPIO_Init(void) {
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
}

void SystemClock_Config(void) {
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
  HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);
}

void Error_Handler(void) {
  while (1) {}
}

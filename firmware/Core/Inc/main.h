/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32wlxx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define RTC_N_PREDIV_S 10
#define RTC_PREDIV_S ((1<<RTC_N_PREDIV_S)-1)
#define RTC_PREDIV_A ((1<<(15-RTC_N_PREDIV_S))-1)
#define INPUT_SLEEVE_Pin GPIO_PIN_15
#define INPUT_SLEEVE_GPIO_Port GPIOB
#define LED_R_Pin GPIO_PIN_3
#define LED_R_GPIO_Port GPIOB
#define BATTERY_DIVIDER_ON_Pin GPIO_PIN_4
#define BATTERY_DIVIDER_ON_GPIO_Port GPIOB
#define INPUT_TIP_Pin GPIO_PIN_7
#define INPUT_TIP_GPIO_Port GPIOB
#define OUTPUT_SLEEVE_Pin GPIO_PIN_10
#define OUTPUT_SLEEVE_GPIO_Port GPIOA
#define LED_B_Pin GPIO_PIN_0
#define LED_B_GPIO_Port GPIOA
#define OUTPUT_TIP_Pin GPIO_PIN_13
#define OUTPUT_TIP_GPIO_Port GPIOB
#define LED_G_Pin GPIO_PIN_10
#define LED_G_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

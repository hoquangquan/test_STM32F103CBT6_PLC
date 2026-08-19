/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

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
#define O1_Pin GPIO_PIN_5
#define O1_GPIO_Port GPIOA
#define O2_Pin GPIO_PIN_6
#define O2_GPIO_Port GPIOA
#define O3_Pin GPIO_PIN_7
#define O3_GPIO_Port GPIOA
#define RS485_DE_Pin GPIO_PIN_8
#define RS485_DE_GPIO_Port GPIOA
#define O4_Pin GPIO_PIN_0
#define O4_GPIO_Port GPIOB
#define O5_Pin GPIO_PIN_1
#define O5_GPIO_Port GPIOB
#define O6_Pin GPIO_PIN_2
#define O6_GPIO_Port GPIOB
#define O7_Pin GPIO_PIN_10
#define O7_GPIO_Port GPIOB
#define O8_Pin GPIO_PIN_11
#define O8_GPIO_Port GPIOB
#define I8_Pin GPIO_PIN_15
#define I8_GPIO_Port GPIOA
#define I7_Pin GPIO_PIN_3
#define I7_GPIO_Port GPIOB
#define I6_Pin GPIO_PIN_4
#define I6_GPIO_Port GPIOB
#define I5_Pin GPIO_PIN_5
#define I5_GPIO_Port GPIOB
#define I4_Pin GPIO_PIN_6
#define I4_GPIO_Port GPIOB
#define I3_Pin GPIO_PIN_7
#define I3_GPIO_Port GPIOB
#define I2_Pin GPIO_PIN_8
#define I2_GPIO_Port GPIOB
#define I1_Pin GPIO_PIN_9
#define I1_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

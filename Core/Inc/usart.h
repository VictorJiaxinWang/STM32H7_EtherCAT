#ifndef __BSP_DEBUG_USART_H__
#define __BSP_DEBUG_USART_H__


#include "stm32h7xx_hal.h"
#include "main.h"

/* USER CODE BEGIN Includes */
#include <stdio.h>
/* USER CODE END Includes */

extern UART_HandleTypeDef huart1;
void MX_USART1_UART_Init(void);


#endif /* __BSP_DEBUG_USART_H__ */



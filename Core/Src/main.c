#include "stm32h7xx_hal.h"
#include "string.h"
#include "main.h"
#include "eth.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "ethercat.h"
#include "control.h"
#include "ethercatmain.h"


static void SystemClock_Config(void);
static void CPU_CACHE_Enable(void);
static void MPU_Config(void);

int main(void)
{
	MPU_Config();
	CPU_CACHE_Enable();
	HAL_Init();
	SystemClock_Config();
	UniverseVarInit();
	MX_GPIO_Init();
	MX_USART1_UART_Init();
	MX_ETH_Init();
	MX_TIM1_Init();
	MX_TIM2_Init();
	MX_TIM3_Init();
	PHY_Init();	
	__enable_irq();
	HAL_TIM_Base_Start_IT(&htim1);
	HAL_Delay(2000);
	
	int ret = -1;

	ret = EcatInit();
	if(ret < 0)
	{
		printf("ECAT Init Failed! ret = %d\r\n", ret);
		while(1);
	}
	else
	{
		printf("ECAT Init Successfully!\r\n");
	}
	
	ret = AxleStartAndServoOn();
	if(ret < 0)
	{
		printf("Servo On Failed! ret = %d\r\n", ret);
		while(1);
	}
	else
	{
		printf("Servo On Successfully!\r\n");
		printf("Axis is running\r\n");
	}	
	
	while (1)
	{
		delatPos = 10;//这里修改电机使能后每个周期运行的脉冲数	
#if ISOBSERVETXPDO == 1
		//打印伺服信息
		for(int index = 0;index < MOTORNUM;index++)
		{
			printf("the %d servo current position is %d\r\n", index + 1, Input_Servo[index]->returnPostion);
	
		}
		printf("\r\n");
		printf("------------------------------------------------\r\n");
		HAL_Delay(1000);
#endif	

	}		

	      
}


//定时器1回调函数
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM1)
	{
		EcatCycleTask();
	}
}

static void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct;
  
  /* Disable the MPU */
  HAL_MPU_Disable();

  /* Configure the MPU attributes as Device not cacheable 
     for ETH DMA descriptors */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.BaseAddress = 0x30040000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_256B;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  
  /* Configure the MPU attributes as Cacheable write through 
     for LwIP RAM heap which contains the Tx buffers */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.BaseAddress = 0x30044000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_16KB;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER1;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /* Enable the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}


static void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct={0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct={0};

  MODIFY_REG(PWR->CR3, PWR_CR3_SCUEN, 0);
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  while ((PWR->D3CR & (PWR_D3CR_VOSRDY)) != PWR_D3CR_VOSRDY)
  {
    
  }
  __HAL_RCC_PLL_PLLSOURCE_CONFIG(RCC_PLLSOURCE_HSE);
  

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 5;  
  RCC_OscInitStruct.PLL.PLLN = 160;
  RCC_OscInitStruct.PLL.PLLP = 2;  
  RCC_OscInitStruct.PLL.PLLQ = 2;  
  RCC_OscInitStruct.PLL.PLLR = 2;  
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;  
  HAL_RCC_OscConfig(&RCC_OscInitStruct);
  

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider  = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider  = RCC_HCLK_DIV2;  
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2; 
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2; 
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2; 
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2; 
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);
  
  

  HAL_RCC_EnableCSS();
  HAL_SYSTICK_Config(SystemCoreClock/1000);
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
  HAL_NVIC_SetPriority(SysTick_IRQn, 1, 1);
}


static void CPU_CACHE_Enable(void)
{

  SCB_EnableICache();
  SCB_EnableDCache();

}

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



#ifndef __eth_H
#define __eth_H
#ifdef __cplusplus
 extern "C" {
#endif


#include "stm32h7xx_hal.h"
#include "main.h"
#include "LAN8720A.h"

#ifndef ETH_TX_DESC_CNT
 #define ETH_TX_DESC_CNT         4 
#endif
	 
#ifndef ETH_RX_DESC_CNT
 #define ETH_RX_DESC_CNT         4 
#endif
/* USER CODE END Includes */

extern ETH_HandleTypeDef heth;

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void MX_ETH_Init(void);
void PHY_Init(void);
int bfin_EMAC_send (void *packet, int length);
int bfin_EMAC_recv (uint8_t * packet, size_t size);


#ifdef __cplusplus
}
#endif
#endif /*__ eth_H */



#include "eth.h"
#include "string.h"
#include "usart.h"

#if defined ( __ICCARM__ ) /*!< IAR Compiler */

#pragma location=0x30040000
ETH_DMADescTypeDef  DMARxDscrTab[ETH_RX_DESC_CNT]; /* Ethernet Rx DMA Descriptors */
#pragma location=0x30040060
ETH_DMADescTypeDef  DMATxDscrTab[ETH_TX_DESC_CNT]; /* Ethernet Tx DMA Descriptors */
#pragma location=0x30040200
uint8_t Rx_Buff[ETH_RX_DESC_CNT][ETH_MAX_PACKET_SIZE]; /* Ethernet Receive Buffers */
#pragma location=0x30044000
uint8_t Tx_Buff[ETH_TX_DESC_CNT][ETH_MAX_PACKET_SIZE]; /* Ethernet Receive Buffers */

#elif defined ( __CC_ARM )  /* MDK ARM Compiler */

__attribute__((at(0x30040000))) ETH_DMADescTypeDef  DMARxDscrTab[ETH_RX_DESC_CNT]; /* Ethernet Rx DMA Descriptors */
__attribute__((at(0x30040060))) ETH_DMADescTypeDef  DMATxDscrTab[ETH_TX_DESC_CNT]; /* Ethernet Tx DMA Descriptors */
__attribute__((at(0x30040200))) uint8_t Rx_Buff[ETH_RX_DESC_CNT][ETH_MAX_PACKET_SIZE]; /* Ethernet Receive Buffer */
__attribute__((at(0x30044000))) uint8_t Tx_Buff[ETH_RX_DESC_CNT][ETH_MAX_PACKET_SIZE]; /* Ethernet Tranform Buffer */

#elif defined ( __GNUC__ ) /* GNU Compiler */

ETH_DMADescTypeDef DMARxDscrTab[ETH_RX_DESC_CNT] __attribute__((section(".RxDecripSection"))); /* Ethernet Rx DMA Descriptors */
ETH_DMADescTypeDef DMATxDscrTab[ETH_TX_DESC_CNT] __attribute__((section(".TxDecripSection")));   /* Ethernet Tx DMA Descriptors */
uint8_t Rx_Buff[ETH_RX_DESC_CNT][ETH_MAX_PACKET_SIZE] __attribute__((section(".RxArraySection"))); /* Ethernet Receive Buffers */

#endif

ETH_TxPacketConfig TxConfig;
ETH_HandleTypeDef heth;

void MX_ETH_Init(void)
{
	uint8_t MACAddr[6] ;
	heth.Instance = ETH;
	MACAddr[0] = 0x01;
	MACAddr[1] = 0x00;
	MACAddr[2] = 0x45;
	MACAddr[3] = 0x19;
	MACAddr[4] = 0x03;
	MACAddr[5] = 0x00;
	heth.Init.MACAddr = &MACAddr[0];
	heth.Init.MediaInterface = HAL_ETH_RMII_MODE;
	heth.Init.TxDesc = DMATxDscrTab;
	heth.Init.RxDesc = DMARxDscrTab;
	heth.Init.RxBuffLen = ETH_MAX_PACKET_SIZE;
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
	HAL_Delay(55);
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);
	HAL_Delay(55);
	HAL_ETH_Init(&heth);
 	if (HAL_ETH_Init(&heth) == HAL_OK)
	{
	  printf("Mac Init Successfully\r\n");
	}
	else
	{
	  printf("Mac Init Failed\r\n");
	}
	
}

void HAL_ETH_MspInit(ETH_HandleTypeDef* ethHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct;
  if(ethHandle->Instance==ETH)
  {

    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    __HAL_RCC_ETH1MAC_CLK_ENABLE();
    __HAL_RCC_ETH1TX_CLK_ENABLE();
    __HAL_RCC_ETH1RX_CLK_ENABLE();
  

    /**ETH GPIO Configuration    
    PC1     ------> ETH_MDC
    PA1     ------> ETH_REF_CLK
    PA2     ------> ETH_MDIO
    PA7     ------> ETH_CRS_DV
    PC4     ------> ETH_RXD0
    PC5     ------> ETH_RXD1
    PB11     ------> ETH_TX_EN
    PB12     ------> ETH_TXD0
    PB13     ------> ETH_TXD1 
		
		PA8			------> ETH_RESET
    */
    GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  }
}

void HAL_ETH_MspDeInit(ETH_HandleTypeDef* ethHandle)
{

  if(ethHandle->Instance==ETH)
  {
  /* USER CODE BEGIN ETH_MspDeInit 0 */

  /* USER CODE END ETH_MspDeInit 0 */
    /* Peripheral clock disable */
   // __HAL_RCC_ETH_CLK_DISABLE();

    /**ETH GPIO Configuration
    PC1     ------> ETH_MDC
    PA1     ------> ETH_REF_CLK
    PA2     ------> ETH_MDIO
    PA7     ------> ETH_CRS_DV
    PC4     ------> ETH_RXD0
    PC5     ------> ETH_RXD1
    PB11     ------> ETH_TX_EN
    PB12     ------> ETH_TXD0
    PB13     ------> ETH_TXD1
    */
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_5);

    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_7);

    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13);

  /* USER CODE BEGIN ETH_MspDeInit 1 */

  /* USER CODE END ETH_MspDeInit 1 */
  }
}

void PHY_Init(void)
{
    uint32_t idx, duplex, speed = 0;
    int32_t PHYLinkState;
    ETH_MACConfigTypeDef MACConf;
    
    ETH_MACFilterConfigTypeDef filterDef;
    
    HAL_ETH_GetMACFilterConfig(&heth,&filterDef);
    filterDef.PromiscuousMode = ENABLE;
    HAL_ETH_SetMACFilterConfig(&heth,&filterDef);
    
    
    for(idx = 0; idx < ETH_RX_DESC_CNT; idx ++)
    {
        HAL_ETH_DescAssignMemory(&heth, idx, Rx_Buff[idx], NULL);
    }
    
    LAN8720Init();
	do
	{
		HAL_Delay(100);
		PHYLinkState = GetSpeedDuplex();
		printf("Connect Device ......\r\n");
			
	}while(PHYLinkState <= LAN8720_STATUS_LINK_DOWN);
	
	printf("Link Status is %d\r\n", PHYLinkState);
    
    
    switch (PHYLinkState)
    {
    case LAN8720_STATUS_100MBITS_FULLDUPLEX:
        duplex = ETH_FULLDUPLEX_MODE;
        speed = ETH_SPEED_100M;
        break;
    case LAN8720_STATUS_100MBITS_HALFDUPLEX:
        duplex = ETH_HALFDUPLEX_MODE;
        speed = ETH_SPEED_100M;
        break;
    case LAN8720_STATUS_10MBITS_FULLDUPLEX:
        duplex = ETH_FULLDUPLEX_MODE;
        speed = ETH_SPEED_10M;
        break;
    case LAN8720_STATUS_10MBITS_HALFDUPLEX:
        duplex = ETH_HALFDUPLEX_MODE;
        speed = ETH_SPEED_10M;
        break;
    default:
        duplex = ETH_FULLDUPLEX_MODE;
        speed = ETH_SPEED_100M;
        break;      
    }
    
    HAL_ETH_GetMACConfig(&heth, &MACConf); 
    MACConf.DuplexMode = duplex;
    MACConf.Speed = speed;
    HAL_ETH_SetMACConfig(&heth, &MACConf);
    HAL_ETH_Start_IT(&heth);
    HAL_ETH_BuildRxDescriptors(&heth);
}


uint32_t sendfinishflag=0;
uint8_t RecvLength=0;
uint32_t current_pbuf_idx =0;

int bfin_EMAC_send (void *packet, int length)
{
	uint32_t framelen = 0;
	memcpy(&Tx_Buff[0][0],packet,length);
	ETH_BufferTypeDef Txbuffer[ETH_TX_DESC_CNT];
	memset(Txbuffer, 0 , ETH_TX_DESC_CNT*sizeof(ETH_BufferTypeDef));
	Txbuffer[0].buffer = Tx_Buff[0];
	Txbuffer[0].len = length;
	framelen += length;		
	TxConfig.Length = framelen;
	TxConfig.TxBuffer = Txbuffer;
	SCB_CleanInvalidateDCache();
	HAL_ETH_Transmit_IT(&heth, &TxConfig);
	sendfinishflag = 1;
	return 0;
}


int bfin_EMAC_recv (uint8_t * packet, size_t size)
{
	ETH_BufferTypeDef RxBuff;
	uint32_t framelength = 0;
	SCB_CleanInvalidateDCache();
	
	HAL_StatusTypeDef status = HAL_ETH_GetRxDataBuffer(&heth, &RxBuff);
	
	if( status == HAL_OK) 
	{
		HAL_ETH_GetRxDataLength(&heth, &framelength);
		SCB_InvalidateDCache_by_Addr((uint32_t *)Rx_Buff, (ETH_RX_DESC_CNT*ETH_MAX_PACKET_SIZE));	
		memcpy(packet, Rx_Buff[current_pbuf_idx], framelength);
		if(current_pbuf_idx < (ETH_RX_DESC_CNT -1))
		{
			current_pbuf_idx++;
		}
		else
		{
			current_pbuf_idx = 0;
		}
		HAL_ETH_BuildRxDescriptors(&heth);
		return framelength;
	}

	return -1;
}

void HAL_ETH_RxCpltCallback(ETH_HandleTypeDef *heth)
{

}

void HAL_ETH_TxCpltCallback(ETH_HandleTypeDef *heth)
{
	sendfinishflag = 0;
}

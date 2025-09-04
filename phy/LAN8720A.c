#include "LAN8720A.h"
#include "usart.h"
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_eth.h"

uint32_t phyAddress;
extern ETH_HandleTypeDef heth;

int LAN8720Init(void)
{
    uint32_t phyregvalue = 0, i = 0;
    HAL_StatusTypeDef ret = HAL_ERROR;
    for(i = 0; i <= 32; i ++)
    {
        ret = HAL_ETH_ReadPHYRegister(&heth, i, LAN8720_SMR, &phyregvalue);      
        if( ret != HAL_OK)
        {
			printf("ReadReg error\r\n");
            continue;
        }
        
        if((phyregvalue & LAN8720_SMR_PHY_ADDR) == i)
        {
            phyAddress = i;
            break;
        }
    } 
    if(phyAddress > 31 )
	{
		printf("Match PHY Address Failed!\r\n");
		return -1;
	}
    ret = HAL_ETH_WritePHYRegister(&heth,phyAddress,LAN8720_BCR,LAN8720_BCR_SOFT_RESET);
    if( ret != HAL_OK )
    {
		printf("ReadReg error\r\n");
        return -1;
    }
    do
    {
        ret = HAL_ETH_ReadPHYRegister(&heth, phyAddress, LAN8720_BCR, &phyregvalue);
        if( ret != HAL_OK )
        {
			printf("ReadReg error\r\n");
            return -1;
        }
    }while( phyregvalue & LAN8720_BCR_SOFT_RESET );
	
    uint32_t regvalue = 0;
	ret = HAL_ETH_ReadPHYRegister(&heth, phyAddress, LAN8720_BCR, &regvalue);
    if(ret != HAL_OK)
	{
		printf("ReadReg error\r\n");
	}
    regvalue |= LAN8720_BCR_AUTONEGO_EN;
	ret = HAL_ETH_WritePHYRegister(&heth, phyAddress, LAN8720_BCR, regvalue);
	if(ret != HAL_OK)
	{
		printf("Start AutoNego error\r\n");
	}      
    return 0;
}


int GetSpeedDuplex(void)
{
    uint32_t phyregvalue = 0;
    int32_t ret;
    ret = HAL_ETH_ReadPHYRegister(&heth, phyAddress, LAN8720_BSR, &phyregvalue);
    if( ret != HAL_OK )
    {
        return LAN8720_STATUS_READ_ERROR;
    }
    ret = HAL_ETH_ReadPHYRegister(&heth, phyAddress, LAN8720_BSR, &phyregvalue);
    if( ret != HAL_OK )
    {
        return LAN8720_STATUS_READ_ERROR;
    }
    if((phyregvalue & LAN8720_BSR_LINK_STATUS) == 0)
    {
        return LAN8720_STATUS_LINK_DOWN;    
    }
    ret = HAL_ETH_ReadPHYRegister(&heth, phyAddress, LAN8720_BCR, &phyregvalue);
    if( ret != HAL_OK )
    {
        return LAN8720_STATUS_READ_ERROR;
    }
    
    if((phyregvalue & LAN8720_BCR_AUTONEGO_EN) != LAN8720_BCR_AUTONEGO_EN)
    {
        if(((phyregvalue & LAN8720_BCR_SPEED_SELECT) == LAN8720_BCR_SPEED_SELECT) && ((phyregvalue & LAN8720_BCR_DUPLEX_MODE) == LAN8720_BCR_DUPLEX_MODE)) 
        {
            return LAN8720_STATUS_100MBITS_FULLDUPLEX;
        }
        else if ((phyregvalue & LAN8720_BCR_SPEED_SELECT) == LAN8720_BCR_SPEED_SELECT)
        {
            return LAN8720_STATUS_100MBITS_HALFDUPLEX;
        }        
        else if ((phyregvalue & LAN8720_BCR_DUPLEX_MODE) == LAN8720_BCR_DUPLEX_MODE)
        {
            return LAN8720_STATUS_10MBITS_FULLDUPLEX;
        }
        else
        {
            return LAN8720_STATUS_10MBITS_HALFDUPLEX;
        }  		
    }
    else /* Auto Nego enabled */
    {
        ret = HAL_ETH_ReadPHYRegister(&heth, phyAddress, LAN8720_PHYSCSR, &phyregvalue);
        if( ret != HAL_OK )
        {
            return LAN8720_STATUS_READ_ERROR;
        }
        if((phyregvalue & LAN8720_PHYSCSR_AUTONEGO_DONE) == 0)
        {
            return LAN8720_STATUS_AUTONEGO_NOTDONE;
        }
        if((phyregvalue & LAN8720_PHYSCSR_HCDSPEEDMASK) == LAN8720_PHYSCSR_100BTX_FD)
        {
            return LAN8720_STATUS_100MBITS_FULLDUPLEX;
        }
        else if ((phyregvalue & LAN8720_PHYSCSR_HCDSPEEDMASK) == LAN8720_PHYSCSR_100BTX_HD)
        {
            return LAN8720_STATUS_100MBITS_HALFDUPLEX;
        }
        else if ((phyregvalue & LAN8720_PHYSCSR_HCDSPEEDMASK) == LAN8720_PHYSCSR_10BT_FD)
        {
            return LAN8720_STATUS_10MBITS_FULLDUPLEX;
        }
        else
        {
            return LAN8720_STATUS_10MBITS_HALFDUPLEX;
        }				
    }
}




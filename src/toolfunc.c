#include "control.h"
#include <stdio.h>

int wSdo8(uint16 slaveIndex, uint16 sdo_index, uint8 sdo_subindex, uint8 sdo_value)
{
	int ret = ec_SDOwrite(slaveIndex, sdo_index, sdo_subindex, FALSE, sizeof(uint8), &sdo_value, EC_TIMEOUTRXM);
	if(!ret) 
	{ 
		printf("slaveIndex %d write SDO8 error, 0x%x\r\n", slaveIndex, sdo_index); 
	}
	else 
	{ 
		printf("slaveIndex %d sdo_index 0x%x sdo_value 0x%x write SDO8 complete!\r\n", slaveIndex, sdo_index, sdo_value); 
	}
	return ret;
}


int wSdo16(uint16 slaveIndex, uint16 sdo_index, uint8 sdo_subindex, uint16 sdo_value)
{
	int ret = ec_SDOwrite(slaveIndex, sdo_index, sdo_subindex, FALSE, sizeof(uint16), &sdo_value, EC_TIMEOUTRXM * 20);
	if(!ret) 
	{ 
		printf("slaveIndex %d write SDO16 error, 0x%x\r\n", slaveIndex, sdo_index); 
	}
	else 
	{ 
		printf("slaveIndex %d sdo_index 0x%x sdo_value 0x%x write SDO16 complete!\r\n", slaveIndex, sdo_index, sdo_value); 
	}
	return ret;
}


int wSdo32(uint16 slaveIndex, uint16 sdo_index, uint8 sdo_subindex, uint32 sdo_value)
{

	int ret = ec_SDOwrite(slaveIndex, sdo_index, sdo_subindex, FALSE, sizeof(uint32), &sdo_value, EC_TIMEOUTRXM * 20);
	if(!ret) 
	{ 
		printf("slaveIndex %d write SDO32 error, 0x%x\r\n", slaveIndex, sdo_index); 
	}
	else 
	{ 
		printf("slaveIndex %d sdo_index 0x%x sdo_value 0x%x write SDO32 complete!\r\n", slaveIndex, sdo_index, sdo_value); 
	}
	return ret;
}







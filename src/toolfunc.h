#ifndef _TOOLFUNC_H_
#define _TOOLFUNC_H_

#include "ethercat.h"
#include "osal.h"

int wSdo8(uint16 slaveIndex, uint16 sdo_index, uint8 sdo_subindex, uint8 sdo_value);
int wSdo16(uint16 slaveIndex, uint16 sdo_index, uint8 sdo_subindex, uint16 sdo_value);
int wSdo32(uint16 slaveIndex, uint16 sdo_index, uint8 sdo_subindex, uint32 sdo_value);

#endif



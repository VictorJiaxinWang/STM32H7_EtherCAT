#include "control.h"
#include "stdio.h"
#include "string.h"
#include "stm32h7xx_hal.h"
#include "tim.h"
#include <math.h>
#include <stdlib.h>
#include "toolfunc.h"

//参与控制的伺服与IO的PDO指针
PDO_Outputs* Output_Servo[TOTALNUMLENGTH];
PDO_Input* Input_Servo[TOTALNUMLENGTH];
IO_PDO_Output* Output_IO[TOTALNUMLENGTH];
IO_PDO_Input* Input_IO[TOTALNUMLENGTH];

//ECAT初始化控制流程状态标志
int MOTORNUM = 0;   //电机数量
char IOmap[4096];
boolean PDOExchangeFlag;  //进入OP之后将此标志置为真 定时器中断处理函数中将进行PDO数据交换
boolean ServoOnCompleteFlag;
int expectedWKC;


//运动控制实验变量
int32_t basePos[TOTALNUMLENGTH];  //基位置
int32_t vel[TOTALNUMLENGTH];      //步进和
int32_t delatPos;                 //每周期步进值
int32_t targetSpeed = 0;
uint32_t value = 0xAA55;

//各边变量参数初始化
void UniverseVarInit(void)
{
	for(int i  = 0; i < TOTALNUMLENGTH; i++)
	{
		basePos[i] = 0;
		vel[i] = 0;	
	}
	PDOExchangeFlag = FALSE;
	ServoOnCompleteFlag = FALSE;
	delatPos = 0;
	return;
}

//根据状态字获取CIA402状态机
static int GetAxleState(uint16_t status_word) 
{
	if ((status_word & 0x4F) == 0x40)
		return switch_on_disable;
	if ((status_word & 0x6F) == 0x21)
		return ready_to_switch_on;
	if ((status_word & 0x6F) == 0x23)
		return switched_on;
	if ((status_word & 0x6F) == 0x27)
		return operation_enable;
	if ((status_word & 0x6F) == 0x07)
		return quick_stop_active;
	if ((status_word & 0x4F) == 0xF)
		return fault_reaction_active;
	if ((status_word & 0x4F) == 0x08)
		return fault;
	else
		return no_ready_to_switch_on;
}

//CIA402状态机切换
int CIA402AxleStart(int slave_index) 
{
	int ret = 0;
	int ss;
	// basePos[slave_index] = Input_Servo[slave_index]->returnPostion;
	// Output_Servo[slave_index]->targetPostion = basePos[slave_index];
	uint16 s = Input_Servo[slave_index]->statusWord;
	ss = GetAxleState(s);
	if (s & 0x8) 
	{
		Output_Servo[slave_index]->controlword = 0x80;
		printf("%d Slave ControlWord 0x%04x StatusWord 0x%04x\r\n",slave_index, Output_Servo[slave_index]->controlword, s);
		return 0;
	}
	switch(ss) 
	{
		case (no_ready_to_switch_on):
		case (switch_on_disable):
			Output_Servo[slave_index]->controlword = contrlword_shutdown(0);
			break;
		case (ready_to_switch_on):
			Output_Servo[slave_index]->controlword = contrlword_switch_on(0);
			break;
		case (switched_on):
			Output_Servo[slave_index]->controlword = contrlword_enable_operation(0);
			break;
		case (operation_enable):
			ret = 1;
			break;
		case (quick_stop_active):
		case (fault_reaction_active):
		case (fault):
		default:
			ret = -1;
	}
	printf("%d Slave ControlWord 0x%04x StatusWord 0x%04x\r\n",slave_index, Output_Servo[slave_index]->controlword, s);
	return ret;
}

//使能轴
int AxleStartAndServoOn(void)
{
	int ServoIndex = 0;
	int AllServoEnableFlag = 1;
	int ret = -1;
	int Cia402ServoOnRet = -1;
	int ServoOnTimes = 0;
	while(ServoOnTimes < 1000)
	{
		AllServoEnableFlag = 1;
		for(ServoIndex = 0; ServoIndex < MOTORNUM; ServoIndex++)
		{
			Cia402ServoOnRet = CIA402AxleStart(ServoIndex);
			if(Cia402ServoOnRet != 1)
			{
				AllServoEnableFlag = -1;
				ServoOnCompleteFlag = FALSE;
			}
		}
		if(AllServoEnableFlag == 1)
		{
			ServoOnCompleteFlag = TRUE;
			HAL_GPIO_WritePin(GPIOE, GPIO_PIN_1, GPIO_PIN_RESET);//SERVO ON成功后LED2常亮
			printf("All AXIS serve on success! Count = %d\r\n", ServoOnTimes);
			break;
		}
		HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_1);//LED2闪烁表示正在SERVO ON
		ServoOnTimes++;
	    HAL_Delay(50);
	}
	if(ServoOnTimes >= 1000)
	{
		ret = -1;
	}
	else
	{
		ret = 1;
	}
	return ret;
}

//检查各从站之间同步性
int IsSlavesSynced(int SlaveCount)
{
	int ret = 1;
	int i;
	int wkc;
	int32_t diff;
	for (i = 0; i < SlaveCount; i++) 
	{
		wkc=ecx_FPRD(ecx_context.port, ec_slave[i + 1].configadr, ECT_REG_DCSYSDIFF, sizeof(diff), &diff, EC_TIMEOUTRET);	
		if (wkc == 1 && abs(diff) < 100000) 
		{
			ret = 1;
		} 
		else 
		{
			ret = -(i + 1);
			break;
		}	
	}
	return ret;
}

// //驱动器CSP配置
// int ServoUniversalConfig(uint16 slave_index)
// {
// 	int rxpdo_mapIndex = 1;
// 	int txpdo_mapIndex = 1;
// 	rxpdo_mapIndex = 1;
// 	txpdo_mapIndex = 1;
// //********************0x1C12配置*****************************************
// 	// 清空RxPDO映射
// 	wSdo8(slave_index, 0x1C12, 00, 0);	 		  
// 	wSdo8(slave_index, 0x1600, 00, 0);	 		  

// 	// 配置RxPDO映射
// 	wSdo32(slave_index, 0x1600, rxpdo_mapIndex++, 0x60400010);	// 控制字	
// 	wSdo32(slave_index, 0x1600, rxpdo_mapIndex++, 0x607A0020);	// 目标位置	
// 	wSdo8(slave_index, 0x1600, 00, rxpdo_mapIndex - 1);			// 映射数量
// 	wSdo16(slave_index, 0x1C12, 01, 0x1600);					// 映射表
// 	wSdo8(slave_index, 0x1C12, 00, 1);       					// 映射数量
	
// //********************0x1C13配置*****************************************
// 	// 清空TxPDO映射		
// 	wSdo8(slave_index, 0x1C13, 00, 00);				  			
// 	wSdo8(slave_index, 0x1A00, 00, 00);		
	
// 	// 配置TxPDO映射
// 	wSdo32(slave_index, 0x1A00, txpdo_mapIndex++, 0x60410010);	// 状态字
// 	wSdo32(slave_index, 0x1A00, txpdo_mapIndex++, 0x60640020);	// 实际位置	
// 	wSdo8(slave_index, 0x1A00, 00, txpdo_mapIndex - 1);			// 映射数量	  
// 	wSdo16(slave_index, 0x1C13, 01, 0x1A00);					// 映射表
// 	wSdo8(slave_index, 0x1C13, 00, 1);                			// 映射数量

// 	// 设置工作模式为CSP
// 	wSdo8(slave_index, 0x6060, 00, op_mode_csp);   
	
// 	return 0;
// }

// 驱动器PV配置
int ServoUniversalConfig(uint16 slave_index)
{
	int rxpdo_mapIndex = 1;
	int txpdo_mapIndex = 1;

//********************0x1C12配置*****************************************
	// 清空RxPDO映射
	wSdo8(slave_index, 0x1C12, 00, 0);	 		  
	wSdo8(slave_index, 0x1600, 00, 0);	 		  

	// 配置RxPDO映射
	wSdo32(slave_index, 0x1600, rxpdo_mapIndex++, 0x60400010);	// 控制字	
	wSdo32(slave_index, 0x1600, rxpdo_mapIndex++, 0x607A0020);	// 目标位置	
	wSdo32(slave_index, 0x1600, rxpdo_mapIndex++, 0x60FF0020);	// 轮廓速度
	wSdo32(slave_index, 0x1600, rxpdo_mapIndex++, 0x60830020);	// 加速度
	wSdo32(slave_index, 0x1600, rxpdo_mapIndex++, 0x60840020);	// 减速度
	wSdo8(slave_index, 0x1600, 00, rxpdo_mapIndex - 1);			// 映射数量
	wSdo16(slave_index, 0x1C12, 01, 0x1600);					// 映射表
	wSdo8(slave_index, 0x1C12, 00, 1);       					// 映射数量
	
//********************0x1C13配置*****************************************
	// 清空TxPDO映射		
	wSdo8(slave_index, 0x1C13, 00, 00);				  			
	wSdo8(slave_index, 0x1A00, 00, 00);		
	
	// 配置TxPDO映射
	wSdo32(slave_index, 0x1A00, txpdo_mapIndex++, 0x60410010);	// 状态字
	wSdo32(slave_index, 0x1A00, txpdo_mapIndex++, 0x60640020);	// 实际位置	
	wSdo32(slave_index, 0x1A00, txpdo_mapIndex++, 0x606C0020);	// 实际速度	
	wSdo8(slave_index, 0x1A00, 00, txpdo_mapIndex - 1);			// 映射数量	  
	wSdo16(slave_index, 0x1C13, 01, 0x1A00);					// 映射表
	wSdo8(slave_index, 0x1C13, 00, 1);                			// 映射数量

	// 设置工作模式为CSP
	wSdo8(slave_index, 0x6060, 00, op_mode_pv); 
	
	return 1;
}

int EcatInit(void)
{
	int SlaveIndex = 0;  //从站索引	
	int ServoIndex = 0;  //驱动器索引
	//int IOIndex = 0;     //IO索引
	int CheckTimeOutCount = 0;  //检查/设置超时时间计数
	int sync_ret;
	int ALL_OP_FLAG = 0;
	if(ec_init("ServoDrive"))
	{
		printf("ec_init succeeded.\r\n");
     
		if (ec_config_init(TRUE) > 0 )
		{
			printf("%d slaves found\r\n", ec_slavecount);
			MOTORNUM = ec_slavecount;
			if (ec_slavecount == MOTORNUM) 
			{
				for(SlaveIndex = 1; SlaveIndex <= MOTORNUM; SlaveIndex++)
				 {									 
					printf("Found %s at position %d\r\n", ec_slave[SlaveIndex].name, SlaveIndex);
					printf("Found %x at position %d\r\n", ec_slave[SlaveIndex].configadr, SlaveIndex);
					ec_slave[SlaveIndex].PO2SOconfig = &ServoUniversalConfig; 
				 }
			}
			else
			{
				printf("Slave num error!\r\n");
				return -1;
			}  
			ec_configdc();
			
			CheckTimeOutCount = 100;
			while(CheckTimeOutCount--)
			{
				sync_ret = IsSlavesSynced(MOTORNUM);
				if(sync_ret == 1)
				{
					break;
				}
				else
				{
					printf("%d slave not sync\r\n", sync_ret);
				}
			}
			if((CheckTimeOutCount == 0) && (sync_ret !=1))
			{
				printf("Slave sync failed\r\n");
			}
			
			
			for(SlaveIndex = 1; SlaveIndex <= MOTORNUM ;SlaveIndex++)
			{
				if(ec_slave[SlaveIndex].hasdc > 0)
				{
					ec_dcsync0(SlaveIndex, TRUE, SYNC0TIME * 1000, SYNC0TIME * 300); // SYNC0 on slave 1
					printf("Slave %d has DC\r\n", SlaveIndex);
				}
				else
				{
					printf("Slave %d has not DC\r\n", SlaveIndex);
				}
				
			}
			
			ec_config_map(&IOmap); 
			printf("segments : %d : %d %d %d %d\r\n",ec_group[0].nsegments ,ec_group[0].IOsegment[0],ec_group[0].IOsegment[1],ec_group[0].IOsegment[2],ec_group[0].IOsegment[3]);
			ec_statecheck(0, EC_STATE_SAFE_OP,  EC_TIMEOUTSTATE);
			ec_readstate();
			for(SlaveIndex = 0; SlaveIndex <= MOTORNUM; SlaveIndex++)
			{
				printf("Slave %d State=0x%04x\r\n", SlaveIndex, ec_slave[SlaveIndex].state);
			}

		
			printf("Request operational state for all slaves\r\n");
			expectedWKC = (ec_group[0].outputsWKC * 2) + ec_group[0].inputsWKC;
			printf("Calculated workcounter %d\r\n", expectedWKC); 
		
			ec_send_processdata();
			ec_receive_processdata(EC_TIMEOUTRET);
			ec_writestate(0);
			HAL_Delay(200);
			CheckTimeOutCount = 1000;
			do
			{
				for(SlaveIndex = 0; SlaveIndex <= MOTORNUM; SlaveIndex++)
				{
					ec_slave[SlaveIndex].state = EC_STATE_OPERATIONAL;
				}
				for(SlaveIndex = 0; SlaveIndex <= MOTORNUM; SlaveIndex++)
				{
					ec_writestate(SlaveIndex);
				}
				for(SlaveIndex = 0; SlaveIndex <= MOTORNUM; SlaveIndex++)
				{
					printf("Slave %d State=0x%04x\r\n", SlaveIndex, ec_slave[SlaveIndex].state);
				}
				ALL_OP_FLAG = 1;
				for(SlaveIndex = 0; SlaveIndex <= MOTORNUM; SlaveIndex++)
				{
					if(ec_slave[SlaveIndex].state != EC_STATE_OPERATIONAL)//遍历所有设备 有一个没进OP就不能离开
					{
						ALL_OP_FLAG = 0;
					}
				}
				if(ALL_OP_FLAG)
				{
					break;
				}
			} 
			while(CheckTimeOutCount--);

			if (ec_slave[0].state == EC_STATE_OPERATIONAL)
			{	

				for(ServoIndex = 1; ServoIndex <= MOTORNUM; ServoIndex++)
				{
					Output_Servo[ServoIndex - 1] = (PDO_Outputs*)ec_slave[ServoIndex].outputs;
					Input_Servo[ServoIndex - 1]  = (PDO_Input*)ec_slave[ServoIndex].inputs;
				}	
#if(IONUM > 0)
				for(IOIndex = 1; IOIndex <= IONUM; IOIndex++)
				{
					Output_IO[IOIndex - 1] = (IO_PDO_Output*)ec_slave[IOIndex + MOTORNUM].outputs;
					Input_IO[IOIndex- 1]  = (IO_PDO_Input*)ec_slave[IOIndex + MOTORNUM].inputs;
				}
#endif				
				PDOExchangeFlag = TRUE;
				HAL_GPIO_WritePin(GPIOE, GPIO_PIN_0, GPIO_PIN_RESET);//进入OP后LED3亮
				printf("All slaves enter OP!\r\n"); 
				return 1;
			}			
			else
			{
				printf("OP Error!\r\n");
				return -2;
			}
		}
		else
		{
			printf("No slaves found!\r\n");
			return -3;
		}
	}
	else
	{
		printf("ec_init failed\r\n");
		return -4;
	}
    
}


//EtherCAT周期任务
void EcatCycleTask(void)
{
	if(PDOExchangeFlag)
	{
		ec_send_processdata();
		ec_receive_processdata(EC_TIMEOUTRET);  	
		if(ServoOnCompleteFlag)
		{
			for(int i = 0; i < MOTORNUM; i++)
			{
				Output_Servo[i] -> targetSpeed = targetSpeed;
				Output_Servo[i] -> acc = 100000;
				Output_Servo[i] -> dec = 100000;
				Output_Servo[i] -> controlword = 0x0F; 
				// printf("%d Slave ControlWord 0x%04x StatusWord 0x%04x Speed %d\r\n",i, Output_Servo[i]->controlword, Input_Servo[i]->statusWord, Input_Servo[i]->returnSpeed);
			}
			
			// for(int i = 0; i < MOTORNUM; i++)
			// {
			// 	vel[i] += delatPos;
			// }
			// for(int j = 0; j < MOTORNUM; j++)
			// {
			// 	Output_Servo[j]->controlword = 0x0F;
			// // 	Output_Servo[j]->targetPostion = basePos[j] + vel[j];		
			// }
#if(IONUM > 0)
			//value = ~value;
			Output_IO[0]->Output = 0xAA55;	
#endif	
		}

	}

}




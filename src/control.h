#ifndef _ECATUSER_H_
#define _ECATUSER_H_

#include "ethercat.h"
#include "osal.h"


#define IONUM  0            //IO数量
#define SLAVETOTALNUM (MOTORNUM + IONUM)   //总数量
#define BASETIME 1000    //时间基 1ms
#define DIV 1            //时间切分 4----->250us
#define MUL 1            //时间增倍
#define SYNC0TIME BASETIME/DIV   //注意 这里单位是微秒us
#define ISOBSERVETXPDO  1   //是否打印从站TxPDO信息
#define TOTALNUMLENGTH 32

PACKED_BEGIN
typedef struct PACKED
{
	// Common Part
	uint16 controlword; 	// 0x6040
	uint8 modeOfOperation;  // 0x6060

	// CSP Mode and PP Mode
	uint32 targetPosition; //607A

	// PP Mode
	int32 maxSpeedPP; // 0x6081
	int32 acc; // 0x6083

	// PP Mode + PV Mode
	int32 dec; // 0x6084
	// uint32 quickStopDec; // 0x6085

	// PV Mode
	int32 targetSpeed; // 0x60FF

	// Home Mode
	int8 homingMethod; // 0x6098
	int32 homingSpeed; // 0x6099
	int32 homingAccDec;  // 0x609A
	int32 homingOffset; // 0x607C
}PDO_Outputs; 
PACKED_END

PACKED_BEGIN
typedef struct PACKED
{
	// Common Part
	uint16 statusWord;  // 0x6041
	uint8 modeOfOperationDisplay; // 0x6061

	// CSP Mode and PP Mode
	int32 actualPosition;   //6064

	// PV Mode
	int32 actualSpeed;   // 0x606C
}PDO_Input; 
PACKED_END


PACKED_BEGIN
typedef struct PACKED
{
	uint16 Input; //16位数字输入
}IO_PDO_Input;
PACKED_END

PACKED_BEGIN
typedef struct PACKED
{
	uint16 Output;  //16位数字输出
}IO_PDO_Output;
PACKED_END


extern PDO_Outputs* Output_Servo[TOTALNUMLENGTH];
extern PDO_Input* Input_Servo[TOTALNUMLENGTH];
extern IO_PDO_Output* Output_IO[TOTALNUMLENGTH];
extern IO_PDO_Input* Input_IO[TOTALNUMLENGTH];

#define    no_ready_to_switch_on   0
#define    switch_on_disable       1
#define    ready_to_switch_on      2
#define    switched_on             3
#define    operation_enable        4
#define    quick_stop_active       5
#define    fault_reaction_active   6
#define    fault                   7

#define    op_mode_no   0
#define    op_mode_pp   1
#define    op_mode_vl   2
#define    op_mode_pv   3
#define    op_mode_hm   6
#define    op_mode_ip   7
#define    op_mode_csp  8
#define    op_mode_csv  9
#define    op_mode_cst  10

#define contrlword_shutdown(c)				(((c) | 0x6) & ~0x81)
#define contrlword_switch_on(c)		 		(((c) | 0x7) & ~0x88)
#define contrlword_disable_voltage(c)		((c)	& ~0x82)
#define contrlword_quick_stop(c)			(((c) | 0x2) & ~0x84)
#define contrlword_disable_operation(c)	 	(((c) | 0x7) & ~0x88)
#define contrlword_enable_operation(c)		(((c) | 0xF) & ~0x80)
#define contrlword_fault_reset(c)			((c) | 0x80)
#define contrlword_new_set_point(c)	 		((c) | 0x10)
#define contrlword_new_set_point_imm(c)	 	((c) | 0x30)


extern int MOTORNUM;
extern int32_t basePos[TOTALNUMLENGTH];
extern int32_t vel[TOTALNUMLENGTH];
extern int32_t delatPos;
extern int32_t targetSpeed;
extern uint32_t targetPos;
extern uint8_t motorIndex;


void UniverseVarInit(void);
int EcatInit(void);
int AxleStartAndServoOn(void);
void EcatCycleTask(void);


#endif





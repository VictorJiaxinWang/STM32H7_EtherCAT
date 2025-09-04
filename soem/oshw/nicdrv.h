/*
 * Licensed under the GNU General Public License version 2 with exceptions. See
 * LICENSE file in the project root for full license information
 */

/** \file
 * \brief
 * Headerfile for nicdrv.c
 */

#ifndef _nicdrvh_
#define _nicdrvh_


#include "ethercattype.h"

//数据包的构成
//数据包里面包含16个长度为1518的buffer
/** pointer structure to Tx and Rx stacks */
typedef struct
{
   /** socket connection used */
   int         *sock;
   /** tx buffer */
	ec_bufT     (*txbuf)[EC_MAXBUF];    //发送数据包的指针
   /** tx buffer lengths */
   int         (*txbuflength)[EC_MAXBUF];  //数据包中每个buffer的长度
   /** temporary receive buffer */
   ec_bufT     *tempbuf;   //临时接收数据包 用于暂存
   /** rx buffers */
   ec_bufT     (*rxbuf)[EC_MAXBUF];  //接收数据包指针
   /** rx buffer status fields */
   int         (*rxbufstat)[EC_MAXBUF]; //接收数据包每个buffer的状态
   /** received MAC source address (middle word) */
   int         (*rxsa)[EC_MAXBUF];   //接收数据包每个buffer的MAC的源地址
} ec_stackT;   //数据堆栈结构体    这个结构体里面保存的都是发送、接收数据还有长度状态的指针 用于开发给其他函数调用

/** pointer structure to buffers for redundant port */
typedef struct
{
   ec_stackT   stack;
   int         sockhandle;
   /** rx buffers */
   ec_bufT rxbuf[EC_MAXBUF];
   /** rx buffer status */
   int rxbufstat[EC_MAXBUF];
   /** rx MAC source address */
   int rxsa[EC_MAXBUF];
   /** temporary rx buffer */
   ec_bufT tempinbuf;
} ecx_redportt;

/** pointer structure to buffers, vars and mutexes for port instantiation */
typedef struct
{
   ec_stackT   stack;     //数据堆栈
   int         sockhandle; 
   /** rx buffers */
   ec_bufT rxbuf[EC_MAXBUF];  //定义接收数据包
	
   /** rx buffer status */
   int rxbufstat[EC_MAXBUF];  //定义接收数据包状态
	
   /** rx MAC source address */
   int rxsa[EC_MAXBUF];  //定义接收数据包mac源地址
	
   /** temporary rx buffer */
   ec_bufT tempinbuf;   // 定义接收数据暂存的buffer
	
   /** temporary rx buffer status */
   int tempinbufs;  //暂存buffer状态
	
   /** transmit buffers */
	ec_bufT txbuf[EC_MAXBUF];  //定义发送数据包
	
   /** transmit buffer lenghts */
   int txbuflength[EC_MAXBUF]; //定义发送数据包每个buffer的长度
	
   /** temporary tx buffer 临时tx缓冲区*/
   ec_bufT txbuf2;
	 
   /** temporary tx buffer length 临时tx缓冲区长度 */
   int txbuflength2;
	 
   /** last used frame index 最后使用的帧索引*/
   int lastidx;
	 
   /** current redundancy state 当前冗余状态*/
   int redstate;
	 
   /** pointer to redundancy port and buffers 指向冗余端口和缓冲区的指针 */
   ecx_redportt *redport;
   //mtx_t * getindex_mutex;
   //mtx_t * tx_mutex;
   //mtx_t * rx_mutex;
} ecx_portt;

extern const uint16 priMAC[3];
extern const uint16 secMAC[3];
#ifdef EC_VER1
extern ecx_portt     ecx_port;
extern ecx_redportt  ecx_redport;

int ec_setupnic(const char * ifname, int secondary);
int ec_closenic(void);
void ec_setbufstat(int idx, int bufstat);
int ec_getindex(void);
int ec_outframe(int idx, int stacknumber);
int ec_outframe_red(int idx);
int ec_waitinframe(int idx, int timeout);
int ec_srconfirm(int idx,int timeout);
#endif

void ec_setupheader(void *p);
int ecx_setupnic(ecx_portt *port, const char * ifname, int secondary);      //网口初始化并打开
int ecx_closenic(ecx_portt *port);                                          //网口关闭
void ecx_setbufstat(ecx_portt *port, int idx, int bufstat);                 //设置idx号缓冲区状态
int ecx_getindex(ecx_portt *port);                                          //获取空闲idx缓冲区号 获取新的帧标识符索引并分配相应的rx缓冲区。    
int ecx_outframe(ecx_portt *port, int idx, int stacknumber);                //发送数据
int ecx_outframe_red(ecx_portt *port, int idx);                             //通过次口发送数据
int ecx_waitinframe(ecx_portt *port, int idx, int timeout);                 //等待idx号返回并接收 接收数据函数
int ecx_srconfirm(ecx_portt *port, int idx,int timeout);                    //发送idx 并等待接收数据的函数

#endif

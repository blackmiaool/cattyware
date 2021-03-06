/************************************************************************************
**	版权所有：Copyright (c) 2005 - 2010 ICRoute INC.
** 硬件说明：STM32f103vet6  8MHZ  内部时钟为72MHZ 
** 					 LD3320 有源晶振 12MHZ  不同的晶振请修改LDChip.h 中的定义值（＃define CLK_IN  12）
** 软件说明：并行读写寄存器方式，如采用其他方式即修改Reg_RW.c 文件即可！
** 修改者:  http://shop64790118.taobao.com/
** 时间：2011.10.15
** 说明：本程序修改于 ICRoute公司 配套的源程序，版权归其所有。
************************************************************************************/

#ifndef LD_CHIP_H
#define LD_CHIP_H
#include "stm32f4lib.h"
//#define uint8 unsigned char
//#define uint16 unsigned int
#define uint32 unsigned long
#define ld3320_print(arg...) {do{if(ini.Debug.audio){DBG(arg);}else{ no_use_printf(arg);}}while(0);}
#define ld3320_putchar(arg) {do{if(ini.Debug.audio){putchar(arg);}else{no_use_putchar(arg);}}while(0);}
//	以下三个状态定义用来记录程序是在运行ASR识别还是在运行MP3播放
#define LD_MODE_IDLE		0x00
#define LD_MODE_ASR_RUN		0x08
#define LD_MODE_MP3		 	0x40


//	以下五个状态定义用来记录程序是在运行ASR识别过程中的哪个状态
#define LD_ASR_NONE				0x00	//	表示没有在作ASR识别
#define LD_ASR_RUNING			0x01	//	表示LD3320正在作ASR识别中
#define LD_ASR_FOUNDOK			0x10	//	表示一次识别流程结束后，有一个识别结果
#define LD_ASR_FOUNDZERO 		0x11	//	表示一次识别流程结束后，没有识别结果
#define LD_ASR_ERROR	 		0x31	//	表示一次识别流程中LD3320芯片内部出现不正确的状态


#define CLK_IN   	30/* user need modify this value according to clock in */
#define LD_PLL_11			(uint8)((CLK_IN/2.0)-1)
#define LD_PLL_MP3_19		0x0f
#define LD_PLL_MP3_1B		0x18
#define LD_PLL_MP3_1D   	(uint8)(((90.0*((LD_PLL_11)+1))/(CLK_IN))-1)

#define LD_PLL_ASR_19 		(uint8)(CLK_IN*32.0/(LD_PLL_11+1) - 0.51)
#define LD_PLL_ASR_1B 		0x48
#define LD_PLL_ASR_1D 		0x1f

// LD chip fixed values.
#define        RESUM_OF_MUSIC               0x01
#define        CAUSE_MP3_SONG_END           0x20

#define        MASK_INT_SYNC				0x10
#define        MASK_INT_FIFO				0x04
#define    	   MASK_AFIFO_INT				0x01
#define        MASK_FIFO_STATUS_AFULL		0x08
struct sttsound{
	uint32 lenth;
	uint32 addr_start;
};

void sound_play(u8 index);
void LD_reset(void);
void  LD3320_main_Init(void);
void LD_Init_Common(void);
void LD_Init_MP3(void);//zarien...
void LD_Init_ASR(void);

void LD_play(uint8 index);//zarien...
void LD_AdjustMIX2SPVolume(uint8 value);//zarien...
void LD_ReloadMp3Data(uint8 index);
void LD_ReloadMp3Data_2(void);

uint8 LD_ProcessAsr(uint32 RecogAddr);
void LD_AsrStart(void);
uint8 LD_AsrRun(void);
uint8 LD_AsrAddFixed(void);
uint8 LD_GetResult(void);
void sound_Init(void );

void LD_ReadMemoryBlock(uint8 dev, uint8 * ptr, uint32 addr, uint8 count);
void LD_WriteMemoryBlock(uint8 dev, uint8 * ptr, uint32 addr, uint8 count);
void LD3320_Init(void);
//*******************************************
extern uint32 nMp3StartPos;
extern uint32 nMp3Size;			// zarien...
extern uint32 nMp3Pos;
extern uint8 bMp3Play;
extern uint8 nDemoFlag;
//*******************************************
extern uint8  nLD_Mode;


////识别码（客户修改处）
//#define CODE_QJ	1	 /*前进*/
//#define CODE_HT	2	 /*后退*/
//#define CODE_JS	3	 /*举手*/
//#define CODE_TT	4	 /*抬腿*/

//23项识别码（修改地方）
#define CODE_QJ	1	 /*你好*/
#define CODE_HT	2	 /*做个自我介绍*/
#define CODE_KNM	3	 /*你来自哪里*/
#define CODE_YMD	4	 /*展示本领*/
#define CODE_A 5 /*想做朋友吗*/
#define CODE_B 6  /*你会唱歌吗*/

#define CODE_C 7	 /*讲个故事吧*/
#define CODE_D 8 /*中国功夫*/
#define CODE_E 9 /*跳个舞吧*/
#define CODE_F 10	 /*前进*/
#define CODE_G 11 /*后退*/
#define CODE_H 12  /*射击*/

#define CODE_I 13	 /*向左转*/
#define CODE_J 14 /*向右转*/
#define CODE_K 15 /*举起手来*/
#define CODE_L 16 /*你真厉害*/
#define CODE_M 17 /*再见*/
#define CODE_N 18  /*学英语*/
			 
#define CODE_O 19	 /*科普知识*/
#define CODE_P 20 /*想啊*/
#define CODE_Q 21 /*不想*/

#define CODE_R 22/*红牌*/
#define CODE_S 23/*智力回答*/
#define CODE_T 24


#define MIC_VOL 0x43
#endif

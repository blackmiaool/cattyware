/*
 * File      : application.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      the first version
 */

/**
 * @addtogroup STM32
 */
/*@{*/

#include <stdio.h>
#include "app_interface.h"
#include "stm32f4xx.h"

#include <board.h>
#include <rtthread.h>
#include "debug.h"
#include "usb_bsp.h"
#include "usbh_core.h"
#include "usbh_usr.h"
#include "usbh_hid_core.h"
#include "stm32f4lib.h"
#include "commu.h"
#include "w25q16.h"
#include "mouse_gesture.h"
volatile u32 flash_addr;
u8 flash_buf[512];
void cmd(u8* content);
void commu_blue_send(u8* buf,u32 lenth);
void delay_ms2(u32 ms);

rt_mq_t mq_commu;
rt_sem_t sem_commu;
rt_sem_t sem_ld3320;
rt_sem_t sem_flash;
//   rt_mailbox_t mb_commu;
#define UART4_GPIO_TX		GPIO_Pin_0
#define UART4_TX_PIN_SOURCE GPIO_PinSource0
#define UART4_GPIO_RX		GPIO_Pin_1
#define UART4_RX_PIN_SOURCE GPIO_PinSource1
#define UART4_GPIO			GPIOA
#define UART4_GPIO_RCC   	RCC_AHB1Periph_GPIOA
#define RCC_APBPeriph_UART4	RCC_APB1Periph_UART4
void blue_putchar(u8 ch)
{
    USART_SendData(UART4, (uint8_t) ch);
    while (USART_GetFlagStatus(UART4, USART_FLAG_TC) == RESET){;}
}
void UART4_IRQHandler(void)//bluetooth
{
    static long lenth=0;
    static u8 buf[9];
    /* enter interrupt */
    rt_interrupt_enter();

    //rt_hw_serial_isr(&uart3_device);
    buf[lenth]=UART4->DR;
    lenth++;
    if(lenth==9)
    {
        lenth=0;
        rt_mq_send (mq_commu, (void*)buf, 9);
    }
    /* leave interrupt */
    rt_interrupt_leave();
    USART_ClearITPendingBit(UART4, USART_IT_RXNE);
}
void blue_tooth_Init()
{
    NVIC_InitTypeDef NVIC_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;
    SCPE(PERIOC);
    SCPE(PERIOA);
    IOConfig(IOCB,PIN13,tuiwanshuchu);


	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;

	/* Enable USART3 clock */
	RCC_APB1PeriphClockCmd(RCC_APBPeriph_UART4, ENABLE);


	GPIO_InitStructure.GPIO_Pin =   UART4_GPIO_TX|UART4_GPIO_RX;
	GPIO_Init(UART4_GPIO, &GPIO_InitStructure);
  //  IOConfig(IOAB,PIN1,fukongshuru);

    /* Connect alternate function */
    GPIO_PinAFConfig(UART4_GPIO, UART4_TX_PIN_SOURCE, GPIO_AF_UART4);
    GPIO_PinAFConfig(UART4_GPIO, UART4_RX_PIN_SOURCE, GPIO_AF_UART4);
	/* DMA clock enable */

    NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(UART4, &USART_InitStructure);
    
    USART_Cmd(UART4, ENABLE);
//        IO1(IOCB,PIN13);
//    rt_thread_delay(1000);
//    //IO(IOCB,PIN13);
//    rt_thread_delay(1000);
//    //blue_puts("at+uart=115200,0,0\r\n");//1st ADDR:14:1:61429
//    rt_thread_delay(1000);
//   // blue_puts("at+bind=14,1,61429\r\n");
//    rt_thread_delay(1000);
//  //  blue_puts("AT+ROLE=0\r\n");
//    rt_thread_delay(1000);
    IO0(IOCB,PIN13);
    rt_thread_delay(300);
    while(0)
    {
        u8 buf[2];
        buf[0]=UART4->DR+0x30;
        buf[1]=0;
        cmd(buf);
        rt_thread_delay(1000);
    } 
}
#include "usbh_hid_mouse.h"
u8 blue_choose=0;
ALIGN(RT_ALIGN_SIZE)
static char thread_commu_stack[1024];
struct rt_thread thread_commu;
void mouse_code(u8 * mouse_data)
{
    mouse_data[0]=mouse_data[0];
    mouse_data[1]=mouse_data[2];
    mouse_data[2]=mouse_data[4];
    mouse_data[3]=mouse_data[6];
    mouse_data[4]=0;
    mouse_data[5]=0;
    mouse_data[6]=0;
    mouse_data[7]=90;
}
static void rt_thread_entry_commu(void* parameter)
{
    //u32 *s;
    u8 mail[9];
    blue_tooth_Init();
    key_cap_Init();
    commu_Init();
    while(1)
    {
        rt_mq_recv (mq_commu, mail,9, RT_WAITING_FOREVER);
        switch(mail[0])
        {
        case 0://keyborad send with cap
        {
        	u8 i=0;
			for(i=0;i<9;i++)	
            DBG("i=%dmail=%d\r\n",i,mail[i]);
            if(key_capture(mail+1))
            {
                mail[8]=92;
                if(blue_choose)
                {
                    mail[0]=6;
                    commu_blue_send(mail,9);
                }
                
                else
                {
                   commu_send(mail,9); 
                }
            }
            break;
        }
        case 1://mouse 
        {

            static u8 release_flag=0;
//            u8 i=0;
//			for(i=0;i<9;i++)	
//               DBG("i=%dmail=%d\r\n",i,mail[i]);
            mail[8]=90;
            if((mail[2]==124||mail[4]==240)&&mail[6]==255)
            {
                DBG("mouse error");
                goto mouseend;
            }
            if(mail[1]==6)
            {
                mail[1]=1;
                commu_blue_send(mail,9);
                commu_send(mail,9); 
                blue_choose=1-blue_choose;
                
                if(blue_choose)
                    cmd("Blue remote");
                else
                {
                    cmd("Blue closed");
                }
                release_flag=1;
            }
            else if(release_flag)
            {
                if(mail[1]==0)
                {
                    release_flag=0;
                }   
            }
            else
            {
                if(mouse_capture(mail+1))
                {
                    mouse_code((mail+1));
                    if(blue_choose)
                    {     
                        commu_blue_send(mail,9);
                    }
                    else
                    {
                        commu_send(mail,9); 
                    }
                }
            }
            mouseend:
            
            break;
        }
        case 3://read flash addr
        {
            u8 buf[4];
            commu_recv(buf,4);
            flash_addr=*((u32*)buf);
            rt_sem_release(sem_commu);
            break;
        }
        case 4://send flash content
        {
            commu_send(flash_buf,512);
            rt_sem_release(sem_commu);
            break;
        }
        case 5://receive flash content 
        {
            commu_recv(flash_buf,512);
            rt_sem_release(sem_commu);
            break;
        }
        case 6://keyborad send immediately
        {
            mail[8]=92;
            if(blue_choose)
            {
                mail[0]=6;
                commu_blue_send(mail,9);
            }
            else
            {
                mail[0]=0;
               commu_send(mail,9); 
            }
            rt_thread_delay(10);
            break;
        }
        case 10:
        {
            mail[8]=92;
            mail[0]=6;
            commu_blue_send(mail,9);
            mail[0]=0;
            commu_send(mail,9);
            rt_thread_delay(10);  
            break;
        }
        case 11://mouse direct
        {
//            u8 i=0;
//			for(i=0;i<9;i++)	
//               DBG("i=%dmail=%d\r\n",i,mail[i]);
            mail[0]=1;
            mouse_code((mail+1));
            if(blue_choose)
            {     
                commu_blue_send(mail,9);
            }
            else
            {
                commu_send(mail,9); 
            }
            break;
        }
        }
        rt_thread_delay(1);
    }


}
void LD3320_main_Init(void);
void ProcessInt0(void );
void LD_loop(void);
ALIGN(RT_ALIGN_SIZE)
static char thread_ld3320_stack[4096];
struct rt_thread thread_ld3320;
static void rt_thread_entry_ld3320(void* parameter)
{
	DBG("Init_start\r\n");
    LD3320_main_Init();
	DBG("Init_end\r\n");
	LD_loop();
    while(1)
    {
		
        while(PCin(2)!=0)
        {
            rt_thread_delay(3); 
        }
        ProcessInt0();
        LD_loop();
    }
}


ALIGN(RT_ALIGN_SIZE)
static char thread_Flash_Read_stack[1024];
struct rt_thread thread_Flash_Read;
static void rt_thread_entry_Flash_Read(void* parameter)
{

    u8 buf[15];
    u8 i=0;
    for(i=0;i<15;i++)
        buf[i]=0;


    SCPE(PERIOB);
    IOConfig(IOBB,PIN6,xialashuru);
    rt_thread_delay(100);
    //	DBG("--------------------SCAN_START\r\n");
    rt_thread_delay(100);
    while(1)
    {
        while(PBin(6)==0)
        {
            rt_thread_delay(1);
        }
        //DBG("----------FOUND\r\n");
        buf[0]=3;//read!
        rt_mq_send (mq_commu, (void*)buf, 9);//ask addr
        rt_sem_take(sem_commu,RT_WAITING_FOREVER);//addr gotton
        if(flash_addr>=1000000)
        {
            buf[0]=5;
            rt_mq_send (mq_commu, (void*)buf, 9);
            rt_sem_take(sem_commu,RT_WAITING_FOREVER);
            rt_sem_take(sem_flash,RT_WAITING_FOREVER);
            SPI_Flash_Write(flash_buf, (flash_addr-1000000)<<9, 512);
            rt_sem_release(sem_flash);
        }
        else
        {
            SPI_Flash_Read(flash_buf, flash_addr<<9, 512);
            buf[0]=4;
            rt_mq_send (mq_commu, (void*)buf, 9);
            rt_sem_take(sem_commu,RT_WAITING_FOREVER);
        }
    }
    

}
void assert_failed(uint8_t* file, uint32_t line)
{
printf("Wrong parameters value: file %s on line %d\r\n",file, line);
}


__ALIGN_BEGIN USB_OTG_CORE_HANDLE           USB_OTG_Core_dev __ALIGN_END ;
__ALIGN_BEGIN USBH_HOST                     USB_Host __ALIGN_END ;
static char thread_usb_stack[1024];
struct rt_thread thread_usb;
extern uint8_t err_flag;
extern char debug_en;
static void rt_thread_entry_usb(void* parameter)
{
    rt_device_t LED_dev;
    u8 led_value;

    LED_dev=rt_device_find("LED");
    //while(1);
    if(LED_dev!=RT_NULL)
    {
        rt_kprintf("LED dev found\n\r");
        rt_device_write(LED_dev,0,&led_value,1);
        led_value++;
    }

    rt_device_open(LED_dev,RT_DEVICE_FLAG_ACTIVATED);

    DBG ("System Initializing...\n");
    #ifndef DEBUG
    #define DEBUG
    #endif
    debug_en=1;

    /* Init Host Library */

    USBH_Init(&USB_OTG_Core_dev,//hardware reg&info
              USB_OTG_FS_CORE_ID,		//use FS not HS(enum)
              &USB_Host,//USB state
              &HID_cb,//Class callback structure
              &USR_Callbacks);//User callback structure
	

	while(1)
	{
		USBH_Process(&USB_OTG_Core_dev , &USB_Host);
        rt_thread_delay(1);       
	} 
}
extern  char thread_app_stack[1024];
extern struct rt_thread thread_app;
void rt_thread_entry_app(void* parameter);
//void delay_ms2(u32);
int rt_application_init()
{


    SPI_Flash_Init();
    delay_ms2(2);
    rt_thread_init(&thread_usb,
                   "thread_usb",
                   rt_thread_entry_usb,
                   RT_NULL,
                   &thread_usb_stack[0],
            sizeof(thread_usb_stack),6,5);
    rt_thread_startup(&thread_usb);

    rt_thread_init(&thread_app,
                   "app",
                   rt_thread_entry_app,
                   RT_NULL,
                   &thread_app_stack[0],
            sizeof(thread_app_stack),10,5);
    rt_thread_startup(&thread_app);


    rt_thread_init(&thread_commu,
                   "commu",
                   rt_thread_entry_commu,
                   RT_NULL,
                   &thread_commu_stack[0],
            sizeof(thread_commu_stack),7,5);
    rt_thread_startup(&thread_commu);


    rt_thread_init(&thread_Flash_Read,
                   "flash",
                   rt_thread_entry_Flash_Read,
                   RT_NULL,
                   &thread_Flash_Read_stack[0],
            sizeof(thread_Flash_Read_stack),11,5);
    rt_thread_startup(&thread_Flash_Read);

//    rt_thread_init(&thread_ld3320,
//                   "ld3320",
//                   rt_thread_entry_ld3320,
//                   RT_NULL,
//                   &thread_ld3320_stack[0],
//            sizeof(thread_ld3320_stack),12,5);
//    rt_thread_startup(&thread_ld3320);

    {
        extern void Jacob_appinit();
        Jacob_appinit();
    }

    mq_commu=rt_mq_create ("mq_commu", 9, 500, RT_IPC_FLAG_FIFO);
    sem_commu=rt_sem_create ("sem_commu", 0, RT_IPC_FLAG_FIFO);
    sem_flash=rt_sem_create ("sem_flash", 1, RT_IPC_FLAG_FIFO);
    return 0;
}

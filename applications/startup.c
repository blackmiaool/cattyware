/*
 * File      : startup.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006, RT-Thread Develop Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://openlab.rt-thread.com/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2006-08-31     Bernard      first implementation
 * 2011-06-05     Bernard      modify for STM32F107 version
 */

#include <rthw.h>
#include <rtthread.h>

#include "stm32f4xx.h"
#include "board.h"

/**
 * @addtogroup STM32
 */

/*@{*/

extern int  rt_application_init(void);
#ifdef RT_USING_FINSH
extern void finsh_system_init(void);
extern void finsh_set_device(const char* device);
#endif

#ifdef __CC_ARM
extern int Image$$RW_IRAM1$$ZI$$Limit;
#define STM32_SRAM_BEGIN    (&Image$$RW_IRAM1$$ZI$$Limit)
#elif __ICCARM__
#pragma section="HEAP"
#define STM32_SRAM_BEGIN    (__segment_end("HEAP"))
#else
extern int __bss_end;
#define STM32_SRAM_BEGIN    (&__bss_end)
#endif

/*******************************************************************************
* Function Name  : assert_failed
* Description    : Reports the name of the source file and the source line number
*                  where the assert error has occurred.
* Input          : - file: pointer to the source file name
*                  - line: assert error line source number
* Output         : None
* Return         : None
*******************************************************************************/
//void assert_failed(u8* file, u32 line)
//{
//	rt_kprintf("\n\r Wrong parameter value detected on\r\n");
//	rt_kprintf("       file  %s\r\n", file);
//	rt_kprintf("       line  %d\r\n", line);

//	while (1) ;
//}

/**
 * This function will startup RT-Thread RTOS.
 */
 
 #include "usb_bsp.h"
#include "usbh_core.h"
#include "usbh_usr.h"
#include "usbh_hid_core.h"
#include "debug.h"
//#include "usb_host.h"
 
 
 
// #ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
//#if defined ( __ICCARM__ ) /*!< IAR Compiler */
//#pragma data_alignment=4
//#endif
//#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */
//__ALIGN_BEGIN USB_OTG_CORE_HANDLE           USB_OTG_Core_dev __ALIGN_END ;

//#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
//#if defined ( __ICCARM__ ) /*!< IAR Compiler */
//#pragma data_alignment=4
//#endif
//#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */
//__ALIGN_BEGIN USBH_HOST                     USB_Host __ALIGN_END ;


 
 
 
 
 void delay_ms2(u32 ms);

void rtthread_startup(void)
{
    
	/* init board */
	rt_hw_board_init();
	/* show version */
	rt_show_version();

	/* init tick */
	rt_system_tick_init();

	/* init kernel object */
	rt_system_object_init();

	/* init timer system */
	rt_system_timer_init();

    rt_system_heap_init((void*)STM32_SRAM_BEGIN, (void*)STM32_SRAM_END);

	/* init scheduler system */
	rt_system_scheduler_init();

	/* init all device */
	rt_device_init_all();

	/* init application */
	rt_application_init();

#ifdef RT_USING_FINSH
	/* init finsh */
	finsh_system_init();
	finsh_set_device( FINSH_DEVICE_NAME );
#endif

    /* init timer thread */
    rt_system_timer_thread_init();

	/* init idle thread */
	rt_thread_idle_init();

	/* start scheduler */
	rt_system_scheduler_start();

	/* never reach here */
	return ;
}

int main(void)
{
	/* disable interrupt first */
	rt_hw_interrupt_disable();

	/* startup RT-Thread RTOS */
	
	
	
	//delay_ms2(10000);
	rtthread_startup();

	return 0;
}
void delay_ms2(u32 ms)
{
	volatile u32 i=0;
	volatile u32 j=0;
	volatile u32 k=0;
	for(i=0;i<ms;i++)
	{
		for(j=0;j<1000;j++)
		for(k=0;k<72;k++);
	}
}

/*@}*/

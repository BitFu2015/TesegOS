/* *********************************************************
	文件:main.c
	功能:TesegOS事件应用示例
	说明：1.等待事件的任务不能为多个
				2.事件具有超时触发功能
	
	MCU: ATMEGA8 
	时钟:7.3728M  
	编译:WINAVR20100110
	芯艺工作室(http://www.chipart.cn)  版权所有
	工程主页: http://www.chipart.cn/projects/prj_hit.asp?id=12
		
	佟长福　2015-01-18
	佟长福　2020-02-18
***********************************************************/
#include "uxos.h"

#define LED_PORT_INIT DDRD|=_BV(PD3)|_BV(PD2)|_BV(PD4)

#define LEDR_SET PORTD&=~_BV(PD2)
#define LEDR_CLR PORTD|=_BV(PD2)
#define LEDR_TOGGLE PORTD^=_BV(PD2)

#define LEDG_SET PORTD&=~_BV(PD3)
#define LEDG_CLR PORTD|=_BV(PD3)
#define LEDG_TOGGLE PORTD^=_BV(PD3)

#define LEDY_SET PORTD&=~_BV(PD4)
#define LEDY_CLR PORTD|=_BV(PD4)
#define LEDY_TOGGLE PORTD^=_BV(PD4)

UX_EVENT g_Event;

uint8_t g_Task1Stack[128];
UX_TCB  g_Task1Tcb;
void Task1(void)
{
	uint8_t flg;
	
	while(1)
	{
		flg=uxWaitEvent(&g_Event,100);
		if(flg) //超时
			LEDR_SET;
		else		//其它任务给的事件信号
			LEDR_CLR;
	}
}

int main(void)
{
	//硬件初始化
	LED_PORT_INIT;
	LEDY_CLR;
	LEDG_CLR;
	
	uxCreateEvent(&g_Event);
	uxCreateTask(&g_Task1Tcb,Task1,g_Task1Stack+127,2);
	
	uxStart(); //调用此函数后不可再创建任务
	
	/*以下的代码作为默认任务运行，默认任务有如下特性：
	1.默认任务在开发环境启动代码配置的堆栈下运行，建议应用程序的主代码在此任务下执行
	2.默认任务的优先级为 1,仅高于空闲任务
	*/

	while(1)
	{
		uxDelayTask(190);	//让第一个等待周期超时，第二个等待周期超时前给信号
		uxSetEvent(&g_Event);
	}
}
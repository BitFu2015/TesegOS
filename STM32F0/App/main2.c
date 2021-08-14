/* *********************************************************
	文件:main.c
	功能:TesegOS 事件超时功能测试程序
	
	MCU: ATMEGA16  
	时钟:4M  
	编译:WINAVR20100110
	
	佟长福　2015-01-18
***********************************************************/
#include "uxos.h"
#include <avr/delay.h>

UX_EVENT g_Event;

uint8_t g_Task1Stack[128];
UX_TCB g_Task1Tcb;
void Task1(void)
{
	while(1)
	{
		uxWaitEvent(&g_Event,100);
		PORTA^=_BV(PA2);
	}
}

int main(void)
{
	uint16_t i;
	DDRA=_BV(PA2)|_BV(PA3)|_BV(PA4);
	PORTA|=_BV(PA2)|_BV(PA3)|_BV(PA4);
	uxInit();
	
	uxCreateEvent(&g_Event);
	uxCreateTask(&g_Task1Tcb,Task1,g_Task1Stack+127,1);
	
	uxStart();
	/*以下的代码作为空闲(IDLE)任务运行,
	在空闲任务中不可调用等待信号，延时等使系统切换任务的api函数*/
	while(1)
	{
		for(i=0;i<1000;i++)
			_delay_loop_2(250*4);
		PORTA^=_BV(PA4);	
	}
}

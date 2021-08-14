/* *********************************************************
	�ļ�:main.c
	����:TesegOS �¼���ʱ���ܲ��Գ���
	
	MCU: ATMEGA16  
	ʱ��:4M  
	����:WINAVR20100110
	
	١������2015-01-18
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
	/*���µĴ�����Ϊ����(IDLE)��������,
	�ڿ��������в��ɵ��õȴ��źţ���ʱ��ʹϵͳ�л������api����*/
	while(1)
	{
		for(i=0;i<1000;i++)
			_delay_loop_2(250*4);
		PORTA^=_BV(PA4);	
	}
}

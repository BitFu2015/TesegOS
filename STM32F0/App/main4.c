/* *********************************************************
	�ļ�:main.c
	����:TesegOS �������ʹ���ź���ͬ��ʾ��
	ע�������ͬ���ȼ�������ȴ�ͬһ���ź���ʱ���������õ�ִ��
	
	MCU: ATMEGA16  
	ʱ��:4M  
	����:WINAVR20100110
	
	١������2015-01-18
***********************************************************/
#include "uxos.h"
#include <avr/delay.h>

UX_SEM g_Sem;

uint8_t g_Task1Stack[128];
UX_TCB g_Task1Tcb;
void Task1(void)
{
	while(1)
	{
		uxWaitSem(&g_Sem);
		PORTA^=_BV(PA2);
	}
}

uint8_t g_Task2Stack[128];
UX_TCB g_Task2Tcb;
void Task2(void)
{
	while(1)
	{
		uxWaitSem(&g_Sem);
		PORTA^=_BV(PA3);
	}
}

uint8_t g_Task3Stack[128];
UX_TCB g_Task3Tcb;
void Task3(void)
{
	while(1)
	{
		uxDelayTask(100);
		uxGiveSem(&g_Sem);
	}
}

int main(void)
{
	uint16_t i;
	DDRA=_BV(PA2)|_BV(PA3)|_BV(PA4);
	PORTA|=_BV(PA2)|_BV(PA3)|_BV(PA4);
	uxInit();
	
	uxCreateSem(&g_Sem);
	uxCreateTask(&g_Task1Tcb,Task1,g_Task1Stack+127,2);
	uxCreateTask(&g_Task2Tcb,Task2,g_Task2Stack+127,2);
	uxCreateTask(&g_Task3Tcb,Task3,g_Task3Stack+127,1);
	
	uxStart(); 
	
	/*���µĴ�����Ϊ����(IDLE)��������,
	�ڿ��������в��ɵ��õȴ��źţ���ʱ��ʹϵͳ�л������api����*/
	while(1)
	{
		for(i=0;i<1000;i++)
			_delay_loop_2(250*4);
		UX_ENTER_CRITICAL();
		PORTA^=_BV(PA4);	
		UX_EXIT_CRITICAL();
	}
}

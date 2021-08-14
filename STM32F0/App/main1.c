/* *********************************************************
	�ļ�:main.c
	����:TesegOS ������ʱ��������ʹ��ʾ��
	
	MCU: ATMEGA16  
	ʱ��:4M  
	����:WINAVR20100110
	
	١������2015-01-18
***********************************************************/
#include "uxos.h"
#include <avr/delay.h>

UX_MUTEX g_Mutex;

uint8_t g_Task1Stack[128];
UX_TCB g_Task1Tcb;
void Task1(void)
{
	while(1)
	{
		uxDelayTask(100);
		uxLockMutex(&g_Mutex);
		PORTA^=_BV(PA2);
		uxUnlockMutex(&g_Mutex);
	}
}

uint8_t g_Task2Stack[128];
UX_TCB g_Task2Tcb;
void Task2(void)
{
	while(1)
	{
		uxDelayTask(100);
		uxLockMutex(&g_Mutex);
		PORTA^=_BV(PA3);
		uxUnlockMutex(&g_Mutex);
	}
}

int main(void)
{
	uint16_t i;
	DDRA=_BV(PA2)|_BV(PA3)|_BV(PA4);
	uxInit();
	
	uxCreateMutex(&g_Mutex);
	uxCreateTask(&g_Task1Tcb,Task1,g_Task1Stack+127,1);
	uxCreateTask(&g_Task2Tcb,Task2,g_Task2Stack+127,2);
	
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

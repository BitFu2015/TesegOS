/* *********************************************************
	�ļ�:main.c
	����:TesegOS ������ʱ��������ʹ��ʾ��
	������ͬ���ȼ�������ͨ���������������һ��io�ڵ�ʾ��
	
	MCU: ATMEGA8  
	ʱ��:7.3728M  
	����:WINAVR20100110
	о�չ�����(http://www.chipart.cn)  ��Ȩ����
	������ҳ: http://www.chipart.cn/projects/prj_hit.asp?id=12
	
	١������2015-01-18
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

UX_MUTEX g_Mutex;

uint8_t g_Task1Stack[128];
UX_TCB g_Task1Tcb;
void Task1(void)
{
	while(1)
	{
		uxLockMutex(&g_Mutex);
		LEDR_SET;
		uxDelayTask(100);
		uxUnlockMutex(&g_Mutex);
	}
}

uint8_t g_Task2Stack[128];
UX_TCB g_Task2Tcb;
void Task2(void)
{
	while(1)
	{
		uxLockMutex(&g_Mutex);
		LEDR_CLR;
		uxDelayTask(100);
		uxUnlockMutex(&g_Mutex);
	}
}

int main(void)
{
	//Ӳ����ʼ��
	LED_PORT_INIT;
	LEDG_CLR;
	LEDY_CLR;
	
	uxCreateMutex(&g_Mutex);
	uxCreateTask(&g_Task1Tcb,Task1,g_Task1Stack+127,2);
	uxCreateTask(&g_Task2Tcb,Task2,g_Task2Stack+127,2);
	
	uxStart(); //���ô˺����󲻿��ٴ�������
	
	/*���µĴ�����ΪĬ���������У�Ĭ���������������ԣ�
	1.Ĭ�������ڿ������������������õĶ�ջ�����У�����Ӧ�ó�����������ڴ�������ִ��
	2.Ĭ����������ȼ�Ϊ 1,�����ڿ�������
	*/

	while(1)
	{
		uxDelayTask(100);
	}
}

/*��������(IDLE)ִ�к����������������������ԣ�
	1.���������в��ɵ��õȴ��źţ���ʱ��ʹϵͳ�л������api����
	2.�����������ȼ�Ϊ���(ֵΪ0)
	3.ϵͳ����Ĭ�ϵĿ�������ִ�к�����������Ҫʱ��ɾ���˺���
	*/
void IdleTask(void)
{
	uint8_t c;
	
	//USART ��ʼ��
	UBRRH=0;
	UBRRL=47;//9600
	UCSRB=_BV(RXEN)|_BV(TXEN);
	
	while(1)
	{
		while(!(UCSRA & (1<<RXC)));//�ȴ�����һ�ֽ�
		c=UDR;//��ȡ���յ��ֽ�
		UDR=c;//�ٷ��ͳ�ȥ
	}
}
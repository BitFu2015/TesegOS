/* *********************************************************
	�ļ�:main.c
	����:TesegOS�¼�Ӧ��ʾ��
	˵����1.�ȴ��¼���������Ϊ���
				2.�¼����г�ʱ��������
	
	MCU: ATMEGA8 
	ʱ��:7.3728M  
	����:WINAVR20100110
	о�չ�����(http://www.chipart.cn)  ��Ȩ����
	������ҳ: http://www.chipart.cn/projects/prj_hit.asp?id=12
		
	١������2015-01-18
	١������2020-02-18
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
		if(flg) //��ʱ
			LEDR_SET;
		else		//������������¼��ź�
			LEDR_CLR;
	}
}

int main(void)
{
	//Ӳ����ʼ��
	LED_PORT_INIT;
	LEDY_CLR;
	LEDG_CLR;
	
	uxCreateEvent(&g_Event);
	uxCreateTask(&g_Task1Tcb,Task1,g_Task1Stack+127,2);
	
	uxStart(); //���ô˺����󲻿��ٴ�������
	
	/*���µĴ�����ΪĬ���������У�Ĭ���������������ԣ�
	1.Ĭ�������ڿ������������������õĶ�ջ�����У�����Ӧ�ó�����������ڴ�������ִ��
	2.Ĭ����������ȼ�Ϊ 1,�����ڿ�������
	*/

	while(1)
	{
		uxDelayTask(190);	//�õ�һ���ȴ����ڳ�ʱ���ڶ����ȴ����ڳ�ʱǰ���ź�
		uxSetEvent(&g_Event);
	}
}
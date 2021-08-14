/* *********************************************************
	�ļ�:main.c
	����:TesegOS �û��ж�ʾ��
	���Թ��ܣ��ڽ����ж������ź���֪ͨ����
	
	MCU: ATMEGA8  
	ʱ��:7.3728M  
	����:WINAVR20100110
	о�չ�����(http://www.chipart.cn)  ��Ȩ����
	������ҳ: http://www.chipart.cn/projects/prj_hit.asp?id=12
		
	١������2020-02-21������
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

UX_SEM g_Sem;
uint8_t g_Dat;  //�������ݻ�����

uint8_t g_Task1Stack[128];
UX_TCB g_Task1Tcb;
void Task1(void)
{
	//F_CPU=7372800
	//���������� U2X=1ʱ(F_CPU/SERIAL_BAUD/16)-1  U2X=0ʱ(F_CPU/SERIAL_BAUD/8)-1 
	UBRRH=0;
	UBRRL=47;  //9.6k:47(U2X=0)  19.2k:23(U2X=0)	38.4k:11(U2X=0)  115.2k:3(U2X=0)
	//UCSRA|=_BV(U2X);	
	UCSRB=_BV(RXEN)|_BV(TXEN)|_BV(RXCIE);	//�շ����������ж�����
	
	while(1)
	{
		uxPendSem(&g_Sem); //�ȴ�����
		UDR=g_Dat;				//ԭ�����ͳ�ȥ
		LEDR_TOGGLE;
	}
}

//UART����һ�ֽ��ж�
ISR(USART_RXC_vect)
{
	g_Dat=UDR;	//��ȡ�����ֽ�
	uxPostSem(&g_Sem);//�ź���֪ͨ����
}

int main(void)
{
	//Ӳ����ʼ��
	LED_PORT_INIT;
	
	//���������ں˶���
	uxCreateSem(&g_Sem,UXC_PEND);//�����ź���
	uxCreateTask(&g_Task1Tcb,Task1,g_Task1Stack+127,3);
	
	uxStart(); //���ô˺����󲻿��ٴ�������
	
	/*���µĴ�����ΪĬ���������У�Ĭ���������������ԣ�
	1.Ĭ�������ڿ������������������õĶ�ջ�����У�����Ӧ�ó�����������ڴ�������ִ��
	2.Ĭ����������ȼ�Ϊ 1,�����ڿ�������
	*/

	while(1)
	{
		uxDelayTask(100);
		LEDY_TOGGLE;
	}
}


/* *********************************************************
	文件:main.c
	功能:TesegOS 用户中断示例
	测试功能：在接收中断中用信号量通知任务
	
	MCU: ATMEGA8  
	时钟:7.3728M  
	编译:WINAVR20100110
	芯艺工作室(http://www.chipart.cn)  版权所有
	工程主页: http://www.chipart.cn/projects/prj_hit.asp?id=12
		
	佟长福　2020-02-21　创建
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
uint8_t g_Dat;  //接收数据缓冲区

uint8_t g_Task1Stack[128];
UX_TCB g_Task1Tcb;
void Task1(void)
{
	//F_CPU=7372800
	//波特率设置 U2X=1时(F_CPU/SERIAL_BAUD/16)-1  U2X=0时(F_CPU/SERIAL_BAUD/8)-1 
	UBRRH=0;
	UBRRL=47;  //9.6k:47(U2X=0)  19.2k:23(U2X=0)	38.4k:11(U2X=0)  115.2k:3(U2X=0)
	//UCSRA|=_BV(U2X);	
	UCSRB=_BV(RXEN)|_BV(TXEN)|_BV(RXCIE);	//收发允许、接收中断允许
	
	while(1)
	{
		uxPendSem(&g_Sem); //等待接收
		UDR=g_Dat;				//原样发送出去
		LEDR_TOGGLE;
	}
}

//UART接收一字节中断
ISR(USART_RXC_vect)
{
	g_Dat=UDR;	//读取接收字节
	uxPostSem(&g_Sem);//信号量通知任务
}

int main(void)
{
	//硬件初始化
	LED_PORT_INIT;
	
	//创建任务及内核对象
	uxCreateSem(&g_Sem,UXC_PEND);//创建信号量
	uxCreateTask(&g_Task1Tcb,Task1,g_Task1Stack+127,3);
	
	uxStart(); //调用此函数后不可再创建任务
	
	/*以下的代码作为默认任务运行，默认任务有如下特性：
	1.默认任务在开发环境启动代码配置的堆栈下运行，建议应用程序的主代码在此任务下执行
	2.默认任务的优先级为 1,仅高于空闲任务
	*/

	while(1)
	{
		uxDelayTask(100);
		LEDY_TOGGLE;
	}
}


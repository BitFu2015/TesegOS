/* *********************************************************
	文件:main.c
	功能:TesegOS 任务延时及互斥量使用示例
	两个相同优先级的任务通过互斥量交替访问一个io口的示例
	
	MCU: ATMEGA8  
	时钟:7.3728M  
	编译:WINAVR20100110
	芯艺工作室(http://www.chipart.cn)  版权所有
	工程主页: http://www.chipart.cn/projects/prj_hit.asp?id=12
	
	佟长福　2015-01-18
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
	//硬件初始化
	LED_PORT_INIT;
	LEDG_CLR;
	LEDY_CLR;
	
	uxCreateMutex(&g_Mutex);
	uxCreateTask(&g_Task1Tcb,Task1,g_Task1Stack+127,2);
	uxCreateTask(&g_Task2Tcb,Task2,g_Task2Stack+127,2);
	
	uxStart(); //调用此函数后不可再创建任务
	
	/*以下的代码作为默认任务运行，默认任务有如下特性：
	1.默认任务在开发环境启动代码配置的堆栈下运行，建议应用程序的主代码在此任务下执行
	2.默认任务的优先级为 1,仅高于空闲任务
	*/

	while(1)
	{
		uxDelayTask(100);
	}
}

/*空闲任务(IDLE)执行函数，空闲任务有如下特性：
	1.空闲任务中不可调用等待信号，延时等使系统切换任务的api函数
	2.空闲任务优先级为最低(值为0)
	3.系统具有默认的空闲任务执行函数，当不需要时可删除此函数
	*/
void IdleTask(void)
{
	uint8_t c;
	
	//USART 初始化
	UBRRH=0;
	UBRRL=47;//9600
	UCSRB=_BV(RXEN)|_BV(TXEN);
	
	while(1)
	{
		while(!(UCSRA & (1<<RXC)));//等待接收一字节
		c=UDR;//读取接收的字节
		UDR=c;//再发送出去
	}
}

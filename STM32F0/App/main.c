/* *********************************************************
	文件:main.c
	功能:TesegOS STM32F0应用示例
	
	MCU: STM32F030C8
	时钟:48M  
	编译:MDK5.12
	
	芯艺工作室(http://www.chipart.cn)  版权所有
	工程主页: http://www.chipart.cn/projects/prj_hit.asp?id=12
	
	佟长福　2020-03-12
***********************************************************/
#include "uxos.h"
#include "uart.h"

#define LED_OFF  GPIO_SetBits(GPIOA , GPIO_Pin_12)
#define LED_ON GPIO_ResetBits(GPIOA,GPIO_Pin_12)
#define LED_TOGGLE 	GPIOA->ODR^=GPIO_Pin_12

#define LCD_BK_ON	GPIOC->ODR|=GPIO_Pin_14
#define LCD_BK_OFF 	GPIOC->ODR&=~GPIO_Pin_14
#define LCD_BK_TOGGLE GPIOC->ODR^=GPIO_Pin_14

UX_SEM g_Sem;
UX_EVENT g_Event;
uint8_t g_Buffer[128];

//用户程序存放地址（也是存放中断向量的flash地址）
#define APPLICATION_ADDRESS ((uint32_t)0x08003000)
 //RAM中的中断向量
__IO uint32_t VectorTable[48] __attribute__((at(0x20000000)));

//中断向量映射到ram中，此操作只有BOOT_APP才需要
void BootConfig(void)
{	
	uint32_t i;

	//从flash中复制中断向量到ram
	for(i = 0; i < 48; i++)
		VectorTable[i] = *(__IO uint32_t*)(APPLICATION_ADDRESS + (i<<2));

	//中断向量映射到RAM（开始地址处）
	SYSCFG_MemoryRemapConfig(SYSCFG_MemoryRemap_SRAM);
}

//任务1
uint32_t g_Task1Stack[128];
UX_TCB g_Task1Tcb;
void Task1(void)
{
	while(1)
	{
		uxPendSem(&g_Sem);
		LED_TOGGLE;
	}
}

//任务2
uint32_t g_Task2Stack[128];
UX_TCB g_Task2Tcb;
void Task2(void)
{
	while(1)
	{
		uxPostSem(&g_Sem);
		uxDelayTask(100);
	}
}

int main(void)
{
	uint8_t len;
	GPIO_InitTypeDef        GPIO_InitStructure;
	
	BootConfig();//中断向量重新映射,非bootloader应用不可用
	
	// GPIOC Periph clock enable
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA|RCC_AHBPeriph_GPIOA|RCC_AHBPeriph_GPIOC, ENABLE);

  //led port init
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	uxCreateSem(&g_Sem,1);
	uxCreateEvent(&g_Event);
	uxCreateTask(&g_Task1Tcb,Task1,g_Task1Stack+127,3);
	uxCreateTask(&g_Task2Tcb,Task2,g_Task2Stack+127,2);
	
	uxStart(); 

	/*以下的代码作为默认任务运行，默认任务有如下特性：
	1.默认任务在开发环境启动代码配置的堆栈下运行，建议应用程序的主代码在此任务下执行
	2.默认任务的优先级为 1,仅高于空闲任务
	*/
	UartInit();
	while(1)
	{
		uxWaitEvent(&g_Event,INFINITE);//等待UART接收到数据包
		len=UartRead(g_Buffer);//读数据包
		UartWrite(g_Buffer,len);//原样发送回去
		LCD_BK_TOGGLE; //测试灯
	}
}

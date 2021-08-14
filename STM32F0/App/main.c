/* *********************************************************
	�ļ�:main.c
	����:TesegOS STM32F0Ӧ��ʾ��
	
	MCU: STM32F030C8
	ʱ��:48M  
	����:MDK5.12
	
	о�չ�����(http://www.chipart.cn)  ��Ȩ����
	������ҳ: http://www.chipart.cn/projects/prj_hit.asp?id=12
	
	١������2020-03-12
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

//�û������ŵ�ַ��Ҳ�Ǵ���ж�������flash��ַ��
#define APPLICATION_ADDRESS ((uint32_t)0x08003000)
 //RAM�е��ж�����
__IO uint32_t VectorTable[48] __attribute__((at(0x20000000)));

//�ж�����ӳ�䵽ram�У��˲���ֻ��BOOT_APP����Ҫ
void BootConfig(void)
{	
	uint32_t i;

	//��flash�и����ж�������ram
	for(i = 0; i < 48; i++)
		VectorTable[i] = *(__IO uint32_t*)(APPLICATION_ADDRESS + (i<<2));

	//�ж�����ӳ�䵽RAM����ʼ��ַ����
	SYSCFG_MemoryRemapConfig(SYSCFG_MemoryRemap_SRAM);
}

//����1
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

//����2
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
	
	BootConfig();//�ж���������ӳ��,��bootloaderӦ�ò�����
	
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

	/*���µĴ�����ΪĬ���������У�Ĭ���������������ԣ�
	1.Ĭ�������ڿ������������������õĶ�ջ�����У�����Ӧ�ó�����������ڴ�������ִ��
	2.Ĭ����������ȼ�Ϊ 1,�����ڿ�������
	*/
	UartInit();
	while(1)
	{
		uxWaitEvent(&g_Event,INFINITE);//�ȴ�UART���յ����ݰ�
		len=UartRead(g_Buffer);//�����ݰ�
		UartWrite(g_Buffer,len);//ԭ�����ͻ�ȥ
		LCD_BK_TOGGLE; //���Ե�
	}
}

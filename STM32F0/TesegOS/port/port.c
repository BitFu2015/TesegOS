//port.c ��TesegOSϵͳMCU����������ز���

#include "uxos.h"

#define NVIC_INT_CTRL			( ( volatile uint32_t *) 0xe000ed04 )
#define NVIC_PENDSVSET			0x10000000

extern PUX_OBJECT g_pCurTask; 	//��ǰ����ָ��,����ǰ����ջ�׵�ַ
static uint32_t g_CriticalNesting=0; //�ٽ�μ�����

uint32_t g_SystemStack[SYS_STACK_SIZE];	//ϵͳջ�ռ�

//��������Ĭ��ִ�к���,Ӧ���п���д�˺���
__weak void IdleTask(void)     
{
	while(1);
}

//�������˳�������
void TaskError(void)
{
	while(1);
}

void UX_ENTER_CRITICAL( void )
{
	__disable_irq();
	g_CriticalNesting++;
}

void UX_EXIT_CRITICAL( void )
{
	if(g_CriticalNesting)
		g_CriticalNesting--;
	
	if( g_CriticalNesting == 0 )
			__enable_irq();
}


//�ֶ��л�����(��ǰ���������ͷ�CPUʱִ��,����ʱ���ȴ��ź��������)
void osSched( void )
{
	//����PendSV�ж�
	*( NVIC_INT_CTRL ) = NVIC_PENDSVSET;
}

//�ж����л�����(ͨ���δ��ж�ǿ���ø������ȼ��������е����)
void osSchedFromISR( void )
{
	uint32_t sta=__get_PRIMASK();
	__ASM("cpsid i ");

	if(osTickHandler()) //�з��и������ȼ��������
		*(NVIC_INT_CTRL) = NVIC_PENDSVSET; //�����л������ж�
		
	__set_PRIMASK(sta);
}

//PendSV�жϴ��������л�
__asm void PendSV_Handler(void) 
{
	extern osSwitchTask 
	extern g_pCurTask

	PRESERVE8

	mrs r0, psp

	ldr	r3, = g_pCurTask 		//��ȡ��ǰ�����ջ��ַ�洢λ��
	ldr	r2, [r3]

	subs r0, #32					//�����ָ�ʱ��ջ�����ܱ���32�ֽڣ�8���Ĵ���
	str r0, [r2]					//����ջλ��
	stmia r0!, {r4-r7}		//�������ж���δ�Զ��洢�ĵͼĴ���
	
	mov r4, r8						//����߼Ĵ���
	mov r5, r9
	mov r6, r10
	mov r7, r11
	stmia r0!, {r4-r7}

	push {r3, r14}	
	cpsid i
	bl osSwitchTask
	cpsie i
	pop {r2, r3}						//ѹջǰ��r3 -> r2  r14 -> r3  

	ldr r1, [r2]
	ldr r0, [r1]						//������ջ��ַ
	adds r0, #16						//�������߼Ĵ�����
	ldmia r0!, {r4-r7}			//�ָ�����Ĵ���
	mov r8, r4
	mov r9, r5
	mov r10, r6
	mov r11, r7

	msr psp, r0							//д��psp������ջ��

	subs r0, #32						//�������ͼĴ���λ��
	ldmia r0!, {r4-r7}      //�ָ��ͼĴ���

	bx r3
	ALIGN
}

//ϵͳ�δ��ж�
void SysTick_Handler ( void )
{
	osSchedFromISR();
}

//Ϊ�����ʼ����ջ,��ʼֵΪģ�������л�ʱ��ѹջ����
ux_stack_t * osTaskStackInit(ux_stack_t *stack,void (*fn)(void) )
{
	stack--; //�ж�ƫ��
	*stack = 0x01000000;	//xPSR�Ĵ�����ʼֵ
	stack--;
	*stack = ( uint32_t ) fn;		//PC�Ĵ���
	stack--;
	*stack = (uint32_t)TaskError;//LR�Ĵ���
	stack -= 5;						// R12, R3, R2 and R1
	*stack = 0;						// R0 
	stack -= 8;					 // R11..R4
	
	return stack;
}

//�л���pspջģʽ����Ϊmsp����ϵͳջ
static void SwitchStackPointer(void)
{
	uint32_t stack;

	stack=__get_MSP();
	__set_MSP((uint32_t)(g_SystemStack + SYS_STACK_SIZE - 1));
	__set_PSP(stack);
	__set_CONTROL(2);
}

//����ϵͳ�δ��ж�
void osSysTickStart(void)
{
	SysTick_Config(480000);//ϵͳʱ��10ms
	NVIC_SetPriority(SysTick_IRQn,3); //�����ж�����Ϊ������ȼ�
	NVIC_SetPriority(PendSV_IRQn,3); 	

	SwitchStackPointer();	//����ǰ��������(default)�л���pspջģʽ����Ϊmspָ��ϵͳջ�ռ�
}


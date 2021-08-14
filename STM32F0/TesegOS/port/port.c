//port.c 　TesegOS系统MCU及编译器相关部分

#include "uxos.h"

#define NVIC_INT_CTRL			( ( volatile uint32_t *) 0xe000ed04 )
#define NVIC_PENDSVSET			0x10000000

extern PUX_OBJECT g_pCurTask; 	//当前任务指针,即当前任务栈首地址
static uint32_t g_CriticalNesting=0; //临界段计数器

uint32_t g_SystemStack[SYS_STACK_SIZE];	//系统栈空间

//空闲任务默认执行函数,应用中可重写此函数
__weak void IdleTask(void)     
{
	while(1);
}

//任务误退出处理函数
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


//手动切换任务(当前任务主动释放CPU时执行,如延时、等待信号量等情况)
void osSched( void )
{
	//触发PendSV中断
	*( NVIC_INT_CTRL ) = NVIC_PENDSVSET;
}

//中断中切换任务，(通过滴答中断强制让更高优先级任务运行的情况)
void osSchedFromISR( void )
{
	uint32_t sta=__get_PRIMASK();
	__ASM("cpsid i ");

	if(osTickHandler()) //有否有更高优先级任务就绪
		*(NVIC_INT_CTRL) = NVIC_PENDSVSET; //触发切换任务中断
		
	__set_PRIMASK(sta);
}

//PendSV中断处理，任务切换
__asm void PendSV_Handler(void) 
{
	extern osSwitchTask 
	extern g_pCurTask

	PRESERVE8

	mrs r0, psp

	ldr	r3, = g_pCurTask 		//获取当前任务堆栈地址存储位置
	ldr	r2, [r3]

	subs r0, #32					//调整恢复时的栈顶，总保存32字节，8个寄存器
	str r0, [r2]					//保存栈位置
	stmia r0!, {r4-r7}		//保存在中断中未自动存储的低寄存器
	
	mov r4, r8						//保存高寄存器
	mov r5, r9
	mov r6, r10
	mov r7, r11
	stmia r0!, {r4-r7}

	push {r3, r14}	
	cpsid i
	bl osSwitchTask
	cpsie i
	pop {r2, r3}						//压栈前后　r3 -> r2  r14 -> r3  

	ldr r1, [r2]
	ldr r0, [r1]						//新任务栈地址
	adds r0, #16						//调整到高寄存器组
	ldmia r0!, {r4-r7}			//恢复高组寄存器
	mov r8, r4
	mov r9, r5
	mov r10, r6
	mov r11, r7

	msr psp, r0							//写入psp新任务栈顶

	subs r0, #32						//调整到低寄存器位置
	ldmia r0!, {r4-r7}      //恢复低寄存器

	bx r3
	ALIGN
}

//系统滴答中断
void SysTick_Handler ( void )
{
	osSchedFromISR();
}

//为任务初始化堆栈,初始值为模拟任务切换时的压栈操作
ux_stack_t * osTaskStackInit(ux_stack_t *stack,void (*fn)(void) )
{
	stack--; //中断偏移
	*stack = 0x01000000;	//xPSR寄存器初始值
	stack--;
	*stack = ( uint32_t ) fn;		//PC寄存器
	stack--;
	*stack = (uint32_t)TaskError;//LR寄存器
	stack -= 5;						// R12, R3, R2 and R1
	*stack = 0;						// R0 
	stack -= 8;					 // R11..R4
	
	return stack;
}

//切换到psp栈模式，并为msp配置系统栈
static void SwitchStackPointer(void)
{
	uint32_t stack;

	stack=__get_MSP();
	__set_MSP((uint32_t)(g_SystemStack + SYS_STACK_SIZE - 1));
	__set_PSP(stack);
	__set_CONTROL(2);
}

//启动系统滴答中断
void osSysTickStart(void)
{
	SysTick_Config(480000);//系统时钟10ms
	NVIC_SetPriority(SysTick_IRQn,3); //两个中断配置为最低优先级
	NVIC_SetPriority(PendSV_IRQn,3); 	

	SwitchStackPointer();	//将当前调用任务(default)切换到psp栈模式，并为msp指定系统栈空间
}


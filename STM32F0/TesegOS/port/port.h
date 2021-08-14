//port.h
#ifndef PORT_H
#define PORT_H

/** \defgroup grp_interrupt 中断
	如何编写中断函数
	\par 系统滴答中断与PendSV中断
	arm内核设计之初充分考虑了RTOS运行问题，例如以下面这三个功能\n
	1.集成的系统定时器SysTick	\n
	2.双堆栈寄存器结构		\n
	3.软件触发中断机制	\n
	ARM集成的系统定时器典型用途就是产生系统滴答中断，由于任务切换在PendSV中断中完成的，所以系统滴答中断与普通的
	中断函数无任可特殊。\n
	PendSV中断是任务或中断将自己想切换任务意愿通知系统的最理想的方式，尤其在中断函数中可以告诉系统：等我退出后请切换一下任务。
	为了实现这样一个逻辑PendSV的优先级必须是最低的，其次是实现抢占逻辑的SysTick中断。实际上这两个中断只要实现互不嵌套就可以
	，所以通常他们的优先级设置成一样，即最低。
	
	\par 普通的用户中断例程
	其它普通的中断函数与前后台系统中的中断是没有区别的，只是有几点事项需要注意：\n
	1.所有中断都使用系统栈，栈大小由SYS_STACK_SIZE定义。\n
	2.中断和任务间的同步可以用全局的状态变量来实现，也可以通过信号量来实现，中断中调用的uxPostSem函数
	会直接切换任务。使用状态变量时等待状态变量的高优先级任务可能会有些延时才会得到执行。\n
	3.由于采用了PendSV中断切换任务机制,加上所有中断都自动使用专用栈，中断嵌套貌似没有任何问题。
	
	\par UX_ENTER_CRITICAL与UX_EXIT_CRITICAL
	进入临界状态（不会被中断）和退出函数，这两个函数需要成对使用，来处理不能被中断的任务。无论在任务还是在普通
	中断中使用，都未发现什么问题，但是为了保证系统和谐特性应少用临界段，在必须使用的前提下应保证其内的代码足够
	短（占用时间短）。\n
	理论上UX_ENTER_CRITICAL与UX_EXIT_CRITICAL的实现就是关和开全局中断的过程。但考虑到用户无意中将其嵌套使用，在两个
	函数的实现上做一个嵌套计数器，以保证只有在最外层的UX_EXIT_CRITICAL被调用时才打开中断，毕竟系统api函数也需要进入
	临界段，嵌套进入临界段也是个大概率事件。
*/
	
/** \defgroup grp_porting 系统移植
*	系统移置需要实现如下五点：
	 	\par 1.任务主动放弃CPU执行权进行任务切换:osSched函数
	 	在任务调用uxDelayTask,uxWaitEvent,uxLockMutex,uxPendSem进入一个等待状态时调用
	 	此函数进行任务切换。在Cortex-M0中osSched函数的实现非常简单，只需要触发一个PendSV中断即可。

		
		\par 2.任务被更高优先级任务抢占cpu执行板时进行任务切换：osSchedFromISR函数
		所有的抢占操作均是在系统滴答中断中完成的，osSchedFromISR实现步骤如下：
		\code
			osTickHandler();
			判断有无任务切换，有无更高优先级任务就绪，如果有触发PendSV中断
		\endcode
		双堆栈+软件中断方式切换任务优势体现在这里，系统滴答中断中不必每次存储和恢复任务环境,
		这大大的提高了系统运行效率。压缩了系统对CPU的占用率。
	 
	 	\par 3.任务堆栈初始化函数
	 	这是一个模拟切换任务时压栈当前环境的函数，把任务的初始栈内容准备好，以便切换到此任务时从栈弹出
	 	任务的运行环境。
	 	\code
		ux_stack_t * osTaskStackInit(ux_stack_t *stack,void (*fn)(void) )　//任务的堆栈初始化	
		\endcode
		
		\par 4.PendSV中断
		完成实际的任务的切换
		
		\par 5.系统滴答功能初始化 osSysTickStart
		系统滴答中断硬件初始化在此函数内实现，通常系统滴答中断周期设置成10~20毫秒。\n
		对于具有双堆栈指针的arm内核这里要多做一件事情，TesegOS将main函数视作default任务,系统上电后main函数是用msp执行，
		此时可视为系统模式，而执行到这里需要做一个身份的转换，即要变成一个普通的任务，就是要将msp交给系统（中断时用），而
		自己切换到psp模式。
		\code
		配置并开启系统滴答定时器
		SwitchStackPointer();	//将当前调用任务(default)切换到psp栈模式，并为msp指定系统栈空间
		\endcode
*/

#include "stm32f0xx.h"

/** \ingroup grp_porting
	\brief 堆栈数据类型定义，对于Cortex-M0每次压栈一个字,所以定义成uint32_t
*/
#define ux_stack_t uint32_t 

/** \ingroup grp_porting	
 * 进入临界操作状态
 * \hideinitializer
*/
void UX_ENTER_CRITICAL(void);

/** \ingroup grp_porting	
 * 退出临界操作状态
 * \hideinitializer
*/
void UX_EXIT_CRITICAL(void);

/** \ingroup grp_porting
	\brief 空闲任务处理函数
	\note 根据编译器特性将此函数实现处做弱声明处理(weak指示),以便用户可以重写此函数。
*/
void IdleTask(void);   

ux_stack_t * osTaskStackInit(ux_stack_t *stack,void (*fn)(void) );

void osSched( void );

void osSysTickStart(void);

/** \ingroup grp_porting	
 * \brief 定义空闲任务堆栈大小设置　
 * \hideinitializer
*/
#define  IDLE_TASK_STACK_SIZE  128

/** \ingroup grp_porting	
 * \brief 定义系统堆栈大小,包括系统滴答中断在内的所有中断在这个系统堆栈下运行
 * \hideinitializer
*/
#define  SYS_STACK_SIZE				128

#endif

//port.h

/** \defgroup grp_interrupt 中断
	如何编写中断函数
	\par 系统滴答中断
	TesegOS系统实现延时及抢占特性总是需要一个心跳例程周期性的触发，这个例程由硬件触发称之为系统滴答中断。
	移植到AVR时系统滴答中断例程使用了c程序实现，
	为了不让编译器在中断函数的首尾增加部分的环境保护代码，将中断函数定义为裸函数(naked),
	
	\note c程序中函数被编译时编译器首先在函数首部将所用到的寄存器压入堆栈保护起来，
	在函数主体内使用这些寄存器后退出时再从堆栈恢复其内容、最后生成返回指令。要注意
	的是它只是把用到寄存器保护起来，不是所有，而任务切换需要全部运行环境压入堆栈。
	
	这样函数内首先需要实现的是保存当前任务的运行环境，
	接着调用函数osTickHandler为系统提供心跳信号，然后任务切换到就绪的最高优先级任务，
	最后需要手动生成一个返回函数指令。
	
	\par 其它用户中断
	其它普通的中断函数与前后台系统中的中断是没有区别的，只是有几点事项需要注意：\n
	1.普通中断发生时使用当前运行的任务堆栈，由于任意一个任务都有可能被中断，所以每个任务的堆栈大小应
	足够应对被中断时的情况，同时应尽可能的把中断代码压缩到最少，这样对RAM的有效利用是非常有益的。\n
	2.中断和任务间的同步可以用全局的状态变量来实现，也可以通过信号量来实现，中断中调用的uxPostSem函数
	会直接切换任务。使用状态变量时等待状态变量的高优先级任务可能会有些延时才会得到执行、而使用信号量时
	要求所有任务的堆栈相应的略大。\n
	3.TesegOS不希望干预普通中断例程的编写形式，以此巩固它的“简洁、易用”特性，但这样做直接的后果就是不能完全应对中断嵌套的情况。
	更具体的描述就是：假如中断可以嵌套，那么比系统滴答中断更高优先级的任务中无法使用TesegOS提供的导致任务切换的接口函数。\n
	AVR系统进入中断时总是先关闭全局中断标志，以避免中断嵌套，使用软件的方法是可以实现中断嵌套的，但建议
	尽量避免使用中断嵌套，如果必须使用中断嵌套，那么应该避免在中断函数中调用内核api。
*/
	
/** \defgroup grp_porting 系统移植
*	系统移置需要实现如下四点：
	 	\par 1.任务主动放弃CPU执行权进行任务切换:osSched函数
	 	在任务调用uxDelayTask,uxWaitEvent,uxLockMutex,uxPendSem进入一个等待状态时调用
	 	此函数进行任务切换。osSched实现步骤如下:
	 	\code
		向堆栈保存当前上下文	
		切换当前任务指针
		从堆栈恢复上下文	
		\endcode
		
		\par 2.任务被更高优先级任务抢占cpu执行板时进行任务切换：osSchedFromISR函数
		所有的抢占操作均是在系统滴答中断中完成的，osSchedFromISR实现步骤如下：
		\code
			向堆栈保存当前上下文	
			osTickHandler();
			切换当前任务指针 
			从堆栈恢复上下文		
		\endcode
		就是比osSched多了一个osTickHandler调用，此函数会根据延时时间调整就绪任务列表，
		以便让更高优先级的任务先得到执行。
	 
	 	\par 3.任务堆栈初始化函数
	 	这是一个模拟切换任务时压栈当前环境的函数，把任务的初始栈内容准备好，以便切换到此任务时从栈弹出
	 	任务的运行环境。
	 	\code
		ux_stack_t * osTaskStackInit(ux_stack_t *stack,void (*fn)(void) )　//任务的堆栈初始化	
		\endcode
		
		\par 4.系统滴答功能初始化
		系统滴答中断硬件初始化在此函数内实现，通常系统滴答中断周期设置成10~20毫秒。
		\code
		void osSysTickStart(void) 		//配置并开启系统滴答定时器
		\endcode
*/
#ifndef PORT_H
#define PORT_H

#include <avr/io.h>
#include <avr/interrupt.h>

/** \ingroup grp_porting	
 * \brief 进入临界操作状态，考虑到中断中也有用到，所以此处实现方法是\n
 		保存sreg寄存器到栈，再关闭全局中断。
 * \hideinitializer
*/
#define UX_ENTER_CRITICAL()		asm volatile ( "in		__tmp_reg__, __SREG__" :: );\
									asm volatile ( "cli" :: );								\
									asm volatile ( "push	__tmp_reg__" :: )

/** \ingroup grp_porting	
 * \brief 退出临界操作状态，考虑到中断中也有用到，所以此处实现方法是\n
 *　对应UX_ENTER_CRITICAL()的压栈操作，直接从栈恢复SREG寄存器。
 * \hideinitializer
*/
#define UX_EXIT_CRITICAL()			asm volatile ( "pop		__tmp_reg__" :: );\
									asm volatile ( "out		__SREG__, __tmp_reg__" :: )

uint8_t * osTaskStackInit(uint8_t *stack,void (*fn)(void) );
void osSched( void ) __attribute__ ( ( naked ) );
void osSysTickStart(void);

/** \ingroup grp_porting
	\brief 空闲任务处理函数
	\note 根据编译器特性将此函数实现处做弱声明处理(weak指示),以便用户可以重写此函数。
*/
void IdleTask(void);

/** \ingroup grp_porting	
 * 空闲任务堆栈大小设置　
 * \hideinitializer
*/
#define  IDLE_TASK_STACK_SIZE  64

/** \ingroup grp_porting
	堆栈数据类型定义，对于AVR每次压栈一个字节,所以定义成uint8_t
*/
#define ux_stack_t uint8_t 

#endif

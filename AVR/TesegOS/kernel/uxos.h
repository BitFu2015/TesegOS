#ifndef UXOS_H
#define UXOS_H
/** \mainpage TesegOS实时操作系统
 
 	\author 芯艺工作室 - http://www.chipart.cn
 	
	\par 简介
	TesegOS面向8位机或32位低端MCU而设计，是一款简单易用，资源利用率高，代码简洁、
	易掌握的微型任务调度器。
	
	\par 任务调度策略
	TesegOS采用抢占式任务调度策略、已准备好的高优先级任务会通过中断的方式优先得到运行，
	相同优先级的任务按准备好时的时间顺序得到执行，同优先等级的任务不会互相抢占。
	
	\par 任务数量
	TesegOS中不限制任务数量，最多的任务数量取决于硬件资源。
	
	\par 应用程序示例
	\code
	
	#include "uxos.h"

	ux_stack_t g_Task1Stack[128];
	UX_TCB g_Task1Tcb;
	void Task1(void)
	{
		while(1)
		{
			//...
		}
	}

	ux_stack_t g_Task2Stack[128];
	UX_TCB g_Task2Tcb;
	void Task2(void)
	{
		while(1)
		{
			//...
		}
	}

	int main(void)
	{
		//硬件初始化
		//...
		
		//创建任务及其它内核对象
		uxCreateTask(&g_Task1Tcb,Task1,g_Task1Stack+127,2);
		uxCreateTask(&g_Task2Tcb,Task2,g_Task2Stack+127,2);
		
		uxStart(); //调用此函数后不可再创建任务
		
		//以下的代码作为默认任务(DefaultTask)运行，默认任务有如下特性：
		//1.默认任务在开发环境启动代码配置的堆栈下运行，建议应用程序的主代码在此任务下执行
		//2.默认任务的优先级为 1,仅高于空闲任务

		while(1)
		{
			//...
		}
	}

	//空闲任务(IDLE)执行函数，空闲任务有如下特性：
	//1.空闲任务中不可调用等待信号，延时等使系统切换任务的api函数
	//2.空闲任务优先级为最低(值为0)
	//3.系统具有默认的空闲任务执行函数，当不需要时可删除此函数
	void IdleTask(void)
	{
		while(1)
		{
			//...
		}
	}	
	
	\endcode
*/

/** \defgroup grp_task 任务

	任务需要在入口函数(main)中创建，必须在调用uxStart前创建好
	TesegOS内置两个特殊任务DefaultTask和IdleTask
	
	\par 默认任务DefaultTask
	DefaultTask 即是入口函数(main)执行的内容,为此main函数的最后必须是个
	无限循环（不可退出），DefaultTask使用的是开发环境启动代码配置的堆栈，
	通常这是最不易溢出的情况，在有限RAM资源的MCU来说这是安全利用RAM资源
	很有效的方式。
	
	\par 空闲任务IdleTask
	IdleTask 在系统内核中创建，用于在调度器发现没有其它准备好的任务时执行，
	IdleTask的执行函数在系统内核中已定义，但用户可在应用中重写来执行CPU空闲时
	的非重要任务，但需要注意的是，IdleTask中试图切换任务可导致系统崩溃，
	为此IdleTask中不可调用系统提供的等待、延时等导致主动放弃CPU执行权的api函数。
	
	\par 任务优先级
	TesegOS任务优先级可设置范围是0~255,数值越高优先级越高。DefaultTask的优先级为1
	IdleTask任务的优先级为0,如果用户创建的任务优先级为0，就会导致无法得到cpu执行
	权的情况，因为同样0优先级的IdleTask任务永远不会主动放弃cpu执行权。所以
	一般用户新建任务使用的优先级为2,3,4...等。
*/

/** \defgroup grp_sem 信号量
	信号量是在两个或多个任务请求多个资源时提供分配和等待机制。
*/

/** \defgroup grp_mutex 互斥量	
	
	互斥量是在两个或多个任务请求一个资源时提供分配和等待机制。
*/

/** \defgroup grp_event 事件
	事件
	TesegOS中事件是两个任务间同步的一种机制，事件具有等待超时功能，
	但等待事件的任务只能为一个。
*/


#include "port.h"

//内核对象类型常数
#define UXC_TASK 0
#define UXC_OBJECT 1

//内核对象状态常数
#define UXC_PEND 0		//挂起
#define UXC_DONE 1		//有信号
#define UXC_WAIT 2		//有任务等待

#define NULL 0
#define INFINITE 0xFFFF
#define TIMEOUT  0
#define TRUE 1
#define FALSE 0

//内核模块结构
typedef struct KernelObjectBlock
{
		union
		{
			ux_stack_t *stack;								//任务堆栈
			struct KernelObjectBlock * task;	//内核对象等待任务列表
		}u1;
		union
		{
		  uint8_t priority; 	//任务优先级
		  uint8_t state;			//内核对象状态
		}u2;
	  uint8_t type;				//内核对象类型
    uint16_t dly;				//延时计数器
    
    struct KernelObjectBlock  *next;    //链表下一个
}UX_OBJECT,*PUX_OBJECT;

typedef UX_OBJECT UX_TCB,*PUX_TCB;			//任务对象
typedef UX_OBJECT UX_EVENT,*PUX_EVENT;	//事件对象
typedef UX_OBJECT UX_SEM,*PUX_SEM;			//信号量
typedef UX_OBJECT UX_MUTEX,*PUX_MUTEX;	//互斥量

typedef PUX_OBJECT UX_LIST,*PUX_LIST; //内核对象列表

//移植调用,移植文件port.c中将调用下面的两函数
void osSwitchTask(void);
uint8_t osTickHandler(void);

//任务
/** \ingroup grp_task
	\brief 创建一个任务
	\param pTCB 任务对象
	\param fn 任务执行函数，原型为 void xxx(void)
	\param stack 任务堆栈在RAM中的位置
	\param priority 优先级(1~255)
*/
void uxCreateTask(PUX_TCB pTCB,void (*fn)(void),ux_stack_t *stack,uint8_t priority);
/** \ingroup grp_task
	\brief 当前任务延时
	\param n 延时的系统滴答周期
*/
void uxDelayTask(uint16_t n);
/** \ingroup grp_task
	\brief 系统开始运行
	\note 一般在入口函数(main)中调用，在调用前创建任务，调用后不可再创建任何任务
*/
void uxStart(void);//系统开始运行

//内核对象
void uxCreateObject(PUX_OBJECT pObj,uint8_t def);
uint8_t uxWaitObject(PUX_OBJECT pObj,uint16_t timeout);
uint8_t uxQueryObject(PUX_OBJECT pObj);
void uxSetObject(PUX_OBJECT pObj);

//事件
/** \ingroup grp_event
	\brief 创建一个系统事件
	\param X 指向事件对象指针
	\hideinitializer
*/
#define uxCreateEvent(X) uxCreateObject(X,UXC_PEND)

/** \ingroup grp_event
	\brief 等待一个事件对象
	\param X 指向事件对象指针
	\param Y 超时系统滴答数
	\return 事件触发原因，非零表示超时触发，零表示正常触发动作而返回
	\hideinitializer
*/
#define uxWaitEvent(X,Y) uxWaitObject(X,Y)

/** \ingroup grp_event
	\brief 触发一个事件
	\param X 指向事件对象指针
	\hideinitializer
*/
#define uxSetEvent(X) 	uxSetObject(X)

//互斥量
/** \ingroup grp_mutex
	\brief 创建一个系统互斥量
	\param X 指向互斥量对象指针
	\hideinitializer
*/
#define uxCreateMutex(X) uxCreateObject(X,UXC_DONE)

/** \ingroup grp_mutex
	\brief　锁定一个互斥量
	\param X 指向互斥量对象指针
	\hideinitializer
*/
#define uxLockMutex(X) uxWaitObject(X,INFINITE)

/** \ingroup grp_mutex
	\brief　解锁（释放）一个互斥量
	\param X 指向互斥量对象指针
	\hideinitializer
*/
#define uxUnlockMutex(X) uxSetObject(X)

//信号量
/** \ingroup grp_sem
	\brief　创建一个系统信号量
	\param X 指向信号量对象指针
	\param Y 可同时执行的任务个数
	\hideinitializer
*/
#define uxCreateSem(X,Y) uxCreateObject(X,Y)

/** \ingroup grp_sem
	\brief　请求一个信号量
	\param X 指向信号量对象指针
	\hideinitializer
*/
#define uxPendSem(X) uxWaitObject(X,INFINITE)

/** \ingroup grp_sem
	\brief　向信号量释放一个请求权限
	\param X 指向信号量对象指针
	\hideinitializer
*/
#define uxPostSem(X) uxSetObject(X)

/** \ingroup grp_sem
	\brief　无等待的请求信号量
	\param X 指向信号量对象指针
	\return UXC_DONE:表示为本次请求释放了一个信号 UXC_PEND:无可释放信号　UXC_WAIT:有其它任务也在等待此信号量
	\hideinitializer
*/
#define uxAcceptSem(X) uxQueryObject(X) 

#endif

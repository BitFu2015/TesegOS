/**************************************************
	文件名：uxos.c
	功  能：TesegOS实时操作系统内核
	
	版权说明：
	芯艺工作室(http://www.chipart.cn)  版权所有
	工程主页: http://www.chipart.cn/projects/prj_hit.asp?id=12
	
	历史记录
	佟长福 V1.0 2015-01-01  创建
	佟长福 V1.1 2020-02-16  增加空闲任务,默认任务机制
	佟长福 V1.2 2020-03-12	兼容性修正
**************************************************/
#include "uxos.h"

PUX_OBJECT g_pCurTask;				//当前执行任务
PUX_OBJECT g_pRdyTaskList=NULL;  	//就绪任务列表
PUX_OBJECT g_pDlyObjList=NULL; 		//延时对象列表
UX_TCB  g_DefaultTaskTcb;				//默认任务对象

//空闲任务
UX_TCB g_IdleTaskTcb;
ux_stack_t g_IdleTaskStack[IDLE_TASK_STACK_SIZE]; //空闲任务栈长度需在port.h中定义

//将一个任务按优先级插入到指定的任务列表
static void osTaskInsert(PUX_LIST pTaskList,PUX_TCB pTask)
{
	PUX_TCB pList=*pTaskList;
	if(pList==NULL) //空列表
	{
		*pTaskList=pTask;
		pTask->next=NULL;
	}
	else if(pList->u2.priority < pTask->u2.priority) //首部插入，pTask为最高优先级
	{
		*pTaskList  =pTask;
		pTask->next =pList;
	}
	else
	{
		while(1)
		{
			if(pList->next == NULL)//尾部插入
			{
				pList->next=pTask;
				pTask->next=NULL;
				break;
			}
			else if(pList->next->u2.priority < pTask->u2.priority)	//中间插入
			{
				pTask->next=pList->next;
				pList->next=pTask;
				break;	
			}
			else
				pList=pList->next;
		}	
	}
}

//将一个内核对象按延时大小顺序插入到指定的列表，用于延时对象列表
static void osObjectInsert(PUX_LIST pObjList,PUX_OBJECT pObj)
{
		uint16_t t;
		PUX_OBJECT pList=*pObjList;
		
		if(pList==NULL) //空时
		{
			*pObjList=pObj;
			pObj->next=NULL;
		}
		else if(pList->dly > pObj->dly) //新插入为延时最少的对象
		{
			//首部插入
			*pObjList=pObj;
			pObj->next=pList;
			t=pList->dly - pObj->dly;
			
			//第一个对象计数器调整
			pList->dly -= pObj->dly;
			pList=pList->next;
			
			//后边对象计数器调整
			while(pList != NULL)
			{
				pList->dly += t;
				pList=pList->next;
			}
		}
		else
		{
			pObj->dly -= pList->dly; //校正延时计数值
			while(1)
			{
				if((pList->next == NULL) || (pList->next->dly > pObj->dly))
				{
					pObj->next=pList->next;
					pList->next=pObj;
					break;
				}
				pList=pList->next;
			}
		}
}

//从指定列表中查找并删除一个内核对象
static void osObjectDelete(PUX_LIST pObjList,PUX_OBJECT pObj)
{
	PUX_OBJECT pTmp=*pObjList;
	if(pTmp==pObj) //如果是队列中第一个对象
	{
		*pObjList=pTmp->next;
		pTmp=pTmp->next;
		while(pTmp != NULL) //延时校正
		{
			pTmp->dly+=pObj->dly;
			pTmp=pTmp->next;
		}
	}
	else
	{
		while(pTmp != NULL)		
		{
			if(pTmp->next == pObj)
			{
				pTmp->next=pObj->next;
				pObj->next=NULL;
				break;	
			}
			pTmp=pTmp->next;
		}
	}
}

//更新当前任务，将需要运行的任务指针指向已就绪且优先级最高的任务
void osSwitchTask(void)
{
	if(g_pRdyTaskList!=g_pCurTask)
		g_pCurTask=g_pRdyTaskList;
}

//系统定时器,需在系统滴答中断中调用
uint8_t osTickHandler(void)
{
	uint16_t num;
	PUX_OBJECT pObj;
	
	if(g_pDlyObjList != NULL) //有等待对象
	{
		if(g_pDlyObjList->dly	== 0 )	//到时
		{
			//移除第一项
			pObj=g_pDlyObjList;
			g_pDlyObjList=pObj->next;
			
			if(pObj->type == UXC_TASK)//等待任务
			{
				osTaskInsert(&g_pRdyTaskList,pObj);
			}
			else //等待的内核对象
			{
				osTaskInsert(&g_pRdyTaskList,pObj->u1.task);
				pObj->u1.task=NULL; //清等待任务	
			}	
			
			//后边等待对象的计数器校正
			pObj=g_pDlyObjList;
			if(pObj != NULL)
			{
				num=pObj->dly;
				pObj=pObj->next;
				while(pObj != NULL)
				{
					pObj->dly-=num;
					pObj=pObj->next;
				}
			}
		}
		else
			g_pDlyObjList->dly--;
	}
	
	if(g_pRdyTaskList!=g_pCurTask)
		return 1;
	
	return 0;
}

//新建一个任务,参数依次：任务控制块、任务处理函数、任务堆栈、任务优先级
void uxCreateTask(PUX_TCB pTCB,void (*fn)(void),ux_stack_t *stack,uint8_t priority)
{
	pTCB->u2.priority=priority;			//设置优先级
	pTCB->type=UXC_TASK;						//对象类型为任务
	pTCB->u1.stack=osTaskStackInit(stack,fn);//堆栈初始化
	
	//UX_ENTER_CRITICAL();
	osTaskInsert(&g_pRdyTaskList,pTCB);//插入到就绪任务列表	
	//UX_EXIT_CRITICAL();	
}

//任务延时
void uxDelayTask(uint16_t n)
{
	g_pCurTask->dly=n;
	
	UX_ENTER_CRITICAL();
	g_pRdyTaskList = g_pRdyTaskList->next;				//从就绪队列中删除	
	osObjectInsert(&g_pDlyObjList,g_pCurTask); 	  //放入等待队列
	osSched();																	  //切换任务	
	UX_EXIT_CRITICAL();
}

//内核对象初始化函数
void uxCreateObject(PUX_OBJECT pObj,uint8_t def)
{
	pObj->u1.task=NULL;				//无等待任务
	pObj->u2.state=def; 			//初始状态
	pObj->type=UXC_OBJECT;		//类型为内核对象
	pObj->dly=0;							//无延时
	pObj->next=NULL;		
}

//等待一个内核对象,超时返回TRUE(非零值),否则返回FALSE(0)
uint8_t uxWaitObject(PUX_OBJECT pObj,uint16_t timeout)
{
	UX_ENTER_CRITICAL();
	if(pObj->u2.state > UXC_PEND)
	{
			pObj->u2.state--;
	}
	else
	{
		g_pRdyTaskList=g_pRdyTaskList->next;
		osTaskInsert(&(pObj->u1.task),g_pCurTask);//当前任务插入到对象等待列表中
		if(timeout != INFINITE) //有超时选项的插入到等待队列
		{
				pObj->dly=timeout;
				osObjectInsert(&g_pDlyObjList,pObj);	
		}	
		osSched();
	}	
	UX_EXIT_CRITICAL();
	
	return (pObj->dly == TIMEOUT);
}

//查询对象状态
uint8_t uxQueryObject(PUX_OBJECT pObj)
{
	uint8_t ret;
	
	UX_ENTER_CRITICAL();
	if(pObj->u2.state > 0)
	{
		pObj->u2.state--;
		ret = UXC_DONE;
	}
	else if(pObj->u1.task == NULL)
		ret=UXC_PEND;
	else
		ret=UXC_WAIT;
	
	UX_EXIT_CRITICAL();
	
	return ret;	
}

//设置信号
void uxSetObject(PUX_OBJECT pObj)
{
	PUX_TCB pTask;
	UX_ENTER_CRITICAL();
	if(pObj->u2.state == UXC_PEND)
	{
		if(pObj->u1.task != NULL) //有等待任务
		{
			pTask=pObj->u1.task;
			pObj->u1.task = pObj->u1.task->next;
			osTaskInsert(&g_pRdyTaskList,pTask);
			
			if(pObj->dly != INFINITE ) //如果已放入超时列表（如事件）
				osObjectDelete(&g_pDlyObjList,pObj);

			osSched();
		}
		else
		{
			pObj->u2.state=UXC_DONE;
		}
	}	
	else
		pObj->u2.state++;
		
	UX_EXIT_CRITICAL();
}

//任务调度器开始工作
void uxStart(void)
{
	//创建空闲任务,空闲任务优先级为0(最低)
	uxCreateTask(&g_IdleTaskTcb,IdleTask,g_IdleTaskStack + IDLE_TASK_STACK_SIZE-1 ,0);
	
	//默认任务初始化
	g_DefaultTaskTcb.u2.priority=1;			//默认任务优先级为 1
	g_DefaultTaskTcb.type=UXC_TASK;			//对象类型为任务
	
	//默认任务为当前运行任务
	g_pCurTask=&g_DefaultTaskTcb;				//当前任务上下文保存地址
	g_DefaultTaskTcb.next=g_pRdyTaskList;//当前任务(默认任务)插入到就绪任务的最前
	g_pRdyTaskList=&g_DefaultTaskTcb;		
	g_pDlyObjList=NULL;								//延时任务列表初始化为空	
	osSysTickStart();									//打开系统滴答定时器
	osSched();												//切换到最优先的任务
}


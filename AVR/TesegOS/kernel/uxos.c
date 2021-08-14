/**************************************************
	�ļ�����uxos.c
	��  �ܣ�TesegOSʵʱ����ϵͳ�ں�
	
	��Ȩ˵����
	о�չ�����(http://www.chipart.cn)  ��Ȩ����
	������ҳ: http://www.chipart.cn/projects/prj_hit.asp?id=12
	
	��ʷ��¼
	١���� V1.0 2015-01-01  ����
	١���� V1.1 2020-02-16  ���ӿ�������,Ĭ���������
	١���� V1.2 2020-03-12	����������
**************************************************/
#include "uxos.h"

PUX_OBJECT g_pCurTask;				//��ǰִ������
PUX_OBJECT g_pRdyTaskList=NULL;  	//���������б�
PUX_OBJECT g_pDlyObjList=NULL; 		//��ʱ�����б�
UX_TCB  g_DefaultTaskTcb;				//Ĭ���������

//��������
UX_TCB g_IdleTaskTcb;
ux_stack_t g_IdleTaskStack[IDLE_TASK_STACK_SIZE]; //��������ջ��������port.h�ж���

//��һ���������ȼ����뵽ָ���������б�
static void osTaskInsert(PUX_LIST pTaskList,PUX_TCB pTask)
{
	PUX_TCB pList=*pTaskList;
	if(pList==NULL) //���б�
	{
		*pTaskList=pTask;
		pTask->next=NULL;
	}
	else if(pList->u2.priority < pTask->u2.priority) //�ײ����룬pTaskΪ������ȼ�
	{
		*pTaskList  =pTask;
		pTask->next =pList;
	}
	else
	{
		while(1)
		{
			if(pList->next == NULL)//β������
			{
				pList->next=pTask;
				pTask->next=NULL;
				break;
			}
			else if(pList->next->u2.priority < pTask->u2.priority)	//�м����
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

//��һ���ں˶�����ʱ��С˳����뵽ָ�����б�������ʱ�����б�
static void osObjectInsert(PUX_LIST pObjList,PUX_OBJECT pObj)
{
		uint16_t t;
		PUX_OBJECT pList=*pObjList;
		
		if(pList==NULL) //��ʱ
		{
			*pObjList=pObj;
			pObj->next=NULL;
		}
		else if(pList->dly > pObj->dly) //�²���Ϊ��ʱ���ٵĶ���
		{
			//�ײ�����
			*pObjList=pObj;
			pObj->next=pList;
			t=pList->dly - pObj->dly;
			
			//��һ���������������
			pList->dly -= pObj->dly;
			pList=pList->next;
			
			//��߶������������
			while(pList != NULL)
			{
				pList->dly += t;
				pList=pList->next;
			}
		}
		else
		{
			pObj->dly -= pList->dly; //У����ʱ����ֵ
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

//��ָ���б��в��Ҳ�ɾ��һ���ں˶���
static void osObjectDelete(PUX_LIST pObjList,PUX_OBJECT pObj)
{
	PUX_OBJECT pTmp=*pObjList;
	if(pTmp==pObj) //����Ƕ����е�һ������
	{
		*pObjList=pTmp->next;
		pTmp=pTmp->next;
		while(pTmp != NULL) //��ʱУ��
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

//���µ�ǰ���񣬽���Ҫ���е�����ָ��ָ���Ѿ��������ȼ���ߵ�����
void osSwitchTask(void)
{
	if(g_pRdyTaskList!=g_pCurTask)
		g_pCurTask=g_pRdyTaskList;
}

//ϵͳ��ʱ��,����ϵͳ�δ��ж��е���
uint8_t osTickHandler(void)
{
	uint16_t num;
	PUX_OBJECT pObj;
	
	if(g_pDlyObjList != NULL) //�еȴ�����
	{
		if(g_pDlyObjList->dly	== 0 )	//��ʱ
		{
			//�Ƴ���һ��
			pObj=g_pDlyObjList;
			g_pDlyObjList=pObj->next;
			
			if(pObj->type == UXC_TASK)//�ȴ�����
			{
				osTaskInsert(&g_pRdyTaskList,pObj);
			}
			else //�ȴ����ں˶���
			{
				osTaskInsert(&g_pRdyTaskList,pObj->u1.task);
				pObj->u1.task=NULL; //��ȴ�����	
			}	
			
			//��ߵȴ�����ļ�����У��
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

//�½�һ������,�������Σ�������ƿ顢���������������ջ���������ȼ�
void uxCreateTask(PUX_TCB pTCB,void (*fn)(void),ux_stack_t *stack,uint8_t priority)
{
	pTCB->u2.priority=priority;			//�������ȼ�
	pTCB->type=UXC_TASK;						//��������Ϊ����
	pTCB->u1.stack=osTaskStackInit(stack,fn);//��ջ��ʼ��
	
	//UX_ENTER_CRITICAL();
	osTaskInsert(&g_pRdyTaskList,pTCB);//���뵽���������б�	
	//UX_EXIT_CRITICAL();	
}

//������ʱ
void uxDelayTask(uint16_t n)
{
	g_pCurTask->dly=n;
	
	UX_ENTER_CRITICAL();
	g_pRdyTaskList = g_pRdyTaskList->next;				//�Ӿ���������ɾ��	
	osObjectInsert(&g_pDlyObjList,g_pCurTask); 	  //����ȴ�����
	osSched();																	  //�л�����	
	UX_EXIT_CRITICAL();
}

//�ں˶����ʼ������
void uxCreateObject(PUX_OBJECT pObj,uint8_t def)
{
	pObj->u1.task=NULL;				//�޵ȴ�����
	pObj->u2.state=def; 			//��ʼ״̬
	pObj->type=UXC_OBJECT;		//����Ϊ�ں˶���
	pObj->dly=0;							//����ʱ
	pObj->next=NULL;		
}

//�ȴ�һ���ں˶���,��ʱ����TRUE(����ֵ),���򷵻�FALSE(0)
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
		osTaskInsert(&(pObj->u1.task),g_pCurTask);//��ǰ������뵽����ȴ��б���
		if(timeout != INFINITE) //�г�ʱѡ��Ĳ��뵽�ȴ�����
		{
				pObj->dly=timeout;
				osObjectInsert(&g_pDlyObjList,pObj);	
		}	
		osSched();
	}	
	UX_EXIT_CRITICAL();
	
	return (pObj->dly == TIMEOUT);
}

//��ѯ����״̬
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

//�����ź�
void uxSetObject(PUX_OBJECT pObj)
{
	PUX_TCB pTask;
	UX_ENTER_CRITICAL();
	if(pObj->u2.state == UXC_PEND)
	{
		if(pObj->u1.task != NULL) //�еȴ�����
		{
			pTask=pObj->u1.task;
			pObj->u1.task = pObj->u1.task->next;
			osTaskInsert(&g_pRdyTaskList,pTask);
			
			if(pObj->dly != INFINITE ) //����ѷ��볬ʱ�б����¼���
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

//�����������ʼ����
void uxStart(void)
{
	//������������,�����������ȼ�Ϊ0(���)
	uxCreateTask(&g_IdleTaskTcb,IdleTask,g_IdleTaskStack + IDLE_TASK_STACK_SIZE-1 ,0);
	
	//Ĭ�������ʼ��
	g_DefaultTaskTcb.u2.priority=1;			//Ĭ���������ȼ�Ϊ 1
	g_DefaultTaskTcb.type=UXC_TASK;			//��������Ϊ����
	
	//Ĭ������Ϊ��ǰ��������
	g_pCurTask=&g_DefaultTaskTcb;				//��ǰ���������ı����ַ
	g_DefaultTaskTcb.next=g_pRdyTaskList;//��ǰ����(Ĭ������)���뵽�����������ǰ
	g_pRdyTaskList=&g_DefaultTaskTcb;		
	g_pDlyObjList=NULL;								//��ʱ�����б��ʼ��Ϊ��	
	osSysTickStart();									//��ϵͳ�δ�ʱ��
	osSched();												//�л��������ȵ�����
}


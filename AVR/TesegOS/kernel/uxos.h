#ifndef UXOS_H
#define UXOS_H
/** \mainpage TesegOSʵʱ����ϵͳ
 
 	\author о�չ����� - http://www.chipart.cn
 	
	\par ���
	TesegOS����8λ����32λ�Ͷ�MCU����ƣ���һ������ã���Դ�����ʸߣ������ࡢ
	�����յ�΢�������������
	
	\par ������Ȳ���
	TesegOS������ռʽ������Ȳ��ԡ���׼���õĸ����ȼ������ͨ���жϵķ�ʽ���ȵõ����У�
	��ͬ���ȼ�������׼����ʱ��ʱ��˳��õ�ִ�У�ͬ���ȵȼ������񲻻ụ����ռ��
	
	\par ��������
	TesegOS�в���������������������������ȡ����Ӳ����Դ��
	
	\par Ӧ�ó���ʾ��
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
		//Ӳ����ʼ��
		//...
		
		//�������������ں˶���
		uxCreateTask(&g_Task1Tcb,Task1,g_Task1Stack+127,2);
		uxCreateTask(&g_Task2Tcb,Task2,g_Task2Stack+127,2);
		
		uxStart(); //���ô˺����󲻿��ٴ�������
		
		//���µĴ�����ΪĬ������(DefaultTask)���У�Ĭ���������������ԣ�
		//1.Ĭ�������ڿ������������������õĶ�ջ�����У�����Ӧ�ó�����������ڴ�������ִ��
		//2.Ĭ����������ȼ�Ϊ 1,�����ڿ�������

		while(1)
		{
			//...
		}
	}

	//��������(IDLE)ִ�к����������������������ԣ�
	//1.���������в��ɵ��õȴ��źţ���ʱ��ʹϵͳ�л������api����
	//2.�����������ȼ�Ϊ���(ֵΪ0)
	//3.ϵͳ����Ĭ�ϵĿ�������ִ�к�����������Ҫʱ��ɾ���˺���
	void IdleTask(void)
	{
		while(1)
		{
			//...
		}
	}	
	
	\endcode
*/

/** \defgroup grp_task ����

	������Ҫ����ں���(main)�д����������ڵ���uxStartǰ������
	TesegOS����������������DefaultTask��IdleTask
	
	\par Ĭ������DefaultTask
	DefaultTask ������ں���(main)ִ�е�����,Ϊ��main�������������Ǹ�
	����ѭ���������˳�����DefaultTaskʹ�õ��ǿ������������������õĶ�ջ��
	ͨ�������������������������RAM��Դ��MCU��˵���ǰ�ȫ����RAM��Դ
	����Ч�ķ�ʽ��
	
	\par ��������IdleTask
	IdleTask ��ϵͳ�ں��д����������ڵ���������û������׼���õ�����ʱִ�У�
	IdleTask��ִ�к�����ϵͳ�ں����Ѷ��壬���û�����Ӧ������д��ִ��CPU����ʱ
	�ķ���Ҫ���񣬵���Ҫע����ǣ�IdleTask����ͼ�л�����ɵ���ϵͳ������
	Ϊ��IdleTask�в��ɵ���ϵͳ�ṩ�ĵȴ�����ʱ�ȵ�����������CPUִ��Ȩ��api������
	
	\par �������ȼ�
	TesegOS�������ȼ������÷�Χ��0~255,��ֵԽ�����ȼ�Խ�ߡ�DefaultTask�����ȼ�Ϊ1
	IdleTask��������ȼ�Ϊ0,����û��������������ȼ�Ϊ0���ͻᵼ���޷��õ�cpuִ��
	Ȩ���������Ϊͬ��0���ȼ���IdleTask������Զ������������cpuִ��Ȩ������
	һ���û��½�����ʹ�õ����ȼ�Ϊ2,3,4...�ȡ�
*/

/** \defgroup grp_sem �ź���
	�ź���������������������������Դʱ�ṩ����͵ȴ����ơ�
*/

/** \defgroup grp_mutex ������	
	
	��������������������������һ����Դʱ�ṩ����͵ȴ����ơ�
*/

/** \defgroup grp_event �¼�
	�¼�
	TesegOS���¼������������ͬ����һ�ֻ��ƣ��¼����еȴ���ʱ���ܣ�
	���ȴ��¼�������ֻ��Ϊһ����
*/


#include "port.h"

//�ں˶������ͳ���
#define UXC_TASK 0
#define UXC_OBJECT 1

//�ں˶���״̬����
#define UXC_PEND 0		//����
#define UXC_DONE 1		//���ź�
#define UXC_WAIT 2		//������ȴ�

#define NULL 0
#define INFINITE 0xFFFF
#define TIMEOUT  0
#define TRUE 1
#define FALSE 0

//�ں�ģ��ṹ
typedef struct KernelObjectBlock
{
		union
		{
			ux_stack_t *stack;								//�����ջ
			struct KernelObjectBlock * task;	//�ں˶���ȴ������б�
		}u1;
		union
		{
		  uint8_t priority; 	//�������ȼ�
		  uint8_t state;			//�ں˶���״̬
		}u2;
	  uint8_t type;				//�ں˶�������
    uint16_t dly;				//��ʱ������
    
    struct KernelObjectBlock  *next;    //������һ��
}UX_OBJECT,*PUX_OBJECT;

typedef UX_OBJECT UX_TCB,*PUX_TCB;			//�������
typedef UX_OBJECT UX_EVENT,*PUX_EVENT;	//�¼�����
typedef UX_OBJECT UX_SEM,*PUX_SEM;			//�ź���
typedef UX_OBJECT UX_MUTEX,*PUX_MUTEX;	//������

typedef PUX_OBJECT UX_LIST,*PUX_LIST; //�ں˶����б�

//��ֲ����,��ֲ�ļ�port.c�н����������������
void osSwitchTask(void);
uint8_t osTickHandler(void);

//����
/** \ingroup grp_task
	\brief ����һ������
	\param pTCB �������
	\param fn ����ִ�к�����ԭ��Ϊ void xxx(void)
	\param stack �����ջ��RAM�е�λ��
	\param priority ���ȼ�(1~255)
*/
void uxCreateTask(PUX_TCB pTCB,void (*fn)(void),ux_stack_t *stack,uint8_t priority);
/** \ingroup grp_task
	\brief ��ǰ������ʱ
	\param n ��ʱ��ϵͳ�δ�����
*/
void uxDelayTask(uint16_t n);
/** \ingroup grp_task
	\brief ϵͳ��ʼ����
	\note һ������ں���(main)�е��ã��ڵ���ǰ�������񣬵��ú󲻿��ٴ����κ�����
*/
void uxStart(void);//ϵͳ��ʼ����

//�ں˶���
void uxCreateObject(PUX_OBJECT pObj,uint8_t def);
uint8_t uxWaitObject(PUX_OBJECT pObj,uint16_t timeout);
uint8_t uxQueryObject(PUX_OBJECT pObj);
void uxSetObject(PUX_OBJECT pObj);

//�¼�
/** \ingroup grp_event
	\brief ����һ��ϵͳ�¼�
	\param X ָ���¼�����ָ��
	\hideinitializer
*/
#define uxCreateEvent(X) uxCreateObject(X,UXC_PEND)

/** \ingroup grp_event
	\brief �ȴ�һ���¼�����
	\param X ָ���¼�����ָ��
	\param Y ��ʱϵͳ�δ���
	\return �¼�����ԭ�򣬷����ʾ��ʱ���������ʾ������������������
	\hideinitializer
*/
#define uxWaitEvent(X,Y) uxWaitObject(X,Y)

/** \ingroup grp_event
	\brief ����һ���¼�
	\param X ָ���¼�����ָ��
	\hideinitializer
*/
#define uxSetEvent(X) 	uxSetObject(X)

//������
/** \ingroup grp_mutex
	\brief ����һ��ϵͳ������
	\param X ָ�򻥳�������ָ��
	\hideinitializer
*/
#define uxCreateMutex(X) uxCreateObject(X,UXC_DONE)

/** \ingroup grp_mutex
	\brief������һ��������
	\param X ָ�򻥳�������ָ��
	\hideinitializer
*/
#define uxLockMutex(X) uxWaitObject(X,INFINITE)

/** \ingroup grp_mutex
	\brief���������ͷţ�һ��������
	\param X ָ�򻥳�������ָ��
	\hideinitializer
*/
#define uxUnlockMutex(X) uxSetObject(X)

//�ź���
/** \ingroup grp_sem
	\brief������һ��ϵͳ�ź���
	\param X ָ���ź�������ָ��
	\param Y ��ͬʱִ�е��������
	\hideinitializer
*/
#define uxCreateSem(X,Y) uxCreateObject(X,Y)

/** \ingroup grp_sem
	\brief������һ���ź���
	\param X ָ���ź�������ָ��
	\hideinitializer
*/
#define uxPendSem(X) uxWaitObject(X,INFINITE)

/** \ingroup grp_sem
	\brief�����ź����ͷ�һ������Ȩ��
	\param X ָ���ź�������ָ��
	\hideinitializer
*/
#define uxPostSem(X) uxSetObject(X)

/** \ingroup grp_sem
	\brief���޵ȴ��������ź���
	\param X ָ���ź�������ָ��
	\return UXC_DONE:��ʾΪ���������ͷ���һ���ź� UXC_PEND:�޿��ͷ��źš�UXC_WAIT:����������Ҳ�ڵȴ����ź���
	\hideinitializer
*/
#define uxAcceptSem(X) uxQueryObject(X) 

#endif

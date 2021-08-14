//port.c ��TesegOSϵͳMCU����������ز���

#include "uxos.h"

extern volatile PUX_OBJECT g_pCurTask; //��ǰ����ָ��,����ǰ����ջ�׵�ַ

void __attribute__((weak)) IdleTask(void)      //��������Ĭ��ִ�к���,Ӧ���п���д�˺���
{
	while(1);
}

#define SAVE_CONTEXT()									\
	asm volatile (	"push	r0						\n\t"	\
					"in		r0, __SREG__					\n\t"	\
					"cli								\n\t"	\
					"push	r0						\n\t"	\
					"push	r1						\n\t"	\
					"clr	r1						\n\t"	\
					"push	r2						\n\t"	\
					"push	r3						\n\t"	\
					"push	r4						\n\t"	\
					"push	r5						\n\t"	\
					"push	r6						\n\t"	\
					"push	r7						\n\t"	\
					"push	r8						\n\t"	\
					"push	r9						\n\t"	\
					"push	r10						\n\t"	\
					"push	r11						\n\t"	\
					"push	r12						\n\t"	\
					"push	r13						\n\t"	\
					"push	r14						\n\t"	\
					"push	r15						\n\t"	\
					"push	r16						\n\t"	\
					"push	r17						\n\t"	\
					"push	r18						\n\t"	\
					"push	r19						\n\t"	\
					"push	r20						\n\t"	\
					"push	r21						\n\t"	\
					"push	r22						\n\t"	\
					"push	r23						\n\t"	\
					"push	r24						\n\t"	\
					"push	r25						\n\t"	\
					"push	r26						\n\t"	\
					"push	r27						\n\t"	\
					"push	r28						\n\t"	\
					"push	r29						\n\t"	\
					"push	r30						\n\t"	\
					"push	r31						\n\t"	\
					"lds	r26, g_pCurTask					\n\t"	\
					"lds	r27, g_pCurTask + 1			\n\t"	\
					"in		r0, 0x3d								\n\t"	\
					"st		x+, r0									\n\t"	\
					"in		r0, 0x3e								\n\t"	\
					"st		x+, r0									\n\t"	\
				);

#define RESTORE_CONTEXT()								\
	asm volatile (	"lds	r26, g_pCurTask		\n\t"	\
					"lds	r27, g_pCurTask + 1				\n\t"	\
					"ld		r28, x+										\n\t"	\
					"out	__SP_L__, r28							\n\t"	\
					"ld		r29, x+										\n\t"	\
					"out	__SP_H__, r29							\n\t"	\
					"pop	r31						\n\t"	\
					"pop	r30						\n\t"	\
					"pop	r29						\n\t"	\
					"pop	r28						\n\t"	\
					"pop	r27						\n\t"	\
					"pop	r26						\n\t"	\
					"pop	r25						\n\t"	\
					"pop	r24						\n\t"	\
					"pop	r23						\n\t"	\
					"pop	r22						\n\t"	\
					"pop	r21						\n\t"	\
					"pop	r20						\n\t"	\
					"pop	r19						\n\t"	\
					"pop	r18						\n\t"	\
					"pop	r17						\n\t"	\
					"pop	r16						\n\t"	\
					"pop	r15						\n\t"	\
					"pop	r14						\n\t"	\
					"pop	r13						\n\t"	\
					"pop	r12						\n\t"	\
					"pop	r11						\n\t"	\
					"pop	r10						\n\t"	\
					"pop	r9						\n\t"	\
					"pop	r8						\n\t"	\
					"pop	r7						\n\t"	\
					"pop	r6						\n\t"	\
					"pop	r5						\n\t"	\
					"pop	r4						\n\t"	\
					"pop	r3						\n\t"	\
					"pop	r2						\n\t"	\
					"pop	r1						\n\t"	\
					"pop	r0						\n\t"	\
					"out	__SREG__, r0	\n\t"	\
					"pop	r0						\n\t"	\
				);

//�ֶ��л�����(��ǰ���������ͷ�CPUʱִ��,����ʱ���ȴ��ź��������)
void osSched( void ) __attribute__ ( ( naked ) );
void osSched( void )
{
	SAVE_CONTEXT();
	osSwitchTask(); //�л���������ȼ�����
	RESTORE_CONTEXT(); 
	
	//��ת����������,����ָ����µ�ջ�е���ִ�е�ַ����ת������
	asm volatile ("ret");
}

//�ж����л�����(ͨ���ж�ǿ���ø������ȼ��������е����)
void osSchedFromISR( void ) __attribute__ ( ( naked ) );
void osSchedFromISR( void )
{
	SAVE_CONTEXT();		//�жϿ�ʼ�������ж�ʱ�����л���
	osTickHandler();
	osSwitchTask();
	RESTORE_CONTEXT();	//�ָ����������л���
	
	//ͨ������ָ����ת��������
	asm volatile ("ret");//��������osSchedFromISRʱ��ѹջ
}

//ϵͳ�δ��ж�,�㺯��(naked)
void TIMER1_COMPA_vect ( void ) __attribute__ ((signal,naked));
void TIMER1_COMPA_vect ( void )
{
	osSchedFromISR();
	asm volatile ("reti");//�����жϲ���ʱ��Ӳ���Զ��ڵ�ǰ�����е�ѹջ
}

//Ϊ�����ʼ����ջ
ux_stack_t * osTaskStackInit(ux_stack_t *stack,void (*fn)(void) )
{
	uint16_t addr;

	//ʵ��һ������ָ����ת��fn����//
	addr = ( uint16_t )fn;
	*stack =   (uint8_t)addr;
	stack--;

	addr >>= 8;
	*stack = (uint8_t)addr;
	stack--;
	///////////////////////////////
	
	//���Ĵ�����ʼֵ����///////////
	*stack =   0x00;	 //R0 
	stack--;
	*stack = 0x80; 		//SREG
	stack--;

	*stack =   0x00;	 //R1 
	stack--;
	*stack =   0x02;	 //R2 
	stack--;
	*stack =   0x03;	 //R3 
	stack--;
	*stack =   0x04;	 //R4 
	stack--;
	*stack =   0x05;	 //R5 
	stack--;
	*stack =   0x06;	 //R6 
	stack--;
	*stack =   0x07;	 //R7 
	stack--;
	*stack =   0x08;	 //R8 
	stack--;
	*stack =   0x09;	 //R9 
	stack--;
	*stack =   0x10;	 //R10 
	stack--;
	*stack =   0x11;	 //R11 
	stack--;
	*stack =   0x12;	 //R12 
	stack--;
	*stack =   0x13;	 //R13 
	stack--;
	*stack =   0x14;	 //R14 
	stack--;
	*stack =   0x15;	 //R15 
	stack--;
	*stack =   0x16;	 //R16 
	stack--;
	*stack =   0x17;	 //R17 
	stack--;
	*stack =   0x18;	 //R18 
	stack--;
	*stack =   0x19;	 //R19 
	stack--;
	*stack =   0x20;	 //R20 
	stack--;
	*stack =   0x21;	 //R21 
	stack--;
	*stack =   0x22;	 //R22 
	stack--;
	*stack =   0x23;	 //R23 
	stack--;	
	*stack =   0x24;	 //R24 
	stack--;
	*stack =   0x25;	 //R25 
	stack--;
	*stack =   0x26;	 //R26 X 
	stack--;
	*stack =   0x27;	 //R27 
	stack--;
	*stack =   0x28;	 //R28 Y 
	stack--;
	*stack =   0x29;	 //R29 
	stack--;
	*stack =   0x30;	 //R30 Z 
	stack--;
	*stack =   0x31;	 //R31 
	stack--;
	
	return stack;
}

//����ϵͳ�δ��ж�
void osSysTickStart(void)
{
  TCCR1A=0;
  TCCR1B=_BV(WGM12)|_BV(CS10);
  OCR1A=0x9C40;
  TIMSK|=_BV(OCIE1A);
  sei();		
}


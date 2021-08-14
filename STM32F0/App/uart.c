//uart.c

#include "uxos.h"

uint8_t g_RxBuffer[128];//���ջ�����
uint8_t g_TxBuffer[128];//���ͻ�����
uint32_t g_RxIndex;//�����ֽ�����
uint32_t g_TxSize;//���ͳ���
uint32_t g_TxIndex;//��������

extern UX_EVENT g_Event; //�¼���������֪ͨ������յ�����

//5ms��ʱ��ʱ������
void TIM3_Config(void)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  NVIC_InitTypeDef NVIC_InitStructure;

  //TIM3ʱ��ʹ��
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

  //TIM3�ж�����
  NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

	//��ʱʱ����㣺((1+TIM_Prescaler)/48M)*(1+TIM_Period) 
	TIM_DeInit(TIM3);            //��λ��ʱ��
	TIM_TimeBaseStructure.TIM_Period=48000-1;  //��ʱ����ʼֵ
	TIM_TimeBaseStructure.TIM_Prescaler=4;         //ʱ��Ԥ��Ƶ
	TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;  // ʱ�ӷָ�
	TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up; //���ϼ���ģʽ
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseStructure);   //��ʼ����ʱ����ֵ
	TIM_ClearFlag(TIM3,TIM_FLAG_Update);               //�����ʱ���жϱ�־  

  TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);//�ж�ʹ��
  //TIM_Cmd(TIM3, ENABLE);//��ʱ��ʹ��
}

//TIM3�жϴ�����
void TIM3_IRQHandler(void)
{
  TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
	TIM_Cmd(TIM3, DISABLE);//��ʱ���ر�
	uxSetEvent(&g_Event);//֪ͨ����,���յ�һ���ݰ�
}

void UartTimerEnable(void)
{
	TIM_Cmd(TIM3, DISABLE);
	TIM3->CNT=0;
	TIM_Cmd(TIM3, ENABLE);
	//TIM3->CR1 |= TIM_CR1_CEN;
}

//��ʼ��uart
void UartInit(void)
{
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	
	//״̬����ʼ��
	g_RxIndex=0;
	g_TxIndex=0;
	g_TxSize=0;

	//ʱ������
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	
	//USART1 Pins configuration 
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_0); 
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_0);  
  
  //Configure pins as AF pushpull
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOB, &GPIO_InitStructure);  
	
	USART_InitStructure.USART_BaudRate = 9600;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; 
  USART_Init(USART1, &USART_InitStructure);	//��ʼ��USART1
	
	//�ж�����
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;	
	NVIC_InitStructure.NVIC_IRQChannelPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure );
	
	USART_ITConfig( USART1, USART_IT_RXNE, ENABLE );  	//�����ж�ʹ��
	
	USART_Cmd(USART1, ENABLE);//USART1ʹ��
	
	TIM3_Config(); //�������ݰ�������������
}

//USART1�жϴ���
void USART1_IRQHandler(void)
{
	uint16_t isr=USART1->ISR;
	
	//���յ�һ���ֽ�
	if((isr & USART_FLAG_RXNE) == USART_FLAG_RXNE)
	{
		g_RxBuffer[g_RxIndex++]=(uint8_t)(USART1->RDR);
		UartTimerEnable();
		USART_ClearITPendingBit(USART1,USART_FLAG_RXNE);
	}
	/*
	else if((isr & USART_FLAG_TC) == USART_FLAG_TC)//�������
	{
		USART_ITConfig( USART1, USART_IT_TC, DISABLE );	//��������ж�ʹ��
		g_UartSendState=0;
		USART_ClearITPendingBit(USART1,USART_FLAG_TC);
	}*/
	else	if((isr & USART_FLAG_TXE) == USART_FLAG_TXE)//���ͼĴ�����
	{
		if(g_TxSize > g_TxIndex)
			 USART_SendData(USART1, g_TxBuffer[g_TxIndex++]);
		else
		{
			USART_ITConfig( USART1, USART_IT_TXE, DISABLE );//���ͼĴ������жϽ�ֹ
			//USART_ITConfig( USART1, USART_IT_TC, ENABLE );	//��������ж�ʹ��
		}
	}
	else;
}

//�������ݰ�
uint8_t UartWrite(uint8_t *buf,uint8_t len)
{
	uint8_t i;
	
	for(i=0;i<len;i++)
		g_TxBuffer[i]=buf[i];
	g_TxSize=len;
	g_TxIndex=0;

	USART_ITConfig( USART1, USART_IT_TXE, ENABLE); //���ͼĴ������ж�����
	return i;
}

//��ȡ�������н��յ������ݰ�
uint8_t UartRead(uint8_t *buf)
{
	uint8_t i,len;
	
	len=g_RxIndex;
	for(i=0;i<len;i++)
		buf[i]=g_RxBuffer[i];
	
	g_RxIndex=0;
	return len;
}

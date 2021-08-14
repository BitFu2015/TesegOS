//uart.c

#include "uxos.h"

uint8_t g_RxBuffer[128];//接收缓冲区
uint8_t g_TxBuffer[128];//发送缓冲区
uint32_t g_RxIndex;//接收字节索引
uint32_t g_TxSize;//发送长度
uint32_t g_TxIndex;//发送索引

extern UX_EVENT g_Event; //事件对象，用于通知任务接收到数据

//5ms超时定时器配置
void TIM3_Config(void)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  NVIC_InitTypeDef NVIC_InitStructure;

  //TIM3时钟使能
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

  //TIM3中断配置
  NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

	//定时时间计算：((1+TIM_Prescaler)/48M)*(1+TIM_Period) 
	TIM_DeInit(TIM3);            //复位定时器
	TIM_TimeBaseStructure.TIM_Period=48000-1;  //定时器初始值
	TIM_TimeBaseStructure.TIM_Prescaler=4;         //时钟预分频
	TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;  // 时钟分割
	TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up; //向上计数模式
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseStructure);   //初始化定时器的值
	TIM_ClearFlag(TIM3,TIM_FLAG_Update);               //清除定时器中断标志  

  TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);//中断使能
  //TIM_Cmd(TIM3, ENABLE);//定时器使能
}

//TIM3中断处理函数
void TIM3_IRQHandler(void)
{
  TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
	TIM_Cmd(TIM3, DISABLE);//定时器关闭
	uxSetEvent(&g_Event);//通知任务,接收到一数据包
}

void UartTimerEnable(void)
{
	TIM_Cmd(TIM3, DISABLE);
	TIM3->CNT=0;
	TIM_Cmd(TIM3, ENABLE);
	//TIM3->CR1 |= TIM_CR1_CEN;
}

//初始化uart
void UartInit(void)
{
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	
	//状态量初始化
	g_RxIndex=0;
	g_TxIndex=0;
	g_TxSize=0;

	//时钟允许
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
  USART_Init(USART1, &USART_InitStructure);	//初始化USART1
	
	//中断设置
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;	
	NVIC_InitStructure.NVIC_IRQChannelPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure );
	
	USART_ITConfig( USART1, USART_IT_RXNE, ENABLE );  	//接收中断使能
	
	USART_Cmd(USART1, ENABLE);//USART1使能
	
	TIM3_Config(); //接收数据包检测计数器配置
}

//USART1中断处理
void USART1_IRQHandler(void)
{
	uint16_t isr=USART1->ISR;
	
	//接收到一个字节
	if((isr & USART_FLAG_RXNE) == USART_FLAG_RXNE)
	{
		g_RxBuffer[g_RxIndex++]=(uint8_t)(USART1->RDR);
		UartTimerEnable();
		USART_ClearITPendingBit(USART1,USART_FLAG_RXNE);
	}
	/*
	else if((isr & USART_FLAG_TC) == USART_FLAG_TC)//发送完成
	{
		USART_ITConfig( USART1, USART_IT_TC, DISABLE );	//发送完成中断使能
		g_UartSendState=0;
		USART_ClearITPendingBit(USART1,USART_FLAG_TC);
	}*/
	else	if((isr & USART_FLAG_TXE) == USART_FLAG_TXE)//发送寄存器空
	{
		if(g_TxSize > g_TxIndex)
			 USART_SendData(USART1, g_TxBuffer[g_TxIndex++]);
		else
		{
			USART_ITConfig( USART1, USART_IT_TXE, DISABLE );//发送寄存器空中断禁止
			//USART_ITConfig( USART1, USART_IT_TC, ENABLE );	//发送完成中断使能
		}
	}
	else;
}

//发送数据包
uint8_t UartWrite(uint8_t *buf,uint8_t len)
{
	uint8_t i;
	
	for(i=0;i<len;i++)
		g_TxBuffer[i]=buf[i];
	g_TxSize=len;
	g_TxIndex=0;

	USART_ITConfig( USART1, USART_IT_TXE, ENABLE); //发送寄存器空中断允许
	return i;
}

//读取缓冲区中接收到的数据包
uint8_t UartRead(uint8_t *buf)
{
	uint8_t i,len;
	
	len=g_RxIndex;
	for(i=0;i<len;i++)
		buf[i]=g_RxBuffer[i];
	
	g_RxIndex=0;
	return len;
}


#include "delay.h"
#include "usart.h"
#include "stdarg.h"	 	 
#include "stdio.h"	 	 
#include "string.h"	 
//#include "timer.h"
#include "dma.h"
#include "FreeRTOS.h"
#include "gps.h"

u8 USART3_RX_BUF[USART3_MAX_LEN];
// [15]-�������δ�����־
// [14-0]-�����ֽ���
u16 USART3_RX_STA;

void USART3_Init(u32 bound)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
		
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);

	USART_DeInit(USART3);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;					// PB10
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;					// PB11
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	USART_InitStructure.USART_BaudRate = bound;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART3, &USART_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;//configLIBRARY_SYS_INTERRUPT_PRIORITY + 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);				// �����ж�
	USART_Cmd(USART3, ENABLE);
	
	USART_DMACmd(USART3,USART_DMAReq_Rx,ENABLE);				// ʹ�ܴ���3��DMA����
	UART_DMA_Config(DMA1_Channel3,(u32)&USART3->DR,(u32)USART3_RX_BUF, DMA_DIR_PeripheralSRC);
	
//	UART_DMA_Enable(DMA1_Channel3, USART3_MAX_LEN);				// ��ʼ����
	
	USART3_RX_STA = 0;
}

void USART3_IRQHandler(void)
{
	BaseType_t xSwitchRequired = pdFALSE;
	
	if(USART_GetITStatus(USART3, USART_IT_IDLE) != RESET)
	{
		USART_ReceiveData(USART3);
//		USART3_RX_STA = USART3_MAX_LEN - DMA_GetCurrDataCounter(DMA1_Channel3);
		USART3_RX_STA |= 0x8000;
		USART_ClearITPendingBit(USART3, USART_IT_IDLE);         // ����жϱ�־
	}
	
	portEND_SWITCHING_ISR(xSwitchRequired);
}

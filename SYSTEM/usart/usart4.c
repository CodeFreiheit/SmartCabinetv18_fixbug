
#include "delay.h"
#include "usart.h"
#include "stdarg.h"	 	 
#include "stdio.h"	 	 
#include "string.h"	 
//#include "timer.h"
#include "adc.h"
//#include "dma.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "etharp.h"
#include "camera.h"

//#include "gps.h"

u16 Uart4_Rec_Cnt=0;             //��֡���ݳ���	
u8  Uart4_Over=0;//����1����һ֡��ɱ�־λ

//u8 USART3_RX_BUF[USART3_MAX_LEN];
//u8 USART3_TX_BUF[USART3_MAX_LEN];
// [15]-�������δ�����־
// [14-0]-�����ֽ���
//u16 USART3_RX_STA;

u8  DMA_Rece_Buf[DMA_Rec_Len];	   //DMA���մ������ݻ�����

void uart4_init(u32 bound);

void UART4_Init(u32 bound)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;
	
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
//	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4,ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);

	USART_DeInit(UART4);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;					// PC10
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;					// PC11
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	USART_InitStructure.USART_BaudRate = bound;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(UART4, &USART_InitStructure);
	
	USART_ITConfig(UART4, USART_IT_IDLE, ENABLE);				// �����ж�
	
	USART_Cmd(UART4, ENABLE);
	USART_ClearFlag(UART4, USART_FLAG_TC);        // ???(???)
               
	
  //��Ӧ��DMA����
	DMA_DeInit(DMA2_Channel3);   //��DMA��ͨ��5�Ĵ�������Ϊȱʡֵ  ����1��Ӧ����DMAͨ��5
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&UART4->DR;//0x40004C04;//(u32)&UART4->DR;  //DMA����ADC����ַ
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)DMA_Rece_Buf;  //DMA�ڴ����ַ
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;  //���ݴ��䷽�򣬴������ȡ���͵��ڴ�
	DMA_InitStructure.DMA_BufferSize = DMA_Rec_Len;  //DMAͨ����DMA����Ĵ�С
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //�����ַ�Ĵ�������
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //�ڴ��ַ�Ĵ�������
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  //���ݿ��Ϊ8λ
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; //���ݿ��Ϊ8λ
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;//DMA_Mode_Normal;  //��������������ģʽ
	DMA_InitStructure.DMA_Priority =DMA_Priority_High;//DMA_Priority_VeryHigh;//DMA_Priority_Medium; //DMAͨ�� xӵ�������ȼ� 
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  //DMAͨ��xû������Ϊ�ڴ浽�ڴ洫��
	DMA_Init(DMA2_Channel3, &DMA_InitStructure);  //����DMA_InitStruct��ָ���Ĳ�����ʼ��DMA��ͨ��USART1_Tx_DMA_Channel����ʶ�ļĴ���
	

  DMA_ITConfig(DMA2_Channel3, DMA_IT_TC, ENABLE);
	DMA_ITConfig(DMA2_Channel3, DMA_IT_TE, ENABLE);
	USART_DMACmd(UART4,USART_DMAReq_Rx,ENABLE);   //ʹ�ܴ���1 DMA����
	DMA_Cmd(DMA2_Channel3, ENABLE);  //��ʽ����DMA����
	
	
	NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority =7;//3;//7;//1; //7;//1;//3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;//1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

/*���»ָ�DMAָ��*/
  void MYDMA_Enable(DMA_Channel_TypeDef*DMA_CHx)
	{ 
		DMA_Cmd(DMA_CHx, DISABLE );  //�ر�USART1 TX DMA1 ��ָʾ��ͨ��      
		DMA_SetCurrDataCounter(DMA_CHx,DMA_Rec_Len);//DMAͨ����DMA����Ĵ�С
		DMA_Cmd(DMA_CHx, ENABLE);  //ʹ��USART1 TX DMA1 ��ָʾ��ͨ�� 
	} 


void UART4_IRQHandler(void)                	//����1�жϷ������
{
	u8 Res;
	u8 Sync;
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;//
	if(USART_GetITStatus(UART4, USART_IT_IDLE) != RESET)  //�����ж�(���յ������ݱ�����0x0d 0x0a��β)
	{
		Res =USART_ReceiveData(UART4);	//��ȡ���յ�������
		Uart4_Rec_Cnt = DMA_Rec_Len-DMA_GetCurrDataCounter(DMA2_Channel3);	//����ӱ�֡���ݳ���
		//printf("Uart4_Rec_Cnt=%d\n",Uart4_Rec_Cnt);
		Uart4_Over=1;
		//printf("Uart4_Over=%d\n",Uart4_Over);
		USART_ClearITPendingBit(UART4, USART_IT_IDLE); 
		MYDMA_Enable(DMA2_Channel3);                   //�ָ�DMAָ�룬�ȴ���һ�εĽ���
		if((DMA_Rece_Buf[0]==0xAA&&DMA_Rece_Buf[1]==0x0E)&&(DMA_Rece_Buf[2]==0x0D)&&(DMA_Rece_Buf[4]==0x00&&DMA_Rece_Buf[5]==0x00) \
		&&DMA_Rece_Buf[6]==0xAA&DMA_Rece_Buf[7]==0x0D&DMA_Rece_Buf[8]==0x00&DMA_Rece_Buf[9]==0x00&DMA_Rece_Buf[10]==0x00&DMA_Rece_Buf[11]==0x00)  //��ʼ��ͬ�����
		{
		 Sync=xSemaphoreGiveFromISR(BinarySemaphore_camera_Sync, &xHigherPriorityTaskWoken);		
		}
		if((DMA_Rece_Buf[0]==0xAA&&DMA_Rece_Buf[1]==0x0A)&&(DMA_Rece_Buf[2]==0x05))//���ճɹ�
		{		
		 xSemaphoreGiveFromISR(BinarySemaphore_camera_data_response, &xHigherPriorityTaskWoken);	
		}
  } 
}


#if 0
void UART4_Init(u32 bound){
   //GPIO�˿�����
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
//	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC,ENABLE); //ʹ��GPIOCʱ��
//	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4,ENABLE);//ʹ��USART1ʱ��
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4,ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);
	
	//RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC,ENABLE); //ʹ��GPIOCʱ��
	//USART_DeInit(UART4);
 
	//����1��Ӧ���Ÿ���ӳ��
//	GPIO_PinAFConfig(GPIOC,GPIO_PinSource10,GPIO_AF_UART4); //GPIOA9����ΪUSART1
//	GPIO_PinAFConfig(GPIOC,GPIO_PinSource11,GPIO_AF_UART4); //GPIOA10����ΪUSART1
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;					// PC10
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;					// PC11
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//GPIO_Mode_IPU;// GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
  //USART1�˿�����
 

   //USART1 ��ʼ������
	USART_InitStructure.USART_BaudRate = bound;//����������
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ
  USART_Init(UART4, &USART_InitStructure); //��ʼ������1
	
  USART_Cmd(UART4, ENABLE);  //ʹ�ܴ���4 
	USART_ClearFlag(UART4,USART_FLAG_TC);
	/*�򿪿����ж�*/
	USART_ITConfig(UART4,USART_IT_IDLE,ENABLE);
	/*�򿪽����ж�*/
	USART_ITConfig(UART4,USART_IT_RXNE,ENABLE);
	
	
	//USART_ClearFlag(USART1, USART_FLAG_TC);
	
#if 1
	//USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);//��������ж�

	//Usart1 NVIC ����
  NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;//����1�ж�ͨ��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=15;//7;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =0;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ�����

#endif	
}
#endif


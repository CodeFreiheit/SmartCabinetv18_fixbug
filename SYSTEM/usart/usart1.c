#include "sys.h"
#include "usart.h"
#include "message.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "camera.h"

struct _Usart_t UART1;
u8  Usart1_Over=0;//����1����һ֡��ɱ�־λ

//extern xSemaphoreHandle USART1_Sem;
extern xQueueHandle MainTaskQueue;

//�������´���,֧��printf����,������Ҫѡ��use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
void _sys_exit(int x) 
{ 
	x = x; 
} 
//�ض���fputc���� 
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0);//ѭ������,ֱ���������   
    USART1->DR = (u8) ch;      
	return ch;
}

_ttywrch(int ch) 
{ 
ch = ch; 
} 
#endif 

/*ʹ��microLib�ķ���*/
 /* 
int fputc(int ch, FILE *f)
{
	USART_SendData(USART1, (uint8_t) ch);

	while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET) {}	
   
    return ch;
}
int GetKey (void)  { 

    while (!(USART1->SR & USART_FLAG_RXNE));

    return ((int)(USART1->DR & 0x1FF));
}
*/
  
void USART1_Init(u32 bound)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	// ��ʼ�����ͻ���������
	UART1.TxWr = 0;
	UART1.TxRd = 0;
	UART1.TxBufLen = 0;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);
  
	// USART1_TX		GPIOA.9
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
   
	// USART1_RX		GPIOA.10
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	// Usart1 NVIC
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;//15;//configLIBRARY_SYS_INTERRUPT_PRIORITY + 3;//15;//configLIBRARY_SYS_INTERRUPT_PRIORITY + 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
  
	// USART
	USART_InitStructure.USART_BaudRate = bound;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure);
	
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	USART_Cmd(USART1, ENABLE);
}

/*--------------------------------------------------------------------------+
| General Subroutines                                                       |
+--------------------------------------------------------------------------*/
void USART1_PutChar(u16 cOutChar)
{
	while (UART1.TxBufLen >= USART_TX_BUF_LENGTH)vTaskDelay(1);

	taskENTER_CRITICAL();
	{
		UART1.TxBuffer[UART1.TxWr++] = cOutChar;
		if (UART1.TxWr >= USART_TX_BUF_LENGTH)
			UART1.TxWr = 0;
		UART1.TxBufLen ++;
		USART1->CR1 |= (1 << 7);		// USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
	}
	taskEXIT_CRITICAL();
}

void SerialPutChar(u16 cOutChar)
{
	if (cOutChar != '\r')
	{
		if (cOutChar == '\n')USART1_PutChar('\r');	
		USART1_PutChar(cOutChar);
	}
}

void SerialPutString(const u16 *pcString, int usStringLength)
{
	char *pxNext;

	/* A couple of parameters that this port does not use. */
	(void) usStringLength;

	/* Send each character in the string, one at a time. */
	pxNext = (portCHAR *) pcString;
	while (*pxNext)
	{
		if (*pxNext != '\r')
		{
			if (*pxNext == '\n')
				USART1_PutChar('\r');
			USART1_PutChar(*pxNext);
		}

		pxNext++;
	}
}

void USART1_IRQHandler(void)                	//����1�жϷ������
{
	u16 cChar;
	MSG  msg;
	BaseType_t xSwitchRequired = pdFALSE;
	static BaseType_t xHigherPriorityTaskWoken;

	// ��������жϣ�����һ����Ҫ���͵��ֽ��ͳ�ȥ
	if (USART_GetITStatus(USART1, USART_IT_TXE) != RESET)
	{
		if (UART1.TxBufLen > 0)
		{
			USART1->DR = UART1.TxBuffer[UART1.TxRd++];
			if (UART1.TxRd >= USART_TX_BUF_LENGTH)
				UART1.TxRd = 0;
			UART1.TxBufLen --;
		}
		else
		{
			USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
		}
	}

	// ��������жϣ������յ����ֽڷ��������
	if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
	{
		u8 test=3;
		cChar = USART_ReceiveData(USART1);
		Usart1_Over=1;
		//printf("recv data......\n");
   // test=xSemaphoreGiveFromISR(BinarySemaphore_photo_command,&xHigherPriorityTaskWoken);
		//printf("test=%d\n",test);
//		if (MainTaskQueue != NULL)
//		{
//			msg.Msg = MSG_UART1_RX_BYTE;
//			msg.wParam = cChar;

			//xQueueSendFromISR(MainTaskQueue, &msg, &xSwitchRequired);
//		}
	}

	 portEND_SWITCHING_ISR(xSwitchRequired);
	//portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
}

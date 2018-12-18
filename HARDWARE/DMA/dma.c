#include "dma.h"

void UART_DMA_Config(DMA_Channel_TypeDef *DMA_CHx, u32 cpar, u32 cmar, u32 cdir)
{
	DMA_InitTypeDef DMA_InitStructure;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1|RCC_AHBPeriph_DMA2, ENABLE);
	DMA_DeInit(DMA_CHx);
	DMA_InitStructure.DMA_PeripheralBaseAddr = cpar;
	DMA_InitStructure.DMA_MemoryBaseAddr = cmar;
	DMA_InitStructure.DMA_DIR = cdir;//DMA_DIR_PeripheralDST;				// ���ݴ��䷽��
	DMA_InitStructure.DMA_BufferSize = 0;									// UART_DMA_Enable()�������ó���
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;		// �����ַ�Ĵ�������
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;					// �ڴ��ַ�Ĵ�������
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;	// �������ݿ��Byte
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;			// �ڴ����ݿ��Byte
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;							// ��ѭ��ģʽ
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;					// CHxӵ�������ȼ�
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;							// ���ڴ浽�ڴ洫��
	DMA_Init(DMA_CHx, &DMA_InitStructure);
}

void UART_DMA_Enable(DMA_Channel_TypeDef *DMA_CHx, u16 len)
{
	DMA_Cmd(DMA_CHx, DISABLE );
	DMA_SetCurrDataCounter(DMA_CHx,len);
	DMA_Cmd(DMA_CHx, ENABLE);
}

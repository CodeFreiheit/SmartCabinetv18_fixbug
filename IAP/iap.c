
#include "iap.h"
#include "sys.h"
#include "stmflash.h"

iapfun  jump2app;

//��ת��Ӧ�ó����
//appxaddr:�û�������ʼ��ַ.
void iap_load_app(u32 appxaddr)
{
	if(((*(vu32*)appxaddr)&0x2FFE0000)==0x20000000)	//���ջ����ַ�Ƿ�Ϸ�.
	{
		jump2app = (iapfun)*(vu32*)(appxaddr+4);		//�û��������ڶ�����Ϊ����ʼ��ַ(��λ��ַ)		
		MSR_MSP(*(vu32*)appxaddr);					    //��ʼ��APP��ջָ��(�û��������ĵ�һ�������ڴ��ջ����ַ)
		jump2app();									    //��ת��APP.
	}
}


//appxaddr:Ӧ�ó�����ʼ��ַ
//appbuf:Ӧ�ó���code
//appsize:Ӧ�ó����С���ֽڣ�
void iap_write_appbin(u32 appxaddr, FIL *fp)
{
	u32 APP_Sector = 0;
	u16 APP_Byte = 0;
	u16 ReadNum;
	u32 i = 0;
	u16 j = 0;
//	u8 ReadAppBuffer[512];
//	u16 ChangeBuffer[256];
	
	u8 ReadAppBuffer[2048];	
  u16 ChangeBuffer[1024];
	
//	APP_Sector = fp->fsize/512;
//	APP_Byte = fp->fsize%512;
	
	APP_Sector = fp->fsize/2048;
	APP_Byte = fp->fsize%2048;
	
	
	
	
	for(i=0; i<APP_Sector; i++)
	{
		f_read(fp, ReadAppBuffer, 2048, (UINT *)&ReadNum);
		//for(j=0; j<256; j++)ChangeBuffer[j] = (ReadAppBuffer[j*2+1]<<8) + ReadAppBuffer[j*2];
		for(j=0; j<1024; j++)ChangeBuffer[j] = (ReadAppBuffer[j*2+1]<<8) + ReadAppBuffer[j*2];
		FlashWrite(appxaddr+i*2048, ChangeBuffer, 1024);	 
	}
	if(APP_Byte != 0)
	{
		f_read(fp, ReadAppBuffer, APP_Byte, (UINT *)&ReadNum);
		for(j=0; j<(APP_Byte/2); j++)ChangeBuffer[j] = (ReadAppBuffer[j*2+1]<<8) + ReadAppBuffer[j*2];
		FlashWrite(appxaddr+i*2048, ChangeBuffer, APP_Byte/2);
	}
}




////����һ������
////Dst_Addr:������ַ 0~511 for w25x16
////����һ������������ʱ��:150ms
//void SPI_Flash_Erase_Sector(u32 Dst_Addr)   
//{   
//	Dst_Addr*=4096;
//	SPI_FLASH_Write_Enable();                  //SET WEL 	 
//	SPI_Flash_Wait_Busy();   
//	SPI_FLASH_CS=0;                            //????   
//	SPI1_ReadWriteByte(W25X_SectorErase);      //???????? 
//	SPI1_ReadWriteByte((u8)((Dst_Addr)>>16));  //??24bit??    
//	SPI1_ReadWriteByte((u8)((Dst_Addr)>>8));   
//	SPI1_ReadWriteByte((u8)Dst_Addr);  
//	SPI_FLASH_CS=1;                            //????     	      
//	SPI_Flash_Wait_Busy();   				   //??????
//}  

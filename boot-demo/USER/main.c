
#include "sys.h"
#include "delay.h"
#include "key.h"
#include "usart.h" 
#include "malloc.h"
#include "w25qxx.h"
#include "mass_mal.h"
#include "usb_lib.h"
#include "hw_config.h"
#include "usb_pwr.h"
#include "memory.h"
#include "usb_bot.h"
#include "exfuns.h"
#include "ff.h"
#include "iap.h"
#include "stmflash.h"

extern u8 Max_Lun;				//֧�ֵĴ��̸���,0��ʾ1��,1��ʾ2��.

 int main(void)
 {
	u8 res = 0;
	u8 key = 0;
	 
	delay_init();
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	KEY_Init();
	uart_init(115200);
	W25QXX_Init();
	my_mem_init(SRAMIN);			//��ʼ���ڲ��ڴ��
	exfuns_init();						//Ϊfatfs��ر��������ڴ�
	res = f_mount(fs[1],"1:",1);	//����FLASH.
	if(res == 0X0D)res = f_mkfs("1:",0,4096);//��ʽ��FLASH
	delay_ms(1800);

#if 1
	f_open(file, "1:RESET", FA_READ|FA_OPEN_ALWAYS);
	f_close (file);
	key = KEY_Scan(0);
	  if(f_open(file, "1:post.bin", FA_READ|FA_OPEN_EXISTING) == 0)
   	{	
      printf("post.bin  app loading....\n");			
	  	iap_write_appbin(FLASH_APP_ADDR,file);
			iap_load_app(FLASH_APP_ADDR);		
		}
		else if(f_open(file, "1:bin_before.bin", FA_READ|FA_OPEN_EXISTING) == 0)
		{
			printf("bin_before.bin  app loading....\n");	
			iap_write_appbin(FLASH_APP_ADDR,file);
		//	printf("************\n");
			iap_load_app(FLASH_APP_ADDR);
      printf("$$$$$$$$$$$\n");			
		}
		else if(f_open(file, "1:INIT", FA_READ|FA_OPEN_EXISTING) == 0)
		{
			iap_write_appbin(FLASH_APP_ADDR,file);
			iap_load_app(FLASH_APP_ADDR);
		}
	
	
#endif
	
	USB_Port_Set(0); 	//USB�ȶϿ�
	delay_ms(700);
	USB_Port_Set(1);	//USB�ٴ����� 
	Data_Buffer = mymalloc(SRAMIN, BULK_MAX_PACKET_SIZE*2*4);	//ΪUSB���ݻ����������ڴ�
	Bulk_Data_Buff = mymalloc(SRAMIN, BULK_MAX_PACKET_SIZE);	//�����ڴ�
 	//USB����
 	USB_Interrupts_Config();    
 	Set_USBClock();
 	USB_Init();	    
	delay_ms(1800);	   	    
	while(1)
	{
	};
}

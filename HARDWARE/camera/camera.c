#include "camera.h"
#include "MQTTPacket.h"
#include "StackTrace.h"
#include "stm32f10x_it.h" 

#include <string.h>
#include "mqtt.h"
#include "usart.h"
#include "tcp.h"
#include "MQTTPacket.h"
#include "MQTTGPRSClient.h"
#include "dma.h"
#include "mqtt.h"
#include "task.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "udp_demo.h" 
#include "netif/etharp.h"
#include "httpd.h"
#include "mqtt.h"
#include "exfuns.h"
#include "ff.h"
#include "usart.h"
#include "tcp_client_demo.h" 
#include "message.h"
#include "delay.h"
#include "rtc.h"
#include "fattester.h"
#include "sensor.h"
#include "usart.h"
#include "print.h"
#include "tcp_server_demo.h"
#include "semphr.h"
#include "iwdg.h"


#include "diskio.h"
#include "message.h"
#include "ff.h"
#include "mass_mal.h"
#include "diskio.h"	
#include "exfuns.h"
#include "usb_prop.h"

#include "hw_config.h"
#include "memory.h"
#include "usb_init.h"




//void* BinarySemaphore_mac;	//��ֵ�ź������  ���ݽ����ź���
//void* BinarySemaphore_mac_config;
void* BinarySemaphore_camera;//����ͷץ���ź���
void* BinarySemaphore_camera_Sync;
void* BinarySemaphore_camera_response;
void* BinarySemaphore_response;
void* BinarySemaphore_camera_data_response;
void* BinarySemaphore_photo_command;
void* test_trap_signal;
void* test_tcp_signal;
void* timestamp_signal;//ʱ����ź���

void* ping_signal;

void* close_tcp_server;
void* open_tcp_server;

void* cameraTask_Handler;
void* WDG_Sem;//���Ź����

void* tcp_command;
void* mqtt_command;

/* �����ź���  for sd write*/
SemaphoreHandle_t sd_Sem = NULL;

TaskHandle_t ServerTask_Handler;

xQueueHandle USB_Queue;

extern xQueueHandle MainTaskQueue;
extern TaskHandle_t TickTask_Handler;
extern TaskHandle_t MainTask_Handler;
extern TaskHandle_t SDTask_Handler;

extern TaskHandle_t PingTask_Handler;


u8 camera_Sync_command[6]={0xAA,0x0D,0x00,0x00,0x00,0x00}; //����ͷͬ������
u8 camera_response_command[6]={0xAA,0x0E,0x0D,0x00,0x00,0x00}; //����ͷͬ������

u8 photo_command_array[6]={0xAA,0x04,0x05,0x00,0x00,0x00}; //����ͷ����ָ��

u8 get_camera_data_command[6]={0xAA,0x0E,0x00,0x00,0x00,0x00}; //ȡͼ����������

u8 end_command[6]={0xAA,0x0E,0x00,0x00,0xF0,0xF0}; //��������

xQueueHandle MsgQueue;  

xQueueHandle Msg_response; 

extern TaskHandle_t USBTask_Handler;
extern TaskHandle_t MqttTask_Handler;

//char  temp_str[17];
char  temp_str[50];

/*
 * ��������void camera_task(void *pvParameters)
 * ����  ������ͷ����
 * ����  ����
 * ���  ����	
 */

void camera_task(void *pvParameters)
{
	u8 ret1,res;
	int  camera_data_len,last_camera_data_len;
	int  count_packet;
	static int count_camera;
	u16 Count=0;
	static int count_tcp;
	Task_MSG msg;
  
	while(1)
	{	
	 printf("camera task\n");
	 UART_Send(camera_Sync_command,6);	
	 memset(DMA_Rece_Buf,0,512);	 	
	 if(xSemaphoreTake(BinarySemaphore_photo_command, portMAX_DELAY)==pdTRUE)
	 {
		 static u8 i;	 
		 printf("the door have been opend\n");			
		 
		 vTaskSuspend(SDTask_Handler);		 
		 vTaskSuspend(MqttTask_Handler);		 
		 //vTaskSuspend(USBTask_Handler);
		 vTaskSuspend(MainTask_Handler);
		 
		 vTaskSuspend(PingTask_Handler);
		 
		 portDISABLE_INTERRUPTS(); //���ж�
		     		 
		 USB_Port_Set(0);
		 
     res=Camera_init(); //��ʼ���ɹ�	
		 for(i=0;i<3;i++)
		 {		 
			UART_Send(photo_command_array,6);	//������������		     	
			while(1)
			{
			 if(xSemaphoreTake(BinarySemaphore_camera_data_response, portMAX_DELAY)) //���ճɹ�
			 {	            				
				 Uart4_Over=0;		
				 camera_data_len=DMA_Rece_Buf[5]*65536+DMA_Rece_Buf[4]*256+DMA_Rece_Buf[3];//�õ����ݳ���
				 printf("camera_data_len=%d\n",camera_data_len);
				 last_camera_data_len=camera_data_len%512;
				 printf("[*]last_camera_data_len=%d\n",last_camera_data_len);
				
				 if(last_camera_data_len>0)
				 count_packet=camera_data_len/512+1;
				 else  count_packet=camera_data_len/512;				 
				 break;						 
			 }				
			} 		
			UART_Send(get_camera_data_command,6);	//ȡͼ����������
      printf("count_packet=%d\n",count_packet);				 
			while(count_packet)   //��ַ�ǲ���������  ÿ�η�������ĵ�ַ
			{      			
			 if(Uart4_Over==1)
			 {
				  Uart4_Over=0;
					if(Camera_check())
					{
						int data_len;										
						if(Count==0)
						{	
							timestamp_flag=timestamp_flag|(1<<7);//�ȷ�ʱ���
							vTaskSuspend(cameraTask_Handler);						
						}			
						data_len=(DMA_Rece_Buf[3]<<8)|(DMA_Rece_Buf[2]&0x00ff);
						printf("data_len=%d\n",data_len);
						tcp_client_flag=tcp_client_flag|(1<<7);//����ͼ������					
						vTaskSuspend(cameraTask_Handler);						
					  count_packet--;	         	
					}
					else
					{
						Count=Count-1;
            if(Count>50)
						{
						 break;
						}							
					}					 				 
				 Count++;
				 printf("Count=%u\n",Count);
				 get_camera_data_command[4]=Count&0xff;//0x01;//j;//��λ
				 get_camera_data_command[5]=(Count>>8)&0xff;//0x00;//j>>8;//��λ	
         if(count_packet>0)	
				 {
				  UART_Send(get_camera_data_command,6);	
				 }
         else
				 {
				  UART_Send(end_command,6);	
					get_camera_data_command[4]=0x00;
					get_camera_data_command[5]=0x00;
				 }					 				
			 }					 
			}			
     Count=0;	  			
		 tcp_close_flag=tcp_close_flag|(1<<7);
			
		 vTaskSuspend(cameraTask_Handler);//�ȹر���TCP�����ٻָ� camera����ִ��		
		 Uart4_Over=0;		
     memset(DMA_Rece_Buf,0,512);
		}
		 
		
	  portENABLE_INTERRUPTS ();//���ж�
		
    vTaskResume(SDTask_Handler);  
    vTaskResume(MqttTask_Handler);		 
		vTaskResume(MainTask_Handler);
		vTaskResume(PingTask_Handler);
		
		 USB_Port_Set(1);
		 USB_Init();		
  }		
 }
}


/*
 * ��������u8 Camera_init(void)
 * ����  ��
 * ����  ����
 * ���  ����	
 */
u8 Camera_init(void)
{
	static int camera_count=0;
	static int count=0;	
	while(1)
	{
		Uart4_Over=0;	
    UART_Send(camera_Sync_command,6);		
		if(xSemaphoreTake(BinarySemaphore_camera_Sync, portMAX_DELAY)) 
		{
		 memset(DMA_Rece_Buf,0,50);
		 printf("Sync sucess\r\n");
		 return 1;//ͬ���ɹ�
		}	
    else
	  {
	   UART_Send(camera_Sync_command,6);
		 camera_count++;
	  }	
		if(camera_count>50)
	  {
		 printf("Sync erro\n");
	   return 0;//ͬ��ʧ��
	  }
	} 
}
/*
 * ��������u8 Camera_check(void)
 * ����  ��ͼ������У��
 * ����  ����
 * ���  ����	
 */
u8 Camera_check(void)
{
	int i;
	u8 sum=0; 
	for(i=0;i<Uart4_Rec_Cnt-2;i++)
	{
	sum=DMA_Rece_Buf[i]+sum;
	}
	if(sum==DMA_Rece_Buf[Uart4_Rec_Cnt-2])
	{
	return 1;//У����ȷ
	}
	else
	{
	return 0;//У�����
	}  
}

/*
 * ��������void UART_Send(u8 *str,u8 len3)
 * ����  ������ͬ������ͷ����
 * ����  ����
 * ���  ����	
 */
void UART_Send(u8 *str,u8 len3)
{
	 u8 i;
	 //while(USART_GetFlagStatus(UART4, USART_FLAG_TC) == RESET);	
   for(i=0;i<len3;i++)
	{
  	 USART_SendData(UART4, *(str+i));
	   while(USART_GetFlagStatus(UART4,USART_FLAG_TC)==RESET);  //while���ʱ��ȵĺܾ����ݻ�û����ȥ  �Ѿ�����һ���ж���
	}
}


/*
 * ��������u8 camera_write_sd_test(void)
 * ����  ��ͼ������д��SD��
 * ����  ����
 * ���  ����	
 */
void write_sd_test(char j)
{ 
	u8 res,i,n4=10;
	FIL	*file_camera;
	char test_buf[17];
	char test_read_buf[10];
	u32 timestamp_buf[10];
	char  str[17];
	char calendar_buf[50];
	int sd_data_len;
	u8 str_len;
	sd_data_len=(DMA_Rece_Buf[3]<<8)|(DMA_Rece_Buf[2]&0x00ff);	
	if(j==0)
	{		
	 time_to_sd_path(calendar_buf);
	 str_len=strlen(calendar_buf);
   memcpy(temp_str,calendar_buf,str_len);			
	}
  if(j==10)
	{	
		file_camera=(FIL*)pvPortMalloc(1000);//�����ڴ�
		{	
			printf("temp_str=%s\n",temp_str);
			res = f_open(file_camera,temp_str,FA_OPEN_ALWAYS|FA_WRITE|FA_READ);		
			if(res!=FR_OK)
			{ 
			 printf("open test3 error!\r\n");
			}
			res=f_lseek(file_camera, file_camera->fsize);			
			n4=f_write(file_camera, &DMA_Rece_Buf[4],sd_data_len,&br);//
			vPortFree(file_camera);
			f_close(file_camera);	
			memset(DMA_Rece_Buf,0,512);		
		}	
	}	
}
/*
 * ��������image_rename(void)
 * ����  ��ͼƬ������
 * ����  ����
 * ���  ����	
 */
void image_rename(void)
{
	u32 timestamp_buf[10];
	char  new_image[40];	
	timestamp_buf[0]=10;//RTC_GetTime(NULL);	
	snprintf(new_image,18, "%d:%d.%s",0,timestamp_buf[0],"jpeg");
	f_rename("0:camera80.jpeg",new_image);			
}

/*
 * ��������char* time_to_timestamp(void)
 * ����  ������ʱ������ʱ�������
 * ����  ����
 * ���  ����	
 */
char* time_to_timestamp(void)
{
	static u32 RtcCounter = 0;
	static u32 RtcCounter2 = 0;
	Rtc_Time RTC_DateStruct;
	struct _date_t  RTC_date_t;
	char timestamp_buf[20];
	char test_buf[100];
	char *timestamp;
	timestamp =timestamp_buf;
	RtcCounter=RTC_GetTime(&RTC_DateStruct);
  printf("RtcCounter=%d",RtcCounter);
  RtcCounter2=_mktime(&RTC_DateStruct);
	return timestamp;
}


/*
 * ��������void time_to_sd_path(void)
 * ����  ��ʱ��ת��Ϊ sdͼƬ·��
 * ����  ����
 * ���  ����	
 */
void time_to_sd_path(char *buff)
{
	char test_buf[100];
	RTC_Get();//����ʱ��	
	sprintf(buff, "%d:%d_%d_%d_%d_%d_%d.%s",0,calendar.w_year,calendar.w_month,calendar.w_date,
	calendar.hour,calendar.min,calendar.sec,"jpeg");	
}
/*
 * ��������USB_Close(void)
 * ����  ��
 * ����  ����
 * ���  ����	
 */
void USB_Close(void)
{
	xEventGroupSetBits(xCreatedEventGroup,USB_ClOSE_BIT);   	
}

/*
 * ��������USB_Open(void)
 * ����  ��
 * ����  ����
 * ���  ����	
 */
void USB_Open(void)
{
	xEventGroupClearBits(xCreatedEventGroup, USB_ClOSE_BIT); 	
}


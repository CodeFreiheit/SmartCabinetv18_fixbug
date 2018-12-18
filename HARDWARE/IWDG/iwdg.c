#include "iwdg.h"
#include "STM32F10x_IWDG.h"
//#include "portmacro.h"
#include "FreeRTOS.h"
#include "event_groups.h"

#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "message.h"
#include "camera.h"
#include "delay.h"

static u8 Flag_camera=10;
static int count;
static u8 Flag_wdg=1;

/* �����¼��� */
EventGroupHandle_t xCreatedEventGroup;

extern TaskHandle_t USBTask_Handler;
extern TaskHandle_t MqttTask_Handler;



//ι�������Ź�
void IWDG_Feed(void)
{   
 	IWDG_ReloadCounter();	 					   
}

/**
  * @brief  ���Ź�����  
  * @param  argument
  * @retval None
  */
void vTaskWDG(void * argument)
{
	static TickType_t	xTicksToWait = 100 / portTICK_PERIOD_MS*50; /* ����ӳ�5s */
	EventBits_t			uxBits;
	
/*
	0:Running
	1:Ready ����
	2:block ����
*/	
	if(xCreatedEventGroup == NULL)
	{
	 printf("xCreatedEventGroup failed\n");		/* û�д����ɹ�ʧ�ܻ��� */
	 return;
	}
  
	while(1)  //���MQTT���ӷ�����
	{
//	 /* �ȴ����е��������¼� */
	uxBits = xEventGroupWaitBits(xCreatedEventGroup, /* �¼���־���� */
										 TASK_BIT_MQTT,		/* �ȴ�WDG_BIT_TASK_ALL������ */
										 pdTRUE,				 /* �˳�ǰTASK_BIT_ALL�������������TASK_BIT_ALL�������òű�ʾ���˳���*/
										 pdTRUE,				 /* ����ΪpdTRUE��ʾ�ȴ�TASK_BIT_ALL��������*/
										 xTicksToWait);			 /* �ȴ��ӳ�ʱ�� */
	 printf("uxBits_MQTT=%04x\n",uxBits);	
	 if(((uxBits&0xff) & (1<<5)))
	 {
	  break;
	 }
	 IWDG_Feed();
	}
	
  while(1)
	{
		/* �ȴ����е��������¼� */
		uxBits = xEventGroupWaitBits(xCreatedEventGroup, /* �¼���־���� */
							         TASK_BIT_ALL,		/* �ȴ�WDG_BIT_TASK_ALL������ */
							         pdTRUE,				 /* �˳�ǰTASK_BIT_ALL�������������TASK_BIT_ALL�������òű�ʾ���˳���*/
							         pdTRUE,				 /* ����ΪpdTRUE��ʾ�ȴ�TASK_BIT_ALL��������*/
							         xTicksToWait);			 /* �ȴ��ӳ�ʱ�� */
		printf("uxBits=%04x\n",uxBits);		
		//printf("camera=%d\n",eTaskGetState(cameraTask_Handler));
	 
	  xEventGroupClearBits(xCreatedEventGroup, TASK_BIT_Clear);
		
   //if((uxBits & (1<<9))==0)  //��ʼ���  camera  
	 if(GPIO_ReadOutputDataBit(GPIOG,GPIO_Pin_15)==0) //
	 {
		uxBits|=(1<<0);//camera
    uxBits|=(1<<1);//usb
		uxBits|=(1<<2);//sd
		uxBits|=(1<<5);//mqtt
    printf("uxBits_begin_camera=%04x\n",uxBits);				 
		count++;
		if(count>10)
		{
		 Flag_wdg=0;//ֹͣι��
		}
    if(Flag_wdg==1)
		{
		  if(((uxBits&0xff)&TASK_BIT_ALL) == 0xff)
			{
				IWDG_Feed();
				printf("camera begin\n");
			}				
		}						 			 
	 }
   //if((uxBits & (1<<9)))  //ֹͣ��� camera  
	 if(GPIO_ReadOutputDataBit(GPIOG,GPIO_Pin_15)==1)
	 {
		uxBits|=(1<<0);//camera
    uxBits|=(1<<1);//usb
    count=0;	
    printf("uxBits_stop_camera=%04x\n",uxBits);		 
		if(((uxBits&0xff)&TASK_BIT_ALL)==0xff)
		{
			IWDG_Feed();
			printf("camera and usb block\n");
		}	 
	 } 
 }
}

		
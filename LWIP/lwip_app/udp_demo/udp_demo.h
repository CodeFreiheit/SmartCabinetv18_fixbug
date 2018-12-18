#ifndef __UDP_DEMO_H
#define __UDP_DEMO_H
#include "sys.h"
#include "lwip_comm.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"
#include "task.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK ս�������� V3
//UDP ���Դ���	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2015/3/15
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved									  
//*******************************************************************************
//�޸���Ϣ
//��
////////////////////////////////////////////////////////////////////////////////// 	   
 
#define UDP_DEMO_RX_BUFSIZE		2000	//����udp���������ݳ��� 
#define UDP_DEMO_PORT			161//8089	//����udp���ӵĶ˿� 

 
void udp_demo_test(void);
void udp_demo_recv(void *arg,struct udp_pcb *upcb,struct pbuf *p,struct ip_addr *addr,u16_t port);
void udp_demo_senddata(struct udp_pcb *upcb);
void udp_demo_connection_close(struct udp_pcb *upcb);
extern u32 data_len_snmp;//���ڽ��յ����� ����snmp
extern void* BinarySemaphore_udp;	//��ֵ�ź������  UDP���ݽ����ź���
extern u8  Flag_test;//��������  ����UDP���� ��־λ
extern void* LED1Task_Handler;
extern void* mainTask_Handler;
extern void* cameraTask_Handler;
extern struct udp_pcb *udppcb;  	//����ǰ���� ��������

extern TaskHandle_t macTask_Handler;//MAC ����task���


#endif


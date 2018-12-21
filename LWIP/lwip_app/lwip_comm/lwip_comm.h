#ifndef _LWIP_COMM_H
#define _LWIP_COMM_H 
#include "dm9000.h" 

#define LWIP_MAX_DHCP_TRIES		4   //DHCP������������Դ���
#define LINK_UP		0
#define LINK_DOWN	1

//lwip���ƽṹ��
typedef struct  
{
	u8 mac[6];      //MAC��ַ
	u8 mqttip[4];	//Զ������IP��ַ
	u8 remoteip[4];	//Զ������IP��ַ
	u16 mqttport;
	u8 snmpip[4];	//Զ������IP��ַ
	u16 snmpport; 
	u8 servip[4];	
	u16 servport; 
	u8 ip[4];       //����IP��ַ
	u8 netmask[4]; 	//��������
	u8 gateway[4]; 	//Ĭ�����ص�IP��ַ
	
	vu8 dhcpstatus;	//dhcp״̬ 
					//0,δ��ȡDHCP��ַ;
					//1,����DHCP��ȡ״̬
					//2,�ɹ���ȡDHCP��ַ
					//0XFF,��ȡʧ��.
}__lwip_dev;
extern __lwip_dev lwipdev;	//lwip���ƽṹ��
extern struct netif lwip_netif;				//����һ��ȫ�ֵ�����ӿ�
extern u8 linkState;
extern int  Flag;//�ж����ݽ��ձ�־
extern char COM_MAC_BUF[10][18];

void lwip_pkt_handle(void);
void lwip_periodic_handle(void);
void lwip_comm_dhcp_creat(void);//DHCP
	
void lwip_comm_default_ip_set(__lwip_dev *lwipx);
u8 lwip_comm_mem_malloc(void);
void lwip_comm_mem_free(void);
u8 lwip_comm_init(void);
void lwip_dhcp_process_handle(void);
void lwIPInit(void *pvParameters);

extern void lwip_periodic_handle(void);

//extern TaskHandle_t LWIPTask_Handler;//

#endif














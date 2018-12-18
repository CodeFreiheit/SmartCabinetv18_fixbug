#ifndef __SENSOR_H
#define __SENSOR_H	 
#include "sys.h"
#include "led.h"




// PB5	��������ʹ��ϵͳ12V
#define EQU_SYS12V_EN()		{PBout(5) = 1;}
#define EQU_SYS12V_DIS()	{PBout(5) = 0;}
// PB6	��������ʹ��UPS12V
#define EQU_UPS12V_EN()		{PBout(6) = 1;}
#define EQU_UPS12V_DIS()	{PBout(6) = 0;}

#define LED1()	{PAout(1) = 0;}







// PG2	��Դ1  ֱ��12V
#define SYS12_SENSOR		PGin(12)
#define SYS12_ON	1
#define SYS12_OFF	0
// PD3	��Դ2
#define EQU12_SENSOR		PDin(3)
#define EQU12_ON	1
#define EQU12_OFF	0
// PD6	UPS��Դ���
#define EQU24_SENSOR		PDin(6)
#define EQU24_ON	1
#define EQU24_OFF	0
// PC4	���12V���
#define BAT12_SENSOR		PCin(4)
#define BAT12_ON	1
#define BAT12_OFF	0





// PE6	AC1-220V�������  L1
#define AC1_SENSOR()		PEin(6)
#define OUT_AC1_220V_ON()		{PEout(6) = 0;}
#define OUT_AC1_220V_OFF()	{PEout(6) = 1;}

// PF7	AC2-220V�������  L2
#define AC2_SENSOR()		PFin(7)
#define OUT_AC2_220V_ON()		{PFout(7) = 0;}
#define OUT_AC2_220V_OFF()	{PFout(7) = 1;}

// PF9	AC3-220V�������  L3
#define AC3_SENSOR()		PFin(9)
#define OUT_AC3_220V_ON()		{PFout(9) = 0;}
#define OUT_AC3_220V_OFF()	{PFout(9) = 1;}

// PE5	���ȿ���
#define out_fan_SENSOR()		PEin(5)
#define out_fan_ON()		{PEout(5) = 0;}
#define out_fan_OFF()	  {PEout(5) = 1;}


//C7  C8
// PE4   ��� DC1
#define DC1_SENSOR()		PEin(4)
#define DC1_ON()		{PEout(4) = 1;}
#define DC1_OFF()	  {PEout(4) = 0;}

// PE3   ��� DC2
#define DC2_SENSOR()		PEin(3)
#define DC2_ON()		{PEout(3) = 1;}
#define DC2_OFF()	  {PEout(3) = 0;}

// PE2   ��� DC3
#define DC3_SENSOR()		PEin(2)
#define DC3_ON()		{PEout(2) = 1;}
#define DC3_OFF()	  {PEout(2) = 0;}

// PB6  �����Դ������� DC4
#define DC4_SENSOR()		PBin(6)
#define DC4_ON()		{PBout(6) = 1;}
#define DC4_OFF()	  {PBout(6) = 0;}

// PF5   ����
#define alarm_SENSOR()		PFin(6)
#define alarm_ON()		{PFout(6) = 0;}
#define alarm_OFF()	  {PFout(6) = 1;}

// PF8   ����
#define light_SENSOR()		PFin(8)
#define light_ON()		{PFout(8) = 0;}
#define light_OFF()	  {PFout(8) = 1;}

// PF10   ����
#define heat_SENSOR()		PFin(10)
#define heat_ON()		{PFout(10) = 0;}
#define heat_OFF()	  {PFout(10) = 1;}

// PB6  �����Դ�������
#define out_special_SENSOR()		PBin(6)
#define out_special_power_ON()		{PBout(6) = 0;}
#define out_special_power_OFF()	  {PBout(6) = 1;}

// PC12	��ʪ�ȴ�����
#define TEM_HUM_SENSOR		PCin(12)
#define AM2301_Write_1()	{PCout(12) = 1;}
#define AM2301_Write_0()	{PCout(12) = 0;}

// PB7	ˮ��״̬ 0-��ˮ 1-����
#define WATER_SENSOR		PBin(7)
#define WATER_IN	0
#define WATER_OUT	1

// PD6	UPS״̬���  0-��ˮ 1-����
#define UPS_SENSOR		PDin(6)
#define UPS_OFF()		{PDout(6) = 0;} //�����
#define UPS_ON()	  {PDout(6) = 1;} //�����

// PB9	�����Դ״̬��� 0-�����,1-�����
#define special_power_SENSOR		PBin(9)
//#define UPS_OFF()		{PDout(6) = 0;} //�����
//#define UPS_ON()	  {PDout(6) = 1;} //�����

// PC7	������״̬ 0-�� 1-��
#define DOOR_SENSOR		PCin(7)
#define DOOR_OPEN	1
#define DOOR_CLOSE	0











extern float TEM, HUM;
extern u8 DOOR_STAT, WATER_STAT, SYS12_STAT, EQU12_STAT, EQU24_STAT, BAT12_STAT,UPS_STAT,special_power_STAT;
extern u8 AC1_STAT, AC2_STAT, AC3_STAT, fan_STAT, alarm_STAT, light_STAT,heat_STAT,DC1_STAT,DC2_STAT,DC3_STAT,DC4_STAT,out_special_STAT;

extern u8 IS_EQU_SYS12V, IS_EQU_UPS12V;
extern u8 stat_changed;

extern int SD_STAT;

void SENSOR_Init(void);
void STAT_CHECK(void);

void sd_out_config(void);

void sd_ipd_config(void);

extern void GET_AM2301_Data(void);
extern void DOOR_SENSOR_CHECK(void); //�ſ��ؽӿ�ɨ�躯��
extern void LIGHT_SENSOR_CHECK(void);

extern void SD_SENSOR_CHECK(void); //SD����ѯ���

void SD_state_machine_CHECK(void);//SD����ѯ���

extern void sd_detect_status(void);

extern u8 test_Select(void);
extern void Init_Iwdg(void);

extern void USB_Reset(void);

typedef struct _EP_BUF_DSCR { 
uint32_t ADDR_TX;    
uint32_t COUNT_TX;    
uint32_t ADDR_RX;   
uint32_t COUNT_RX;
} EP_BUF_DSCR;

#endif

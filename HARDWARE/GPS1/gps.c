
#include "gps.h"
#include "string.h"
#include "print.h"

const u32 BAUD_id[9]={4800,9600,19200,38400,57600,115200,230400,460800,921600};

// ����ֵ:	0~0XFE,����������λ�õ�ƫ��.
//			0XFF,�������ڵ�cx������							  
u8 NMEA_Comma_Pos(u8 *buf,u8 cx)
{
	u8 *p=buf;
	while(cx)
	{
		if(*buf=='*'||*buf<' '||*buf>'z')return 0XFF;	// ����'*'���߷Ƿ��ַ�,�򲻴��ڵ�cx������
		if(*buf==',')cx--;
		buf++;
	}
	return buf-p;
}

// m^n����
// ����ֵ:m^n�η�.
u32 NMEA_Pow(u8 m,u8 n)
{
	u32 result=1;
	while(n--)result*=m;
	return result;
}

// strת��Ϊ����,��','����'*'����
// buf:���ִ洢��
// dx:С����λ��,���ظ����ú���
// ����ֵ:ת�������ֵ
int NMEA_Str2num(u8 *buf,u8*dx)
{
	u8 *p=buf;
	u32 ires=0,fres=0;
	u8 ilen=0,flen=0,i;
	u8 mask=0;
	int res;
	while(1)							// �õ�������С���ĳ���
	{
		if(*p=='-'){mask|=0X02;p++;}	// �Ǹ���
		if(*p==','||(*p=='*'))break;	// ����������
		if(*p=='.'){mask|=0X01;p++;}	// ����С������
		else if(*p>'9'||(*p<'0'))		// �зǷ��ַ�
		{
			ilen = 0;
			flen = 0;
			break;
		}
		if(mask&0X01)flen++;
		else ilen++;
		p++;
	}
	if(mask&0X02)buf++;		// ȥ������
	for(i=0; i<ilen; i++)	// �õ�������������
	{
		ires += NMEA_Pow(10,ilen-1-i)*(buf[i]-'0');
	}
	if(flen > 5)flen = 5;	// ���ȡ5λС��
	*dx = flen;	 			// С����λ��
	for(i=0; i<flen; i++)	// �õ�С����������
	{
		fres += NMEA_Pow(10, flen-1-i)*(buf[ilen+1+i]-'0');
	}
	res = ires*NMEA_Pow(10, flen) + fres;
	if(mask&0X02)res = - res;
	return res;
}	  							 

// ����GNRMC��Ϣ
// gpsx:nmea��Ϣ�ṹ��
// buf:���յ���GPS���ݻ������׵�ַ
int NMEA_GNRMC_Analysis(nmea_msg *gpsx, u8 *buf)
{
	u8 *p1, dx, posx;
	u32 temp;
	float rs;
	
	p1 = (u8*)strstr((const char *)buf, "$GNRMC");
	if(p1 == NULL)
	{
		p1 = (u8*)strstr((const char *)buf, "$GPRMC");
		if(p1 == NULL)return -1;
	}
	
	posx = NMEA_Comma_Pos(p1, 1);							// ʱ����
	if(posx != 0XFF)
	{
		temp = NMEA_Str2num(p1+posx,&dx)/NMEA_Pow(10,dx);
		gpsx->utc.hour = temp/10000;
		gpsx->utc.min = (temp/100)%100;
		gpsx->utc.sec = temp%100;
	}
	else return -1;
	
	// ���Ӿ�γ���Ƿ���Ч�ж�
	posx = NMEA_Comma_Pos(p1, 2);							// V��Ч��λ
	if(posx != 0XFF)
	{
		if(*(p1+posx) == 'V')
		{
			Printf("��Ч��γ��");
			return -1;
		}
		else Printf("�ѻ�ȡ��γ��");
	}
	
	posx = NMEA_Comma_Pos(p1, 3);							// γ��
	if(posx != 0XFF)
	{
		temp = NMEA_Str2num(p1+posx, &dx);
		gpsx->latitude = temp/NMEA_Pow(10, dx+2);
		rs = temp%NMEA_Pow(10, dx+2);
		gpsx->latitude = gpsx->latitude*NMEA_Pow(10, 5)+(rs*NMEA_Pow(10, 5-dx))/60;
	}
	else return -1;
	
	posx = NMEA_Comma_Pos(p1, 4);							// ��γ/��γ 
	if(posx != 0XFF)gpsx->nshemi = *(p1+posx);
	else return -1;
	
 	posx = NMEA_Comma_Pos(p1, 5);							// ����
	if(posx != 0XFF)
	{
		temp = NMEA_Str2num(p1+posx, &dx);
		gpsx->longitude = temp/NMEA_Pow(10, dx+2);
		rs = temp%NMEA_Pow(10, dx+2);
		gpsx->longitude = gpsx->longitude*NMEA_Pow(10, 5)+(rs*NMEA_Pow(10, 5-dx))/60;
	}
	else return -1;
	
	posx = NMEA_Comma_Pos(p1, 6);							//����/����
	if(posx != 0XFF)gpsx->ewhemi = *(p1+posx);
	else return -1;
	
	posx = NMEA_Comma_Pos(p1, 9);							//������
	if(posx != 0XFF)
	{
		temp = NMEA_Str2num(p1+posx, &dx);
		gpsx->utc.date = temp/10000;
		gpsx->utc.month = (temp/100)%100;
		gpsx->utc.year = 2000+temp%100;
	}
	else return -1;
	
	return 0;
}

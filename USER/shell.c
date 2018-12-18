/*------------------------------------------------------------------------------+
|  File Name:  Shell.c, v1.1.1													|
|  Author:     aaron.xu1982@gmail.com                                         	|
|  Date:       2011.09.25                                                     	|
+-------------------------------------------------------------------------------+
|  Description: �����ն���������������֧������Ļ��˹��ܡ�					|
|                                                                             	|
+-------------------------------------------------------------------------------+
|  Release Notes:                                                             	|
|                                                                             	|
|  Logs:                                                                      	|
|  WHO       WHEN         WHAT                                                	|
|  ---       ----------   ------------------------------------------------------|
|  Aaron     2008.05.04   born                                                	|
|  Aaron     2011.06.22   �޸�Ϊͨ�õ�������ģʽ                              	|
|  Aaron     2011.09.25   ��ֲ��STM32��Ƭ����FreeRTOS����ϵͳ������           	|
|                                                                             	|
+------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------+
| Include files                                                               	|
+------------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"


/* Library includes. */
//#include "stm32f10x_lib.h"

//#include "SD_SPI.h"
//#include "SD_SDIO.h"
//#include "spi_flash.h"

//#include "BKP.h"
#include "rtc.h"
#include "USART.h"
#include "print.h"
#include "Shell.h"
//#include "config.h"
#include "ff.h"
#include "fattester.h"

/*----------------------------------------------------------------------------+
| Type Definition & Macro                                                     |
+----------------------------------------------------------------------------*/
// ����������
#define MAX_CMD_LEN				64		// ���������������ַ�����󳤶�
#define MAX_CMD_BUF_BACKS		8		// ������״̬֧�ֻ��˵���ߴ���
#define MAX_CMD_BUF_LEN			128		// ��Ż�������Ļ������ĳ���
#define MAX_CMD_ARGS			8		// ����������ܴ�8������

// �ͳ����ն���صĺ궨��
#define KEY_BACKSPACE	0x08
#define KEY_TABLE		0x09			// Table��
#define KEY_UP_ARROW	"\x1B\x5B\x41"
#define KEY_DOWN_ARROW	"\x1B\x5B\x42"
#define KEY_RIGHT_ARROW	"\x1B\x5B\x43"
#define KEY_LEFT_ARROW	"\x1B\x5B\x44"
#define KEY_HOME		"\x1B\x5B\x48"
#define KEY_END			"\x1B\x5B\x4B"

typedef int (*cmdproc)(int argc, char *argv[]);
typedef struct {
	char *cmd;
	char *hlp;
	cmdproc proc;
}t_CMD;

/*----------------------------------------------------------------------------+
| Function Prototype                                                          |
+----------------------------------------------------------------------------*/

int CMD_Serial(int argc, char *argv[]);

/*----------------------------------------------------------------------------+
| Global Variables                                                            |
+----------------------------------------------------------------------------*/
//extern u32 BeepTimer;		// ������������ʱ�䣬��ϵͳ��������Ϊ��λ���ڶ�ʱ���ж��н����жϺʹ���

const u8 *StrDayOfWeek[] = {
	"��",		//"Sun", 
	"һ",		//"Mon", 
	"��",		//"Tues", 
	"��",		//"Wed", 
	"��",		//"Thu", 
	"��",		//"Fri", 
	"��",		//"Sat",  
	"��",		//"Sun"
};

const t_CMD CMD_INNER[] = {
	{"?",		"help", 								CMD_Help	},
 	{"help",	"Show this list",						CMD_Help	},
	{"date", 	"Show or set current date: YY-MM-DD",	CMD_GetDate	},
	{"time", 	"Show or set current time: HH.MM.SS",	CMD_GetTime	},
	{"clock", 	"Show system running clock",			CMD_SysClk	},
	{"serial",	"Show the device serial number",		CMD_Serial	},
	{"dir", 	"Show Directory content",				CMD_Dir		},
	{"type",	"display a file content",				CMD_Type	},
	{"fdump", 	"dump a file content",					CMD_FDump	},
//	{"test",	"get leds status",						CMD_Test	},
	{"cls",		"clean screen",							CMD_Cls		},
	{NULL, NULL, NULL}
};

/*----------------------------------------------------------------------------+
| Internal Variables                                                          |
+----------------------------------------------------------------------------*/
u32 BeepTimer = 0;

volatile int CMD_RxLength;				// �����ն��µ�ǰ��һ�е��ַ�����
volatile int CMD_RxCurrentPos;			// �����ն��µ�ǰ����λ��
char CMD_RxPreByte;						// ͨ�������ն�������Ʒ������Ҽ�ͷ��һ�ΰ�������3���ַ���ǰ��2���ַ��̶�Ϊ0x1B��0x5B����������������ַ������������ʾ��һ���ַ�Ϊ���Ʒ�
char CMD_RxBuffer[MAX_CMD_LEN];			// �������ݻ�����

#if MAX_CMD_BUF_BACKS > 0
	struct t_CMD_Line CMD_Line[MAX_CMD_BUF_BACKS];	// ֮ǰ������Ļ������ĳ��Ⱥͻ�����ָ��
	int  CMD_LineNum;					// ֮ǰ���յ��������е���Ŀ����ֹ�տ�����ʱ����������ͷ
	int  CMD_CurrentLine;				// ��ǰ���ڵ������У����ڳ����ն��°��ϡ��¼�ͷʱ�ص��ϴη��͵�����
	char CMD_Buffer[MAX_CMD_BUF_LEN];	// �����л���������ν��յ������ж��������棬ͨ��ָ�붨λ
	int  CMD_BufferLength;				// ��������������ܳ��ȣ�Ϊ��ֹ���������
#endif // MAX_CMD_BUF_BACKS

/*----------------------------------------------------------------------------+
| System Initialization Routines                                              |
+----------------------------------------------------------------------------*/
// ��ʼ����������������
void Init_Shell(void)
{
	int i;

	// ��ʼ���ʹ����йص�ȫ�ֱ���
	CMD_RxPreByte = 0;
	CMD_RxLength = 0;
	CMD_RxCurrentPos = 0;
	for (i=0; i<sizeof(CMD_RxBuffer); i++)
	{
		CMD_RxBuffer[i] = 0x00;
	}

#if MAX_CMD_BUF_BACKS > 0
	CMD_LineNum = 0;					// �տ�����ʱ���ܻ���
	CMD_CurrentLine = 0;
	CMD_BufferLength = 0;
	for (i=0; i<MAX_CMD_BUF_BACKS; i++)
	{
		CMD_Line[i].nLength = 0;
		CMD_Line[i].Buffer = CMD_Buffer;
	}
#endif // MAX_CMD_BUF_BACKS
}

/*----------------------------------------------------------------------------+
| General Subroutines                                                         |
+----------------------------------------------------------------------------*/
int str2int(char *str)
{
	int i = 0;

	while (*str != '\0')
	{
		if ((*str < '0') || (*str > '9'))
		{
			break;
		}
		i *= 10;
		i += *str - '0';
		str ++;
	}
	return i;
}

void str2lower(char *str)
{
	while (*str != '\0')
	{
		if ((*str >= 'A') && (*str <= 'Z'))
		{
			*str += 'a' - 'A';
		}
		str ++;
	}
}

int ParseCmd(char *cmdline, int cmd_len)
{
	int i;
	int argc = 0;					// �����в����ĸ���
	int state = 0;					// ��ǰ�������ַ��Ƿ�ո�
	int command_num = -1;			// ��Ӧƥ����������
	char *argv[MAX_CMD_ARGS];
	char *pBuf;

	// Parse Args -------------------------------------------------------------
	cmdline[cmd_len] = '\0';
	pBuf = cmdline;
	// �ж��������AT��ͷ�����ʾ����һ��ATָ����͸�Modem���ȴ�Modem����
	if (((cmdline[0] == 'a') || (cmdline[0] == 'A'))
	 && ((cmdline[1] == 't') || (cmdline[1] == 'T')))
	{
//		if (ModemState == ModemState_Cmd)
//		{
			// ���������Modem
//			USART2_PutString(cmdline);
			// �����н�����0x0D
//			USART2_PutChar(0x0D);
//		}
		return 0;
	}
	// find out all words in the command line
	while (*pBuf != '\0')
	{
		if (state == 0)					// ǰһ���ַ��ǿո��tab
		{
			if ((*pBuf != ' ')
			 && (*pBuf != '\t'))
			{
				argv[argc] = pBuf;		// ��argv[]ָ��ǰ�ַ�
				argc ++;
				state = 1;
			}
		}
		else							// ǰһ���ַ����ǿո����жϵ�ǰ�ַ��Ƿ�ո�
		{
			if ((*pBuf == ' ')
			 || (*pBuf == '\t'))
			{
				*pBuf = '\0';
				state = 0;
			}
		}
		pBuf ++;
		if (argc >= MAX_CMD_ARGS) break;//
	}
	if(argc == 0)
		return 0;
	// Get Matched Command ----------------------------------------------------
	for (i=0; CMD_INNER[i].cmd!=NULL; i++)
	{
		str2lower(argv[0]);
		if (strcmp(argv[0], CMD_INNER[i].cmd) == 0)	// ƥ��
		{
			command_num = i;
			break;
		}
	}
	if (command_num < 0)
	{
		PrintS("\nBad command: ");		PrintS(argv[0]);
		PrintS("\nplease type 'help' or '?' for a command list");
		return -1;
	}
		
	if(CMD_INNER[command_num].proc != NULL)
	{
		return CMD_INNER[command_num].proc(argc, argv);
	}
	return 0;			
}

int CMD_Help(int argc, char *argv[])
{
	int i;	
	
	for(i=0; CMD_INNER[i].cmd != NULL; i++)
	{
		if(CMD_INNER[i].hlp!=NULL)
		{
			Printf("\n%-9s  ---  %s", CMD_INNER[i].cmd, CMD_INNER[i].hlp);
		}
	}
	
	return i;
}

int CMD_GetDate(int argc, char *argv[])
{
	BYTE temp;
	struct _date_t Date;

	Rtc_ReadTime();
	Printf("\nDate: %04d-%02d-%02d %s", RtcTime.Year, RtcTime.Month, RtcTime.DayOfMonth, StrDayOfWeek[RtcTime.DayOfWeek]);
	if(argc<2)
		return 0;

	argv[1][2] = '\0';
	Date.Year  = 2000 + str2int(&argv[1][0]);
	argv[1][5] = '\0';
	Date.Month = str2int(&argv[1][3]);
	argv[1][8] = '\0';
	Date.Date  = str2int(&argv[1][6]);

	if ((Date.Year > 2099) || (Date.Month > 12) || (Date.Date > 31))
	{
		PrintS("\ninput date error");
		return 0;
	}
	if ((temp = RTC_SetDate(Date.Year, Date.Month, Date.Date)) != TIME_SET_OK)
	{
		Printf("\nerror occured when set data: %02X", temp);
		return 0;
	}

	Printf("\nSet date: %04d-%02d-%02d", Date.Year, Date.Month, Date.Date);
	return 1;
}

int CMD_GetTime(int argc, char *argv[])
{
	BYTE temp;
	struct _time_t Time;

	Rtc_ReadTime();
	Printf("\nTime: %02d:%02d:%02d", RtcTime.Hour, RtcTime.Minute, RtcTime.Second);
	if(argc<2)
		return 0;
		
	argv[1][2] = '\0';
	Time.Hour   = str2int(&argv[1][0]);
	argv[1][5] = '\0';
	Time.Minute = str2int(&argv[1][3]);
	argv[1][8] = '\0';
	Time.Second = str2int(&argv[1][6]);

	if ((Time.Hour > 23) || (Time.Minute > 59) || (Time.Second > 59))
	{
		PrintS("\ninput time error");
		return 0;
	}
	if ((temp = RTC_SetTime(Time.Hour, Time.Minute, Time.Second)) != TIME_SET_OK)
	{
		Printf("\nerror occured when set time: %02X", temp);
		return 0;
	}

	Printf("\nSet time: %02d:%02d:%02d OK!", Time.Hour, Time.Minute, Time.Second);
	return 1;
}

int CMD_SysClk(int argc, char *argv[])
{
	u32 apbclock = 0x00;
	RCC_ClocksTypeDef RCC_ClocksStatus;

	RCC_GetClocksFreq(&RCC_ClocksStatus);
	apbclock = RCC_ClocksStatus.PCLK2_Frequency;
	Printf("\nSystem is running @ %dHz", apbclock);
	return apbclock;
}

void GetSerialNum(u8 *pBuf)
{
	u32 *pDest;
	volatile u32 *pSerial;

	pDest = (u32 *)(pBuf);
	pSerial = (volatile u32 *)(0x1FFFF7E8);	//0x1FFFF7E8

	*pDest++ = *pSerial++;
	*pDest++ = *pSerial++;
	*pDest++ = *pSerial++;
}

int CMD_Serial(int argc, char *argv[])
{
	u8 buf[12];

	GetSerialNum(buf);
	PrintS("\nSERIAL: ");
	PrintHex(buf, 12, 0, 0);
	return 0;
}

int CMD_Dir(int argc, char *argv[])
{
#if (_USE_LFN == 0)
	BYTE rt;
	u32 j, k, lTemp;
	DIR Dir;					/* Directory object */
	FILEINFO Finfo;
	int IsRoot = 0;


	j = 0;
	k = 0;
	lTemp = 0;
	if (argc < 2)
	{
		rt = f_opendir(&Dir, "\\");
		IsRoot = 1;
	}
	else
	{
		rt = f_opendir(&Dir, argv[1]);
	}
	if (rt == FR_OK)
	while (1)
	{
		PrintC('\n');
		rt = f_readdir(&Dir, &Finfo);
		if ((rt != FR_OK) || (Finfo.fname[0] == '\0')) break;
		Printf("%04d-%02d-%02d  ", (Finfo.fdate>>9)+1980, (Finfo.fdate>>5)&0x0F, (Finfo.fdate & 0x1F));
		Printf("%02d:%02d    ", (Finfo.ftime>>11), (Finfo.ftime>>5)&0x3F);
		if (Finfo.fattrib & ATTR_DIR) 	// Ŀ¼
		{
			k ++;
			PrintS("<DIR>          ");
		}
		else							// �ļ�
		{
			j ++;
			lTemp += Finfo.fsize;
			Printf("%14d ", Finfo.fsize);
		}
		{
		PrintS((const char *)Finfo.fname);
		}
	}

	Printf("\n%16d ���ļ�%12d �ֽ�", j, lTemp);
	Printf("\n%16d ��Ŀ¼", k);
	if (IsRoot)							// ����Ǹ�Ŀ¼���������̵Ŀ��ÿռ�
	{
		FATFS *fs;
		rt = f_getfreeclust("\\", &lTemp, &fs);	// get free clusts
		lTemp *= fs->csize;		// get free sectors
		lTemp *= SECT_SIZE(fs);		// get free bytes
		Printf("%12d �����ֽ�", lTemp);
	}
#else
	BYTE rt;
	u32 j, k, lTemp;
	DIR Dir;					/* Directory object */
	char lfn[_MAX_LFN+1];
	FILINFO Finfo;
	int IsRoot = 0;


	j = 0;
	k = 0;
	lTemp = 0;
	if (argc < 2)
	{
		rt = f_opendir(&Dir, "\\");
		IsRoot = 1;
	}
	else
	{
		rt = f_opendir(&Dir, argv[1]);
	}
	if (rt == FR_OK)
	while (1)
	{
		PrintC('\n');
		Finfo.lfname = lfn;
		Finfo.lfsize = sizeof(lfn);
		rt = f_readdir(&Dir, &Finfo);
		if ((rt != FR_OK) || (Finfo.fname[0] == '\0')) break;
		Printf("%04d-%02d-%02d  ", (Finfo.fdate>>9)+1980, (Finfo.fdate>>5)&0x0F, (Finfo.fdate & 0x1F));
		Printf("%02d:%02d    ", (Finfo.ftime>>11), (Finfo.ftime>>5)&0x3F);
//		if (Finfo.fattrib & ATTR_DIR) 	// Ŀ¼
		if (Finfo.fattrib & 0x10)
		{
			k ++;
			PrintS("<DIR>          ");
		}
		else							// �ļ�
		{
			j ++;
			lTemp += Finfo.fsize;
			Printf("%14d ", Finfo.fsize);
		}
		if (Finfo.lfname[0])
		{
			PrintS((const char *)Finfo.lfname);
		}
		else
		{
			PrintS((const char *)Finfo.fname);
		}
	}
	if (rt == FR_OK)
	{
		Printf("\n%16d ���ļ�%12d �ֽ�", j, lTemp);
		Printf("\n%16d ��Ŀ¼", k);
		if (IsRoot)							// ����Ǹ�Ŀ¼���������̵Ŀ��ÿռ�
		{
			FATFS *fs;
//			rt = f_getfreeclust(0, &lTemp, &fs);	// get free clusts
			lTemp *= fs->csize;			// get free sectors
//			lTemp *= SECT_SIZE(fs);		// get free bytes
			Printf("%12d �����ֽ�", lTemp);
		}
	}
	else
	{
		PrintS("\nDisk Read Error!");
	}
#endif
	return 0;
}

int CMD_Type(int argc, char *argv[])
{
	UINT length;
	u8 temp;
	s8 BUF[33];

	if (argc < 2)
	{
		PrintS("\n��ʾ�ı��ļ������ݡ�");
		PrintS("\nTYPE [drive:][path][filename]\n");
	}
	else
	{
		FIL* fp;
    
		fp = FileOpen(argv[1], FA_READ);
		if (fp != NULL)
		{
			PrintC('\n');
//			f_seek(fp, 0, SEEK_SET);
			while (1)
			{
				temp = f_read(fp, BUF, 32, &length);
				if (temp != FR_OK) break;
				if (length == 0) break;
				BUF[length] = '\0';
				PrintS((const char *)BUF);
			}
			FileClose(fp);
		}
	}
	return 0;
}

int CMD_FDump(int argc, char *argv[])
{
	UINT length;
	UINT offset;
	FIL* fp;
	BYTE temp;
	u8 BUF[33];

	if (argc < 2)
	{
		PrintS("\n��HEX��ʽ��ʾ�ļ������ݡ�");
		PrintS("\nFDUMP [drive:][path][filename]\n");
	}
	else
	{
		offset = 0;
		fp = FileOpen(argv[1], FA_READ);
		if (fp != NULL)
		{
			PrintC('\n');
//			f_seek(fp, 0, SEEK_SET);
			while (1)
			{
				temp = f_read(fp, BUF, 32, &length);
				if (temp != FR_OK) break;
				if (length == 0) break;
				PrintHex(BUF, length, 1, offset);
				offset += length;
			}
			FileClose(fp);
		}
	}
	return 0;
}

int CMD_Cls(int argc, char *argv[])
{
  SerialPutChar('\r');
  SerialPutChar(0x0c);
  return 0;
}
/*----------------------------------------------------------------------------+
| General Subroutines                                                         |
+----------------------------------------------------------------------------*/
// ���յ�һ������Ĵ���
void CMD_RxFrame(void)
{
	u32 i;
	char *pTemp;

#if MAX_CMD_BUF_BACKS > 0
	// �����ε�������ϴεĲ�һ�����򱣴���һ�ε�����
	if (CMD_RxLength > 0)
	{
		if ((CMD_RxLength != CMD_Line[0].nLength) ||			// ������ε�������ȫһ�����򲻱���
			(memcmp(CMD_RxBuffer, CMD_Line[0].Buffer, CMD_RxLength) != 0))
		{
			// �����ڳ�����������ʹ�����㹻�ռ���Դ����һ�ε�����
			while (CMD_LineNum > 0)
			{
				if ((CMD_RxLength + CMD_BufferLength) < sizeof(CMD_Buffer))
				{
					break;				// �������Ѿ��㹻��������ѭ��
				}
				CMD_BufferLength -= CMD_Line[CMD_LineNum-1].nLength;
				CMD_LineNum --;
			}

			if ((CMD_RxLength + CMD_BufferLength) < sizeof(CMD_Buffer))
			{							// ������㹻�ռ�
				if (CMD_LineNum >= MAX_CMD_BUF_BACKS)	// ��ҪŲ����ǰ���һ������
				{
					CMD_LineNum --;
					CMD_BufferLength -= CMD_Line[CMD_LineNum].nLength;
				}

				for (i=CMD_LineNum; i>0; i--)
				{
					CMD_Line[i].nLength = CMD_Line[i-1].nLength;
					CMD_Line[i].Buffer = CMD_Line[i-1].Buffer;
				}
				CMD_Line[0].nLength = CMD_RxLength;
				CMD_Line[0].Buffer = CMD_Line[1].Buffer + CMD_Line[1].nLength;
				if (CMD_Line[0].Buffer >= (CMD_Buffer + sizeof(CMD_Buffer)))
				{
					CMD_Line[0].Buffer -= sizeof(CMD_Buffer);
				}
				// ����������������
				pTemp = CMD_Line[0].Buffer;
				for (i=0; i<CMD_RxLength; i++)
				{
					*pTemp = CMD_RxBuffer[i];
					pTemp ++;
					if (pTemp >= (CMD_Buffer + sizeof(CMD_Buffer)))
					{
						pTemp -= sizeof(CMD_Buffer);
					}
				}
				CMD_BufferLength += CMD_RxLength;

				if (CMD_LineNum < MAX_CMD_BUF_BACKS)
				{
					CMD_LineNum ++;
				}
			}
			else						// ����ռ䲻�㣬��ʲôҲ����
			{
			}
		}
		CMD_CurrentLine = 0;
	}
#endif // MAX_CMD_BUF_BACKS

	// �����н���
	ParseCmd((char *)CMD_RxBuffer, CMD_RxLength);
	PrintS("\n\\>");

	// ��ս��ջ�����
	for (i=0; i<=CMD_RxLength; i++)
	{
		CMD_RxBuffer[i] = 0x00;
	}
	CMD_RxLength = 0;
	CMD_RxCurrentPos = 0;
}

// ���յ������ֽڵĴ���
void CMD_RxByte(char aData)
{
	char  temp;
	char *pTemp;
	u32 i;

	if (CMD_RxPreByte == 0x01)
	{
		if (aData == 0x5B)				// ���ֵڶ������Ʒ�
		{
			CMD_RxPreByte = 0x02;
		}
		else
		{
			CMD_RxPreByte = 0x00;
		}
	}
	else if (CMD_RxPreByte == 0x02)		// ǰ���������ֹ�2�����Ʒ�������һ���ַ���ʾ������ʲô���Ʒ�
	{
		if (aData == 0x41)				// Up Arrow, �ص���һ�ε�����
		{
#if MAX_CMD_BUF_BACKS > 0
			if (CMD_CurrentLine < CMD_LineNum)
			{
				// ɾ�������ն��ϵĵ�ǰ�У��Ȼص���ʼ����Ȼ���Ϳո����Ȼ���ڻص���ʼ
				for (i=0; i<CMD_RxCurrentPos; i++)
				{
					PrintC(KEY_BACKSPACE);
				}
				for (i=0; i<CMD_RxLength; i++)
				{
					PrintC(' ');
				}
				for (i=0; i<CMD_RxLength; i++)
				{
					PrintC(KEY_BACKSPACE);
				}
				// ����������
				CMD_RxLength = CMD_Line[CMD_CurrentLine].nLength;
				CMD_RxCurrentPos = CMD_RxLength;		// ���ָ��������ĩβ
				pTemp = CMD_Line[CMD_CurrentLine].Buffer;
				for (i=0; i<CMD_RxLength; i++)
				{
					temp = *pTemp;
					pTemp ++;
					if (pTemp >= (CMD_Buffer + sizeof(CMD_Buffer)))
					{
						// ���������������Χ����ص���������ʼ��
						pTemp = CMD_Buffer;
					}
					CMD_RxBuffer[i] = temp;
					// ���³����ն˵���ʾ
					PrintC(temp);
				}
				CMD_CurrentLine ++;
			}
			else
			{
				BeepTimer = SYS_TICK_RATE_HZ / 10;	// ����������100ms
			}
#endif // MAX_CMD_BUF_BACKS
		}
		else if (aData == 0x42)			// Down Arrow, �ص���һ�ε�����
		{
#if MAX_CMD_BUF_BACKS > 0
			if (CMD_CurrentLine > 0)
			{
				// ɾ�������ն��ϵĵ�ǰ�У��Ȼص���ʼ����Ȼ���Ϳո����Ȼ���ڻص���ʼ
				for (i=0; i<CMD_RxCurrentPos; i++)
				{
					PrintC(KEY_BACKSPACE);
				}
				for (i=0; i<CMD_RxLength; i++)
				{
					PrintC(' ');
				}
				for (i=0; i<CMD_RxLength; i++)
				{
					PrintC(KEY_BACKSPACE);
				}
				CMD_CurrentLine --;
				if (CMD_CurrentLine > 0)
				{
					// ����������
					CMD_RxLength = CMD_Line[CMD_CurrentLine-1].nLength;
					CMD_RxCurrentPos = CMD_RxLength;	// ���ָ��������ĩβ

					pTemp = CMD_Line[CMD_CurrentLine-1].Buffer;
					for (i=0; i<CMD_RxLength; i++)
					{
						temp = *pTemp;
						pTemp ++;
						if (pTemp >= (CMD_Buffer + sizeof(CMD_Buffer)))
						{
							// ���������������Χ����ص���������ʼ��
							pTemp = CMD_Buffer;
						}
						CMD_RxBuffer[i] = temp;
						// ���³����ն˵���ʾ
						PrintC(temp);
					}
				}
				else							// 0xFF��ʾ�ص����һ�ε������ʱ������Ϊ��
				{
					CMD_RxLength = 0;
					CMD_RxCurrentPos = 0;
					CMD_RxBuffer[0] = 0x00;
				}
			}
			else
			{
				BeepTimer = SYS_TICK_RATE_HZ / 10;	// ����������100ms
			}
#endif // MAX_CMD_BUF_BACKS
		}
		else if (aData == 0x43)			// Rigth Arrow,
		{
			if (CMD_RxCurrentPos < CMD_RxLength)
			{
				CMD_RxCurrentPos ++;
				PrintS(KEY_RIGHT_ARROW);
			}
		}
		else if (aData == 0x44)			// Left Arrow,
		{
			if (CMD_RxCurrentPos > 0)
			{
				CMD_RxCurrentPos --;
				PrintS(KEY_LEFT_ARROW);
			}
		}
		else if (aData == 0x48)			// Home������������Ϊ�ڳ����ն���'Home'��ʾ�ص�������Ļ�����Ͻ�
		{
			while (CMD_RxCurrentPos)
			{
				CMD_RxCurrentPos --;
				PrintS(KEY_LEFT_ARROW);
			}
		}
		else if (aData == 0x4B)			// End������������Ϊ�ڳ����ն���'End'�Ὣ��һ�й�������ַ��������
		{
			while (CMD_RxCurrentPos < CMD_RxLength)
			{
				CMD_RxCurrentPos ++;
				PrintS(KEY_RIGHT_ARROW);
			}
		}
		CMD_RxPreByte = 0x00;
	}
	else
	{
		if (aData == KEY_BACKSPACE)		// �˸����ǰ��һ���ַ�����������ַ���ǰ��
		{
			if ((CMD_RxLength > 0) && (CMD_RxCurrentPos > 0))
			{
				// �����괦���ַ�
				for (i=CMD_RxCurrentPos; i<CMD_RxLength; i++)
				{
					CMD_RxBuffer[i-1] = CMD_RxBuffer[i];
				}
				CMD_RxCurrentPos --;
				CMD_RxLength --;
				CMD_RxBuffer[CMD_RxLength] = 0x00;
				// ����һ���ַ�
				PrintC(KEY_BACKSPACE);
				// ���³����ն��ϵ���ʾ
				for (i=CMD_RxCurrentPos; i<CMD_RxLength; i++)
				{
					PrintC(CMD_RxBuffer[i]);
				}
				// �����һ���ַ��ÿո����
				PrintC(' ');
				PrintC(KEY_BACKSPACE);
				// ������ƻص��ղŵ�λ��
				for (i=CMD_RxLength; i>CMD_RxCurrentPos; i--)
				{
					PrintC(KEY_BACKSPACE);
				}
			}
			else
			{
				BeepTimer = SYS_TICK_RATE_HZ / 10;	// ����������100ms
			}
		}
		else if (aData == KEY_TABLE)	// Table��
		{
			BeepTimer = SYS_TICK_RATE_HZ / 10;		// ����������100ms
		}
		else if (aData == 0x0A)			// ���У�������
		{
		}
		else if (aData == 0x0D)			// �س�
		{
			CMD_RxBuffer[CMD_RxLength] = 0x00;		// ������־
			CMD_RxFrame();				// ִ�д�����
		}
		else if (aData == 0x1B)			// ���Ʒ���һ���ַ�
		{
			CMD_RxPreByte = 0x01;
			return;
		}
		else if (aData >= 0x20)			// �ɴ�ӡ�ַ�����
		{
			if (CMD_RxLength < sizeof(CMD_RxBuffer))
			{
				for (i=CMD_RxLength; i>CMD_RxCurrentPos; i--)
				{
					CMD_RxBuffer[i] = CMD_RxBuffer[i-1];
				}
				CMD_RxBuffer[CMD_RxCurrentPos] = aData;
				CMD_RxLength ++;
				CMD_RxCurrentPos ++;
				// ���³����ն��ϵ��ַ���ʾ
				for (i=CMD_RxCurrentPos; i<=CMD_RxLength; i++)
				{
					PrintC(CMD_RxBuffer[i-1]);
				}
				// ������ƻص������ַ���λ��
				for (i=CMD_RxLength; i>CMD_RxCurrentPos; i--)
				{
					PrintS(KEY_LEFT_ARROW);
				}
			}
		}
		CMD_RxPreByte = 0x00;
	}
}

/*----------------------------------------------------------------------------+
| Interrupt Service Routines                                                  |
+----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------+
| End of source file                                                          |
+----------------------------------------------------------------------------*/
/*------------------------ Nothing Below This Line --------------------------*/

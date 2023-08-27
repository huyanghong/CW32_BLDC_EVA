#include "font.h"



uint8_t everRead=0;//��¼Flash�Ƿ��ʼ��	0û�г�ʼ����1��ʼ��

sFONT Font8x16 = {
  10*1024, /*������Flash��ʼ��ַ*/
  8, /* Width */
  16, /* Height */
#if !FLASH_FONT	//��ʹ���ڴ��е��ֿ�ʱ�������ڴ��ַ
  ASCII8x16_Table,	/* �����ڴ��ַ */
#endif
};

sFONT Font16x24 = {
  12*1024, /*��ʼ��ַ*/
  16, /* Width */
  24, /* Height */
#if !FLASH_FONT	//��ʹ���ڴ��е��ֿ�ʱ�������ڴ��ַ
  ASCII16x24_Table,	/* �����ڴ��ַ */
#endif
};

sFONT Font24x32 = {
  17*1024, /*��ʼ��ַ*/
  24, /* Width */
  32, /* Height */
#if !FLASH_FONT	//��ʹ���ڴ��е��ֿ�ʱ�������ڴ��ַ
  ASCII24x32_Table,	/* �����ڴ��ַ */
#endif
};
sFONT GB2312_H1616 = {
  256*1024, /*��ʼ��ַ*/
  WIDTH_CH_CHAR, /* Width */
  HEIGHT_CH_CHAR, /* Height */
  /* ����û���ڴ��ַ���ڴ�ռ��̫���� */
};


//ѡ��ʹ��FLASH��ģ����SD������ģ
#if GBKCODE_FLASH


// 0��HZLIB��ģ(�ɰ棬������ʹ��), 1��GB2312��ģ
#if 1

/**
  * @brief  ��ȡFLASH������ʾ�ֿ�����
  * @param  pBuffer:�洢�ֿ����Ļ�����
  * @param  c �� Ҫ��ȡ������
  * @retval None.
  */
uint8_t GetGBKCode_from_EXFlash( uint8_t * pBuffer, uint16_t c)
{
#if FLASH_FONT	//��ʹ���ڴ��е��ֿ�ʱ����֧������
	unsigned char High8bit,Low8bit;
	unsigned int pos,datnum;
	
	High8bit= c >> 8;     /* ȡ��8λ���� */
	Low8bit= c & 0x00FF;  /* ȡ��8λ���� */
	
	//Ҫ��ȡ���ֽ�����
	datnum=GB2312_H1616.Width*GB2312_H1616.Height/8;
	/*GB2312 ��ʽ*/	
	pos = ( (High8bit-0xa1)*94 + Low8bit-0xa1 )*datnum;
	//��ȡ�ֿ�����  
	SPI_FLASH_BufferRead(pBuffer , GB2312_H1616.AddrStart + pos , datnum);
#else
	FONT_Bebug("��ǰ���岻֧������\n");
#endif
	return 0;  
}

/**
  * @brief  ��ȡFlash�е�Ӣ���ַ�
  * @param  pBuffer:�洢�ֿ����Ļ�����
  * @param  c �� Ҫ��ȡ������
  * @param  Fonts�� ��������
  * @retval None.
  */
uint8_t GetConslonsCode_from_EXFlash( uint8_t * pBuffer, const char c , sFONT *Fonts)
{
	unsigned char chr;
	unsigned int pos,datnum;
	
	chr = c - ' ';
	
	//Ҫ��ȡ���ֽ�����(��������ռdatnum���ֽ�)
	datnum = (Fonts->Width * Fonts->Height)/8;
	/*��ģƫ�Ƶ�ַ*/
	pos = chr*datnum;
	#if FLASH_FONT
	//��ȡ�ֿ�����  
	SPI_FLASH_BufferRead(pBuffer , Fonts->AddrStart + pos , datnum);
	#else
	for(uint16_t i=0;i<datnum;i++)
	{
		*(pBuffer+i) = (Fonts->Font_Table[pos+i]);//��ȡ�������ĳ���ַ������׵�ַ
	}
	#endif
	return 0;  
}


#else	//�ɰ���ģHZLIB ���׵ĺ���

	/*HZLIB�����ֿ�洢��FLASH����ʼ��ַ*/
	#define HZLIB_START_ADDRESS   1*4096

	//�ɰ�HZLIB�ֿ�ʹ�õĺ�����HZLIB��֧�ֱ��
	//������ʹ�õ�Ѱַ��ʽ��GB2312��Ҳ��������
	int GetGBKCode_from_EXFlash( uint8_t * pBuffer, uint16_t c)
	{ 
		unsigned char High8bit,Low8bit;
		unsigned int pos;
		static uint8_t everRead=0;
		
		High8bit= c >> 8;     /* ȡ��8λ���� */
		Low8bit= c & 0x00FF;  /* ȡ��8λ���� */	
		
		//����ģʹ�õ�Ѱַ��ʽ��GB2312����������
		pos = ((High8bit-0xa0-16)*94+Low8bit-0xa1)*WIDTH_CH_CHAR*HEIGHT_CH_CHAR/8;
		//��ַָ��HZLIB��ģ�ĵ�ַHZLIB_START_ADDRESS
		SPI_FLASH_BufferRead(pBuffer,HZLIB_START_ADDRESS+pos,WIDTH_CH_CHAR*HEIGHT_CH_CHAR/8);
		//printf ( "%02x %02x %02x %02x\n", pBuffer[0],pBuffer[1],pBuffer[2],pBuffer[3]);
		
		return 0;
	}
	#endif

#else //SD����ģ

	static FIL fnew;	/* file objects */
	static FATFS fs;	/* Work area (file system object) for logical drives */
	static FRESULT res_sd; 
	static UINT br;		/* File R/W count */

	/**
	  * @brief  ��ȡSD��������ʾ�ֿ�����
		* @param  pBuffer:�洢�ֿ����Ļ�����
		* @param  c �� Ҫ��ȡ������
	  * @retval None.
	  */
	int GetGBKCode_from_sd ( uint8_t * pBuffer, uint16_t c)
	{ 
		unsigned char High8bit,Low8bit;
		unsigned int pos;
		static uint8_t everRead = 0;
		
		High8bit= c >> 8;     /* ȡ��8λ���� */
		Low8bit= c & 0x00FF;  /* ȡ��8λ���� */
			
		pos = ((High8bit-0xa1)*94+Low8bit-0xa1)*WIDTH_CH_CHAR*HEIGHT_CH_CHAR/8;
		
		/*��һ��ʹ�ã������ļ�ϵͳ����ʼ��sd*/
		if(everRead == 0)
		{
			res_sd = f_mount(&fs,"0:",1);
			everRead = 1;
		}
		res_sd = f_open(&fnew , GBKCODE_FILE_NAME, FA_OPEN_EXISTING | FA_READ);
		if ( res_sd == FR_OK ) 
		{
			f_lseek (&fnew, pos);		//ָ��ƫ��
			f_read( &fnew, pBuffer, WIDTH_CH_CHAR*HEIGHT_CH_CHAR/8, &br );//16*16��С�ĺ��� ����ģ ռ��16*16/8���ֽ�
			f_close(&fnew);
			return 0;  
		}
		else
			return -1;    
	}

#endif







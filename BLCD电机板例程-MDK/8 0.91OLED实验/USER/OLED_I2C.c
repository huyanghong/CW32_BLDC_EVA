
#include "OLED_I2C.h"

#include "font.h"


sFONT *Fonttemp = &Font8x16;

short I2C_OFFSET_W = I2C_PIC_W / 2 - I2C_OLED_W / 2;
short I2C_OFFSET_H = I2C_PIC_H / 2 - I2C_OLED_H / 2; //��ʾ���뻭��ƫ����

uint8_t I2C_P_x = 0, I2C_P_y = 0; //OLED_Print�������ַ�����
uint8_t I2C_OLED_GRAM[I2C_PIC_H][I2C_PIC_W]; //����256*128����ʾ����(64,32)~(191,95)



void I2C_init(void)
{
	  GPIO_InitTypeDef GPIO_InitStructure;
	  I2C_InitTypeDef I2C_InitStruct;
	
		__RCC_I2C1_CLK_ENABLE();
		__RCC_GPIOB_CLK_ENABLE();	
    
	  PB06_AFx_I2C1SCL();
	  PB07_AFx_I2C1SDA(); 

		GPIO_InitStructure.Pins = GPIO_PIN_6 | GPIO_PIN_7;
		GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_OD;   ////I2C���뿪©���
		GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
		GPIO_Init(CW_GPIOB, &GPIO_InitStructure);
	
		I2C_InitStruct.I2C_BaudEn = ENABLE;
		I2C_InitStruct.I2C_Baud = 0x08;   
    I2C_InitStruct.I2C_FLT = DISABLE;
    I2C_InitStruct.I2C_AA = DISABLE;  //DISABLE ENABLE
    
		I2C1_DeInit(); 
    I2C_Master_Init(CW_I2C1,&I2C_InitStruct);//��ʼ��ģ��
		I2C_Cmd(CW_I2C1,ENABLE);  //ģ��ʹ��
}

void I2C_MasterWriteEepromData1(I2C_TypeDef *I2Cx,uint8_t u8Addr,uint8_t *pu8Data,uint32_t u32Len)
{
    uint8_t u8i=0,u8State;
	I2C_GenerateSTART(I2Cx, ENABLE);
	while(1)
	{
		while(0 == I2C_GetIrq(I2Cx))
		{;}
		u8State = I2C_GetState(I2Cx);
		switch(u8State)
		{
			case 0x08:   //������START�ź�
				I2C_GenerateSTART(I2Cx, DISABLE);
				I2C_Send7bitAddress(I2Cx, OLED_ADDRESS,0X00);  //���豸��ַ����
				break;
			case 0x18:   //������SLA+W�ź�,ACK���յ�
				I2C_SendData(I2Cx,u8Addr);//���豸�ڴ��ַ����
				break;
			case 0x28:   //������1�ֽ����ݣ�����EEPROM��memory��ַҲ����������ͺ��������Ҳ�����	
				I2C_SendData(I2Cx,pu8Data[u8i++]);
				break;
			case 0x20:   //������SLA+W��ӻ�����NACK
			case 0x38:    //�����ڷ��� SLA+W �׶λ��߷������ݽ׶ζ�ʧ�ٲ�  ����  �����ڷ��� SLA+R �׶λ��߻�Ӧ NACK �׶ζ�ʧ�ٲ�
				I2C_GenerateSTART(I2Cx, ENABLE);
				break;
			case 0x30:   //������һ�������ֽں�ӻ�����NACK
				I2C_GenerateSTOP(I2Cx, ENABLE);
				break;
			default:
				break;
		}			
		if(u8i>u32Len)
		{
			I2C_GenerateSTOP(I2Cx, ENABLE);//��˳���ܵ�������ֹͣ����
			I2C_ClearIrq(I2Cx);
			break;
		}
		I2C_ClearIrq(I2Cx);			
	}
}


void WriteCmd(unsigned char I2C_Command)//д����
{
	I2C_MasterWriteEepromData1(CW_I2C1,COM,&I2C_Command,1);
}

void WriteDat(unsigned char I2C_Data)//д����
{
	I2C_MasterWriteEepromData1(CW_I2C1,DAT,&I2C_Data,1);
}

//����OLED��ʾ
void I2C_OLED_DisPlay_On(void)
{
	WriteCmd(0x8D); //��ɱ�ʹ��
	WriteCmd(0x14); //������ɱ�
	WriteCmd(0xAF); //������Ļ
}

//�ر�OLED��ʾ
void I2C_OLED_DisPlay_Off(void)
{
	WriteCmd(0x8D); //��ɱ�ʹ��
	WriteCmd(0x10); //�رյ�ɱ�
	WriteCmd(0xAF); //�ر���Ļ
}


//�����Դ浽OLED
void I2C_OLED_UPdata(void)
{
	uint8_t i, n;
	//ˮƽģʽ������
	WriteCmd(0x21); //������
	WriteCmd(0x00); //��������ʼ��ַ
	WriteCmd(0x7f); //�����н�����ַ
	WriteCmd(0x22); //����ҳ
	WriteCmd(0x00); //����ҳ��ʼ��ַ
	WriteCmd(0x07); //����ҳ������ַ
	for (i = 0; i < I2C_OLED_H; i++)
	{
		for (n = 0; n < I2C_OLED_W; n++)
		{
			WriteDat(I2C_OLED_GRAM[i + I2C_OFFSET_H][n + I2C_OFFSET_W]);
		}
	}
}

/**
  * @brief  ���ĳ���ߵ�����
  * @param  Line:ѡ��Ҫ������У�0��1��2��3��4��5��6��7
  * @param  Start_x:x�Ὺʼ����
  * @param  End_x:  x���������
  * @retval ��
  */
void I2C_OLED_Clear_Line(uint8_t Line,uint8_t Start_x,uint8_t End_x)
{
	for(uint8_t x=Start_x;x<End_x;x++)
		I2C_OLED_GRAM[Line][x]=0;
}


//��������
//ѡ��λ������
//Inside,Outside,All
void I2C_OLED_Clear(uint8_t OLEDShowUP)
{
	uint16_t i, n;
	for (i = 0; i < I2C_PIC_H; i++)
	{
		for (n = 0; n < I2C_PIC_W; n++)
		{
			I2C_OLED_GRAM[i][n] = 0; //�����������
		}
	}
	I2C_P_x = 0;
	I2C_P_y = 0; //���OLED_Print������д���ַ�����
	I2C_OFFSET_W = I2C_PIC_W / 2 - I2C_OLED_W / 2;
	I2C_OFFSET_H = I2C_PIC_H / 2 - I2C_OLED_H / 2; //������ʾ���뻭��ƫ����Ϊ��������
	if (OLEDShowUP == 1)
		I2C_OLED_UPdata(); //������ʾ
}

//����
//x:0~127
//y:0~63
void I2C_OLED_DrawPoint(uint8_t x, uint8_t y)
{
	uint8_t i, m, n;
	i = y / 8; //�����������ҳ��
	m = y % 8; //���ƫ��λ��
	n = 1 << m;
	I2C_OLED_GRAM[i + I2C_OFFSET_H][x + I2C_OFFSET_W] |= n; //�����ݴ���ͼ��֡
}


//���һ����
//x:0~127
//y:0~63
void I2C_OLED_ClearPoint(uint8_t x, uint8_t y)
{
	uint8_t i, m, n;
	i = y / 8;
	m = y % 8;
	n = 1 << m;
	I2C_OLED_GRAM[i + I2C_OFFSET_H][x + I2C_OFFSET_W] = ~I2C_OLED_GRAM[i + I2C_OFFSET_H][x + I2C_OFFSET_W];
	I2C_OLED_GRAM[i + I2C_OFFSET_H][x + I2C_OFFSET_W] |= n;
	I2C_OLED_GRAM[i + I2C_OFFSET_H][x + I2C_OFFSET_W] = ~I2C_OLED_GRAM[i + I2C_OFFSET_H][x + I2C_OFFSET_W]; //�����
}

//����
//x:0~128
//y:0~64
void I2C_OLED_DrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2)
{
	int i = 0;
	float k, b;
	if (x1 == x2) //������
	{
		for (i = 0; i < (y2 - y1); i++)
		{
			I2C_OLED_DrawPoint(x1, y1 + i);
		}
	}
	/*else if(y1==y2)   //������
	{
			for(i=0;i<(x2-x1);i++)
			{
				OLED_DrawPoint(x1+i,y1);
			}
  }*/
	else //��б��
	{
		k = (y2 - y1) * 10 / (x2 - x1); //б��*10��ֹб��С��1ʱʧ��
		b = y1 - k * x1 / 10;
		while ((x1 + i) != x2)
		{
			I2C_OLED_DrawPoint(x1 + i, (int)((x1 + i) * k / 10 + b));
			if (x1 < x2)
			{
				i++;
			}
			else
			{
				i--;
			}
		}
	}
}


//x,y:Բ������
//r:Բ�İ뾶
void I2C_OLED_DrawCircle(uint8_t x, uint8_t y, uint8_t r)
{
	int a, b, num;
	a = 0;
	b = r;
	while (2 * b * b >= r * r)
	{
		I2C_OLED_DrawPoint(x + a, y - b);
		I2C_OLED_DrawPoint(x - a, y - b);
		I2C_OLED_DrawPoint(x - a, y + b);
		I2C_OLED_DrawPoint(x + a, y + b);

		I2C_OLED_DrawPoint(x + b, y + a);
		I2C_OLED_DrawPoint(x + b, y - a);
		I2C_OLED_DrawPoint(x - b, y - a);
		I2C_OLED_DrawPoint(x - b, y + a);

		a++;
		num = (a * a + b * b) - r * r; //���㻭�ĵ���Բ�ĵľ���
		if (num > 0)
		{
			b--;
			a--;
		}
	}
}


void I2C_OLED_ShowChar(uint8_t x, uint8_t y, char chr)
{
	uint8_t i, m, temp, size = Fonttemp->Height * Fonttemp->Width / 8;
	uint8_t x0 = x;
	uint8_t Pfont[size];

	//��ȡ�ַ�����
	GetConslonsCode(Pfont, chr, Fonttemp);

	//	size=Fonttemp->Height*Fonttemp->Width/8;  //�õ�����һ���ַ���Ӧ������ռ���ֽ���
	for (i = 0; i < size; i++)
	{
		temp = Pfont[i];
		for (m = 0; m < 8; m++) //д������
		{
			if (temp & 0x80)
				I2C_OLED_DrawPoint(x, y);
			else
				I2C_OLED_ClearPoint(x, y);
			temp <<= 1;
			x++;
		}
		if ((x - x0) == (Fonttemp->Width))
		{
			x = x0;
			y++;
		}
	}
}


void I2C_OLED_ShowChar_CH(uint8_t x, uint8_t y, uint16_t usChar)
{
	uint8_t rowCount, bitCount;
	uint8_t ucBuffer[WIDTH_CH_CHAR * HEIGHT_CH_CHAR / 8];
	uint16_t usTemp;
	uint8_t x0 = x;
	//ȡ��ģ����
	GetGBKCode(ucBuffer, usChar);

	for (rowCount = 0; rowCount < HEIGHT_CH_CHAR; rowCount++)
	{
		/* ȡ�������ֽڵ����ݣ���lcd�ϼ���һ�����ֵ�һ�� */
		usTemp = ucBuffer[rowCount * 2]; //ȡ��λ
		usTemp = (usTemp << 8);
		usTemp |= ucBuffer[rowCount * 2 + 1]; //ȡ��λ

		for (bitCount = 0; bitCount < WIDTH_CH_CHAR; bitCount++)
		{
			if (usTemp & (0x8000 >> bitCount)) //��λ��ǰ
				I2C_OLED_DrawPoint(x, y);
			else
				I2C_OLED_ClearPoint(x, y);
			x++;
		}
		if (x - x0 == WIDTH_CH_CHAR)
		{
			x = x0;
			y++;
		}
	}
}

//dir �ƶ�����
//pix �ƶ�����
//cir �����������߽�ʱ���ص�����߽磬����ѭ��Ч��,0��ʾ��ѭ����1��ʾѭ��
void I2C_OLED_Move(uint8_t dir, uint8_t pix, uint8_t cir) //��Ļ��ʾ�����ƶ�
{
	switch (dir)
	{
	case UP:
	{
		I2C_OFFSET_H += pix;
		if (I2C_OFFSET_H >= I2C_PIC_H - I2C_OLED_H)
		{
			if (cir == 0)
				I2C_OFFSET_H = I2C_PIC_H - I2C_OLED_H;
			else
				I2C_OFFSET_H = 0;
		}
		break;
	}
	case DOWN:
	{
		I2C_OFFSET_H -= pix;
		if (I2C_OFFSET_H <= 0)
		{
			if (cir == 0)
				I2C_OFFSET_H = 0;
			else
				I2C_OFFSET_H = I2C_PIC_H - I2C_OLED_H;
		}
		break;
	}
	case LEFT:
	{
		I2C_OFFSET_W += pix;
		if (I2C_OFFSET_W >= I2C_PIC_W - I2C_OLED_W)
		{
			if (cir == 0)
				I2C_OFFSET_W = I2C_PIC_W - I2C_OLED_W;
			else
				I2C_OFFSET_W = 0;
		}
		break;
	}
	case RIGHT:
	{
		I2C_OFFSET_W -= pix;
		if (I2C_OFFSET_W <= 0)
		{
			if (cir == 0)
				I2C_OFFSET_W = 0;
			else
				I2C_OFFSET_W = I2C_PIC_W - I2C_OLED_W;
		}
		break;
	}
	}
	I2C_OLED_UPdata();
}

/**
 * @brief  ����ת���ַ�
 * @param  usX ����ǰ����X
 * @param  usY ����ǰ����Y
 * @param  pStr ��Ҫ��ʾ��Ӣ���ַ�
 * @note 
 * @retval �����ַ����е�ת���ַ��������ش������ַ�����ַ
 */
char* OLED_StrEscCharacter(uint8_t *x,uint8_t *y,char *chr)
{
	uint8_t TabSize;
	while(*chr < ' '&&*chr != '\0')
	{
		switch (*chr)
		{
			case '\b': //�˸�BS��
			{
				*x = *x - Fonttemp->Width;
				break;
			}
			case '\n': //���У�LF��
			{
				switch(Fonttemp->Height)
				{
					case 32:I2C_OLED_Clear_Line((*y/8)+3,*x,128);//�����3��
					case 24:I2C_OLED_Clear_Line((*y/8)+2,*x,128);//�����2��
					case 16:I2C_OLED_Clear_Line((*y/8)+1,*x,128);//�����1��
					case 8:I2C_OLED_Clear_Line((*y/8),*x,128);break;//�����0��
				}
				*x = 0;
				*y = *y + Fonttemp->Height;
				if (*y > (I2C_OLED_H * 8) - Fonttemp->Height) //��Ļд��
				{
					I2C_OLED_Move(UP, 2, 0);
					*y = *y - Fonttemp->Height;
				}
				break;
			}
			case '\r': //�س���CR��
			{
				switch(Fonttemp->Height)
				{
					case 32:I2C_OLED_Clear_Line((*y/8)+3,*x,128);//�����3��
					case 24:I2C_OLED_Clear_Line((*y/8)+2,*x,128);//�����2��
					case 16:I2C_OLED_Clear_Line((*y/8)+1,*x,128);//�����1��
					case 8:I2C_OLED_Clear_Line((*y/8),*x,128);break;//�����0��
				}
				*x = 0;
				*y = *y + Fonttemp->Height;
				if (*y > (I2C_OLED_H * 8) - Fonttemp->Height) //��Ļд��
				{
					I2C_OLED_Move(UP, 2, 0);
					*y = *y - Fonttemp->Height;
				}
				break;
			}
			case '\t': //ˮƽ�Ʊ�HT��
			{
				TabSize = 4 * Fonttemp->Width - (*x % (4 * Fonttemp->Width));
				switch(Fonttemp->Height)
				{
					case 32:I2C_OLED_Clear_Line((*y/8)+3,*x,*x+TabSize);//�����3��
					case 24:I2C_OLED_Clear_Line((*y/8)+2,*x,*x+TabSize);//�����2��
					case 16:I2C_OLED_Clear_Line((*y/8)+1,*x,*x+TabSize);//�����1��
					case 8:I2C_OLED_Clear_Line((*y/8),*x,*x+TabSize);break;//�����0��
				}
				*x = *x + TabSize;
				break;
			}
		}
		chr++;
	}
	return chr;
}

//��ʾ�ַ���
//x,y:�������
//size1:�����С
//*chr:�ַ�����ʼ��ַ
void I2C_OLED_ShowString(uint8_t x, uint8_t y, char *chr)
{
	uint16_t chinese_chr;
	while (*chr != '\0') //�ж��ǲ��ǷǷ��ַ�!
	{
		chr=OLED_StrEscCharacter(&x,&y,chr);
		if( * chr == '\0' )
			break;
		if (*chr < 127) //Ӣ���ַ�
		{
			I2C_OLED_ShowChar(x, y, *chr);
			x += Fonttemp->Width;
			if (x > 128 - Fonttemp->Width) //����
			{
				x = 0;
				y += Fonttemp->Height;
			}
			chr++;
		}
		else if (*chr > 127) //�����ַ�
		{
			chinese_chr = *(uint16_t *)chr;
			chinese_chr = (chinese_chr << 8) + (chinese_chr >> 8); //�õ������ַ�
			I2C_OLED_ShowChar_CH(x, y, chinese_chr);
			x += WIDTH_CH_CHAR;
			if (x > 128 - WIDTH_CH_CHAR) //����
			{
				x = 0;
				y += HEIGHT_CH_CHAR;
			}
			chr += 2;
		}
	}
}

//m^n
uint32_t I2C_OLED_Pow(uint8_t m, uint8_t n)
{
	uint32_t result = 1;
	while (n--)
	{
		result *= m;
	}
	return result;
}

//strָ��Ҫ��ʾ�ַ���ָ��
//size��ʾ�ַ��ֺ�
void I2C_OLED_Print(char *str) //��OLED���ϴ�ӡ�ַ���
{
	uint16_t chinese_str;

	while (*str != '\0')
	{
		str=OLED_StrEscCharacter(&I2C_P_x,&I2C_P_y,str);
		if( * str == '\0' )
			break;
		if (*str < 127) //Ӣ���ַ�
		{
			I2C_OLED_ShowChar(I2C_P_x, I2C_P_y, *str);
			I2C_P_x = I2C_P_x + Fonttemp->Width;
			if (I2C_P_x > 128 - Fonttemp->Width) //����
			{
				I2C_P_x = 0;
				I2C_P_y = I2C_P_y + Fonttemp->Height;
				if (I2C_P_y > 32 - Fonttemp->Height) //��Ļд��
				{
					I2C_OLED_Move(UP, 2, 0);
					I2C_P_y = I2C_P_y - Fonttemp->Height;
				}
			}
			str++;
		}
		else //�����ַ�
		{
			chinese_str = *(uint16_t *)str;
			chinese_str = (chinese_str << 8) + (chinese_str >> 8); //�õ������ַ�
			I2C_OLED_ShowChar_CH(I2C_P_x, I2C_P_y, chinese_str);
			I2C_P_x += WIDTH_CH_CHAR;

			if (I2C_P_x > 128 - WIDTH_CH_CHAR) //����
			{
				I2C_P_x = 0;
				I2C_P_y = I2C_P_y + HEIGHT_CH_CHAR;
				if (I2C_P_y > 32 - HEIGHT_CH_CHAR) //��Ļд��
				{
					I2C_OLED_Move(UP, 2, 0);
					I2C_P_y = I2C_P_y - HEIGHT_CH_CHAR;
				}
			}
			str += 2; //һ�����������ֽ�
		}
	}
	I2C_OLED_UPdata();
}


void I2C_OLED_ShowPicture(uint8_t x, uint8_t y, uint8_t Bmp_W, uint8_t Bmp_H, uint8_t *BMP)
{
	uint8_t x0, y0; //��ʱ����x��y��ֵ
	x0 = x;
	y0 = y;
	for (y = y0; y < y0 + Bmp_H; y++)
	{
		for (x = x0; x < x0 + Bmp_W; x++)
		{
			I2C_OLED_GRAM[y + I2C_OFFSET_H][x + I2C_OFFSET_W] = *BMP;
			BMP++;
		}
	}
	I2C_OLED_UPdata();
}


//OLED�ĳ�ʼ��
void I2C_OLED_Init(void)
{
	delay1ms(200);

	WriteCmd(0xAE);//�ر���ʾ
	
	WriteCmd(0x40);//---set low column address
	WriteCmd(0xB0);//---set high column address

	WriteCmd(0xC8);//-not offset

	WriteCmd(0x81);//���öԱȶ�
	WriteCmd(0xff);

	WriteCmd(0xa1);//���ض�������

	WriteCmd(0xa6);//
	
	WriteCmd(0xa8);//��������·��
	WriteCmd(0x1f);
	
	WriteCmd(0xd3);
	WriteCmd(0x00);
	
	WriteCmd(0xd5);
	WriteCmd(0xf0);
	
	WriteCmd(0xd9);
	WriteCmd(0x22);
	
	WriteCmd(0xda);
	WriteCmd(0x02);
	
	WriteCmd(0x20); //-Set Page Addressing Mode (0x00/0x01/0x02)
	//WriteCmd(0x02);//ҳ��ַģʽ
	WriteCmd(0x00); //ˮƽѰַģʽ

	WriteCmd(0xdb);
	WriteCmd(0x49);
	
	WriteCmd(0x8d);
	WriteCmd(0x14);
	
	WriteCmd(0xaf); 
}

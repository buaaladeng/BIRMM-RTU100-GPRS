/*******
#include <iom8v.h>
#include <macros.h>
#define DATA      PIND
#define IO_PORT   PORTD
#define DRITION   DDRD
#define IO_BIT    3

#define SET_DRITION_IN()    DRITION &=~( 1<<IO_BIT )
//#define SET_DRITION_IN()    DRITION &=~( 1<<IO_BIT ) //DAT_OUT_1()
//#define SET_DRITION_OUT()   DRITION |= ( 1<<IO_BIT ) 
//#define READ_DAT()          (PIND&0x08)
#define READ_DAT()          (DATA&(1<<IO_BIT))
#define DAT_OUT_1()         IO_PORT |= ( 1<<IO_BIT ) 
#define DAT_OUT_0()         IO_PORT &=~( 1<<IO_BIT ) 
***************/
#include "DS2780.h"
#include "bsp_SysTick.h"
#include "bsp_usart.h"
#include "SPI_Flash.h"
//P1DIR |= 0x01;                            // Set P1.0 to output direction
//#define SET_DRITION_OUT()   P1DIR |= BIT5   //ÉèÖÃP1.5ÎªÊä³ö
//#define SET_DRITION_IN()    P1DIR &=~BIT5  //ÉèÖÃP1.5ÎªÊäÈë
//#define READ_DAT()          (P1IN&0x20) 
//#define DAT_OUT_1()         P1OUT |=0x20// P1.5 Êä³ö¸ßµçÆ½
//#define DAT_OUT_0()         P1OUT &=0xDF//p1.5 Êä³öµÍµçÆ½


static  uint8_t   RegB=0x00;
extern  char      PowerOffReset;          //µôµçÖØÆô±êÖ¾Î»
//extern  uint16_t  BATT_Capacity ;                    //Ò»×éµç³Øµ±Ç°Ê£ÓàµçÁ¿£¨ÂúÈİÁ¿19Ah£
/*******************************************************************************
* Function Name  : XX
* Description    : XX
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
uint8_t DS2780_Test(void)
{
    uint16_t  dats1=0,dats2=0,dats3=0,dats6=0;
    uint16_t  temp=0;
    uint8_t   FULL_status=0;
	  float     RAAC_OUT=0;
    
	  if (PowerOffReset ==0)                             //Ã»ÓĞ·¢Éúµôµç£¬ÔòÖ±½Ó´ÓFlashÖĞ¶Á³öµç³ØÈİÁ¿ÏµÊı
	  {
			DataRead_From_Flash(0,1,0, &RegB,1);             //´ÓFlashÖĞ¶Á³öµç³ØÈİÁ¿ÏµÊı
			if(RegB>113)    //·ÀÖ¹·¢ÉúFlashÒì³£¸´Î»
			{
        RegB=113;
				printf("\n\r RegB  ERROR!!\r\n");
      }
			printf("\n\r RegB:%d \r\n",RegB);                //²âÊÔÊ¹ÓÃ
    }
		else
		{	
			DataWrite_To_Flash(0,1,0, &RegB,1);              //³õ´ÎÉÏµçÊ±£¬½«µç³ØÈİÁ¿ÏµÊıĞ´ÈëFlash
			PowerOffReset =0;                                //µôµçÖØÆô±êÖ¾±äÁ¿¸´Î»
		}	
	  dats1 = get_voltage();
    dats2 = get_current();
    dats3 = get_ACR();
	//temp = -10;
	//dats2 = temp;
	//temp = dats2;
  	temp = dats2&0x7FFF;
	  printf("\n\r µç³ØµçÁ÷current:%d (mA)\r\n",temp);   //²âÊÔÊ¹ÓÃ
    temp = RegB;            //ÔİÊ±½«µç³ØµçÁ¿ÏµÊı±£´æ£¬±ãÓÚºóĞø×ö±È½Ï
	//get_accumulate();
	//get_RAAC();µçÁ¿ÀÛ¼Æ£¬·ÅµçµÃ¸º£¬³äµçµÃÕı£¬µ¥Î»ÊÇmah
	//dats4 = get_AE();
  //dats5 = get_RARC(); 
//	  get_AE();
//    get_RARC(); 
    dats6 = get_RAAC();      //µÃµ½Ê£ÓàµÄµçÁ¿£¬µçÁ¿ÊÇmah
//    dats7 = get_FULL();      //¶ÁÏÖÓĞµçÁ¿×´Ì¬£¬°Ù·Ö±È
	  printf("\n\r BATT Capacity left:%d mAh\r\n", dats6);   //²âÊÔÊ¹ÓÃ
		
		if(dats6 == 0)
		{
			set_ACR(1000);
			Delay_ms(1000);       // ÑÓÊ±10ms£¬¾ßÌåÑÓÊ±Ê±¼äÓĞ´ıÈ·ÈÏ
			if(RegB !=0)           //·ÀÖ¹³öÏÖÒç³ö
			{
        RegB--;	
				dats6 = get_RAAC();  //µç³ØÏµÊıĞŞÕıÒÔºó£¬ĞèÒªÖØĞÂ¶ÁÈ¡µç³ØÈİÁ¿£¬·ñÔòºóĞøµç³ØÏà¶ÔÊ£ÓàÈİÁ¿»á³öÏÖ²¨¶¯
      }		
		}
		
		RAAC_OUT = RegB + dats6/1000;    //µÃµ½Ê£ÓàµçÁ¿£»
		FULL_status=(RAAC_OUT*100)/114;     //Ê£ÓàµçÁ¿°Ù·Ö±È
		printf("\n\r µç³ØµçÑ¹voltage:%d (mV)\r\n",dats1);
		//printf("\n\r µç³ØµçÁ÷current:%f (uA)\r\n",dats2);
		printf("\n\r µçÁ¿ÀÛ¼Æaccumulate:%d mAh\r\n",dats3);
		printf("\n\r Ê£Óà¾ø¶ÔµçÁ¿RAAC:%d mAh\r\n",dats6);
		//printf("\n\r FULL",dats7);
		printf("\n\r Ê£ÓàÏà¶ÔµçÁ¿FULL_status:%d%%\r\n",FULL_status);
    //FULL_status = FULL_status*2.55;
		if(temp != RegB)      //µ±µç³ØµçÁ¿ÏµÊı·¢Éú±ä»¯Ê±£¬½«×îĞÂµÄÊıÖµĞ´ÈëFlash£»Ã»ÓĞ±ä»¯Ê±£¬²»¸üĞÂFlash
		{
       DataWrite_To_Flash(0,1,0, &RegB,1);   //½«µç³ØÈİÁ¿ÏµÊıĞ´ÈëFlash
    }
    return FULL_status;
}
/*******************************************************************************
* Function Name  : XX
* Description    : XX
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DS2780_CapacityInit(void)
{
   RegB = 54;   //¿âÂØ¼Æ³õÊ¼»¯²ÎÊı£¬ĞèÒª±£´æÔÚFlashÖĞ£¬ÉÏµçÖ»ÄÜ¸³ÖµÒ»´Î£¬ºóĞøÓĞ´ıÍêÉÆ
}
/*******************************************************************************
* Function Name  : XX
* Description    : XX
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DS2780_DQ_OUT(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO , ENABLE);
  
  /* DQ -PC7 configration */	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
}
void DS2780_DQ_IN(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO , ENABLE);
  /* DQ -PC7 configration */	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
}
unsigned char reset(void)
{ 
  unsigned char presence = 0 ;

  DS2780_DQ_OUT(); // ¸ÄPC_7 ÎªÊä³ö¿Ú	
	DQ_OUT_0; // À­µÍDQ Ïß
	Delay_us(t_RSTL); //ÑÓÊ±ÖÁÉÙ t_RSTL ¦Ìs
	DQ_OUT_1;// ½«DQ ÏßÉèÖÃÎªÂß¼­¸ß
	DS2780_DQ_IN(); // ¸ÄPC_7 ÎªÊäÈë¿Ú
	Delay_us(t_PDH); // ÑÓÊ±µÈ´ıpresence ÏìÓ¦
	//Delay_us(t_PDL/2);
	//Delay_us(530);
	presence = GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_7);
	//presence=READ_DAT(); // ²ÉÑùpresenceĞÅºÅ
	//presence= PIND&0x08;
	Delay_us(t_RSTH-t_PDH-t_PDL/2);//µÈ´ıÊ±Ğò½áÊø
	return(presence); //ÓĞpresence ĞÅºÅÊ±·µ»Ø0 £¬·ñÔò·µ»Ø1
}

//¶ÁÒ»Î»º¯Êı
unsigned char read_bit(void)
{
  unsigned char result;
	DS2780_DQ_OUT(); // ¸ÄPC_7ÎªÊä³ö¿Ú
	DQ_OUT_0;
	Delay_us(1);// ×ÜÏßÎªµÍµçÆ½µÄÊ±¼äÖÁÉÙ³ÖĞø1 ¦Ìs
	DS2780_DQ_IN(); // ¸ÄPC_7 ÎªÊäÈë¿Ú
	Delay_us(t_RDV);// ×ÜÏßÎªµÍµçÆ½µÄÊ±¼äÖÁÉÙ³ÖĞø1 ¦Ìs
	result = GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_7); // ²ÉÑùresult ĞÅºÅ
	Delay_us(t_SLOT-t_RDV-1);// µÈ´ıÊ±Ğò½áÊø
	return(result); // ·µ»Ø²ÉÑùÖµ
}
/*//Ğ´Ò»Î»º¯Êı
void write_bit(unsigned char bit_value)
{
    // Èç¹ûĞ´Âß¼­"1"bit£¬Ê¹µÃDQ Îª¸ß
	//if(bit_value == 1)
	//DS2780_DQ_OUT();
	if(bit_value)
	{
	    DQ_OUT_0
		Delay_us(t_LOW1); //µ÷ÓÃDelay_us()º¯Êı´óÔ¼ĞèÒª5 ¦Ìs
		DQ_OUT_1;
		Delay_us(t_SLOT-t_LOW1);
	}
    else
	{
	    DQ_OUT_0
		Delay_us(t_LOW0);
		DQ_OUT_1;
		//Delay_us(t_REC); // ±£³ÖÑÓÊ±ÖÁÊ±Ğò½áÊø
		Delay_us(t_SLOT-t_LOW0);
    }
}*/////
//Ğ´Ò»¸ö×Ö½Úº¯Êı
void write_byte(unsigned char value)
{
  unsigned char i;
//	unsigned char temp;
	DS2780_DQ_OUT();
  for(i=0;i<8;i++)
	{
		if(value&0x01)
		{
	      DQ_OUT_0; 
		    Delay_us(t_LOW1); //µ÷ÓÃDelay_us()º¯Êı´óÔ¼ĞèÒª5 ¦Ìs
		    DQ_OUT_1;
		    Delay_us(t_SLOT-t_LOW1);		
		}
		else
		{
			DQ_OUT_0;
		    Delay_us(t_LOW0);
		    DQ_OUT_1;
		    Delay_us(t_SLOT-t_LOW0);
		}
		value=value>>1; // ½«value Î»ÓÒÒÆi Î»¸³Öµ¸øtemp
  }
}
//¶ÁÒ»¸ö×Ö½Úº¯Êı
unsigned char read_byte(void)
{
  unsigned char i,value=0;
//	  unsigned char m;
//	  m = 1;
	value = 0;
	//DS2780_DQ_IN(); // ¸ÄPC_7 ÎªÊäÈë¿Ú
	for(i=0;i<8;i++)
	{
	    value>>=1; // Ã¿¶ÁÒ»Î»Êı¾İ£¬×óÒÆÒ»Î»
	    if(read_bit()) 
		  {
		    value|=0x80;
      }
  }
  return(value);
}	

/*
Read_data    0x69
#define Write_data   0x6c
#define Copy_data    0x48
#define recall_data  0xb8
#define lock_data    0x6a*/
//¸øds2780Ö¸¶¨µØÖ·¶Á³öÒ»¸ö×Ö½Ú
unsigned char read_byte_data(unsigned char start_address)
{
//    unsigned char i,k;
	unsigned char one_byte_data=0;
	write_ds2780_cmd(Read_data,start_address);
	one_byte_data = read_byte();
	return(one_byte_data);
}
//Íùds2780Ö¸¶¨µØÖ·Ğ´ÈëÒ»¸ö×Ö½Ú
void write_data(unsigned char write_address,unsigned char dat)
{
//  unsigned char i;
//	unsigned char one_byte_data;
//	unsigned char pres;
	write_ds2780_cmd(Write_data,write_address);
	write_byte(dat);//Ğ´ÈëÊı¾İ
	Delay_us(10000);
}

void write_ds2780_cmd(unsigned char cmd,unsigned char address)
{
    unsigned char pres;
restart:
	pres = reset();
	if(pres) //return fause; // Èç¹ûÃ»ÓĞÆ÷¼ş£¬·µ»Ø0
	{
	   Delay_us(100);
	   goto restart;
	}
	write_byte(0xCC); //Ö÷»ú·¢³öSkip Net AddressÃüÁî
  write_byte(cmd); // Ö÷»ú·¢³öxxÃüÁî
	write_byte(address); // Ö÷»ú·¢³öµØÖ·ÃüÁî
	//return ture;
}
void recall_data_ds2780(unsigned char address)
{
  write_ds2780_cmd(recall_data,address);
	Delay_us(10000);
}
void copy_data_ds2780(unsigned char address)
{
  write_ds2780_cmd(Copy_data,address);
	Delay_us(10000);
}

//µÃµ½µçÑ¹
unsigned int get_voltage(void)
{
  unsigned char v_h=0,v_l=0;
	unsigned int voltage=0;
	
  v_h = read_byte_data(0x0c);
	voltage = v_h;
	voltage <<=3;
	Delay_us(10000);
	v_l = read_byte_data(0x0d);
	voltage += v_l>>5;
	voltage = voltage&0x3ff;
	voltage = voltage*4.88;
	return voltage;
}
//Ê£ÓàµçÁ¿µÄ°Ù·Ö±È
/*unsigned int get_RSAC(void)
{
	unsigned int voltage;
    voltage = read_byte_data(0x06);
	return voltage;
}*/

//Ê£ÓàµçÁ¿µÄ°Ù·Ö±È
unsigned int get_RARC(void)
{
	unsigned int voltage=0;
  voltage = read_byte_data(0x06);
	//voltage = read_byte_data(0x07);
	return voltage;
}

//µçÁ÷²âÁ¿£¬µ¥Î»ÊÇma
unsigned int get_current(void)
{
    unsigned char a_h,a_l,pos_neg;
    unsigned int current=0;
    a_h = read_byte_data(0x0e);
	//a_h = read_byte_data(0x08);
	current = a_h;
	if(a_h&0x80)//·Åµç´ËÊ±ÊÇ¸ºÊı£¬ÔòÎª²¹Âë
	{
	    pos_neg = 0;
	}
	else
	{
	    pos_neg = 1;
	}
	current <<=8;
	Delay_us(10000);
    //a_l = read_byte_data(0x09);
	a_l = read_byte_data(0x0f);
	current += a_l;
	current = current&0x7fff;
	if( pos_neg == 0 )//Ëã³öÔ­Âë
	{
	    current =(~(current-1))&0x7fff;
	    current = current*0.07813;//current = current*0.3125;//0.03125¶ÔÓ¦5mO
 
	    return current|0x8000;
	}
	
	current = current*0.07813;//current = current*0.3125;
   
	return current;
}

//ÎÂ¶È£¬·Å´óÁË10±¶£¬µ¥Î»ÊÇ¶È
unsigned int get_temperature(void)
{
  unsigned char t_h,t_l,pos_neg;
	unsigned int temper=0;
	
  t_h = read_byte_data(0x0a);
	temper = t_h;
	if(t_h&0x80)//·Åµç´ËÊ±ÊÇ¸ºÊı£¬ÔòÎª²¹Âë
	{
	    pos_neg = 0;
	}
	else
	{
	    pos_neg = 1;
	}
	temper <<=3;
	Delay_us(10000);
	t_l = read_byte_data(0x0b);
	temper += t_l>>5;
	temper = temper&0x03ff;
	if( pos_neg == 0 )//Ëã³öÔ­Âë
	{
	    temper =(~(temper-1))&0x03ff;
	    //temper = temper*1.25;
       temper = temper*0.3125;//¶ÔÓ¦20mo
	    return temper|0x8000;
	}
	
	//temper = temper*1.25;
	temper = temper*0.3125;//¶ÔÓ¦20mo
   
	return temper;
}


//µçÁ¿ÀÛ¼Æ£¬·ÅµçµÃ¸º£¬³äµçµÃÕı£¬µ¥Î»ÊÇmah
unsigned int get_ACR(void)
{
    unsigned char t_h,t_l,pos_neg;
  	unsigned int accumulate=0;
	//t_h = read_byte_data(0x04);
    t_h = read_byte_data(0x10);
	accumulate = t_h;
	if(t_h&0x80)//·Åµç´ËÊ±ÊÇ¸ºÊı£¬ÔòÎª²¹Âë
	{
	    pos_neg = 0;
	}
	else
	{
	    pos_neg = 1;
	}
	accumulate <<=8;
	Delay_us(10000);
	//t_l = read_byte_data(0x05);
	t_l = read_byte_data(0x11);
	accumulate += t_l;
	if( pos_neg == 0 )//Ëã³öÔ­Âë
	{
	    accumulate =(~(accumulate-1))&0x7fff;
	   // accumulate = accumulate*1.25;//1.25¶ÔÓ¦5mo£»
     accumulate = accumulate*0.3125;//¶ÔÓ¦20mo
	    return accumulate|0x8000;
	}
	//accumulate = accumulate*1.25;
	accumulate = accumulate*0.3125;
   
	return accumulate;
}
//µÃµ½Ê£ÓàµÄµçÁ¿£¬µçÁ¿ÊÇmah
unsigned int get_RAAC(void)
{
  unsigned char t_h=0,t_l=0;
	unsigned int  accumulate=0;
	
  t_h = read_byte_data(0x02);
	accumulate = t_h;
	accumulate <<=8;
	Delay_us(10000);
	t_l = read_byte_data(0x03);
	accumulate += t_l;
	
	accumulate = accumulate*1.6;
   
	return accumulate;
}


//¶ÁÏÖÓĞµçÁ¿×´Ì¬£¬°Ù·Ö±È
unsigned int get_FULL(void)
{
    unsigned char t_h,t_l;
	unsigned int accumulate=0;
  t_h = read_byte_data(0x16);
	accumulate = t_h&0x3f;
	accumulate <<=8;
	Delay_us(10000);
	t_l = read_byte_data(0x17);
	accumulate += t_l;
	accumulate = accumulate*0.0061;
	return accumulate;
	
}

//¶ÁÏÖÓĞµçÁ¿×´Ì¬£¬°Ù·Ö±È
unsigned int get_AE(void)
{
  unsigned char t_h,t_l;
	unsigned int accumulate=0;
  t_h = read_byte_data(0x18);
	accumulate = t_h&0x1f;
	accumulate <<=8;
	Delay_us(10000);
	t_l = read_byte_data(0x19);
	accumulate += t_l;
	accumulate = accumulate*0.0061;
	return accumulate;
}

//¶Á¿ØÖÆ¼Ä´æÆ÷
unsigned int get_STATUS(void)
{
    unsigned char cmds=0;
	//recall_data_ds2780(0x7b);
	//Delay_us(10000);
    cmds = read_byte_data(0x01);
	//cmds = read_byte_data(0x7b);
   /*recall_data_ds2780(0x68);
	Delay_us(10000);
    cmds = read_byte_data(0x68);*/
	return cmds;
}
//Ğ´¿ØÖÆ¼Ä´æÆ÷
void set_STATUS(unsigned char cmds)
{
	write_ds2780_cmd(cmds,0x01);
	Delay_us(10000);
  copy_data_ds2780(0x01);
}

//¶Á¿ØÖÆ¼Ä´æÆ÷
unsigned int get_sssd(void)
{
  unsigned char cmds=0;
	
	recall_data_ds2780(0x66);
	Delay_us(10000);
  cmds = read_byte_data(0x66);
   
	return cmds;
}

//¶Á¿ØÖÆ¼Ä´æÆ÷
void set_ACR(unsigned int raac)
{
   unsigned int dats;
	 unsigned char b;
	 //dats = raac/1.25;//¶ÔÓ¦5mO
   dats = raac/0.3125;//¶ÔÓ¦20mO
	 b = dats>>8;
   write_data(0X10,b);
	 Delay_ms(40);
	 copy_data_ds2780(0X10);
	 Delay_ms(40);
	 b = dats&0x00ff;
	 write_data(0X11,b);
	 Delay_ms(40);
	 copy_data_ds2780(0X11);

}



/********************************************
Õë¶ÔËÄ½ÚÌúï®µç³ØÉèÖÃµÄ²ÎÊı
³äÂúµçµçÑ¹Îª14.4v       VCHG      0xb4
³äÂúµçµçÁ÷ÉèÖÃÎª50ma    IMIN      0x05
·ÅÍêµçµçÑ¹Îª11.8v       VAE       0x93
·ÅÍêµçµçÁ÷Îª300ma       IAE       0x07

µç×è²ÉÓÃ5mÅ·            RSNSP     0x01
µçÁ÷²¹³¥·Å´óÎª1.270±¶   RSGAIN    0x0514
FULL´æ´¢40¶ÈÏÂµÄÈİÁ¿100%    FULLS 0X3FFF
µç³ØÈİÁ¿6000mah         AC        0x12c0
Ê§µ÷µçÁ÷0.5ma           AB        0x02
µç×èµÄÎÂ¶È²¹³¥ÉèÎª0     RSGAIN    0X0000

FULL3040   0x0f
FULL2030   0x1c
FULL1020   0x26
FULL0010   0x27

AE3040   0x07
AE2030   0x10
AE1020   0x1E
AE0010   0x12 

SE3040   0x02
SE2030   0x05
SE1020   0x05
SE0010   0x0A

¿ØÖÆ¼Ä´æÆ÷ CONTROL  0x00
*******************************************/
/********************************************
¸ßÄÜ¾ÛºÏÎïµç³ØÉèÖÃµÄ²ÎÊı(Ò»´ÎĞÔ·Ç³äµçµç³Ø)
³äµçÑ¹Îª7.2v       VCHG      0x5A
³äÂúµçµçÁ÷ÉèÖÃÎª50ma    IMIN      0x05
·ÅÍêµçµçÑ¹Îª6v       VAE       0x4B
·ÅÍêµçµçÁ÷Îª10ma (¹À¼ÆÖµ)      IAE       0x01

µç×è²ÉÓÃ20mÅ·            RSNSP     0x32
µçÁ÷²¹³¥·Å´óÎª1.270±¶   RSGAIN    0x0514
FULL´æ´¢40¶ÈÏÂµÄÈİÁ¿100%    FULLS 0X3FFF
µç³ØÈİÁ¿19000mah         AC        0xED80 £»
Ê§µ÷µçÁ÷0.5ma           AB        0x02
µç×èµÄÎÂ¶È²¹³¥ÉèÎª0     RSGAIN    0X0000

FULL3040   0x0f
FULL2030   0x1c
FULL1020   0x26
FULL0010   0x27

AE3040   0x07
AE2030   0x10
AE1020   0x1E
AE0010   0x12 

SE3040   0x02
SE2030   0x05
SE1020   0x05
SE0010   0x0A

¿ØÖÆ¼Ä´æÆ÷ CONTROL  0x00

  


*******************************************/
void Set_register_ds2780(void)
{
//    unsigned char pres;
   write_data(CONTROL,0x00);
	 copy_data_ds2780(CONTROL);
	//Delay_ms(1);
 	
    write_data(AB,0x02);
	//write_data(AB,0x00);
	copy_data_ds2780(AB);	
	
    write_data(15,50);
	copy_data_ds2780(15);	
	
	write_data(AS,28);
	copy_data_ds2780(AS);	
	
	//write_data(ACMSB,0x12);
	//copy_data_ds2780(ACMSB);
   //write_data(ACLSB,0xc0);
	//copy_data_ds2780(ACLSB);	
		
//	write_data(ACMSB,0x1f);
//	copy_data_ds2780(ACMSB);
//  write_data(ACLSB,0x40);
//	copy_data_ds2780(ACLSB);	
	write_data(ACMSB,0xED);//19Ah
	copy_data_ds2780(ACMSB);
  write_data(ACLSB,0x80);
	copy_data_ds2780(ACLSB);
	
	
	//write_data(VCHG,0xa0);//µçÑ¹Ê±13.6v
	//write_data(VCHG,0xaf);//µçÑ¹Ê±14.0v
//	write_data(VCHG,0xb4);//µçÑ¹Ê±14.4v
//	copy_data_ds2780(VCHG);
	//write_data(VCHG,0x5A);//µçÑ¹Ê±7.2v
	write_data(VCHG,0xB8);//µçÑ¹Ê±3.6v
//	write_data(VCHG,0xA9);//µçÑ¹Ê±3.3v
	
	copy_data_ds2780(VCHG);
	
  write_data(IMIN,0x14);//µçÁ÷50mA
	copy_data_ds2780(IMIN);		
	
	//write_data(VAE,0x4B);//6VÊ±4B,11.3VÊ±0x8d,11.8vÊ±0x93
//	write_data(VAE,0x9C);//3.05VÊ±9C
	write_data(VAE,0x8F);//2.8VÊ±98F
	copy_data_ds2780(VAE);
	
	write_data(IAE,0x08);//80mA
	copy_data_ds2780(IAE);
	
	//write_data(RSNSP,0x00);
	//copy_data_ds2780(RSNSP);
	write_data(RSNSP,0x32);
	copy_data_ds2780(RSNSP);
	
	
	/*write_data(FULLSMSB,0x3f);
	copy_data_ds2780(FULLSMSB);	
	write_data(FULLSLSB,0xff);
	copy_data_ds2780(FULLSLSB);	*/

	write_data(FULLSMSB,0x1f);
	copy_data_ds2780(FULLSMSB);	
	write_data(FULLSLSB,0x40);
	copy_data_ds2780(FULLSLSB);		
	
	write_data(0x68,0x06);
	copy_data_ds2780(0x68);	
/*
	write_data(FULL3040,0x00);
	copy_data_ds2780(FULL3040);	
	
	write_data(FULL2030,0x00);
	copy_data_ds2780(FULL2030);	
	
	write_data(FULL1020,0x00);
	copy_data_ds2780(FULL1020);	
	
	write_data(FULL0010,0x00);
	copy_data_ds2780(FULL0010);	
	
	write_data(AE3040,0x00);
	copy_data_ds2780(AE3040);	
	
	write_data(AE2030,0x00);
	copy_data_ds2780(AE2030);
	
	write_data(AE1020,0x00);
	copy_data_ds2780(AE1020);	
	
	write_data(AE0010,0x00);
	copy_data_ds2780(AE0010);	
	
	write_data(SE3040,0x00);
	copy_data_ds2780(SE3040);	
	
	write_data(SE2030,0x00);
	copy_data_ds2780(SE2030);
	
	write_data(SE1020,0x00);
	copy_data_ds2780(SE1020);	
	
	write_data(SE0010,0x00);
	copy_data_ds2780(SE0010);	
	*/
	
	write_data(FULL3040,0x0F);
	copy_data_ds2780(FULL3040);	
	
	write_data(FULL2030,0x1C);
	copy_data_ds2780(FULL2030);	
	
	write_data(FULL1020,0x26);
	copy_data_ds2780(FULL1020);	
	
	write_data(FULL0010,0x27);
	copy_data_ds2780(FULL0010);	
	
	write_data(AE3040,0x07);
	copy_data_ds2780(AE3040);	
	
	write_data(AE2030,0x10);
	copy_data_ds2780(AE2030);
	
	write_data(AE1020,0x1E);
	copy_data_ds2780(AE1020);	
	
	write_data(AE0010,0x12);
	copy_data_ds2780(AE0010);	
	
	write_data(SE3040,0x02);
	copy_data_ds2780(SE3040);	
	
	write_data(SE2030,0x05);
	copy_data_ds2780(SE2030);
	
	write_data(SE1020,0x05);
	copy_data_ds2780(SE1020);	
	
	write_data(SE0010,0x0A);
	copy_data_ds2780(SE0010);	
	
   /* write_data(RSGAINMSB,0x04);
	copy_data_ds2780(RSGAINMSB);	
	//write_data(RSGAINLSB,0x0f);
	write_data(RSGAINLSB,0x50);//´óµçÁ÷Öµ
	copy_data_ds2780(RSGAINLSB);	*/
	
	write_data(RSGAINMSB,0x04);
	copy_data_ds2780(RSGAINMSB);	
	//write_data(RSGAINLSB,0x0f);
	write_data(RSGAINLSB,0x08);//´óµçÁ÷Öµ
	copy_data_ds2780(RSGAINLSB);	
	/*
	write_data(RSGAINMSB,0x05);
	copy_data_ds2780(RSGAINMSB);	
	write_data(RSGAINLSB,0x19);
	copy_data_ds2780(RSGAINLSB);	*/
	
	write_data(RSTC,0x00);
	copy_data_ds2780(RSTC);
		
}

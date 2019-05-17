#include "header.h"
#include "timerapp.h"
#include "ds18b20.h"

#define PORTADDRESS GPIO_Pin_15
#define TEMP_SENSOR	GPIOA
#define DS18B20_DQ_IN GPIO_ReadInputDataBit(TEMP_SENSOR,PORTADDRESS)
u8 retry1;
void tickDelay (u16 time)
{
	dELAY_us(time);
}
//-----------------------------------------------------------------------------
// Setup DQ pin
//

//#define DS18B20_IO_IN()  {TEMP_SENSOR->CRH&=0x0FFFFFFF;TEMP_SENSOR->CRH|=8<<28;}
//#define DS18B20_IO_OUT() {TEMP_SENSOR->CRH&=0x0FFFFFFF;TEMP_SENSOR->CRH|=3<28;}
void outp(u32 port)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);
	TEMP_SENSOR->CRH &= 0x0FFFFFFF;
	TEMP_SENSOR->CRH |= (u32) 3<<28;  //PA15 �����������
}

void inp(u32 port)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);
	TEMP_SENSOR->CRH &= 0x0FFFFFFF;
	TEMP_SENSOR->CRH|= (u32) 8<<28;
}



//-----------------------------------------------------------------------------
// Send a 1-Wire write bit. Provide 10us recovery time.
//

void OWWriteBit(int bit)
{
        if (bit)
        {
					// Write '1' bit
//					outp(PORTADDRESS); // Drives DQ low
					GPIO_ResetBits(TEMP_SENSOR,PORTADDRESS);
					tickDelay(6);
					GPIO_SetBits(TEMP_SENSOR,PORTADDRESS); // Releases the bus
					tickDelay(64); // Complete the time slot and 10us recovery
        }
        else
        {
					// Write '0' bit
					GPIO_ResetBits(TEMP_SENSOR,PORTADDRESS); // Drives DQ low
					tickDelay(64);
					GPIO_SetBits(TEMP_SENSOR,PORTADDRESS); // Releases the bus
					tickDelay(5);
        }
}

//-----------------------------------------------------------------------------
// Read a bit from the 1-Wire bus and return it. Provide 10us recovery time.
//
int OWReadBit(void)
{
        int dat;

        outp(PORTADDRESS); // Drives DQ low
				GPIO_ResetBits(TEMP_SENSOR,PORTADDRESS);
				tickDelay(2);
				GPIO_SetBits(TEMP_SENSOR,PORTADDRESS);

        inp(PORTADDRESS); // Releases the bus
        tickDelay(12);
        dat = (GPIO_ReadInputDataBit(TEMP_SENSOR, PORTADDRESS))&0x01; // Sample the bit value from the slave
        tickDelay(50); // Complete the time slot and 10us recovery

        return dat;
}

//-----------------------------------------------------------------------------
// Write 1-Wire data byte
//
void OWWriteByte(int data)
{
        int loop;
				outp(PORTADDRESS); 
			
				tickDelay(6);
        // Loop to write each bit in the byte, LS-bit first
        for (loop = 0; loop < 8; loop++)
        {
                OWWriteBit(data & 0x01);

                // shift the data byte for the next bit
                data >>= 1;
        }
}

//-----------------------------------------------------------------------------
// Read 1-Wire data byte and return it
//

int OWReadByte(void)
{
        int loop, result=0,j;

        for (loop = 0; loop < 8; loop++)
        {
                // shift the result to get it ready for the next bit
                j = OWReadBit();

                // if result is one, then set MS bit
								
                result = (j<<7)|(result>>1);
                        
        }
        return result;
}

//-----------------------------------------------------------------------------
// Reset a 1-Wire device
//
void DS18B20_Rst(void)	   
{                 
	outp(PORTADDRESS); 	
  GPIO_ResetBits(TEMP_SENSOR,PORTADDRESS); //Drive the DQ low
  tickDelay(750);  //760.8us
  GPIO_SetBits(TEMP_SENSOR,PORTADDRESS); 	//DQ=1 
	tickDelay(15);     	//15US   //27.2us
}

//-----------------------------------------------------------------------------
// Check the 1-Wire device on the bus
// return 0: there is a device being ready for use. 
// return 1: no reponse from any device on the bus.
//
u8 DS18B20_Check(void) 	   
{   
	u8 retry=0;
	inp(PORTADDRESS);
	
    while (DS18B20_DQ_IN&&retry<200)
	{
		retry++;
		tickDelay(1);
	};	 
	if(retry>=200)return 1;
	else retry=0;
    while (!DS18B20_DQ_IN && retry<24)
	{
		retry++;
		tickDelay(10);
	};
	if(retry>=24)return 1;	    
	return 0;
}

//-----------------------------------------------------------------------------
// Initiate DS18B20
//
u8 DS18B20_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;

 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE); 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);
	
 	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;				
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		  
 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_SetBits(GPIOA,GPIO_Pin_15); 

	DS18B20_Rst();

	return DS18B20_Check();

}  

//-----------------------------------------------------------------------------
// Start DS18B20
//
void DS18B20_Start(void) 
{   						               
    DS18B20_Rst();	   
		DS18B20_Check();	 
    OWWriteByte(0xcc);	// skip rom
    OWWriteByte(0x44);	// convert
} 


//-----------------------------------------------------------------------------
// Convert the temperature and reterive the data
// the result of temperature is 10 times of the real value. 
//
short DS18B20_Get_Temp(void)
{
    u8 temp;
    u8 TL,TH;
	short tem;
    DS18B20_Start ();  			// ds1820 start convert
    DS18B20_Rst();
    temp = DS18B20_Check();	 
    OWWriteByte(0xcc);	// skip rom
    OWWriteByte(0xbe);	// 
    TL=OWReadByte(); 	// LSB   
    TH=OWReadByte(); 	// MSB  

	
    if(TH>7)
    {
        TH=~TH;
        TL=~TL; 
        temp=0;				
    }else temp=1;					  
    tem=TH; 				
    tem<<=8;    
    tem+=TL;		
    tem=(short)tem*0.625;		   
	if(temp)return tem; 		
	else return -tem;    

}

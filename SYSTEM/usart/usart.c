#include "sys.h"
#include "usart.h"	  
#include <stdio.h>
#include <string.h>
#include "header.h"
u8 cmd;

void USART1_DMA_Config(void);
//////////////////////////////////////////////////////////////////
//�������´���,֧��printf����,������Ҫѡ��use MicroLIB	  
#if UART1_PRINTF
#pragma import(__use_no_semihosting)             
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
void _sys_exit(int x) 
{ 
	x = x; 
} 

void _ttywrch(int ch)
{
ch = ch;
}
//�ض���fputc���� 
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0);//ѭ������,ֱ���������   
    USART1->DR = (u8) ch;      
	return ch;
}
#endif 

 
#ifdef EN_USART1   //���ʹ���˽���
//����1�жϷ������


void uart_init(u32 bound){

  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOB| RCC_APB2Periph_AFIO, ENABLE);	
	GPIO_PinRemapConfig(GPIO_Remap_USART1, ENABLE);
	
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6; 
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
  GPIO_Init(GPIOB, &GPIO_InitStructure);
   
  //USART2_RX	  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
  GPIO_Init(GPIOB, &GPIO_InitStructure);//��ʼ��GPIOA. 

   //USART ��ʼ������

	USART_InitStructure.USART_BaudRate = bound;//���ڲ�����
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ

  USART_Init(USART1, &USART_InitStructure); //��ʼ������1
  USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);//�������ڽ����ж�
	USART_DMACmd(USART1,USART_DMAReq_Rx,ENABLE);
	USART1_DMA_Config();
	
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
	
  USART_Cmd(USART1, ENABLE);                    //ʹ�ܴ���
	
}


void USART1_DMA_Config(void)  
{  
  DMA_InitTypeDef DMA_InitStructure;  
	
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART1->DR; ;  //DMA�������ַ
	DMA_InitStructure.DMA_MemoryBaseAddr =(uint32_t) gGlobal.m_UsartData.p_buffer;  //DMA�ڴ����ַ
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;  //���ݴ��䷽�򣬴����贫���ڴ�
	DMA_InitStructure.DMA_BufferSize = PACK_SIZE;  //DMAͨ����DMA����Ĵ�С
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //�����ַ�Ĵ�������
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //�ڴ��ַ�Ĵ�������
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  //���ݿ��Ϊ8λ
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; //���ݿ��Ϊ8λ
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  //����������ģʽ
	DMA_InitStructure.DMA_Priority = DMA_Priority_High; //DMAͨ�� xӵ�������ȼ� 
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  //DMAͨ��xû������Ϊ�ڴ浽�ڴ洫��
	DMA_Init(DMA1_Channel5, &DMA_InitStructure);  //����DMA_InitStruct��ָ���Ĳ�����ʼ��DMA��ͨ��USART1_Tx_DMA_Channel����ʶ�ļĴ���	
	DMA_Cmd(DMA1_Channel5, ENABLE);
} 
#else

void AuxiliaryPorts_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7; 
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	//�����������
  GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void NJST_Flipping(void)
{
	if ( GetValue_NJST_Pin = 1)
		PBout(6) = 0;
	else
		PBout(6) = 1;
}


#endif	

uint8_t USART1_ComSend(u8 *pBuffer,uint32_t btLen)
{
    int i;
    for (i=0;i<btLen;i++)
    {
      USART_SendData(USART1, pBuffer[i]);
			while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
			{}
			USART_ClearFlag(USART1,USART_FLAG_TC);
    }
    return btLen;
}





BYTE CalcXORSum(u8 *pData, WORD wDataLen)
{
    BYTE i=0,ResuValue=0;
    for(; i<wDataLen; i++)
    {
       ResuValue ^= pData[i];
    }
    return ResuValue;
}


/*****************************************************CheckPWD<PWD>����У��********************************************************/
BYTE CheckPWD(BYTE *btPWD)
{
    BYTE i=0;
    for(; i<UPDATE_PASSWD_LENGTH; i++)
    {
        if(gGlobal.m_btPWD[i] != btPWD[i])
            return FALSE;
    }
    return TRUE;
}


/*Ӧ������ķ��ͺ���*/
void SendUp(All_PACK *pPack)
{
    switch(gGlobal.m_LastActiveComType)
    {
        case COMM_TYPE_COM2:
//           USART2_ComSend(pPack->m_pBuffer,pPack->m_Len);
            break;
        case COMM_TYPE_COM1:
            USART1_ComSend(pPack->m_pBuffer,pPack->m_Len);
            break;
        default:
            break;            
    }
}


void SendErrCode(WORD wdErrType, u8 cmd)
{
		All_PACK ErrSendPack;
    InitPack(ErrSendPack);
    PUSHPackTagHeader(ErrSendPack);  // push header into the pack
    BYTEToPack(ErrSendPack,ERR_LENGTH);
    BYTEToPack(ErrSendPack,0x01);    //Flag
    BYTEToPack(ErrSendPack,ERR_LEN); //data length
    BYTEToPack(ErrSendPack,cmd);  // cmd
    WORDToPack(ErrSendPack,wdErrType);
    BYTEToPack(ErrSendPack,CalcXORSum(&ErrSendPack.m_pBuffer[2], ErrSendPack.m_Pos-2));//У���ֽ� ǰ����һ����ͷ0xAA 0x55�ֽڲ���Ϊ�������
    SendUp(&ErrSendPack);
}
/* ͨ�õ���ȷӦ�� */
void AppCommonRightRes()
{
	All_PACK SendPack;
	//������Ӧ������
    InitPack(SendPack);//��ʼ��
    PUSHPackTagHeader(SendPack);//��ͷ�ֽ�0xAA 0x55ѹջ
    BYTEToPack(SendPack,0x03);//�ܳ���ѹջ Length
    BYTEToPack(SendPack,CMD_BAG_FLAG_BYTE);//Flag�ֽ�
    BYTEToPack(SendPack,0x00);//���ݳ����ֽ� Len
    BYTEToPack(SendPack,0x01);//cmd
    BYTEToPack(SendPack,CalcXORSum(&SendPack.m_pBuffer[2], SendPack.m_Pos-2));//У���ֽ� ǰ����һ����ͷ0xAA 0x55�ֽڲ���Ϊ�������
    SendUp(&SendPack);
}


/* ping֡�������� */
void AppPingTest(All_PACK *pack)
{
    if(gGlobal.m_AppDataLen != 0)  // if the length of data (in payload) is not zero
    {
        SendErrCode(ERR_FORMAT, 0x01);
        return;
    }
	AppCommonRightRes();
}
/* ��ȡ�豸�ͺ����� */
void AppReadDeviceinfor(All_PACK *pack)
{
    BYTE i,*DeviceInformation;
    All_PACK SendPack;
    if(gGlobal.m_AppDataLen != 0)
    {
        SendErrCode(ERR_FORMAT, 0x02);
        return;
    }
    //������Ӧ������
    InitPack(SendPack);//��ʼ��
    PUSHPackTagHeader(SendPack);//��ͷ�ֽ�0xAA 0x55ѹջ
    BYTEToPack(SendPack,0x21);//�ܳ���ѹջ Length
    BYTEToPack(SendPack,CMD_BAG_FLAG_BYTE);//Flag�ֽ�
    BYTEToPack(SendPack,0x1E);//���ݳ����ֽ� Len
    BYTEToPack(SendPack,0x01);//�ɹ���־�ֽ�
    DeviceInformation = (BYTE *)&gGlobal.m_DeviceInfo; 
    for(i=0; i< sizeof(gGlobal.m_DeviceInfo); i++)
    {
        BYTEToPack(SendPack,DeviceInformation[i]);//�ɹ���־�ֽ�
    }
    BYTEToPack(SendPack,CalcXORSum(&SendPack.m_pBuffer[2], SendPack.m_Pos-2));//У���ֽ� ǰ����һ����ͷ0xAA 0x55�ֽڲ���Ϊ�������
    SendUp(&SendPack);
}
/* Soft reset ��λ���� */
void AppRestDevice(All_PACK *pack)
{
    if(gGlobal.m_AppDataLen != 0)
    {
        SendErrCode(ERR_FORMAT, 0x10);
        return;
    }
		TIM_Cmd(TIM5, DISABLE); 
}
/* ���������������� */
void AppStartRespond(All_PACK *pack)
{
    if(gGlobal.m_AppDataLen != 0)
    {
        SendErrCode(ERR_FORMAT, 0x11);
        return;
    }
	gGlobal.m_UsartData.p_broadcastflag = 1;
}
///* ֹͣ������������ */
void AppEndRespond(All_PACK *pack)
{
    if(gGlobal.m_AppDataLen != 0)
    {
        SendErrCode(ERR_FORMAT, 0x12);
        return;
    }
	gGlobal.m_UsartData.p_broadcastflag = 0;
}

/* ��ʼ�������� */ 
void AppStartUpdata(All_PACK *pack)
{
    BYTE i,boardIndex;
    DWORD dwStart;
		uint32_t JumpAddress=0;
    pFunction Jump_To_Application;
    BYTE btpasswd[UPDATE_PASSWD_LENGTH];
    if(gGlobal.m_AppDataLen != (UPDATE_PASSWD_LENGTH+1))
    {
        SendErrCode(ERR_FORMAT, 0x15);
        return;
    }
	BYTEFromPack(*pack,boardIndex);
    for(i=0; i<gGlobal.m_AppDataLen; i++)
    {
        BYTEFromPack(*pack,btpasswd[i]);
    }
    if(!CheckPWD(btpasswd))
    {
        SendErrCode(ERR_FORMAT, 0x15);
        return;
    }
	if(boardIndex == 0x01)
	{
		FLASH_Unlock();
		FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
		FLASH_ErasePage(BOOT_PARAM_ADDRESS_UART);
		FLASH_ProgramWord(BOOT_PARAM_ADDRESS_UART, 0x01);
		FLASH_Lock();
	}
    if(*(unsigned long *)BOOT_PARAM_ADDRESS_UART == 0x01)
    {    
		AppCommonRightRes();
        dwStart = gGlobal.m_LocalTime; //��ʱ
        while(Check1MSTick(gGlobal.m_LocalTime, dwStart, 5))//��ʱ,Ϊ���ڷ���������ȡʱ��
        {
        }
		//��תָ��
		JumpAddress = *(__IO uint32_t*) (BOOTCROCESS_ADDRESS + 4);
		/* Jump to user application */
		Jump_To_Application = (pFunction) JumpAddress;
		/* Initialize user application's Stack Pointer */
		__set_MSP(*(__IO uint32_t*) BOOTCROCESS_ADDRESS);
		Jump_To_Application();
    }
    else
    {
        SendErrCode(ERR_OPERATION, 0x15);
        return;
    }

}

/* ���ػ�֪ͨ��Դ��ϵ� */ 
void AppCutdownPower(All_PACK *pack)
{
//    if(gGlobal.m_AppDataLen != 2)
//    {
//        SendErrCode(ERR_FORMAT);
//        return;
//    }
//	WORDFromPack(*pack,gGlobal.m_SystemRebootIntervalTime);
//	if(gGlobal.m_SystemRebootIntervalTime == 0)
//	{
//		SendErrCode(ERR_FORMAT);
//    return;
//	}
//	AppCommonRightRes();
//	dELAY_ms(3000); //�ȴ�3s���жϹ���
//	Power_off_Scheme();  //�жϵ�Դ
//	TIM_Cmd(TIM5, DISABLE); //��������
	
	
}

/* ���ػ�֪ͨ��Դ�弱ͣ������ */ 
void AppStopMotodrv(All_PACK *pack)
{
//		BYTE scramButtonDown;
//			if(gGlobal.m_AppDataLen != 1)
//			{
//					SendErrCode(ERR_FORMAT);
//					return;
//			}
//		BYTEFromPack(*pack,scramButtonDown);
//		if (scramButtonDown == 1)
//			Detection_Motor();
//		if (scramButtonDown == 0)
//			RESET_Motor();

//		AppCommonRightRes();
}


/*Ӧ�ô���ľ���ʵ��*/
void ProcessUpCmd(All_PACK *pack)
{
    BYTE TotalLengthByte;
    BYTE FlagByte;
    BYTE DataLenByte;
    BYTE btCmd;
		pack->m_Pos = 2;
    BYTEFromPack((*pack),TotalLengthByte); // fetch length of the data
    BYTEFromPack((*pack),FlagByte);   // fetch Flag from the data pack  
    if(FlagByte != CMD_BAG_FLAG_ANZHUO2WAISHE)
    {
        SendErrCode(ERR_OPERATION,cmd);
        return; 
    }
    BYTEFromPack((*pack),DataLenByte);
    BYTEFromPack((*pack),btCmd);
		/*
		1. pack->m_Len is the total bytes being received from the USART
		2. TotalLengthByte is the third byte of the received data
		3. DataLenByte is the 5th byte of the received data 
		*/
    if( (TotalLengthByte > PACK_SIZE) || (pack->m_Len != (TotalLengthByte+4/* header+length�ֽ�+У���ֽ�*/) ) || (TotalLengthByte != (DataLenByte+3)) )//Length + Payload(Flag + Len + Cmd + Data)
    {                                                                                                                                            //  0x09 +  		0x00 + 0x06 + 0x15 + 'UPDATE'
        SendErrCode(ERR_OPERATION, cmd);
        return; 
    }                                                                              
    gGlobal.m_AppDataLen = DataLenByte;
//    LEDOff();
//    gGlobal.m_LEDOnTime = gGlobal.m_LocalTime;//������ͨ��ʱ������˸
		
    switch(btCmd)
	{
		case CMD_PING_TEST://ping֡��������
				AppPingTest(pack);
				break;
		case CMD_READ_DEVICEINFOR://��ȡ�豸�ͺ�����
				AppReadDeviceinfor(pack);
				break;
		case CMD_REST_DEVICE://��λ����
				AppRestDevice(pack);
				break;
		case CMD_START_RESPOND://����������������
				AppStartRespond(pack);
				break;
		case CMD_END_RESPOND://ֹͣ������������
				AppEndRespond(pack);
				break;     
		case CMD_START_UPDATA://��ʼ��������
				AppStartUpdata(pack);
				break;	
		case CMD_CUTDOWN_POWER://���ػ�֪ͨ��Դ��ϵ�
				AppCutdownPower(pack);
				break;
		 	
		default:
				SendErrCode(ERR_CMD, cmd);//�������
				break;
	}
}


void UsartFun(void)
{
	u8 btComIndex, btPackIndex, chksum, xorlen;
	if(gGlobal.m_UsartData.p_flag==0x55)
	{
		btComIndex = 1;
		btPackIndex =1;	
		cmd = gGlobal.m_ComSet[btComIndex].m_RecvPack[btPackIndex].m_pBuffer[3]; // cmd ����
	//	memcpy(gGlobal.m_ComSet[btComIndex].m_RecvPack[btPackIndex].m_pBuffer,gGlobal.m_UsartData.p_buffer,gGlobal.m_UsartData.p_length);
	//	gGlobal.m_ComSet[btComIndex].m_RecvPack[btPackIndex].m_Len = gGlobal.m_UsartData.p_length;
		xorlen = gGlobal.m_ComSet[btComIndex].m_RecvPack[btPackIndex].m_Len - 3;
		chksum = CalcXORSum(&gGlobal.m_ComSet[btComIndex].m_RecvPack[btPackIndex].m_pBuffer[2],xorlen);
		if(chksum == gGlobal.m_ComSet[btComIndex].m_RecvPack[btPackIndex].m_pBuffer[xorlen+2])
		{
			ProcessUpCmd(&gGlobal.m_ComSet[btComIndex].m_RecvPack[btPackIndex]);
			gGlobal.m_UsartData.p_flag=0;
		}	
		else
		{
			SendErrCode(ERR_XOR,cmd);
			memset(gGlobal.m_UsartData.p_buffer,0,sizeof(gGlobal.m_UsartData.p_buffer));
			memset(gGlobal.m_ComSet[btComIndex].m_RecvPack[btPackIndex].m_pBuffer,0,181);
			gGlobal.m_UsartData.p_length = 0;
			
		}
	}
}

void USART1_IRQHandler(void)                	//����2�жϷ������
	{
		u8 Res;
		if(USART_GetITStatus(USART1, USART_IT_IDLE) != RESET)  
		{
			__nop(); 
			__nop(); 
			__nop(); 
			Res=USART2->SR;
			Res=USART2->DR;
		  DMA_Cmd(DMA1_Channel5,DISABLE);  
			
      Res = PACK_SIZE - DMA_GetCurrDataCounter(DMA1_Channel5);
			gGlobal.m_UsartData.p_length = Res;
			gGlobal.m_UsartData.p_flag=0x55;
			memcpy(gGlobal.m_ComSet[1].m_RecvPack[1].m_pBuffer,gGlobal.m_UsartData.p_buffer,gGlobal.m_UsartData.p_length);
			gGlobal.m_ComSet[1].m_RecvPack[1].m_Len = gGlobal.m_UsartData.p_length;
      DMA_SetCurrDataCounter(DMA1_Channel5,PACK_SIZE);  
      DMA_Cmd(DMA1_Channel5,ENABLE);  					
		}
} 
	


#include "sys.h"
#include "usart.h"	  
#include <stdio.h>
#include <string.h>
#include "header.h"
u8 cmd;

void USART1_DMA_Config(void);
//////////////////////////////////////////////////////////////////
//加入以下代码,支持printf函数,而不需要选择use MicroLIB	  
#if UART1_PRINTF
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
void _sys_exit(int x) 
{ 
	x = x; 
} 

void _ttywrch(int ch)
{
ch = ch;
}
//重定义fputc函数 
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
    USART1->DR = (u8) ch;      
	return ch;
}
#endif 

 
#ifdef EN_USART1   //如果使能了接收
//串口1中断服务程序


void uart_init(u32 bound){

  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOB| RCC_APB2Periph_AFIO, ENABLE);	
	GPIO_PinRemapConfig(GPIO_Remap_USART1, ENABLE);
	
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6; 
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
  GPIO_Init(GPIOB, &GPIO_InitStructure);
   
  //USART2_RX	  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
  GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIOA. 

   //USART 初始化设置

	USART_InitStructure.USART_BaudRate = bound;//串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

  USART_Init(USART1, &USART_InitStructure); //初始化串口1
  USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);//开启串口接受中断
	USART_DMACmd(USART1,USART_DMAReq_Rx,ENABLE);
	USART1_DMA_Config();
	
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
	
  USART_Cmd(USART1, ENABLE);                    //使能串口
	
}


void USART1_DMA_Config(void)  
{  
  DMA_InitTypeDef DMA_InitStructure;  
	
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART1->DR; ;  //DMA外设基地址
	DMA_InitStructure.DMA_MemoryBaseAddr =(uint32_t) gGlobal.m_UsartData.p_buffer;  //DMA内存基地址
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;  //数据传输方向，从外设传到内存
	DMA_InitStructure.DMA_BufferSize = PACK_SIZE;  //DMA通道的DMA缓存的大小
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //外设地址寄存器不变
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //内存地址寄存器递增
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  //数据宽度为8位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; //数据宽度为8位
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  //工作在正常模式
	DMA_InitStructure.DMA_Priority = DMA_Priority_High; //DMA通道 x拥有中优先级 
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  //DMA通道x没有设置为内存到内存传输
	DMA_Init(DMA1_Channel5, &DMA_InitStructure);  //根据DMA_InitStruct中指定的参数初始化DMA的通道USART1_Tx_DMA_Channel所标识的寄存器	
	DMA_Cmd(DMA1_Channel5, ENABLE);
} 
#else

void AuxiliaryPorts_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7; 
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	//复用推挽输出
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


/*****************************************************CheckPWD<PWD>口令校验********************************************************/
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


/*应答命令的发送函数*/
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
    BYTEToPack(ErrSendPack,CalcXORSum(&ErrSendPack.m_pBuffer[2], ErrSendPack.m_Pos-2));//校验字节 前面有一个包头0xAA 0x55字节不作为检验对象
    SendUp(&ErrSendPack);
}
/* 通用的正确应答 */
void AppCommonRightRes()
{
	All_PACK SendPack;
	//后面是应答数据
    InitPack(SendPack);//初始化
    PUSHPackTagHeader(SendPack);//包头字节0xAA 0x55压栈
    BYTEToPack(SendPack,0x03);//总长度压栈 Length
    BYTEToPack(SendPack,CMD_BAG_FLAG_BYTE);//Flag字节
    BYTEToPack(SendPack,0x00);//数据长度字节 Len
    BYTEToPack(SendPack,0x01);//cmd
    BYTEToPack(SendPack,CalcXORSum(&SendPack.m_pBuffer[2], SendPack.m_Pos-2));//校验字节 前面有一个包头0xAA 0x55字节不作为检验对象
    SendUp(&SendPack);
}


/* ping帧测试命令 */
void AppPingTest(All_PACK *pack)
{
    if(gGlobal.m_AppDataLen != 0)  // if the length of data (in payload) is not zero
    {
        SendErrCode(ERR_FORMAT, 0x01);
        return;
    }
	AppCommonRightRes();
}
/* 读取设备型号命令 */
void AppReadDeviceinfor(All_PACK *pack)
{
    BYTE i,*DeviceInformation;
    All_PACK SendPack;
    if(gGlobal.m_AppDataLen != 0)
    {
        SendErrCode(ERR_FORMAT, 0x02);
        return;
    }
    //后面是应答数据
    InitPack(SendPack);//初始化
    PUSHPackTagHeader(SendPack);//包头字节0xAA 0x55压栈
    BYTEToPack(SendPack,0x21);//总长度压栈 Length
    BYTEToPack(SendPack,CMD_BAG_FLAG_BYTE);//Flag字节
    BYTEToPack(SendPack,0x1E);//数据长度字节 Len
    BYTEToPack(SendPack,0x01);//成功标志字节
    DeviceInformation = (BYTE *)&gGlobal.m_DeviceInfo; 
    for(i=0; i< sizeof(gGlobal.m_DeviceInfo); i++)
    {
        BYTEToPack(SendPack,DeviceInformation[i]);//成功标志字节
    }
    BYTEToPack(SendPack,CalcXORSum(&SendPack.m_pBuffer[2], SendPack.m_Pos-2));//校验字节 前面有一个包头0xAA 0x55字节不作为检验对象
    SendUp(&SendPack);
}
/* Soft reset 复位命令 */
void AppRestDevice(All_PACK *pack)
{
    if(gGlobal.m_AppDataLen != 0)
    {
        SendErrCode(ERR_FORMAT, 0x10);
        return;
    }
		TIM_Cmd(TIM5, DISABLE); 
}
/* 启动自助反馈命令 */
void AppStartRespond(All_PACK *pack)
{
    if(gGlobal.m_AppDataLen != 0)
    {
        SendErrCode(ERR_FORMAT, 0x11);
        return;
    }
	gGlobal.m_UsartData.p_broadcastflag = 1;
}
///* 停止自助反馈命令 */
void AppEndRespond(All_PACK *pack)
{
    if(gGlobal.m_AppDataLen != 0)
    {
        SendErrCode(ERR_FORMAT, 0x12);
        return;
    }
	gGlobal.m_UsartData.p_broadcastflag = 0;
}

/* 开始升级命令 */ 
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
        dwStart = gGlobal.m_LocalTime; //延时
        while(Check1MSTick(gGlobal.m_LocalTime, dwStart, 5))//延时,为串口发送数据争取时间
        {
        }
		//跳转指令
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

/* 工控机通知电源板断电 */ 
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
//	dELAY_ms(3000); //等待3s后切断供电
//	Power_off_Scheme();  //切断电源
//	TIM_Cmd(TIM5, DISABLE); //重新启动
	
	
}

/* 工控机通知电源板急停被按下 */ 
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


/*应用处理的具体实现*/
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
    if( (TotalLengthByte > PACK_SIZE) || (pack->m_Len != (TotalLengthByte+4/* header+length字节+校验字节*/) ) || (TotalLengthByte != (DataLenByte+3)) )//Length + Payload(Flag + Len + Cmd + Data)
    {                                                                                                                                            //  0x09 +  		0x00 + 0x06 + 0x15 + 'UPDATE'
        SendErrCode(ERR_OPERATION, cmd);
        return; 
    }                                                                              
    gGlobal.m_AppDataLen = DataLenByte;
//    LEDOff();
//    gGlobal.m_LEDOnTime = gGlobal.m_LocalTime;//有数据通信时，灯闪烁
		
    switch(btCmd)
	{
		case CMD_PING_TEST://ping帧测试命令
				AppPingTest(pack);
				break;
		case CMD_READ_DEVICEINFOR://读取设备型号命令
				AppReadDeviceinfor(pack);
				break;
		case CMD_REST_DEVICE://复位命令
				AppRestDevice(pack);
				break;
		case CMD_START_RESPOND://启动自助反馈命令
				AppStartRespond(pack);
				break;
		case CMD_END_RESPOND://停止自助反馈命令
				AppEndRespond(pack);
				break;     
		case CMD_START_UPDATA://开始升级命令
				AppStartUpdata(pack);
				break;	
		case CMD_CUTDOWN_POWER://工控机通知电源板断电
				AppCutdownPower(pack);
				break;
		 	
		default:
				SendErrCode(ERR_CMD, cmd);//命令错误
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
		cmd = gGlobal.m_ComSet[btComIndex].m_RecvPack[btPackIndex].m_pBuffer[3]; // cmd 类型
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

void USART1_IRQHandler(void)                	//串口2中断服务程序
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
	


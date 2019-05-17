#include "header.h"
#include "OCPSampling.h"

void ADCTRGO(void);

void DMAInit(void)
{
 	DMA_InitTypeDef DMA_InitStructure; 
	NVIC_InitTypeDef   NVIC_InitStructure;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);	//使能DMA传输
	
	DMA_DeInit(DMA1_Channel1);   //将DMA的通道1寄存器重设为缺省值

	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&ADC1->DR; ;  //DMA外设基地址
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32) gGlobal.m_OCPData.rawdata;  //DMA内存基地址
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;  //数据传输方向，从内存读取发送到外设
	DMA_InitStructure.DMA_BufferSize = NumberofChannel*NumberofCycle;  //DMA通道的DMA缓存的大小
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //外设地址寄存器不变
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //内存地址寄存器递增
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;  //数据宽度为16位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord; //数据宽度为16位
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  //工作在正常模式
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium; //DMA通道 x拥有中优先级 
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  //DMA通道x没有设置为内存到内存传输
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);  //根据DMA_InitStruct中指定的参数初始化DMA的通道USART1_Tx_DMA_Channel所标识的寄存器	
	
	DMA_ITConfig(DMA1_Channel1,DMA_IT_TC, ENABLE);  
	NVIC_InitStructure.NVIC_IRQChannel  =DMA1_Channel1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = IRQ_ADC_Priority;//
	NVIC_InitStructure.NVIC_IRQChannelCmd   = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	DMA_Cmd(DMA1_Channel1, ENABLE);
}


/* ADC检测通道初始化*/

void VoltSampling_Init(void)
{
	ADC_InitTypeDef ADC_InitStructure; 
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOC, ENABLE );
	RCC_ADCCLKConfig(RCC_PCLK2_Div6); 	
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		
	GPIO_Init(GPIOA, &GPIO_InitStructure);	

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		//模拟输入引脚
	GPIO_Init(GPIOC, &GPIO_InitStructure);	
	
	ADC_DeInit(ADC1);
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;	//模数转换工作在单通道模式
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;	
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T4_CC4;	
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel = NumberofChannel;
	ADC_Init(ADC1, &ADC_InitStructure);	  

	ADC_RegularChannelConfig(ADC1, BUSVOLTAGE, 		1, 	ADC_SampleTime_28Cycles5 ); 
	ADC_RegularChannelConfig(ADC1, MOTOROCP_01, 	2, 	ADC_SampleTime_28Cycles5 );	
	ADC_RegularChannelConfig(ADC1, MOTOROCP_02,  	3,	ADC_SampleTime_28Cycles5 );
	ADC_RegularChannelConfig(ADC1, MOTOROCP_03,  	4, 	ADC_SampleTime_28Cycles5 );
	ADC_RegularChannelConfig(ADC1, MOTOROCP_04,  	5, 	ADC_SampleTime_28Cycles5 );

	ADC_Cmd(ADC1, ENABLE);	//使能指定的ADC1
	ADC_ResetCalibration(ADC1);	//使能复位校准   
	while(ADC_GetResetCalibrationStatus(ADC1));	//等待复位校准结束
	
	ADC_StartCalibration(ADC1);	 //开启AD校准
	while(ADC_GetCalibrationStatus(ADC1));	
	
	ADC_ExternalTrigConvCmd(ADC1, ENABLE);
	DMAInit();
	ADC_DMACmd(ADC1, ENABLE);
	ADCTRGO();
}


void ADCTRGO(void)
{  
		TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
		TIM_OCInitTypeDef TIM_OCInitStructure;
		/* TIM4 clock enable */
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4 , ENABLE);

		/* Configure TIM4 to generate each 1us */
		TIM_TimeBaseStructure.TIM_Period  = 10000;  //计数周期为0.1s， 
		TIM_TimeBaseStructure.TIM_Prescaler = (SystemCoreClock/100000)-1;  // 1MHz
		TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
		TIM_TimeBaseStructure.TIM_CounterMode =  TIM_CounterMode_Up;
		TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

		TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
		TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;//TIM_OutputState_Disable;
		TIM_OCInitStructure.TIM_Pulse = 5000;
		TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
		TIM_OC4Init(TIM4, & TIM_OCInitStructure);

		TIM_Cmd(TIM4, ENABLE);
		TIM_InternalClockConfig(TIM4);
		TIM_OC4PreloadConfig(TIM4, TIM_OCPreload_Enable);
		TIM_UpdateDisableConfig(TIM4, DISABLE);	
}


/*  OCP 通道初始化 */

void OCPChannel_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;	
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC,ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 
	GPIO_Init(GPIOA, &GPIO_InitStructure);	
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14 | GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 
	GPIO_Init(GPIOB, &GPIO_InitStructure);	
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE); 
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource4);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource0);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource14);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC,GPIO_PinSource9);
	
	EXTI_InitStructure.EXTI_Line=	MOTORUPPER_INT_EXTI_LINE | MOTORLOWER_INT_EXTI_LINE | LIFTER_INT_EXTI_LINE | LOCKER_INT_EXTI_LINE;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn ;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = IRQ_EXTI_Priority;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
	NVIC_Init(&NVIC_InitStructure); 
	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn ;
	NVIC_Init(&NVIC_InitStructure);
	NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn ;
	NVIC_Init(&NVIC_InitStructure);		
//	NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQn ;
//	NVIC_Init(&NVIC_InitStructure);	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn ;
	NVIC_Init(&NVIC_InitStructure);	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn ;
	NVIC_Init(&NVIC_InitStructure);	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn ;
	NVIC_Init(&NVIC_InitStructure);	

}

void LockerChannelEnable(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14 ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 
	
	GPIO_Init(GPIOB, &GPIO_InitStructure);	
}

void LockerChannelDisable(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14 ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);	
	LOCKERCHANNEL_DISABLE = 0;
}
/* 限位开关初始化 */
void ONPOSITIONChannel_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;	
	EXTI_InitTypeDef EXTI_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO , ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_3 | GPIO_Pin_12 | GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 
	GPIO_Init(GPIOB, &GPIO_InitStructure);		
//		GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource3);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource1);
//	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource12);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource13);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD , ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 
	GPIO_Init(GPIOD, &GPIO_InitStructure);		
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOD,GPIO_PinSource2);	

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC , ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 
	GPIO_Init(GPIOC, &GPIO_InitStructure);		
//	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC,GPIO_PinSource10);	
//	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC,GPIO_PinSource11);
//	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC,GPIO_PinSource12);			

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA , ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 
	GPIO_Init(GPIOA, &GPIO_InitStructure);		
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource10);	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource9);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource8);	

	EXTI_InitStructure.EXTI_Line=	LIFTER_TOP_INT_EXTI_LINE | LIFTER_BOTTOM_INT_EXTI_LINE | UPPERDOOR_CLOSE_INT_EXTI_LINE | UPPERDOOR_OPEN_INT_EXTI_LINE
																| LOWERDOOR_CLOSE_INT_EXTI_LINE | LOWERDOOR_OPEN_INT_EXTI_LINE;
																
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
}


void ADCCHANNEL_IRQHandler(void)
{
   if(DMA_GetITStatus(DMA1_IT_TC1))
    {  
			DMA_Cmd(DMA1_Channel1, DISABLE);
			ADC_Cmd(ADC1, DISABLE);
			DMA_SetCurrDataCounter(DMA1_Channel1,NumberofChannel*NumberofCycle);
			DMA_ClearITPendingBit(DMA1_IT_TC1);
			DMA_ClearFlag(DMA1_IT_TC1);
			gGlobal.m_ADCstatus =1;
//			DMA_Cmd(DMA1_Channel1, ENABLE);			
    }	
}

void FetchingCurrentValue(void)
{
	gGlobal.m_OCPData.voltbus = (uint16_t) gGlobal.m_OCPData.rawdata[0]*1000/4095*3.3*11;
	gGlobal.m_OCPData.currentCH1 = (uint16_t) gGlobal.m_OCPData.rawdata[1]*1000/4095*3.3*4/3/20/0.2;
	gGlobal.m_OCPData.currentCH2 = (uint16_t) gGlobal.m_OCPData.rawdata[2]*54/10;
	gGlobal.m_OCPData.currentCH3 = (uint16_t) gGlobal.m_OCPData.rawdata[3]*27/100;
	gGlobal.m_OCPData.currentCH4 = (uint16_t) gGlobal.m_OCPData.rawdata[4]*1000/4095*3.3*4/3/20/0.2;

}

/* 更新桶门的状态， 升降隔板的状态， 锁的状态， 电机状态*/
void FetchingItemStatus(void)
{
	if (LIFTER_TOP_IN ==1 && LIFTER_BOTTOM_IN == 0)
	{
		/* code */
		gGlobal.m_Status.LIFTER_STATUS = 1;
	}
	else if (LIFTER_TOP_IN ==0 && LIFTER_BOTTOM_IN == 1)
	{
		/* code */
		gGlobal.m_Status.LIFTER_STATUS = 2;
	}
	else if (LIFTER_TOP_IN ==0 && LIFTER_BOTTOM_IN == 0)
	{
		/* code */
		gGlobal.m_Status.LIFTER_STATUS = 3;
	}
	else
	{
		gGlobal.m_Status.LIFTER_STATUS = 0;
		printf("Error: FW bug or HW fault happen to lifter sensors\n");
	}

	if (UPPERDOOR_CLOSE_IN == 1 && UPPERDOOR_OPEN_IN == 0)
	{
		gGlobal.m_Status.UPPERDOOR_STATUS = 2;
	}
	else if (UPPERDOOR_CLOSE_IN == 0 && UPPERDOOR_OPEN_IN == 1)
	{
		/* code */
		gGlobal.m_Status.UPPERDOOR_STATUS = 1;
	}
	else if (UPPERDOOR_CLOSE_IN == 0 && UPPERDOOR_OPEN_IN == 0)
	{
		gGlobal.m_Status.UPPERDOOR_STATUS = 3;
	}
	else
	{
		gGlobal.m_Status.UPPERDOOR_STATUS = 0;
		printf("Error: FW bug or HW fault happen to upperdoor sensors\n");
	}

	if (LOWERDOOR_CLOSE_IN == 1 && LOWERDOOR_OPEN_IN == 0)
	{
		gGlobal.m_Status.LOWERDOOR_STATUS = 2;
	}
	else if (LOWERDOOR_CLOSE_IN == 0 && LOWERDOOR_OPEN_IN == 1)
	{
		/* code */
		gGlobal.m_Status.LOWERDOOR_STATUS = 1;
	}
	else if (LOWERDOOR_CLOSE_IN == 0 && LOWERDOOR_OPEN_IN == 0)
	{
		gGlobal.m_Status.LOWERDOOR_STATUS = 3;
	}
	else
	{
		gGlobal.m_Status.LOWERDOOR_STATUS = 0;
		printf("Error: FW bug or HW fault happen to lowerdoor sensors\n");
	}
}


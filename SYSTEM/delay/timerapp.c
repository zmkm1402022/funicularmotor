#include "timerapp.h"
#include "header.h"


void TasksectionInit(void)
{  
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
		NVIC_InitTypeDef NVIC_InitStructure;
    /* TIM4 clock enable */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6 , ENABLE);

    /* Configure TIM4 to generate each 1us */
    TIM_TimeBaseStructure.TIM_Period  = 65535;//最大计数值为65535ms，即为65.535ms
    TIM_TimeBaseStructure.TIM_Prescaler = (SystemCoreClock/1000)-1;  // 1KHz
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode =  TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);
    TIM_ARRPreloadConfig(TIM6, ENABLE);
    TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE);
    NVIC_InitStructure.NVIC_IRQChannel = TIM6_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_Init(&NVIC_InitStructure); 
    TIM_Cmd(TIM6, DISABLE);
}

/*
TIM7 is used for time delay;
*/
void TIM_DELAY_INIT(void)
{  
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    /* TIM4 clock enable */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7 , ENABLE);

    /* Configure TIM4 to generate each 1us */
    TIM_TimeBaseStructure.TIM_Period  = 65535;//最大计数值为65535us，即为65.535ms
    TIM_TimeBaseStructure.TIM_Prescaler = (SystemCoreClock/1000000)-1;  // 1MHz
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode =  TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM7, &TIM_TimeBaseStructure);
}

void dELAY_us(uint16_t nus)
{
    if(nus > 60000) //不能超过65535，因为计数器是16位的  
        return;	
    TIM_SetCounter(TIM7,0);
    TIM_Cmd(TIM7, ENABLE); 
    while(nus >= TIM_GetCounter(TIM7))
    {
    } 
    TIM_Cmd(TIM7, DISABLE);  //关闭计数器   
    TIM_SetCounter(TIM7,0);  //清空计数器	     
}
void delay_us(uint16_t nus)
{
    if(nus > 60000) //不能超过65535，因为计数器是16位的  
        return;	
    TIM_SetCounter(TIM7,0);
    TIM_Cmd(TIM7, ENABLE); 
    while(nus >= TIM_GetCounter(TIM7))
    {
    } 
    TIM_Cmd(TIM7, DISABLE);  //关闭计数器   
    TIM_SetCounter(TIM7,0);  //清空计数器	     
}

void dELAY_ms(uint16_t ums)
{
//    u16 wdg_cnt=0;
		if(ums > 60000) //不能超过65535，因为计数器是16位的  
        return;	
    while(ums--)
    {
			TIM_SetCounter(TIM7,0);
			TIM_Cmd(TIM7, ENABLE); 
			while(1000 >= TIM_GetCounter(TIM7))//1ms
			{
			} 
			TIM_Cmd(TIM7, DISABLE);  //关闭计数器   
			TIM_SetCounter(TIM7,0);  //清空计数器
/*			wdg_cnt++;
			if (wdg_cnt>=900 && ums>900){
				WatchDog_Oper();
				wdg_cnt =0;
			}
			else if (ums<=900 && wdg_cnt>=(ums-10)){

				wdg_cnt =0;				
			}*/
    }    
}

void delay_ms(uint16_t ums)
{
//    u16 wdg_cnt=0;
		if(ums > 60000) //不能超过65535，因为计数器是16位的  
        return;	
    while(ums--)
    {
			TIM_SetCounter(TIM7,0);
			TIM_Cmd(TIM7, ENABLE); 
			while(1000 >= TIM_GetCounter(TIM7))//1ms
			{
			} 
			TIM_Cmd(TIM7, DISABLE);  //关闭计数器   
			TIM_SetCounter(TIM7,0);  //清空计数器
/*			wdg_cnt++;
			if (wdg_cnt>=900 && ums>900){
				WatchDog_Oper();
				wdg_cnt =0;
			}
			else if (ums<=900 && wdg_cnt>=(ums-10)){

				wdg_cnt =0;				
			}*/
    }    
}

u32 dTime_Counting(u32 startingtime, u32 endingtime)
{
	int32_t time;
	time = endingtime - startingtime;
	if (time <0)
		return (0xFFFFFFFF - startingtime + endingtime);
	else
		return (time);
}



void Timer1Init(void)
{  
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    TIM_OCInitTypeDef  TIM_OCInitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    /* TIM1 clock enable */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1 , ENABLE);
    TIM_DeInit(TIM1); 
    /* Configure TIM1 to generate each 1us */
    TIM_TimeBaseStructure.TIM_Period  = 10000;//最大计数值为10ms
    TIM_TimeBaseStructure.TIM_Prescaler = (SystemCoreClock/1000000)-1;  // f = 1MHz
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode =  TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

    TIM_OCInitStructure.TIM_OCMode =TIM_OCMode_PWM1; 
    TIM_OCInitStructure.TIM_OutputState =TIM_OutputState_Disable;
    TIM_OCInitStructure.TIM_OCPolarity =TIM_OCPolarity_High; 
    TIM_OCInitStructure.TIM_Pulse =2000;    
    TIM_OC1Init(TIM1,&TIM_OCInitStructure);

    TIM_OCInitStructure.TIM_Pulse =2500; 
    TIM_OC2Init(TIM1,&TIM_OCInitStructure);

    TIM_ITConfig(TIM1,TIM_IT_CC1,DISABLE ); 
    TIM_ITConfig(TIM1,TIM_IT_CC2,DISABLE ); 
    NVIC_InitStructure.NVIC_IRQChannel = TIM1_CC_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_Init(&NVIC_InitStructure); 
    TIM_Cmd(TIM1, DISABLE);
}

void TIM1_CC_IRQHandler(void)
{
    if(TIM_GetITStatus(TIM1, TIM_IT_CC1) != RESET)
    {
        TIM_ClearITPendingBit(TIM1, TIM_IT_CC1 ); //清除 TIM 更新中断标志  
        TIM_ClearFlag(TIM1,TIM_IT_CC1);
        gGlobal.m_stack.operationDoorAcc.realtimeIntCnt ++ ;
        if(gGlobal.m_stack.operationDoorAcc.realtimeIntCnt < gGlobal.m_stack.operationDoorAcc.accTime)
            gGlobal.m_stack.operationDUTY  +=gGlobal.m_stack.operationDoorAcc.accStep;
        else if(gGlobal.m_stack.operationDoorAcc.realtimeIntCnt < gGlobal.m_stack.operationDoorAcc.accTime+gGlobal.m_stack.operationDoorAcc.deaccTime)
        {
            gGlobal.m_stack.operationDUTY  -=gGlobal.m_stack.operationDoorAcc.deaccStep ;
        }

        if(gGlobal.m_stack.operationID != 3)
            DoorSingleMode_Running(gGlobal.m_stack.operationDIR,\
                                     gGlobal.m_stack.operationDUTY,gGlobal.m_stack.operationID);
        else 
            DoorDualMode_Running(gGlobal.m_stack.operationDIR, gGlobal.m_stack.operationDUTY);
    }

    if (TIM_GetITStatus(TIM1, TIM_IT_CC2) != RESET)
    {
        /* code */
        TIM_ClearFlag(TIM1,TIM_IT_CC2);
        TIM_ClearITPendingBit(TIM1,TIM_IT_CC2);
        gGlobal.m_stack.operationIntCNTforLifter ++;
        if (gGlobal.m_stack.operationIntCNTforLifter < gGlobal.m_stack.operationIntCNTforLifter)
        {
            /* code */
            gGlobal.m_MOTORLifter.dutycycle +=20;
        }
        Lifter_Running(gGlobal.m_MOTORLifter.dir, gGlobal.m_MOTORLifter.dutycycle);
        
    }
}



void TIM6_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM6, TIM_IT_Update))
    {
        /* code */
        TIM_ClearITPendingBit(TIM6, TIM_IT_Update);
        TIM_ClearFlag(TIM6, TIM_IT_Update);
        TIM_Cmd(TIM6, DISABLE);
        if (gGlobal.m_Status.LOCKER_Freeze == 1)
        {
            /* code */
            gGlobal.m_Status.LOCKER_OCPFLAG =0;
            gGlobal.m_Status.LOCKER_Freeze = 0;
            LockerChannelEnable();
						printf("Error: the locker channel is recovered!\r\n");
        }

    }

}

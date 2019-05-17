#include "header.h"
#include "sys.h"	

void LEDTimerConfig(void);



//LED IO��ʼ��
void LED_Init(void)
{
 
	GPIO_InitTypeDef  GPIO_InitStructure;
 	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);	 //ʹ��PB,PE�˿�ʱ��
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8;				 //LED0-->PB.5 �˿�����
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 		 //�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
	GPIO_Init(GPIOC, &GPIO_InitStructure);					 //�����趨������ʼ��GPIOB.5
	LEDTimerConfig();
	
//	TIM_SetCompare1(TIM8, 0);
//	TIM_SetCompare2(TIM8, 0);
//	TIM_SetCompare3(TIM8, 0);
}

/** �ƴ�������� **/


void LEDTimerConfig(void)
{
    
		TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;    
    TIM_OCInitTypeDef  TIM_OCInitStructure;  
	    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);    
	
    TIM_TimeBaseStructure.TIM_Period = 1000;	//1M=1000000/1000=1000(Hz) 2ms
    TIM_TimeBaseStructure.TIM_Prescaler = 71; //72000000/72=1M 
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;                               
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;      
    TIM_TimeBaseInit(TIM8, &TIM_TimeBaseStructure);    
      
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;   
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;                             
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;  
		TIM_OCInitStructure.TIM_Pulse = 100;//����Ϊ2000,ռ�ձ�25% 1/4�ߵ�ƽ 3/4�͵�ƽ
		TIM_OC1Init(TIM8, &TIM_OCInitStructure);
    TIM_OC2Init(TIM8, &TIM_OCInitStructure); // OC2 is for BLUE LED                        
    TIM_OC3Init(TIM8, &TIM_OCInitStructure); //                      

  	TIM_OC1PreloadConfig(TIM8, TIM_OCPreload_Enable);	
		TIM_OC2PreloadConfig(TIM8, TIM_OCPreload_Enable);	
		TIM_OC3PreloadConfig(TIM8, TIM_OCPreload_Enable);	
          
    TIM_ARRPreloadConfig(TIM8, ENABLE);   
		TIM_CtrlPWMOutputs(TIM8,ENABLE);
    TIM_Cmd(TIM8, ENABLE);      
}




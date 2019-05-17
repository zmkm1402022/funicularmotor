#ifndef  __EXDCMOTOR_H
#define  __EXDCMOTOR_H

#include "header.h"


#define LOCKER_FREQ 2000
#define TIM3_TICK_HZ 1000
#define TIM3_PERIOD_CNT ((uint32_t) SystemCoreClock/TIM3_TICK_HZ/LOCKER_FREQ)
#define DUTYCYCLE ((uint32_t) TIM3_PERIOD_CNT/2)

/*  Definitions of lockers , 继电器切换开关 */
#define UPPERLOCKER PCout(0)	//支持切换上门锁电机使能通道

#define LOWERLOCKER PCout(1)  //支持切换下门锁电机使能通道

#define LIFTERLOCKER PCout(2) //支持切换中间隔板电机使能通道

/* 过流之路开关定义*/
#define LOCKERCHANNEL_DISABLE PBout(14)

#define LOWERLOCKER_ENABLE LOWERLOCKER=0
#define LOWERLOCKER_DISABLE LOWERLOCKER=1
#define UPPERLOCKER_ENABLE UPPERLOCKER=0
#define UPPERLOCKER_DISABLE UPPERLOCKER=1

#define ULMOTORENABLE 				PCout(3)  //支持enable/disable 上/下门电机通道
#define LIFTERENABLE 				PBout(15) //支持enable/disable 升降隔板电机通道
#define ULMOTORTIMERENABLE			TIM_Cmd(TIM5, ENABLE)
#define LIFTERTIMERENABLE				TIM_Cmd(TIM2, ENABLE)
#define LOCKERTIMERENABLE				TIM_Cmd(TIM3, ENABLE)
#define ULMOTORTIMERDISABLE			TIM_Cmd(TIM5, DISABLE)
#define LIFTERTIMERDISABLE			TIM_Cmd(TIM2, DISABLE)
#define LOCKERTIMERDISABLE			TIM_Cmd(TIM3, DISABLE)



#define UPPERMOTORBRAKE 			{TIM_SetCompare1(TIM5,0);TIM_SetCompare2(TIM5,0);}
#define LOWERMOTORBRAKE 			{TIM_SetCompare3(TIM5,0);TIM_SetCompare4(TIM5,0);}
#define MOTORIDLE							ULMOTORENABLE = 0
#define LIFTERIDLE   						LIFTERENABLE = 0
#define LIFTERBRAKE						{TIM_SetCompare3(TIM2,0);TIM_SetCompare4(TIM2,0);}
#define LOCKERSTANDBY						{TIM_SetCompare1(TIM3,0);TIM_SetCompare2(TIM3,0);}

#define disableOVC_IRQ 			EXTI->IMR &= ~(1<<OVCurrent_PC14)
#define enableOVC_IRQ 			EXTI->IMR |= 1<<OVCurrent_PC14
#define enableTIM_CC1_IRQ	  	NVIC->ISER[TIM1_CC_IRQn >> 0x05] =(uint32_t)0x01 << (TIM1_CC_IRQn & (uint8_t)0x1F)
#define disableTIM_CC1_IRQ		NVIC->ICER[TIM1_CC_IRQn >> 0x05] =(uint32_t)0x01 << (TIM1_CC_IRQn & (uint8_t)0x1F)
#define enableTIM_UP_IRQ 		NVIC->ISER[TIM1_UP_IRQn >> 0x05] =(uint32_t)0x01 << (TIM1_UP_IRQn & (uint8_t)0x1F)
#define disableTIM_UP_IRQ		NVIC->ICER[TIM1_UP_IRQn >> 0x05] =(uint32_t)0x01 << (TIM1_UP_IRQn & (uint8_t)0x1F)


#define			PWM_FREQ	1
#define   	PWM_DUTYFACTOR_100			1000*PWM_FREQ				//???100%
#define   	PWM_DUTYFACTOR_95				950*PWM_FREQ				//???95%
#define   	PWM_DUTYFACTOR_90				900*PWM_FREQ				//???90%
#define   	PWM_DUTYFACTOR_85				850*PWM_FREQ				//???85%
#define   	PWM_DUTYFACTOR_80				800*PWM_FREQ					//???80%
#define   	PWM_DUTYFACTOR_75				750*PWM_FREQ					//???75%
#define   	PWM_DUTYFACTOR_70				700*PWM_FREQ					//???70%
#define   	PWM_DUTYFACTOR_50				500*PWM_FREQ					//???50%
#define   	PWM_DUTYFACTOR_35				350*PWM_FREQ					//???35%
#define   	PWM_DUTYFACTOR_25				250*PWM_FREQ					//???25%
#define   	PWM_DUTYFACTOR_20				200*PWM_FREQ					//???20%
#define   	PWM_DUTYFACTOR_10				100*PWM_FREQ					//???10%
#define   	PWM_DUTYFACTOR_5				50*PWM_FREQ					//???5%
#define   	PWM_DUTYFACTOR_1				10*PWM_FREQ					//???1%
#define   	PWM_DUTYFACTOR_0				0				//???0%

#define  	SWITCHSTATE_ON				0
#define  	SWITCHSTATE_OFF				1
#define		NOMINAL_CURRENT				100
#define     NOMINAL_CURRENT_MAX			(uint16_t)(NOMINAL_CURRENT*1.1)
#define		NOMINAL_CURRENT_MIN			(uint16_t) (NOMINAL_CURRENT*0.9)
#define     NOMINAL_CURRENT_DYNAMIC		100
#define     NOMINAL_CURRENT_DYNAMIC_MAX			(uint16_t)(NOMINAL_CURRENT_DYNAMIC*1.1)
#define		NOMINAL_CURRENT_DYNAMIC_MIN			(uint16_t) (NOMINAL_CURRENT_DYNAMIC*0.9)
#define		NOMINAL_OPEREATION_TIME_OPEN	3000
#define		NOMINAL_OPEREATION_TIME_MAX 	(uint16_t)(NOMINAL_OPEREATION_TIME_OPEN*1.5)
#define		NOMINAL_OPEREATION_TIME_MIN 	(uint16_t)(NOMINAL_OPEREATION_TIME_OPEN*0.8)


void Motor_CTRL_Init(void);
void Locker_Switching_Init(void);

void DoorEnable_Config(void);
void LifterEnable_Config(void);
void Lifter_Running(u8 dir, u16 dutycycle);
void MotorProcessMonitoring(void);
void MotorFaultMonitoring(void);
void LockerLatch_Config(uint8_t lockerID);
void LockerRelease_Config(uint8_t lockerID);
void DoorDualMode_Running(u8 dir, u16 dutycycle);
void DoorSingleMode_Running(u8 dir, u16 dutycycle,u8 door);
#endif


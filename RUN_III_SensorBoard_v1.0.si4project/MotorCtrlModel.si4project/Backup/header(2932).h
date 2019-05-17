#ifndef __HEADER_H
#define __HEADER_H
#include <stdio.h>
#include "stm32f10x.h"
#include "usart.h"
#include "led.h"
#include "can.h"
#include "OCPSampling.h"
#include "timerapp.h"
#include "ds18b20.h"
#include "led.h"
#include "exDCMotor.h"
#include "stm32f10x_tim.h"
/********************************************************************************************************************************/


//ÒÔÏÂÎªÈí¼þ°æ±¾ºÅ¶¨Òå 21´ú±íV2.1
#define 	SYS_SW_VERSION_H8bit          1
#define 	SYS_SW_VERSION_L8bit          0
//ÒÔÏÂÎªÓ²¼þ°æ±¾ºÅ¶¨Òå 21´ú±íV2.1
#define 	SYS_HW_VERSION_H8bit          1
#define 	SYS_HW_VERSION_L8bit          0

#define CLOCKWISE 						0x11
#define COUNTERCLOCKWISE 				0x12
#define IDLE 							0x13
#define BRAKE							0x14
#define RUNNING  						0x15

#define BUSVOLTAGE	ADC_Channel_5
#define MOTOROCP_01 ADC_Channel_6
#define MOTOROCP_02 ADC_Channel_7
#define MOTOROCP_03 ADC_Channel_14
#define MOTOROCP_04 ADC_Channel_15

#define NumberofChannel 5
#define NumberofCycle 1

#define IRQ_ADC_Priority 5
#define IRQ_EXTI_Priority 2

/*  Over current protection ports */
#define MOTORUPPER_INT_EXTI_LINE			EXTI_Line0
#define MOTORLOWER_INT_EXTI_LINE			EXTI_Line4
#define LIFTER_INT_EXTI_LINE				EXTI_Line9
#define LOCKER_INT_EXTI_LINE				EXTI_Line14
/*  switching sensor ports*/
#define LIFTER_TOP_INT_EXTI_LINE 			EXTI_Line2
#define LIFTER_BOTTOM_INT_EXTI_LINE 		EXTI_Line10
#define UPPERDOOR_CLOSE_INT_EXTI_LINE 		EXTI_Line13
#define UPPERDOOR_OPEN_INT_EXTI_LINE 		EXTI_Line1
#define LOWERDOOR_CLOSE_INT_EXTI_LINE 		EXTI_Line8
#define LOWERDOOR_OPEN_INT_EXTI_LINE 		EXTI_Line9





#define ADCCHANNEL_IRQHandler							DMA1_Channel1_IRQHandler
#define OCP0_IRQHandler									EXTI0_IRQHandler
#define OCP4_IRQHandler									EXTI4_IRQHandler
#define OCP9_5_IRQHandler								EXTI9_5_IRQHandler
#define OCP15_10_IRQHandler								EXTI15_10_IRQHandler
#define Upperdoor_IRQHandler							EXTI1_IRQHandler
#define LifterMidder_IRQHandler							EXTI2_IRQHandler

/* EXTI-LINE input state*/
#define LIFTER_TOP_IN 									GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_2)		//¸ô°åÉÏÉýµ½ÖÐ¼ä´¥Åöµ½µÄ´«¸ÐÆ÷
#define LIFTER_BOTTOM_IN 					GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_10)	//¸ô°åÏÂ½µµ½µ×²¿´¥Åöµ½µÄ´«¸ÐÆ÷
#define UPPERDOOR_CLOSE_IN					GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_13)	//ÉÏÃÅ¹Ø±Õ½«Òª´¥Åöµ½µÄ´«¸ÐÆ÷
#define UPPERDOOR_OPEN_IN					GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_1)  //ÉÏÃÅ¿ªÆô½«Òª´¥Åöµ½µÄ´«¸ÐÆ÷
#define LOWERDOOR_CLOSE_IN								GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_8)	//ÏÂÃÅ¹Ø±Õ½«Òª´¥Åöµ½µÄ´«¸ÐÆ÷
#define LOWERDOOR_OPEN_IN								GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_9)	//ÏÂÃÅ¿ªÆô½«Òª´¥Åöµ½µÄ´«¸ÐÆ÷


/* fetch the input state of locker switches*/
#define LOWERLOCKER_RELEASED_IN								GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_11)
#define LOWERLOCKER_LOCKED_IN								GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_12)
#define UPPERLOCKER_RELEASED_IN								GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_10)
#define UPPERLOCKER_LOCKED_IN								GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_12)

/* OCP Function*/
#define UPPERMOTOR_OCP_IN								GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_0)		
#define LOWERMOTOR_OCP_IN								GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_4)	
#define LIFTERMOTOR_OCP_IN								GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_9)		
#define LOCKERMOTOR_OCP_IN								GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_14)		

/*RESET Function*/
#define GetValue_NJST_Pin								GPIO_ReadOutputDataBit(GPIOB,GPIO_Pin_6)		

/**********************Threshold Parameters for Protection*************************/
typedef struct{
	uint8_t 	status; // default = 0 , CW/CCW/IDLE/BRAKE 
	uint16_t	err_flag; 	// no error = 0x00, overcurrent = 0x01, undervoltage = 0x02,
							// timeout = 0x03 (0x03是错误处理过程中的超时状况）, 
							// others = 0x4X (比如到位开关坏了 0x41=开启到位开关坏， 0x42=关门到位开关坏)
	
	uint16_t	interval;
	uint32_t  timetostart;
	int16_t  dutycycle;
	uint8_t   dir;
	uint8_t 	err_OCPcnt;
} MOTORPARAM;


typedef struct{
	uint8_t mode; // 
	uint16_t	dutycycle; //
} LEDPARAM;

typedef struct{
	uint16_t rawdata[NumberofChannel*NumberofCycle];
	uint16_t voltbus;
	uint16_t currentCH1;
	uint16_t currentCH2;
	uint16_t currentCH3;
	uint16_t currentCH4;
} CURRENTVECTOR;



typedef struct
{
    BYTE 	model_name[10];//Ä£¿éÃû³Æ
    BYTE 	firmware_version_H8bit;//¹Ì¼þ°æ±¾ÕûÊýÎ»
    BYTE 	firnware_version_L8bit;//¹Ì¼þ°æ±¾Ð¡ÊýÎ»
    BYTE 	hardware_version_H8bit;//Ó²¼þ°æ±¾ÕûÊýÎ»
    BYTE 	hardware_version_L8bit;//Ó²¼þ°æ±¾Ð¡ÊýÎ»
    BYTE 	serial_number[16];//ÐòÁÐºÅ
}DeviceInfo;

typedef struct{
	uint8_t NJRSTFlag;
	uint8_t BROADCASTFlag;       // 0x01 = broadcast; 0x02 = stop broadcasting
	uint8_t UPPERLOCKER_STATUS; //0X01 = locked ; 0x02 = unlocked ; 0x03 = in-between
	uint8_t LOWERLOCKER_STATUS;//0X01 = locked ; 0x02 = unlocked; 0x03 = in-between
	uint8_t LIFTER_STATUS;     //0X01 = middle position ; 0x02 = bottom position;0x03 = in-between
	uint8_t UPPERDOOR_STATUS;  //0X01 = open state ; 0x02 = closed state;0x03 = in-between
	uint8_t LOWERDOOR_STATUS;	//0X01 = open state ; 0x02 = closed state; 0x03 = in-between
	uint8_t Electric_LockerChannel; // 0 = on; 1 = off;
	uint16_t UPPERDOOR_ErrCNT;
	uint16_t LOWERDOOR_ErrCNT;
	uint16_t LIFTER_ErrCNT;
	uint16_t LOCKER_ErrCNT;  //累加开机以后出现的过流保护的次数
	uint8_t  LOCKER_OCPFLAG; // 标记门锁这部分出现过流
	uint8_t  LOCKER_RECOVERY_CNT; //用来标记自动重启的次数，如果>=3则自动锁死该路供电，只能通过上位机来启动。
	uint8_t  LOCKER_Freeze;
	uint16_t Lifter_OCPCNT;
	uint8_t  Lifter_OCPFLAG;

} MOTIONFLAG;


typedef struct{
	uint8_t operation_mode;  // 暂时不用
	uint8_t door_ready;  // 0 = ready; 0x55 = running  用来指示目前正在执行开/关门的动作，忽略再次发送过来的指令
	uint8_t lifter_ready;	// 0 = ready; 0x55 = running 用来指示目前正在执行升降的动作
	uint8_t locker_ready;   // 用来指示目前正在执行锁的动作
	uint8_t locker_msg; //用来回复执行结果用的. 1 = Upperlocker, 2 = lowerlocker
	uint8_t locker_msg_result; //用来标记动作执行的结果。 1 = 成功， 2 = 失败， 0 = default。
	uint8_t door_msg_result; //用来标记门电机动作的结构。 1 = 成功， 2 = 失败， 0 = default。
	uint8_t lifter_msg_result;
} COMMUNICATION;


typedef struct
{
	uint16_t accTime;    // 加速区间时间
	uint16_t deaccTime;  //减速区间时间
	uint8_t  accStep;   // 加加速度大小
	uint8_t  deaccStep;    // 记录减加速度大小
	uint16_t realtimeIntCnt;  // 记录进入TIM1中断的次数
}ACCELERATIONPARAM;

typedef struct 
{
	uint8_t operationID;  // 1= uppermotor, 2 = lowermotor, 3 = dualmotor
	uint8_t operationDIR; //  0x11 = CW, 0x12 = CCW
	ACCELERATIONPARAM operationDoorAcc;
	int16_t operationDUTY;  // 门电机动作时的瞬时占空比
	uint16_t operationIntCNTforLifter; 
	uint16_t operationAccTimeforLifter;
	uint32_t temperature_interval;
	uint32_t boradcast_interval;
	uint16_t	door_interval;
	uint32_t door_timetostart;
	uint8_t	 doorEndingSuccess;
	uint8_t  lifterEndingSuccess;
} OPERATIONSTACK;

typedef struct
{
	DeviceInfo          m_DeviceInfo;
	uint32_t						m_LocalTime;
	uint8_t							m_ADCstatus;
	uint16_t 					m_Errcode;
	MOTORPARAM					m_MOTORUpper;
	MOTORPARAM					m_MOTORLower;
	MOTORPARAM					m_MOTORLifter;
	MOTORPARAM					m_LOCKERUpper;
	MOTORPARAM					m_LOCKERLower;
	MOTORPARAM					m_LOCKERLifter;
	CURRENTVECTOR 			m_OCPData;
	LEDPARAM						m_LED;
	COMMUNICATION				m_CAN;
	MOTIONFLAG					m_Status;
	OPERATIONSTACK				m_stack;
	short 					temperature;
}GlobalSet;





    
//¶ÁÐ´Æ÷´æ´¢Î»ÖÃ
//#define BOOT_ADDRESS            (uint32_t)0x08000000
//#define REMAIN_ADDRESS          (uint32_t)0x08004000
//#define BOOT_PARAM_ADDRESS_UART      (uint32_t)0x08004800
//#define BOOT_LAST_ADDRESS       (uint32_t)0x08004FFF

//#define PROCESS_ADDRESS         (uint32_t)0x08005000
//#define PROCESS_LAST_ADDRESS    (uint32_t)0x0800FFFF  

//#define PARAM_ADDRESS           (uint32_t)0x0800F800  

//#define CHIP_LAST_ADDRESS   	(uint32_t)0x0800FFFF 


extern GlobalSet gGlobal;


extern BOOL Check1MSTick(DWORD dwCurTickCount, DWORD dwStart, DWORD DelayTime);
#endif

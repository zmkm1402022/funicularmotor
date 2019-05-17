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


//define the firmware version as v1.0
#define 	SYS_SW_VERSION_H8bit          1
#define 	SYS_SW_VERSION_L8bit          0
//define the hardware version as v1.2
#define 	SYS_HW_VERSION_H8bit          1
#define 	SYS_HW_VERSION_L8bit          2

#define CLOCKWISE 						0x11
#define COUNTERCLOCKWISE 				0x12
#define IDLE 							0x13
#define BRAKE							0x14
#define RUNNING  						0x15

#define BUSVOLTAGE						ADC_Channel_5
#define MOTOROCP_01 					ADC_Channel_6
#define MOTOROCP_02 					ADC_Channel_7
#define MOTOROCP_03 					ADC_Channel_14
#define MOTOROCP_04 					ADC_Channel_15

#define NumberofChannel 				5
#define NumberofCycle 					1

#define IRQ_ADC_Priority 				5
#define IRQ_EXTI_Priority 				2

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
#define LIFTER_TOP_IN 									GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_2)		// PD2: switch sensor at middle position for lifter  (CONN: J25)
#define LIFTER_BOTTOM_IN 								GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_10)	// PA10: switch sensor at bottom position for lifter (CONN: J25)

#define UPPERDOOR_CLOSE_IN								GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_13)	// PB13: switch sensor at closed position for upperdoor (CONN: J19)
#define UPPERDOOR_OPEN_IN								GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_1)  //PB1 : switch sensor at opened position for upperdoor (CONN: J19)

#define LOWERDOOR_CLOSE_IN								GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_8)	//PA8: switch sensor at closed position for upperdoor (CONN: J21)
#define LOWERDOOR_OPEN_IN								GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_9)	//PB9 : switch sensor at opened position for upperdoor (CONN: J21)


/* fetch the input state of locker switches*/
#define UPPERLOCKER_RELEASED_IN								GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_10)
#define UPPERLOCKER_LOCKED_IN								GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_12)

#define LOWERLOCKER_RELEASED_IN								GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_11)
#define LOWERLOCKER_LOCKED_IN								GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_12)



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
	uint16_t	err_flag; 	// no error = 0x00, overcurrent = 0x01, undervoltage = 0x02, timeout = 0x03   
	uint16_t	interval;
	uint32_t  	timetostart;
	int16_t 	 dutycycle;
	uint8_t   	dir;
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
    BYTE 	model_name[10];
    BYTE 	firmware_version_H8bit;
    BYTE 	firnware_version_L8bit;
    BYTE 	hardware_version_H8bit;
    BYTE 	hardware_version_L8bit;
    BYTE 	serial_number[16];
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
	uint16_t LOCKER_ErrCNT;
	uint8_t  LOCKER_RECOVERY_CNT; 
	uint16_t Lifter_OCPCNT;
	uint8_t  Lifter_OCPFLAG;
	uint8_t  LOCKER_OCPFLAG;
	uint8_t  LOCKER_Freeze;
} MOTIONFLAG;


typedef struct{
	uint8_t operation_mode;  
	uint8_t door_ready;  
	uint8_t locker_msg_result;
	uint8_t door_msg_result; 
	uint8_t lifter_msg_result;
	uint8_t locker_ready;
	uint8_t locker_msg;
	uint8_t lifter_ready;
} COMMUNICATION;


typedef struct
{
	uint16_t accTime;    
	uint16_t deaccTime;  
	uint8_t  accStep;   
	uint8_t  deaccStep;    
	uint16_t realtimeIntCnt;  
} ACCELERATIONPARAM;

typedef struct 
{
	uint8_t operationID;  // 1= uppermotor, 2 = lowermotor, 3 = dualmotor
	uint8_t operationDIR; //  0x11 = CW, 0x12 = CCW
	ACCELERATIONPARAM operationDoorAcc;
	int16_t operationDUTY;  
	uint16_t operationAccTimeforLifter;
	uint16_t operationIntCNTforLifter;
	uint32_t temperature_interval;
	uint32_t boradcast_interval;
	uint16_t	door_interval;
	uint32_t door_timetostart;
	uint8_t	 doorEndingSuccess;
	uint8_t  lifterEndingSuccess;
} OPERATIONSTACK;

typedef struct
{
	DeviceInfo          		m_DeviceInfo;
	uint32_t					m_LocalTime;
	uint8_t						m_ADCstatus;
	uint16_t 					m_Errcode;
	MOTORPARAM					m_MOTORUpper;
	MOTORPARAM					m_MOTORLower;
	MOTORPARAM					m_MOTORLifter;
	MOTORPARAM					m_LOCKERUpper;
	MOTORPARAM					m_LOCKERLower;
	MOTORPARAM					m_LOCKERLifter;
	CURRENTVECTOR 				m_OCPData;
	LEDPARAM					m_LED;
	COMMUNICATION				m_CAN;
	MOTIONFLAG					m_Status;
	OPERATIONSTACK				m_stack;
	short 						temperature;
}GlobalSet;

extern GlobalSet gGlobal;


extern BOOL Check1MSTick(DWORD dwCurTickCount, DWORD dwStart, DWORD DelayTime);
#endif

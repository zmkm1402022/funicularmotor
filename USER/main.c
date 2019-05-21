#include "header.h"  
#include <stdio.h>
/************************************************
1. ChamberControlUnit Firmware V1.0
************************************************/
/*
FW1.0

*/
GlobalSet gGlobal;
u16 param_temp;
uint32_t temp_interval;
uint8_t sw1, sw2;
#define PWRSwitch 			PBout(2)

BOOL Check1MSTick(DWORD dwCurTickCount, DWORD dwStart, DWORD DelayTime);
void PWRInit(void);


BOOL Check1MSTick(DWORD dwCurTickCount, DWORD dwStart, DWORD DelayTime)
{
    if (dwCurTickCount >= dwStart)
        return ((dwCurTickCount - dwStart) < DelayTime);
    else
        return ((0xFFFFFFFF - dwStart + dwCurTickCount) < DelayTime);
}
u8 t1,t2,t3,t4,t5,t6,t7,t8,t9,t10;

int main(void)
{	 
		SysTick_Config(SystemCoreClock / 1000);  
		NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);	 
		param_temp = (*(uint32_t *)0x08038000)<<2 | (*(uint32_t *)0x08038001);
		if (param_temp != 0xAA55)
		{
		
		}
		else {
			if(*(uint32_t *)0x08038003 == 1)
			{
				gGlobal.m_MOTORLower.interval = (*(uint32_t *)0x08038008)<<2 | (*(uint32_t *)0x08038009); //default parameter
			}
			else if (*(uint32_t *)0x08038003 == 2)
				gGlobal.m_MOTORLower.interval = (*(uint32_t *)0x0803800A)<<2 | (*(uint32_t *)0x0803800B); //updated parameter
			
			if(*(uint32_t *)0x08038002 == 1)
			{
				gGlobal.m_MOTORUpper.interval = (*(uint32_t *)0x08038004)<<2 | (*(uint32_t *)0x08038005); //default parameter
			}
			else if (*(uint32_t *)0x08038002 == 2)
				gGlobal.m_MOTORUpper.interval = (*(uint32_t *)0x08038006)<<2 | (*(uint32_t *)0x08038007);	//updated parameter		
		}
		TIM_DELAY_INIT();
		#ifdef EN_USART1
		uart_init(115200);
		#else
		AuxiliaryPorts_Init();
		#endif
		VoltSampling_Init();
		Timer1Init();
		LED_Init();
		OCPChannel_Init();       // define the motor OC-protection ports
		Locker_Switching_Init(); // define the relay ports
		ONPOSITIONChannel_Init(); //define the switcher ports
		TasksectionInit();
		Motor_CTRL_Init();
		ULMOTORENABLE = 1;  
		LIFTERENABLE = 1;	
		if(!DS18B20_Init())
			printf ("Warning: a DS1820 has been detected!\r\n");
		else
			printf ("Warning: failing to detect the DS1820!\r\n");

		ULMOTORTIMERENABLE;
		LIFTERTIMERENABLE;
		LOCKERBRAKE
		LOCKERTIMERENABLE;
		CAN_Configuration();
		PWRInit();
		PWRSwitch = 1;
//		gGlobal.m_MOTORLifter.status = RUNNING;
//		Lifter_Running(0x11, PWM_DUTYFACTOR_50);
//		gGlobal.m_Status.BROADCASTFlag = 1;
		while(1)
		{
			t1 = UPPERLOCKER_RELEASED_IN;
			t2 = UPPERLOCKER_LOCKED_IN;
			t3 = LOWERLOCKER_RELEASED_IN;
			t4 = LOWERLOCKER_LOCKED_IN;
			t5 = UPPERDOOR_CLOSE_IN;
			t6 = UPPERDOOR_OPEN_IN;
			t7 = LOWERDOOR_CLOSE_IN;
			t8 = LOWERDOOR_OPEN_IN;
			t9 = LIFTER_TOP_IN;
			t10 = LIFTER_BOTTOM_IN;
			UavcanFun();
			MotorProcessMonitoring();
			UavcanMsgReply();
			MotorFaultMonitoring();
			sw2 = LOWERDOOR_OPEN_IN;
			sw1 = LOWERDOOR_CLOSE_IN;
			if( gGlobal.m_ADCstatus == 1 )
			{
				FetchingCurrentValue();
				DMA_Cmd(DMA1_Channel1, ENABLE);	
				ADC_Cmd(ADC1, ENABLE);
				gGlobal.m_ADCstatus = 0;
			}
			if (gGlobal.m_Status.BROADCASTFlag == 1)
			{
				/* code */
				if(!Check1MSTick(gGlobal.m_LocalTime, gGlobal.m_stack.boradcast_interval, 5000))
				{
					gGlobal.m_stack.boradcast_interval = gGlobal.m_LocalTime;
					Uavcan_Broadcast();
//					if(UPPERLOCKER_RELEASED_IN ==1 && UPPERLOCKER_LOCKED_IN ==0)
//						LockerbacktoOrigin(1, COUNTERCLOCKWISE);
//					else if(UPPERLOCKER_RELEASED_IN ==0 && UPPERLOCKER_LOCKED_IN ==1)
//						LockerbacktoOrigin(1, CLOCKWISE);
				}
			}

			if (!Check1MSTick(gGlobal.m_LocalTime, gGlobal.m_stack.temperature_interval, 20000))
			{
				/* code */
				gGlobal.m_stack.temperature_interval = gGlobal.m_LocalTime;
				gGlobal.temperature = DS18B20_Get_Temp();
				if (gGlobal.temperature > 500)
				{
					/* code */
					gGlobal.temperature = 0;
				}
				printf("current temperaturer is %.1f degree!\r\n", (float) gGlobal.temperature/10);
			}
		}
}



void PWRInit(void)
{
		GPIO_InitTypeDef  GPIO_InitStructure;
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOB, &GPIO_InitStructure);	
		PWRSwitch = 0;
}


/*to trigger an OCP stop for uppermotor operation*/
void OCP0_IRQHandler(void)
{
	if (EXTI_GetFlagStatus(MOTORUPPER_INT_EXTI_LINE) == SET)
	{
		EXTI_ClearITPendingBit(MOTORUPPER_INT_EXTI_LINE);
		EXTI_ClearFlag(MOTORUPPER_INT_EXTI_LINE);
		if (UPPERMOTOR_OCP_IN == RESET)
		{
			DoorSingleMode_Running(BRAKE, 0,1) ;
			gGlobal.m_Status.Upperdoor_OCPFLAG = 1;
			gGlobal.m_Status.UPPERDOOR_ErrCNT ++;
		}
	}
}


/* input to trigger the stop of lifter at middle position PD2*/
void LifterMidder_IRQHandler(void)
{
	if (EXTI_GetFlagStatus(LIFTER_TOP_INT_EXTI_LINE) == SET)
	{
		/* code */
		EXTI_ClearFlag(LIFTER_TOP_INT_EXTI_LINE);
		EXTI_ClearITPendingBit(LIFTER_TOP_INT_EXTI_LINE);
		if (LIFTER_TOP_IN)
		{
			/* code */
			Lifter_Running(BRAKE,0);
			TIM_ITConfig(TIM1,TIM_IT_CC2,DISABLE ); 
			if ( gGlobal.m_MOTORLifter.dir == CLOCKWISE)
			{
				/* code */
				gGlobal.m_stack.lifterEndingSuccess = 1;
			}
			else
				printf("BUG: Lifter top switch--->false triggering\r\n");
			
		}
	}	
}

/* to trigger the stop at PB1 position*/
void Upperdoor_IRQHandler(void)
{
	if (EXTI_GetFlagStatus(UPPERDOOR_OPEN_INT_EXTI_LINE) == SET)
	{
		/* UPPERDOOR OPEN EXTI LINE */
		EXTI_ClearITPendingBit(UPPERDOOR_OPEN_INT_EXTI_LINE);
		EXTI_ClearFlag(UPPERDOOR_OPEN_INT_EXTI_LINE);
		if (UPPERDOOR_OPEN_IN )
		{		
			DoorSingleMode_Running(BRAKE, 0,1) ;
			if (gGlobal.m_CAN.door_ready ==1 && gGlobal.m_stack.operationDIR == CLOCKWISE)
			{
				/* code */
				gGlobal.m_stack.doorEndingSuccess++;
			}
			else
				printf("Warning : upperdoor-opened sensor problem!\r\n");
		}		
	}

}


/* to trigger an OCP for protecting the lowermotor*/
void OCP4_IRQHandler(void)
{

	if(EXTI_GetFlagStatus(MOTORLOWER_INT_EXTI_LINE) == SET)
	{
		EXTI_ClearITPendingBit(MOTORLOWER_INT_EXTI_LINE);
		EXTI_ClearFlag(MOTORLOWER_INT_EXTI_LINE);	
		if( LOWERMOTOR_OCP_IN == RESET)
		{
			DoorSingleMode_Running(BRAKE, 0,2) ;
			gGlobal.m_Status.Lowerdoor_OCPFLAG = 1;
			gGlobal.m_Status.LOWERDOOR_ErrCNT ++;
		}
	}
}

void OCP9_5_IRQHandler(void)
{
	/* to trigger an OCP for protecting the lifter */
	if(EXTI_GetFlagStatus(LIFTER_INT_EXTI_LINE) == SET)
	{
		EXTI_ClearITPendingBit(LIFTER_INT_EXTI_LINE);
		EXTI_ClearFlag(LIFTER_INT_EXTI_LINE);	
/* to trigger the stop of lowerdoor at PA9 position*/
		if (LOWERDOOR_OPEN_IN == SET)
		{
			/* code */
			DoorSingleMode_Running(BRAKE, 0,2) ;
			gGlobal.m_stack.doorEndingSuccess++;
		}		
		if( LIFTERMOTOR_OCP_IN == RESET)
		{
			gGlobal.m_MOTORLifter.err_OCPcnt ++;
			if (gGlobal.m_MOTORLifter.err_OCPcnt >=2)
			{
				/* code */
				Lifter_Running(IDLE, 0);
			}
		}

	}
	
/* to trigger the stop of lowerdoor at PA8 position*/
	if (EXTI_GetFlagStatus(LOWERDOOR_CLOSE_INT_EXTI_LINE) == SET)
	{
		/* code */
		EXTI_ClearFlag(LOWERDOOR_CLOSE_INT_EXTI_LINE);
		EXTI_ClearITPendingBit(LOWERDOOR_CLOSE_INT_EXTI_LINE);
		if (LOWERDOOR_CLOSE_IN)
		{
			/* code */
			DoorSingleMode_Running(BRAKE, 0,2) ;
			gGlobal.m_stack.doorEndingSuccess++;
		}
	}
}

void OCP15_10_IRQHandler(void)
{
	if(EXTI_GetFlagStatus(LOCKER_INT_EXTI_LINE) == SET)
	{
		EXTI_ClearITPendingBit(LOCKER_INT_EXTI_LINE);
		EXTI_ClearFlag(LOCKER_INT_EXTI_LINE);	
		if( LOCKERMOTOR_OCP_IN == RESET)
		{
			if ( gGlobal.m_LOCKERUpper.status != 0 && gGlobal.m_LOCKERLower.status == 0)
			{
				/* code */
				gGlobal.m_LOCKERUpper.err_OCPcnt ++;
				if (gGlobal.m_LOCKERUpper.err_OCPcnt >=2)
				{
					/* code */
					gGlobal.m_LOCKERUpper.err_OCPcnt = 0; 
					gGlobal.m_Status.LOCKER_OCPFLAG = 1;
					gGlobal.m_Status.LOCKER_ErrCNT ++;
				}
			}
		}
	}
/* to trigger the stop of upperdoor at PB13 position*/
	if (EXTI_GetFlagStatus(UPPERDOOR_CLOSE_INT_EXTI_LINE) == SET)
	{
		/* code */
		EXTI_ClearITPendingBit(UPPERDOOR_CLOSE_INT_EXTI_LINE);
		EXTI_ClearFlag(UPPERDOOR_CLOSE_INT_EXTI_LINE);
		if (UPPERDOOR_CLOSE_IN )
		{
			DoorSingleMode_Running(BRAKE, 0,1) ;
			if (gGlobal.m_CAN.door_ready ==1 && gGlobal.m_stack.operationDIR == COUNTERCLOCKWISE )
			{
				/* code */
				gGlobal.m_stack.doorEndingSuccess++;
			}
			else
				printf("BUG : upperdoor-closed sensor problem!\r\n");
		}	
	}
	/* input to trigger the stop of lifter at bottom position PA10*/
	if (EXTI_GetFlagStatus(LIFTER_BOTTOM_INT_EXTI_LINE) == SET)
	{
		/* code */
		EXTI_ClearFlag(LIFTER_BOTTOM_INT_EXTI_LINE);
		EXTI_ClearITPendingBit(LIFTER_BOTTOM_INT_EXTI_LINE);
		if (LIFTER_BOTTOM_IN)
		{
			/* code */
			Lifter_Running(BRAKE,0);
			TIM_ITConfig(TIM1,TIM_IT_CC2,DISABLE ); 
			if ( gGlobal.m_MOTORLifter.dir == COUNTERCLOCKWISE)
			{
				/* code */
				gGlobal.m_stack.lifterEndingSuccess = 1;
			}
			else
				printf("BUG: Lifter bottom--->false triggering\r\n");
			
		}
	}
}


void runIII_init(void)
{
	int err;
	if (UPPERDOOR_CLOSE_IN != SET)
	{
		err = LockerbacktoOrigin( 1 , CLOCKWISE);
		if (err !=0)
			printf("Error: fail to unlock\r\n");
		else
		{
				if(UPPERLOCKER_LOCKED_IN != SET)
				{
					err = DoorbacktoOrigin(1, COUNTERCLOCKWISE);
					if(err != RESET)
					{
						printf("error: fail to close the upperdoor");
					}
				}
				
				if(LOWERLOCKER_LOCKED_IN != SET)
				{
					err = DoorbacktoOrigin(2, COUNTERCLOCKWISE);
					if(err != RESET)
						printf("error: fail to close the upperdoor");
				}
		}
	}
}






















































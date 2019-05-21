#include <string.h>
#include <stdint.h>
#include "header.h"
#include "can.h"
#include "canard.h"
#include "stm32f10x.h"
#include "timerapp.h"
#include "rtc.h"
#include "ds18b20.h"
/*

*/

uint8_t UAVCAN_NODE_ID_SELF_ID;
uint8_t CAN_rx_buf[50];
uint8_t CAN_tx_buf[50];
u16 CAN_RX_LEN;
uint8_t RX_FLAG = 0;

uint8_t IAP_BUFFER[40];
uint32_t flash_hex_baseaddr = 0;

uint8_t is_xdigit(uint8_t data);
uint8_t ascii_to_xdigit(uint8_t data);
uint8_t IAP_FRAM_CHECK(uint8_t Data[],uint8_t *const pResult);
uint8_t IAP_Handle(uint8_t data[]);
uint8_t  iap_flash_write(uint32_t baseaddr,uint8_t data[]);
uint32_t iap_flash_get_baseaddr(uint8_t data[]);
uint8_t iap_flash_set_jumpaddr(void);
//void FLASH_EraseAllNextZONE(uint32_t Page_Address,u16 size);
uint8_t FLASH_EraseAllNextZONE(uint32_t Page_Address,u16 size);
/*
 * Some useful constants defined by the UAVCAN specification.
 * Data type signature values can be easily obtained with the script show_data_type_info.py
 */
#define UAVCAN_NODE_ID_MASTER_ID                                    0x01   //������ַ
#define UAVCAN_NODE_ID_SELF_ID                                      0x08   //������ַ

 
#define UAVCAN_NODE_ID_ALLOCATION_DATA_TYPE_ID                      1
#define UAVCAN_NODE_ID_ALLOCATION_DATA_TYPE_SIGNATURE               0x0b2a812620a11d40
#define UAVCAN_NODE_ID_ALLOCATION_RANDOM_TIMEOUT_RANGE_USEC         400000UL
#define UAVCAN_NODE_ID_ALLOCATION_REQUEST_DELAY_OFFSET_USEC         600000UL

//�������----��������
#define UAVCAN_SERVICE_ID_PING                                  0xC9  //PING�ź�
#define UAVCAN_SERVICE_ID_INFO                                  0xCA  //��ȡ��Ϣ
#define UAVCAN_SERVICE_ID_RESET                                 0xCB  //RESET
#define UAVCAN_SERVICE_ID_START                                 0xCC  //START
#define UAVCAN_SERVICE_ID_STOP                                  0xCD  //STOP 

//�������----
#define UAVCAN_SERVICE_ID_DOOROPERATION                         0xD8
#define UAVCAN_SERVICE_ID_LOCKER                          		0xD9
#define UAVCAN_SERVICE_ID_LIFTER                           		0xDA
#define UAVCAN_SERVICE_ID_PWRSWITCH_LOCKER                      0xDB
//#define UAVCAN_SERVICE_ID_SWITCH_24VOUT                         0xDC
//#define UAVCAN_SERVICE_ID_SWITCH_POWER													0xE1
//#define UAVCAN_SERVICE_ID_OPSTATUS															0xDD
//#define UAVCAN_SERVICE_ID_CHARGESTATUS													0xDE
//#define UAVCAN_SERVICE_ID_CURRENT																0xDF
//#define UAVCAN_SERVICE_ID_ERRSTATUS															0xDD

//�������----���÷���246-256
#define UAVCAN_SERVICE_FIRMWAWR_UPDATE							0xFE  //�̼�����
#define UAVCAN_SERVICE_GET_ZONE									0xFF  //��ȡ��ǰ�̼�����

//SIGNATURE
#define UAVCAN_SERVICE_DEFAULT_SIGNATURE										0x00
#define UAVCAN_SERVICE_PING_SIGNATURE                        0xD9DECB8B9DD64068
#define UAVCAN_SERVICE_NODE_INFO_SIGNATURE                   0x6AB3E1F001ECE006
#define UAVCAN_SERVICE_RESET_SIGNATURE                       0x7E087F0F029578EC
#define UAVCAN_SERVICE_START_SIGNATURE                       0xEC7F28DAC87C4379
#define UAVCAN_SERVICE_STOP_SIGNATURE                        0x644C8832B567A1DC
#define UAVCAN_SERVICE_ID_DOOROPERATION_SIGNATURE                   0x00
#define UAVCAN_SERVICE_ID_LOCKER_SIGNATURE                  		0x00
#define UAVCAN_SERVICE_ID_LIFTER_SIGNATURE                  		0x00
#define UAVCAN_SERVICE_ID_PWRSWITCH_LOCKER_SIGNATURE		 0x644C8832B567A1DD
//#define UAVCAN_SERVICE_ID_SWITCH_24VOUT_SIGNATURE							0x00
//#define UAVCAN_SERVICE_ID_SWITCH_POWER_SIGNATURE								0x00
//#define UAVCAN_SERVICE_ID_CURRENT_SIGNATURE									 0x00
//#define UAVCAN_SERVICE_SET_PARAM_SIGNATURE                   0x00
#define UAVCAN_SERVICE_FIRMWAWR_SIGNATURE                    0x812EBFD0290C78A1
#define UAVCAN_SERVICE_GET_ZONE_SIGNATURE                    0xD1D00F451131DBE2
//#define UAVCAN_SERVICE_ID_ERRSTATUS_SIGNATURE								 0x00
#define UAVCAN_SERVICE_ID_BROADCAST_SIGNATURE 							 0x15C93ADF28B04B9E

//FLASH ADDRESS
#define BOOTLOADER_ADDRESS				(uint32_t)0x08000000	  //bootloader   32k
#define FUNCTION_A_ADDRESS				(uint32_t)0x08008000    //����A��  96K   
#define FUNCTION_B_ADDRESS				(uint32_t)0x08020000   //����B��  96K
#define APP_PARAM_ADDRESS				  (uint32_t)0x08038000    //�û������� 24K
#define BOOT_PARAM_ADDRESS				(uint32_t)0x0803E000    //BOOT������ 6K,������Ϊ�û�������
#define FUNCTION_JUMP_ADDRESS			(uint32_t)0x0803F800    //��ת��ַ�洢 2K
#define IAP_LENTH                 (uint16_t)0xC000     //IAP����������


//#define UAVCAN_NODE_STATUS_MESSAGE_SIZE                             7
//#define UAVCAN_NODE_STATUS_DATA_TYPE_ID                             341
//#define UAVCAN_NODE_STATUS_DATA_TYPE_SIGNATURE                      0x0f0868d0c1a7c6f1

#define UAVCAN_NODE_HEALTH_OK                                       0
#define UAVCAN_NODE_HEALTH_WARNING                                  1
#define UAVCAN_NODE_HEALTH_ERROR                                    2
#define UAVCAN_NODE_HEALTH_CRITICAL                                 3

#define UAVCAN_NODE_MODE_OPERATIONAL                                0
#define UAVCAN_NODE_MODE_INITIALIZATION                             1

#define UAVCAN_GET_NODE_INFO_RESPONSE_MAX_SIZE                      ((3015 + 7) / 8)
#define UAVCAN_GET_NODE_INFO_DATA_TYPE_SIGNATURE                    0xee468a8121c46a9e
#define UAVCAN_GET_NODE_INFO_DATA_TYPE_ID                           1

#define UNIQUE_ID_LENGTH_BYTES                                      16

//�㲥���͵���----��������5080-50BF
#define UAVCAN_TOPIC_ID_AUTO_TDATA											0x5041 //�㲥����
//#define UAVCAN_TOPIC_ID_LED_SET										0x5081 		//LED mode set;
//#define UAVCAN_TOPIC_ID_LOWMHZ_SET_CMD								0x5082 		//433MHZ mode set cmd;
//#define UAVCAN_TOPIC_ID_LOWMHZ_TRANSPARENT_TX						0x5083 		//433MHZ ͸������ ����; ----����CAN��--MCU���ڷ���--433Mģ��--���߷���
//#define UAVCAN_TOPIC_ID_LOWMHZ_TRANSPARENT_RX						0x5084 		//433MHZ ͸������ ����; ----���߽���--433Mģ��--MCU���ڽ���--����CAN����
//#define UAVCAN_TOPIC_ID_BLE_SET_CMD									0x5086 		//���� mode set cmd;
//#define UAVCAN_TOPIC_ID_BLE_TRANSPARENT_TX							0x5087 		//���� ͸������ ����; ----����CAN��--MCU���ڷ���--BLEģ��--��������
//#define UAVCAN_TOPIC_ID_BLE_TRANSPARENT_RX							0x5088 		//���� ͸������ ����; ----��������--BLEģ��--MCU���ڽ���--����CAN����


/*
 * Library instance.
 * In simple applications it makes sense to make it static, but it is not necessary.
 */
CanardInstance canard;                       ///< The library instance
uint8_t canard_memory_pool[1024];            ///< Arena for memory allocation, used by the library
uint8_t transfer_id;
/*
 * Variables used for dynamic node ID allocation.
 * RTFM at http://uavcan.org/Specification/6._Application_level_functions/#dynamic-node-id-allocation
 */
//static uint64_t send_next_node_id_allocation_request_at;    ///< When the next node ID allocation request should be sent
//static uint8_t node_id_allocation_unique_id_offset;         ///< Depends on the stage of the next request

/*
 * Node status variables
 */
//static uint8_t node_health = UAVCAN_NODE_HEALTH_OK;
//static uint8_t node_mode   = UAVCAN_NODE_MODE_INITIALIZATION;
static CanardRxTransfer RxTransfer;

/**
 * This callback is invoked by the library when a new message or request or response is received.
 */
static void onTransferReceived(CanardInstance* ins,
                               CanardRxTransfer* transfer)
{
	uint8_t i=0;
	if(RX_FLAG == 1)
	{
		return;   //����û�д���������������
	}
	if(transfer->payload_len <= 7)
	{
		memcpy(CAN_rx_buf,transfer->payload_head, transfer->payload_len); //������֡����
	}
	else if (transfer->payload_len <= 12)
	{                                                                     //���ƶ�֡����
		memcpy(CAN_rx_buf,transfer->payload_head, 6); //������֡ͷ����
		i += 6;    
		memcpy(&CAN_rx_buf[i],transfer->payload_tail, transfer->payload_len-6); //������֡ͷ����
	}
	else
	{   
		int16_t len = transfer->payload_len;
		memcpy(CAN_rx_buf,transfer->payload_head, 6); //������֡ͷ����
		i += 6;
		len -=5;
		CanardBufferBlock *pbuffer =transfer->payload_middle;
		while( pbuffer != NULL)
		{
			if (len>CANARD_BUFFER_BLOCK_DATA_SIZE)
			{
				memcpy(&CAN_rx_buf[i],pbuffer->data,CANARD_BUFFER_BLOCK_DATA_SIZE); //������֡ͷ����
			}
			else
			{
				memcpy(&CAN_rx_buf[i],pbuffer->data,len); //������֡ͷ����
			}
			pbuffer =pbuffer->next;
			len += CANARD_BUFFER_BLOCK_DATA_SIZE;
			i += CANARD_BUFFER_BLOCK_DATA_SIZE;
		}
		memcpy(&CAN_rx_buf[i],transfer->payload_tail, len);//������֡ͷ����  
	}
	CAN_RX_LEN = transfer->payload_len;		//���յ����ݳ���
	memcpy(&RxTransfer,transfer, sizeof(CanardRxTransfer)); //������֡ͷ���� 
//	canardReleaseRxTransferPayload(ins, transfer);
	RX_FLAG = 1;
}


static bool shouldAcceptTransfer(const CanardInstance* ins,
                                 uint64_t* out_data_type_signature,
                                 uint16_t data_type_id,
                                 CanardTransferType transfer_type,
                                 uint8_t source_node_id)
{
	   
	//(void)source_node_id;

//    if (source_node_id == UAVCAN_NODE_ID_MASTER_ID)
//    {
        /*
         * If we're in the process of allocation of dynamic node ID, accept only relevant transfers.
         */
        if (transfer_type == CanardTransferTypeBroadcast)
        {
					switch (data_type_id)
          {
						//*out_data_type_signature = 
					}
            *out_data_type_signature = UAVCAN_NODE_ID_ALLOCATION_DATA_TYPE_SIGNATURE;
            return true;
        }
//    }
    if (transfer_type == CanardTransferTypeRequest)
    {
//            *out_data_type_signature = UAVCAN_GET_NODE_INFO_DATA_TYPE_SIGNATURE;
//            return true;
		switch (data_type_id)
		{
			case UAVCAN_SERVICE_ID_PING:
			    *out_data_type_signature = UAVCAN_SERVICE_PING_SIGNATURE;	 /*����ͨѶ�Ƿ�����*/
			break;
			case UAVCAN_SERVICE_ID_INFO:
				*out_data_type_signature = UAVCAN_SERVICE_NODE_INFO_SIGNATURE;	/*�ڵ���Ϣ*/
			break;
			case UAVCAN_SERVICE_ID_RESET:
				*out_data_type_signature = UAVCAN_SERVICE_RESET_SIGNATURE; /*��������*/
			break;
			case UAVCAN_SERVICE_ID_START:	
			  *out_data_type_signature = UAVCAN_SERVICE_START_SIGNATURE;
			break;
			case UAVCAN_SERVICE_ID_STOP:	
			  *out_data_type_signature = UAVCAN_SERVICE_STOP_SIGNATURE;	
			break;
			case UAVCAN_SERVICE_ID_DOOROPERATION:
			  *out_data_type_signature = UAVCAN_SERVICE_ID_DOOROPERATION_SIGNATURE;
			break;
			case UAVCAN_SERVICE_ID_LOCKER:
				*out_data_type_signature = UAVCAN_SERVICE_ID_LOCKER_SIGNATURE;
			break;
			case UAVCAN_SERVICE_ID_LIFTER:
				*out_data_type_signature = UAVCAN_SERVICE_ID_LIFTER_SIGNATURE;
			break;
//			case UAVCAN_SERVICE_ID_SET_RARAM:
//				*out_data_type_signature = UAVCAN_SERVICE_SET_PARAM_SIGNATURE;
//			break;
			case UAVCAN_SERVICE_FIRMWAWR_UPDATE:
			   *out_data_type_signature = UAVCAN_SERVICE_FIRMWAWR_SIGNATURE;				
			break;
			case UAVCAN_SERVICE_GET_ZONE:
			   *out_data_type_signature = UAVCAN_SERVICE_GET_ZONE_SIGNATURE;				
			break;
			default:
		       *out_data_type_signature = 0;	
		}
		return true;
    }
	  return false;
}




/********************************************************************************
* ��    �ƣ�
* ��    �ܣ����������
* ��ڲ�������
* ���ڲ�������
* ˵    ����
* ���÷������� 
*********************************************************************************/
void CAN_Configuration(void)                              //CAN���ú���
{
    CAN_InitTypeDef CAN_InitStructure;
    CAN_FilterInitTypeDef CAN_FilterInitStructure;
	  NVIC_InitTypeDef NVIC_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;
    u32 mask =0;
	  u32 ext_id = 0;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1,ENABLE);	   //ʹ��CAN1����ʱ��
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
//	GPIO_PinRemapConfig(GPIO_Remap_CAN_PortA,ENABLE);

    /***********CAN IO��ʼ��**************/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;              //CAN rx: PINA 11
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;           //��������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;              //CAN TX : PINA12
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;         //�����������
    GPIO_Init(GPIOA, &GPIO_InitStructure);


    CAN_DeInit(CAN1);
    CAN_StructInit(&CAN_InitStructure);

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1,ENABLE);	//ʹ��CAN1����ʱ��
                             
    CAN_InitStructure.CAN_TTCM = DISABLE;    //��ֹʱ�䴥��ͨ��ģʽ
    //�����CAN_MCR�Ĵ�����INRQλ������1�����0��һ��Ӳ����⵽128��11λ������    //����λ�����˳�����״̬��
    CAN_InitStructure.CAN_ABOM = DISABLE;    
    //˯��ģʽͨ�����CAN_MCR�Ĵ�����SLEEPλ�����������
    CAN_InitStructure.CAN_AWUM = DISABLE;   
    CAN_InitStructure.CAN_NART = ENABLE;     //DISABLE;CAN����ֻ������1�Σ����ܷ��͵Ľ����Σ��ɹ���������ٲö�ʧ��
    CAN_InitStructure.CAN_RFLM = DISABLE;    //�ڽ������ʱFIFOδ��������������FIFO�ı���δ����������һ���յ��ı��ĻḲ��ԭ�еı���
    CAN_InitStructure.CAN_TXFP = ENABLE;    //����FIFO���ȼ��ɱ��ĵı�ʶ�������� CAN_InitStructure.CAN_Mode=CAN_Mode_LoopBack;
    CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;  //CANӲ������������ģʽ  CAN_Mode_LoopBack
    CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;  //����ͬ����Ծ���1��ʱ�䵥λ
    CAN_InitStructure.CAN_BS1 = CAN_BS1_6tq;  //ʱ���1Ϊ8��ʱ�䵥λ
    CAN_InitStructure.CAN_BS2 = CAN_BS2_5tq;  //ʱ���2Ϊ7��ʱ�䵥λ

    CAN_InitStructure.CAN_Prescaler = 6;   

	/*
	f = 24M/4/(6+5+1)=500K
	*/
    CAN_Init(CAN1,&CAN_InitStructure);
    /**********************************************����֡�˲�******************************/                             
/*    CAN_FilterInitStructure.CAN_FilterNumber=0;                 
    CAN_FilterInitStructure.CAN_FilterMode=CAN_FilterMode_IdMask;
	  CAN_FilterInitStructure.CAN_FilterScale=CAN_FilterScale_32bit;
	  CAN_FilterInitStructure.CAN_FilterIdHigh=0x0000;  
    CAN_FilterInitStructure.CAN_FilterIdLow=0x0000;    	         
    CAN_FilterInitStructure.CAN_FilterMaskIdHigh=0x0000;  
    CAN_FilterInitStructure.CAN_FilterMaskIdLow=0x0000;   
    CAN_FilterInitStructure.CAN_FilterFIFOAssignment=CAN_FIFO0;    
    CAN_FilterInitStructure.CAN_FilterActivation=ENABLE;             
    CAN_FilterInit(&CAN_FilterInitStructure); */
    ext_id = (UAVCAN_NODE_ID_SELF_ID<<8) + 0x80;   //ֻ���յ�ַΪ������ַ�ķ���֡
		mask =   0x00007F80;                           
    
    CAN_FilterInitStructure.CAN_FilterNumber=0;                 
    CAN_FilterInitStructure.CAN_FilterMode=CAN_FilterMode_IdMask;
	  CAN_FilterInitStructure.CAN_FilterScale=CAN_FilterScale_32bit;
	  CAN_FilterInitStructure.CAN_FilterIdHigh=((ext_id<<3) >>16) &0xffff;  
    CAN_FilterInitStructure.CAN_FilterIdLow=(u16)(ext_id<<3) | CAN_ID_EXT;    	         
    CAN_FilterInitStructure.CAN_FilterMaskIdHigh=((mask<<3) >>16)&0xffff;  
    CAN_FilterInitStructure.CAN_FilterMaskIdLow=(u16)(mask<<3) | CAN_ID_EXT;   
    CAN_FilterInitStructure.CAN_FilterFIFOAssignment=CAN_FIFO0;    
    CAN_FilterInitStructure.CAN_FilterActivation=ENABLE;             
    CAN_FilterInit(&CAN_FilterInitStructure); 
		
	 /**********************************�㲥֡�˲�*ϵͳʹ��*****************************/
    /*ext_id = 0x00800000;   //ͨ��0x8000-0xffff
		mask =   0x00800080;        	
	  CAN_FilterInitStructure.CAN_FilterNumber=1;             
    CAN_FilterInitStructure.CAN_FilterMode=CAN_FilterMode_IdMask;
	  CAN_FilterInitStructure.CAN_FilterScale=CAN_FilterScale_32bit;
	  CAN_FilterInitStructure.CAN_FilterIdHigh=((ext_id<<3) >>16) &0xffff;  
    CAN_FilterInitStructure.CAN_FilterIdLow=(u16)(ext_id<<3) | CAN_ID_EXT;   	          
    CAN_FilterInitStructure.CAN_FilterMaskIdHigh=((mask<<3) >>16)&0xffff;  
    CAN_FilterInitStructure.CAN_FilterMaskIdLow=(u16)(mask<<3) | CAN_ID_EXT;   
    CAN_FilterInitStructure.CAN_FilterFIFOAssignment=CAN_FIFO0;    
    CAN_FilterInitStructure.CAN_FilterActivation=ENABLE;             
    CAN_FilterInit(&CAN_FilterInitStructure);*/
		
	 /**********************************�㲥֡�˲�*ϵͳʹ��*****************************/
    /*ext_id = 0x00000000;   //ͨ��0x0000-0x3fff
		mask =   0x00C00080;        	
	  CAN_FilterInitStructure.CAN_FilterNumber=2;             
    CAN_FilterInitStructure.CAN_FilterMode=CAN_FilterMode_IdMask;
	  CAN_FilterInitStructure.CAN_FilterScale=CAN_FilterScale_32bit;
	  CAN_FilterInitStructure.CAN_FilterIdHigh=((ext_id<<3) >>16) &0xffff;   
    CAN_FilterInitStructure.CAN_FilterIdLow=(u16)(ext_id<<3) | CAN_ID_EXT;    	          
    CAN_FilterInitStructure.CAN_FilterMaskIdHigh=((mask<<3) >>16)&0xffff;  
    CAN_FilterInitStructure.CAN_FilterMaskIdLow=(u16)(mask<<3) | CAN_ID_EXT;    
    CAN_FilterInitStructure.CAN_FilterFIFOAssignment=CAN_FIFO0;    
    CAN_FilterInitStructure.CAN_FilterActivation=ENABLE;             
    CAN_FilterInit(&CAN_FilterInitStructure);	*/	
		

	 /**********************************�㲥֡�˲�*ϵͳʹ��*****************************/
    /*ext_id = 0x00400000;;   //ͨ��0x4000-0x4fff
		mask =   0x00f00080;        	
	  CAN_FilterInitStructure.CAN_FilterNumber=3;             
    CAN_FilterInitStructure.CAN_FilterMode=CAN_FilterMode_IdMask;
	  CAN_FilterInitStructure.CAN_FilterScale=CAN_FilterScale_32bit;
	  CAN_FilterInitStructure.CAN_FilterIdHigh=((ext_id<<3) >>16) &0xffff; 
    CAN_FilterInitStructure.CAN_FilterIdLow=(u16)(ext_id<<3) | CAN_ID_EXT; 	          
    CAN_FilterInitStructure.CAN_FilterMaskIdHigh=((mask<<3) >>16)&0xffff; 
    CAN_FilterInitStructure.CAN_FilterMaskIdLow=(u16)(mask<<3) | CAN_ID_EXT;    
    CAN_FilterInitStructure.CAN_FilterFIFOAssignment=CAN_FIFO0;    
    CAN_FilterInitStructure.CAN_FilterActivation=ENABLE;             
    CAN_FilterInit(&CAN_FilterInitStructure);	*/	

	 /**********************************�㲥֡�˲�*ϵͳʹ��*****************************/
    /*ext_id = 0x00700000;;   //ͨ��0x6000-0x7fff
		mask =   0x00E00080;        	
	  CAN_FilterInitStructure.CAN_FilterNumber=4;             
    CAN_FilterInitStructure.CAN_FilterMode=CAN_FilterMode_IdMask;
	  CAN_FilterInitStructure.CAN_FilterScale=CAN_FilterScale_32bit;
	  CAN_FilterInitStructure.CAN_FilterIdHigh=((ext_id<<3) >>16) &0xffff;  
    CAN_FilterInitStructure.CAN_FilterIdLow=(u16)(ext_id<<3) | CAN_ID_EXT;   	          
    CAN_FilterInitStructure.CAN_FilterMaskIdHigh=((mask<<3) >>16)&0xffff;  
    CAN_FilterInitStructure.CAN_FilterMaskIdLow=(u16)(mask<<3) | CAN_ID_EXT;    
    CAN_FilterInitStructure.CAN_FilterFIFOAssignment=CAN_FIFO0;    
    CAN_FilterInitStructure.CAN_FilterActivation=ENABLE;             
    CAN_FilterInit(&CAN_FilterInitStructure);*/		

	 /**********************************�㲥֡�˲�*ϵͳʹ��*****************************/
    /*ext_id = 0x00580000;   //ͨ��0x5800-0x5FFF
		mask =   0x00F80080;        	
	  CAN_FilterInitStructure.CAN_FilterNumber=5;             
    CAN_FilterInitStructure.CAN_FilterMode=CAN_FilterMode_IdMask;
	  CAN_FilterInitStructure.CAN_FilterScale=CAN_FilterScale_32bit;
	  CAN_FilterInitStructure.CAN_FilterIdHigh=((ext_id<<3) >>16) &0xffff;  
    CAN_FilterInitStructure.CAN_FilterIdLow=(u16)(ext_id<<3) | CAN_ID_EXT;    	          
    CAN_FilterInitStructure.CAN_FilterMaskIdHigh=((mask<<3) >>16)&0xffff;   
    CAN_FilterInitStructure.CAN_FilterMaskIdLow=(u16)(mask<<3) | CAN_ID_EXT;    
    CAN_FilterInitStructure.CAN_FilterFIFOAssignment=CAN_FIFO0;    
    CAN_FilterInitStructure.CAN_FilterActivation=ENABLE;             
    CAN_FilterInit(&CAN_FilterInitStructure);	*/	

    /**********************************�㲥֡�˲�*ϵͳʹ��*****************************/
    /*ext_id = 0x00540000;;   //ͨ��0x5400-0x57FF
		mask =   0x00FC0080;        	
	  CAN_FilterInitStructure.CAN_FilterNumber=6;             
    CAN_FilterInitStructure.CAN_FilterMode=CAN_FilterMode_IdMask;
	  CAN_FilterInitStructure.CAN_FilterScale=CAN_FilterScale_32bit;
	  CAN_FilterInitStructure.CAN_FilterIdHigh=((ext_id<<3) >>16) &0xffff;   
    CAN_FilterInitStructure.CAN_FilterIdLow=(u16)(ext_id<<3) | CAN_ID_EXT;   	          
    CAN_FilterInitStructure.CAN_FilterMaskIdHigh=((mask<<3) >>16)&0xffff;  
    CAN_FilterInitStructure.CAN_FilterMaskIdLow=(u16)(mask<<3) | CAN_ID_EXT;   
    CAN_FilterInitStructure.CAN_FilterFIFOAssignment=CAN_FIFO0;    
    CAN_FilterInitStructure.CAN_FilterActivation=ENABLE;             
    CAN_FilterInit(&CAN_FilterInitStructure);	*/	

    /**********************************�㲥֡�˲�*ϵͳʹ��*****************************/
    /*ext_id = 0x00520000;;   //ͨ��0x5200-0x53FF
		mask =   0x00FE0080;        	
	  CAN_FilterInitStructure.CAN_FilterNumber=7;             
    CAN_FilterInitStructure.CAN_FilterMode=CAN_FilterMode_IdMask;
	  CAN_FilterInitStructure.CAN_FilterScale=CAN_FilterScale_32bit;
	  CAN_FilterInitStructure.CAN_FilterIdHigh=((ext_id<<3) >>16) &0xffff;  
    CAN_FilterInitStructure.CAN_FilterIdLow=(u16)(ext_id<<3) | CAN_ID_EXT;   	          
    CAN_FilterInitStructure.CAN_FilterMaskIdHigh=((mask<<3) >>16)&0xffff;  
    CAN_FilterInitStructure.CAN_FilterMaskIdLow=(u16)(mask<<3) | CAN_ID_EXT;   
    CAN_FilterInitStructure.CAN_FilterFIFOAssignment=CAN_FIFO0;    
    CAN_FilterInitStructure.CAN_FilterActivation=ENABLE;             
    CAN_FilterInit(&CAN_FilterInitStructure);	*/


    /**********************************�㲥֡�˲�*��������*****************************/
//    ext_id = 0x00504000;;   //ͨ��0x5040-0x507F
//		mask =   0x00ffc080;        	
//	  CAN_FilterInitStructure.CAN_FilterNumber=8;             
//    CAN_FilterInitStructure.CAN_FilterMode=CAN_FilterMode_IdMask;
//	  CAN_FilterInitStructure.CAN_FilterScale=CAN_FilterScale_32bit;
//	  CAN_FilterInitStructure.CAN_FilterIdHigh=((ext_id<<3) >>16) &0xffff;  
//    CAN_FilterInitStructure.CAN_FilterIdLow=(u16)(ext_id<<3) | CAN_ID_EXT;   	          
//    CAN_FilterInitStructure.CAN_FilterMaskIdHigh=((mask<<3) >>16)&0xffff;  
//    CAN_FilterInitStructure.CAN_FilterMaskIdLow=(u16)(mask<<3) | CAN_ID_EXT;     
//    CAN_FilterInitStructure.CAN_FilterFIFOAssignment=CAN_FIFO0;    
//    CAN_FilterInitStructure.CAN_FilterActivation=ENABLE;             
//    CAN_FilterInit(&CAN_FilterInitStructure);	


//    
//	    /**************************************�㲥֡����㲥******************************/
//    ext_id = 0x00503F6F;   //ͨ��0x500040-0x503F7F
//		mask =   0x00ffc0c0;        	
//	  CAN_FilterInitStructure.CAN_FilterNumber=8;             
//    CAN_FilterInitStructure.CAN_FilterMode=CAN_FilterMode_IdMask;
//	  CAN_FilterInitStructure.CAN_FilterScale=CAN_FilterScale_32bit;
//	  CAN_FilterInitStructure.CAN_FilterIdHigh=((ext_id<<3) >>16) &0xffff;  
//    CAN_FilterInitStructure.CAN_FilterIdLow=(u16)(ext_id<<3) | CAN_ID_EXT;    	          
//    CAN_FilterInitStructure.CAN_FilterMaskIdHigh=((mask<<3) >>16)&0xffff;  
//    CAN_FilterInitStructure.CAN_FilterMaskIdLow=(u16)(mask<<3) | CAN_ID_EXT;    
//    CAN_FilterInitStructure.CAN_FilterFIFOAssignment=CAN_FIFO0;    
//    CAN_FilterInitStructure.CAN_FilterActivation=ENABLE;             
//    CAN_FilterInit(&CAN_FilterInitStructure);	

//		
//	   /**************************************�㲥֡����㲥******************************/
//    ext_id = 0x00500210;   //ͨ��0x5000-0-0x503F0F
//		mask =   0x00FFFFFF;        	
//	  CAN_FilterInitStructure.CAN_FilterNumber=9;             
//    CAN_FilterInitStructure.CAN_FilterMode=CAN_FilterMode_IdMask;
//	  CAN_FilterInitStructure.CAN_FilterScale=CAN_FilterScale_32bit;
//	  CAN_FilterInitStructure.CAN_FilterIdHigh=((ext_id<<3) >>16) &0xffff;   
//    CAN_FilterInitStructure.CAN_FilterIdLow=(u16)(ext_id<<3) | CAN_ID_EXT;   	          
//    CAN_FilterInitStructure.CAN_FilterMaskIdHigh=((mask<<3) >>16)&0xffff; 
//    CAN_FilterInitStructure.CAN_FilterMaskIdLow=(u16)(mask<<3) | 0xffff;    
//    CAN_FilterInitStructure.CAN_FilterFIFOAssignment=CAN_FIFO0;    
//    CAN_FilterInitStructure.CAN_FilterActivation=ENABLE;             
//    CAN_FilterInit(&CAN_FilterInitStructure);	
//		
//		
//			
//	   /**************************************�㲥֡����㲥******************************/
//    ext_id = 0x00503F2F;   //ͨ��0x5000-0-0x503F0F
//		mask =   0x00ffc0E0;
//	  CAN_FilterInitStructure.CAN_FilterNumber=10;             
//    CAN_FilterInitStructure.CAN_FilterMode=CAN_FilterMode_IdMask;
//	  CAN_FilterInitStructure.CAN_FilterScale=CAN_FilterScale_32bit;
//	  CAN_FilterInitStructure.CAN_FilterIdHigh=((ext_id<<3) >>16) &0xffff;   
//    CAN_FilterInitStructure.CAN_FilterIdLow=(u16)(ext_id<<3) | CAN_ID_EXT;    	          
//    CAN_FilterInitStructure.CAN_FilterMaskIdHigh=((mask<<3) >>16)&0xffff;  
//    CAN_FilterInitStructure.CAN_FilterMaskIdLow=(u16)(mask<<3) | CAN_ID_EXT;    
//    CAN_FilterInitStructure.CAN_FilterFIFOAssignment=CAN_FIFO0;    
//    CAN_FilterInitStructure.CAN_FilterActivation=ENABLE;             
//    CAN_FilterInit(&CAN_FilterInitStructure);		
			
    CAN_ITConfig(CAN1,CAN_IT_FMP0, ENABLE);    //ʹ��ָ����CAN�ж�
		NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
	  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;			//��ռʽ���ȼ�(0~1)
	  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;				//��Ӧ���ȼ�(0~7)
	  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	  NVIC_Init(&NVIC_InitStructure);
		
		canardInit(&canard, canard_memory_pool, sizeof(canard_memory_pool), onTransferReceived, shouldAcceptTransfer, NULL);
    canardSetLocalNodeID(&canard, UAVCAN_NODE_ID_SELF_ID);
}

/****************************************************************************
* ��    �ƣ�Uavcan_Broadcast
* ��    �ܣ�CAN���߹㲥���Զ��ϴ���
* ��ڲ�������
* ���ڲ�������
* ˵    ����
* ���÷������� 
****************************************************************************/
void Uavcan_Broadcast(void)
{

	CAN_tx_buf[0]= 0x11;
	CAN_tx_buf[1]= 0x22;
	CAN_tx_buf[2]= 0x33;
	CAN_tx_buf[3]= 0x44;
	CAN_tx_buf[4]= 0x55;
	canardBroadcast(&canard, UAVCAN_SERVICE_ID_BROADCAST_SIGNATURE, UAVCAN_TOPIC_ID_AUTO_TDATA, &transfer_id,31,CAN_tx_buf, 5);
}

/****************************************************************************
* ��    �ƣ�UavcanFun 
* ��    �ܣ�CAN���������շ�
* ��ڲ�������
* ���ڲ�������
* ˵    ����
* ���÷������� 
****************************************************************************/
void UavcanMsgReply(void)
{
	if (gGlobal.m_CAN.locker_ready == 1 && gGlobal.m_CAN.locker_msg_result != 0)
	{
		/* code */
		gGlobal.m_CAN.locker_ready = 0;
		CAN_tx_buf[0] = gGlobal.m_CAN.locker_msg;
		CAN_tx_buf[1] = gGlobal.m_CAN.locker_msg_result;
		canardRequestOrRespond(&canard, RxTransfer.source_node_id,\
								UAVCAN_SERVICE_ID_LOCKER_SIGNATURE,\
								UAVCAN_SERVICE_ID_LOCKER,\
								&RxTransfer.transfer_id,\
								RxTransfer.priority,\
								CanardResponse,CAN_tx_buf, 2);		
		gGlobal.m_CAN.locker_msg = 0;
		gGlobal.m_CAN.locker_msg_result = 0;	
	}
	if (gGlobal.m_CAN.door_ready == 1 && gGlobal.m_CAN.door_msg_result != 0)
	{
		/* code */
		TIM_ITConfig(TIM1,TIM_IT_CC1,DISABLE ); 
		CAN_tx_buf[0] = gGlobal.m_CAN.locker_msg_result;
		CAN_tx_buf[1] = gGlobal.m_stack.operationID;
		CAN_tx_buf[2] = gGlobal.m_stack.operationDIR;
		CAN_tx_buf[3] = gGlobal.m_MOTORUpper.err_flag;
		CAN_tx_buf[4] = gGlobal.m_MOTORLower.err_flag;
		CAN_tx_buf[5] = gGlobal.m_LOCKERUpper.err_flag;
		CAN_tx_buf[6] = gGlobal.m_LOCKERLower.err_flag;
		canardRequestOrRespond(&canard, RxTransfer.source_node_id,\
								UAVCAN_SERVICE_ID_DOOROPERATION_SIGNATURE,\
								UAVCAN_SERVICE_ID_DOOROPERATION,\
								&RxTransfer.transfer_id,\
								RxTransfer.priority,\
								CanardResponse,CAN_tx_buf, 7);	
		gGlobal.m_CAN.door_msg_result = 0;
		gGlobal.m_CAN.door_ready = 0;
		gGlobal.m_stack.operationID =0 ;
		gGlobal.m_stack.operationDIR =0 ;
		gGlobal.m_CAN.operation_mode = 0;
		memset(CAN_tx_buf, 0, sizeof(CAN_tx_buf));
	}

	if (gGlobal.m_CAN.lifter_ready == 1  && gGlobal.m_CAN.lifter_msg_result != 0)
	{
		/* code */
		TIM_ITConfig(TIM1,TIM_IT_CC2,DISABLE ); 
		CAN_tx_buf[0] = gGlobal.m_CAN.lifter_msg_result;
		CAN_tx_buf[1] = gGlobal.m_MOTORLifter.err_flag;
		CAN_tx_buf[2] = gGlobal.m_MOTORLifter.dir;
		canardRequestOrRespond(&canard, RxTransfer.source_node_id,\
								UAVCAN_SERVICE_ID_LIFTER_SIGNATURE,\
								UAVCAN_SERVICE_ID_LIFTER,\
								&RxTransfer.transfer_id,\
								RxTransfer.priority,\
								CanardResponse,CAN_tx_buf, 3);	
		printf("Warning: action to operate the lifter is finished, the result is = %d\r\n", gGlobal.m_CAN.lifter_msg_result);
		gGlobal.m_CAN.lifter_ready = 0;
		gGlobal.m_CAN.lifter_msg_result = 0;
		gGlobal.m_MOTORLifter.dir = 0;		
		
	}
}


void UavcanFun(void)
{
	CanTxMsg cantxframe; 
	uint8_t loadlength;
	if( RX_FLAG == 1)
	{
		if(RxTransfer.transfer_type == CanardTransferTypeBroadcast)
		{
			if(RxTransfer.data_type_id == 0x5002)
			{
	
				;
			}

		}
		else if (RxTransfer.transfer_type == CanardTransferTypeRequest)
		{				   
			switch(RxTransfer.data_type_id)
			{
				case UAVCAN_SERVICE_ID_PING:					//PING		
					CAN_tx_buf[0] = 1;
					loadlength = 1;
					canardRequestOrRespond(&canard, RxTransfer.source_node_id,
																UAVCAN_SERVICE_PING_SIGNATURE,
																UAVCAN_SERVICE_ID_PING,
																&RxTransfer.transfer_id,
																RxTransfer.priority,
																CanardResponse,CAN_tx_buf, loadlength);
				break;
				
				case UAVCAN_SERVICE_ID_INFO:					//READ Info��ȡ��Ϣ
					canardRequestOrRespond(&canard, RxTransfer.source_node_id,UAVCAN_SERVICE_NODE_INFO_SIGNATURE,
																UAVCAN_SERVICE_ID_INFO,&RxTransfer.transfer_id,RxTransfer.priority,
																CanardResponse,&gGlobal.m_DeviceInfo,sizeof(gGlobal.m_DeviceInfo));
				break;
				
				
				case UAVCAN_SERVICE_ID_RESET:					//RESET ��λ
					CAN_tx_buf[0] = 0x55;
					CAN_tx_buf[1] = 0xAA;
					loadlength = 2;
					canardRequestOrRespond(&canard, RxTransfer.source_node_id,
																UAVCAN_SERVICE_RESET_SIGNATURE,
																UAVCAN_SERVICE_ID_RESET,
																&RxTransfer.transfer_id,
																RxTransfer.priority,
																CanardResponse,CAN_tx_buf, loadlength);					
				break;			

				
				case UAVCAN_SERVICE_ID_START:					//START ������������
					if(gGlobal.m_Status.BROADCASTFlag != 1)
					{
						gGlobal.m_Status.BROADCASTFlag = 1;
						CAN_tx_buf[0] = 0x55;
						CAN_tx_buf[1] = 0xAA;
						loadlength = 2;
						canardRequestOrRespond(&canard, RxTransfer.source_node_id,
																	UAVCAN_SERVICE_START_SIGNATURE,
																	UAVCAN_SERVICE_ID_START,
																	&RxTransfer.transfer_id,
																	RxTransfer.priority,
																	CanardResponse,CAN_tx_buf, loadlength);		
					}

					else
					{
						/*�ظ�ָ���*/
						CAN_tx_buf[0] = 0x53;
						CAN_tx_buf[1] = 0xA9;
						loadlength = 2;
						canardRequestOrRespond(&canard, RxTransfer.source_node_id,
																	UAVCAN_SERVICE_START_SIGNATURE,
																	UAVCAN_SERVICE_ID_START,
																	&RxTransfer.transfer_id,
																	RxTransfer.priority,
																	CanardResponse,CAN_tx_buf, loadlength);			
					}
					
				break;			

					
				case UAVCAN_SERVICE_ID_STOP:					//STOP ֹͣ��������
					if(gGlobal.m_Status.BROADCASTFlag != 2){
					gGlobal.m_Status.BROADCASTFlag = 2;
					CAN_tx_buf[0] = 0x55;
					CAN_tx_buf[1] = 0xAA;
					loadlength = 2;
					canardRequestOrRespond(&canard, RxTransfer.source_node_id,
																UAVCAN_SERVICE_STOP_SIGNATURE,
																UAVCAN_SERVICE_ID_STOP,
																&RxTransfer.transfer_id,
																RxTransfer.priority,
																CanardResponse,CAN_tx_buf, loadlength);		
					}

					else{
						/*�ظ�ָ���*/
					CAN_tx_buf[0] = 0x53;
					CAN_tx_buf[1] = 0xA9;
					loadlength = 2;
					canardRequestOrRespond(&canard, RxTransfer.source_node_id,
																UAVCAN_SERVICE_STOP_SIGNATURE,
																UAVCAN_SERVICE_ID_STOP,
																&RxTransfer.transfer_id,
																RxTransfer.priority,
																CanardResponse,CAN_tx_buf, loadlength);			
					}
				break;
					
				
				case UAVCAN_SERVICE_ID_DOOROPERATION:   // D8
					if(gGlobal.m_CAN.door_ready == 0)
					{
						gGlobal.m_CAN.door_ready = 1;
						/* single door mode*/
						if (CAN_rx_buf [0] ==1)  
						{
							//gGlobal.m_CAN.operation_mode = 1;
							//��������  example: 01 01 00 01(open upper door);01 01 00 02 (close the upper door)
							if(CAN_rx_buf[1] ==1 && CAN_rx_buf[2] ==0)  
							{
								gGlobal.m_stack.operationID = 1;
								//���Ż��߹���
								switch (CAN_rx_buf[3])
								{
									case 1:
											gGlobal.m_stack.operationDIR = 0x11; //open the door = clockwise
										break;
									case 2:
											gGlobal.m_stack.operationDIR  = 0x12; //close the door
										break;
								}
							
							}	
							//�������� 
							//example: 01 00 01 01 (open the lower door); 01 00 01 02��close the lower door)
							else if(CAN_rx_buf[1] ==0 && CAN_rx_buf[2] ==1)
							{
								gGlobal.m_stack.operationID = 2;
								// ���Ż��߹���
								switch(CAN_rx_buf[3])
								{
									case 1:
										gGlobal.m_stack.operationDIR = 0x11;
									break;
									
									case 2:
										gGlobal.m_stack.operationDIR = 0x12;
									break;		
								}
							}
							
							else
							{
								gGlobal.m_CAN.door_ready = 0;
								printf("Error: there is a problem of parameters for single mode operation \r\n");
							}
								
						
						}
						/* dual door mode*/
						// example : 02 01 01 01 open both doors; 02 01 01 02 close both doors
						else if (CAN_rx_buf [0] ==2)
						{
							//gGlobal.m_CAN.operation_mode =2;
							gGlobal.m_stack.operationID = 3;
							if (CAN_rx_buf[1] ==1 && CAN_rx_buf[2] ==1 && CAN_rx_buf[3] ==1)
								gGlobal.m_stack.operationDIR = 0x11;  //door open
							else if(CAN_rx_buf[1] ==1 && CAN_rx_buf[2] ==1 && CAN_rx_buf[3] ==2)
								gGlobal.m_stack.operationDIR = 0x12;  //door close
							else
							{
								gGlobal.m_CAN.door_ready = 0;
								printf("Error: there is a problem of parameters for dual mode operation \r\n");								
							}
						}
						
						else
						{
							gGlobal.m_CAN.door_ready =0;
							printf("Error: Fail to select the mode for door operation\r\n");
						}
						
						CAN_tx_buf[0] = 0x55;
						CAN_tx_buf[1] = 0xAA;
						loadlength = 2;
						canardRequestOrRespond(&canard, RxTransfer.source_node_id,
																	UAVCAN_SERVICE_ID_DOOROPERATION_SIGNATURE,
																	UAVCAN_SERVICE_ID_DOOROPERATION,
																	&RxTransfer.transfer_id,
																	RxTransfer.priority,
																	CanardResponse,CAN_tx_buf, loadlength);		
						if (gGlobal.m_CAN.door_ready != 0)
						{
							/* code */
							printf("Note: start to open the door!\r\n");
							DoorEnable_Config();
						}
											
					}
					else  // the last door-operation has not finished
					{
						CAN_tx_buf[0] = 0x56;
						CAN_tx_buf[1] = 0xAB;
						loadlength = 2;
						canardRequestOrRespond(&canard, RxTransfer.source_node_id,
																	UAVCAN_SERVICE_ID_DOOROPERATION_SIGNATURE,
																	UAVCAN_SERVICE_ID_DOOROPERATION,
																	&RxTransfer.transfer_id,
																	RxTransfer.priority,
																	CanardResponse,CAN_tx_buf, loadlength);		
						printf("Warning:the last door-operation has not finished\r\n ");
					}
				break;
					
					
				case UAVCAN_SERVICE_ID_LOCKER:
					if (gGlobal.m_CAN.locker_ready == 0)
					{
						/* code */
						gGlobal.m_CAN.locker_ready = 1;
						gGlobal.m_CAN.locker_msg = CAN_rx_buf[0];
						if ( CAN_rx_buf [0] == 1)
						{
							/* to release the upperlocker */
							gGlobal.m_LOCKERUpper.status = CLOCKWISE;
						}
						else if (CAN_rx_buf [0] == 2)
						{
							/* to lock the upperlocker */
							gGlobal.m_LOCKERUpper.status = COUNTERCLOCKWISE;
						}
						else if (CAN_rx_buf [0] == 3)
						{
							/* to release the lowerlocker */
							gGlobal.m_LOCKERLower.status = CLOCKWISE;
						}
						else if (CAN_rx_buf [0] == 4)
						{
							/* to lock the lowerlocker */
							gGlobal.m_LOCKERLower.status = COUNTERCLOCKWISE;
						}
						else
						{
							gGlobal.m_CAN.locker_ready = 0;
							printf("Error: there is a problem of parameters for locker operation \r\n");

						}
						CAN_tx_buf[0] = 0x55;
						CAN_tx_buf[1] = 0xAA;
						loadlength = 2;
						canardRequestOrRespond(&canard, RxTransfer.source_node_id,
																	UAVCAN_SERVICE_ID_LOCKER_SIGNATURE,
																	UAVCAN_SERVICE_ID_LOCKER,
																	&RxTransfer.transfer_id,
																	RxTransfer.priority,
																	CanardResponse,CAN_tx_buf, loadlength);	
					}
					else{
						/*�ظ�ָ���*/
					CAN_tx_buf[0] = 0x53;
					CAN_tx_buf[1] = 0xA9;
					loadlength = 2;
					canardRequestOrRespond(&canard, RxTransfer.source_node_id,
																UAVCAN_SERVICE_ID_LOCKER_SIGNATURE,
																UAVCAN_SERVICE_ID_LOCKER,
																&RxTransfer.transfer_id,
																RxTransfer.priority,
																CanardResponse,CAN_tx_buf, loadlength);			
					}
				break;
				
				
				case UAVCAN_SERVICE_ID_LIFTER:
					if(gGlobal.m_CAN.lifter_ready ==0 )
					{
						gGlobal.m_CAN.lifter_ready = 1;
						if (CAN_rx_buf [0] ==1)  //CW  lifter moving upwards
						{
							/* code */
							gGlobal.m_MOTORLifter.dir = 0x11;
						}
						else if(CAN_rx_buf [0] ==2) //CCW lifter moving downwards
						{
							gGlobal.m_MOTORLifter.dir = 0x12;
						}
						else
						{
							gGlobal.m_CAN.lifter_ready = 0;
							printf("Error: there is a problem of parameters for lifter operation \r\n");	
						}
						CAN_tx_buf[0] = 0x55;
						CAN_tx_buf[1] = 0xAA;
						loadlength = 2;
						canardRequestOrRespond(&canard, RxTransfer.source_node_id,
																	UAVCAN_SERVICE_ID_LIFTER_SIGNATURE,
																	UAVCAN_SERVICE_ID_LIFTER,
																	&RxTransfer.transfer_id,
																	RxTransfer.priority,
																	CanardResponse,CAN_tx_buf, loadlength);	

						if (gGlobal.m_CAN.lifter_ready != 0)
						{
							/* prepare to config the lifter motor */
							LifterEnable_Config();
						}
					}
					else
					{
						CAN_tx_buf[0] = 0x57;
						CAN_tx_buf[1] = 0xAC;
						loadlength = 2;
						canardRequestOrRespond(&canard, RxTransfer.source_node_id,
																	UAVCAN_SERVICE_ID_LIFTER_SIGNATURE,
																	UAVCAN_SERVICE_ID_LIFTER,
																	&RxTransfer.transfer_id,
																	RxTransfer.priority,
																	CanardResponse,CAN_tx_buf, loadlength);							
					}	
				
				break;
				/* �����֧·���翪���л�*/
				case UAVCAN_SERVICE_ID_PWRSWITCH_LOCKER :
					if ( CAN_rx_buf[0] == 1) //��֧·
					{
						/* �յ��򿪹���ָ�� */
						LockerChannelEnable();	
						gGlobal.m_Status.LOCKER_OCPFLAG = 0;
						gGlobal.m_Status.LOCKER_Freeze = 0 ;
						gGlobal.m_Status.LOCKER_RECOVERY_CNT = 0 ;
					}
					else if (CAN_rx_buf[0] == 2) //�ر�֧·
					{
						/* code */
						LockerChannelDisable();
						gGlobal.m_Status.LOCKER_OCPFLAG = 0;
						gGlobal.m_Status.LOCKER_Freeze = 0 ;
						gGlobal.m_Status.LOCKER_RECOVERY_CNT = 0 ;
					}

					CAN_tx_buf[0] = 0x55;
					CAN_tx_buf[1] = 0xAA;
					loadlength = 2;
					canardRequestOrRespond(&canard, RxTransfer.source_node_id,
																UAVCAN_SERVICE_ID_PWRSWITCH_LOCKER_SIGNATURE,
																UAVCAN_SERVICE_ID_PWRSWITCH_LOCKER,
																&RxTransfer.transfer_id,
																RxTransfer.priority,
																CanardResponse,CAN_tx_buf, loadlength);							
					
				break;
			}
			
		}
//		else if (RxTransfer.transfer_type == CanardTransferTypeResponse)
//		{
//		}
		RX_FLAG = 0;
		memset(CAN_tx_buf,0,50);
	}
		
	for(const CanardCANFrame* txf = NULL; (txf = canardPeekTxQueue(&canard)) != NULL;)  //���ݷ���
  	{
		cantxframe.ExtId = txf->id;
		cantxframe.DLC = txf->data_len;
		cantxframe.IDE = CAN_ID_EXT;         // �趨��Ϣ��ʶ��������
		cantxframe.RTR = CAN_RTR_DATA;       // �趨��������Ϣ��֡����
		memcpy(cantxframe.Data,txf->data, txf->data_len); //��������
		const uint8_t TransmitMailbox = CAN_Transmit(CAN1,&cantxframe);

		if(TransmitMailbox != CAN_TxStatus_NoMailBox)   
	  	{
			canardPopTxQueue(&canard);        
		}
	}
}





uint8_t IAP_FRAM_CHECK(uint8_t Data[], uint8_t * const pResult)
{
	uint8_t i;
	uint8_t *pdata;
	uint8_t checksum=0;
	uint8_t len;
	pdata = pResult;
	
	if (Data[0] != 0x3A)
	{
		return 0; 
	}
	i=1;
	while (Data[i] != 0x0D)
	{
		 if(!is_xdigit(Data[i]))
        {
            return 0;
        }
		if(i%2)
		{
            *pdata =  ascii_to_xdigit(Data[i])<<4;
		}
		else
		{
			*pdata +=  ascii_to_xdigit(Data[i]); 
			 pdata++;
		}
		i++;
		if(i > 2*(*pResult+6))
		{
		    return 0;
		}
		
	}
    pdata = pResult;
	len = *pdata + 5;
    for (i=0; i < len - 1; i++)
    {
       checksum += *pdata;
       pdata++; 
    }
	checksum = ~checksum;
	checksum += 1;
    if (checksum == *pdata)
    {
        return 1;
    }  
    return 0;	
}


uint8_t is_xdigit(uint8_t data)
{
    if((data >= 0x30) && (data <= 0x39))
    {
        return 1;
    }
    
    if((data >= 0x41) && (data <= 0x46))
    {
       return 1;  
    } 
    return 0;  
    
}

uint8_t ascii_to_xdigit(uint8_t data)
{
    if((data >= 0x30) && (data <= 0x39))
    {
        return data - 0x30;
    }
    
    if((data >= 0x41) && (data <= 0x46))
    {
       return data - 0x37;  
    }
    return 0xff;   
}

uint8_t IAP_Handle(uint8_t data[])
{
	uint8_t err;
	switch(data[3])
	{
		case 0x00:
		    err = iap_flash_write(flash_hex_baseaddr,data);                     
		    break;
		case 0x01:         //�ļ���д����
			err = iap_flash_set_jumpaddr();
			break;
		case 0x02:         //���ƫ�Ƶ�ַ
			err = 0x00;
			break;
		case 0x03:
			err = 0x00;
			break;
		case 0x04:
			flash_hex_baseaddr = iap_flash_get_baseaddr(data);
		    if( flash_hex_baseaddr == 0)
			{
			    err = 0x02;
			}
			else
			{
				err = 0x00;
			}
			break;
		case 0x05:
			err = 0x00;
			break;
		default:
			err = 0x00;
	}
	return err;
}
/*******************************************************************************
*****************************************************************************/
uint8_t iap_flash_write(uint32_t baseaddr,uint8_t data[])
{
   uint32_t addr;
   uint8_t len;
   uint8_t *pdata;
   uint8_t err;
	uint8_t ErasePageStatus;
	uint8_t ErasePageCnt;
   addr = baseaddr + (data[1]<<8) + data[2];  //����д���ַ
   len = data[0];
   pdata = &data[4];
   if (baseaddr != 0x08000000 && baseaddr != 0x08010000 && baseaddr != 0x08020000 &&baseaddr != 0x08030000)
   {
	
		
			return 0x03;
		
   }
	 

		
   if ((addr%0x4000) == 0)
   {
	   u8 cnt_page;
		 u32 addrs;
		 addrs = addr;
		 FLASH_Unlock();
	   FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
	   for (cnt_page=0; cnt_page<16;cnt_page++){
		 ErasePageStatus = FLASH_ErasePage(addrs);
	   if(ErasePageStatus != FLASH_COMPLETE)
	   {
		   ErasePageCnt++;
		   if(ErasePageCnt >= 2)
		   {
			   err = 0x03;
			   return err;
		   }
		   else
		   {
				FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
				ErasePageStatus = FLASH_ErasePage(addrs);
		   }
	   }
		 addrs += 0x400;
		 
	 if(GPIO_ReadOutputDataBit(GPIOB,GPIO_Pin_9)==TRUE)
		GPIO_ResetBits(GPIOB,GPIO_Pin_9);
	 else
		GPIO_SetBits(GPIOB,GPIO_Pin_9);
	 }
	   FLASH_Lock();
   }
   
   FLASH_Unlock();
   while(len)
   {
		 
		if(GPIO_ReadOutputDataBit(GPIOB,GPIO_Pin_9)==TRUE)
			GPIO_ResetBits(GPIOB,GPIO_Pin_9);
		else
			GPIO_SetBits(GPIOB,GPIO_Pin_9);
		
		 if (len/4)
        {
			err = FLASH_ProgramWord(addr, *((uint32_t*)pdata));
            addr += 4;
            len -= 4; 
            pdata +=4;
        }
        else if (len/2)
        {
			err = FLASH_ProgramHalfWord(addr, *((uint16_t*)pdata));
            addr += 2;
            len -=2;
            pdata += 2;
        }
        else
        {
			err = FLASH_ProgramOptionByteData(addr, *pdata);
            addr += 1;
            len -= 1;
            pdata += 1;
        }
		if (err != FLASH_COMPLETE)
		{
			break;
		}		 
   }
   FLASH_Lock();
   if(err != FLASH_COMPLETE)
   {
	   return 0x03;
   }
   return 0x00;
}

uint32_t iap_flash_get_baseaddr(uint8_t data[])
{
	uint32_t addr;
	addr = (data[4]<<8) + data[5];
	if ((addr == 0x0800) || (addr == 0x0801)|| (addr == 0x0802)|| (addr == 0x0803))
	{
		return addr<<16;
	}
	return 0;
	
}

uint8_t iap_flash_set_jumpaddr(void)
{
	uint8_t err = 0x03;
	if ((uint32_t)&gGlobal.m_DeviceInfo >= FUNCTION_B_ADDRESS)
	{
		FLASH_Unlock();
	    FLASH_ErasePage(FUNCTION_JUMP_ADDRESS);
	    FLASH_Lock();
		FLASH_Unlock();
		FLASH_ProgramWord(FUNCTION_JUMP_ADDRESS, FUNCTION_A_ADDRESS);
		FLASH_Lock();
		
		if ( *(uint32_t *)FUNCTION_JUMP_ADDRESS == FUNCTION_A_ADDRESS)
		{
			err = 0x00;
		}
		
	}
	else if ( (uint32_t)&gGlobal.m_DeviceInfo >= FUNCTION_A_ADDRESS)
	{
		FLASH_Unlock();
	    FLASH_ErasePage(FUNCTION_JUMP_ADDRESS);
	    FLASH_Lock();
	    FLASH_Unlock();
		FLASH_ProgramWord(FUNCTION_JUMP_ADDRESS, FUNCTION_B_ADDRESS);
		FLASH_Lock();
		
		if ( *(uint32_t *)FUNCTION_JUMP_ADDRESS == FUNCTION_B_ADDRESS)
		{
			err = 0x00;
		}
	}
	return err;
  
}

//uint8_t FLASH_EraseAllNextZONE(uint32_t Page_Address,u16 size)
//{
//	u16 i;
//	uint32_t newerasepage;
//	uint8_t ErasePageStatus;
//	
//	newerasepage = Page_Address;
//	i = size/1024;
//	while((i--)&& (ErasePageStatus == FLASH_COMPLETE))
//	{
//		FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
//		ErasePageStatus = FLASH_ErasePage(newerasepage);
//		newerasepage = newerasepage + 0x400;
//	}
//	return ErasePageStatus;
//}


/*******************************************************************************
* Function Name  : USB_LP_CAN_RX0_IRQHandler
* Description    : This function handles USB Low Priority or CAN RX0 interrupts 
*                  requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
CanRxMsg RxMessage;
void USB_LP_CAN1_RX0_IRQHandler(void)
{
  //CanRxMsg RxMessage;
  CanardCANFrame rx_frame;
  CAN_Receive(CAN1,CAN_FIFO0,&RxMessage);//��ȡ����	
  rx_frame.id = RxMessage.ExtId;
  rx_frame.data_len = RxMessage.DLC;

  memcpy(rx_frame.data,RxMessage.Data,RxMessage.DLC); //��������	 
//	memcpy(CAN_rx_buf,rx_frame.data,10); //��������	
// 	if ((rx_frame.id&0x00FFFFFF) == 0x500210)
//	{
//		CAN_rx_buf[148] = rx_frame.data[2]& 0x0F;
//		CAN_rx_buf[149] = (rx_frame.data[2]>>4)& 0x0F;
//	} 
	canardHandleRxFrame(&canard, &rx_frame, 1000);
}
/******************************************************************************/

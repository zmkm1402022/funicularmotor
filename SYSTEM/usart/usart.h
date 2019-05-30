#ifndef __USART_H
#define __USART_H
#include "header.h"

typedef  void (*pFunction)(void);


/* Define the address from where user application will be loaded.
   Note: the 1st sector 0x08000000-0x08003FFF is reserved for the IAP code */
#define BOOTCROCESS_ADDRESS         (uint32_t)BOOT_ADDRESS 
#define BOOTCROCESS_LAST_ADDRESS    (uint32_t)BOOT_LAST_ADDRESS 
#define APPLICATION_ADDRESS         (uint32_t)PROCESS_ADDRESS 
#define APPLICATION_LAST_ADDRESS    (uint32_t)PROCESS_LAST_ADDRESS
#define FLASH_PAGE_SIZE        0x400 //1K


#ifdef EN_USART1
void uart_init(u32 bound);
#else
void AuxiliaryPorts_Init(void);
void NJST_Flipping(void);
#endif

#endif



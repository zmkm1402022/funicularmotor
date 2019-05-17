#ifndef __USART_H
#define __USART_H
#include "header.h"

#ifdef EN_USART1
void uart_init(u32 bound);
#else
void AuxiliaryPorts_Init(void);
void NJST_Flipping(void);
#endif

#endif



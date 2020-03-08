
// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef USART_HEADER_FILE_H
#define	USART_HEADER_FILE_H

#include <xc.h>


void USART_Init(long);
void USART_TransmitChar(char);
void USART_SendString(const char *);
void MSdelay(unsigned int val);
char USART_ReceiveChar();

#define F_CPU 8000000/64  //#define Baud_value(baud_rate) (((float)(F_CPU)/(float)baud_rate)-1)
#define Baud_value (((float)(F_CPU)/(float)baud_rate)-1)

#endif	/* USART_HEADER_FILE_H */


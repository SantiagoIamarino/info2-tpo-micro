/*
 * UART0.cpp
 *
 *  Created on: 12 ago. 2025
 *      Author: gusta
 */

#include "Defines.h"

extern UART0	Uart0;
extern PC_CON   PC_CONNECTION;

/*****************************************************************************
** Function name:		UART0_IRQHandler
**
** Descriptions:		UART interrupt handler
**
** parameters:			None
** Returned value:		None
**
*****************************************************************************/
void UART0_IRQHandler (void) // se ejecuta en cada interrupcion
{

	const uint32_t	Int = USART0->STAT;


	if(Int & (1 << 0)) // termino de leer un byte
	{
		//RX
		Uart0.PushRx((uint8_t)USART0->RXDAT);
	}

	if(Int & (1 << 2)) // termino de escribir un byte
	{
		//TX
		int32_t Temporal = Uart0.PopTx();
		if(Temporal >= 0)
			USART0->TXDAT = (uint8_t)Temporal;
		else
			USART0->INTENCLR = (1 << 2); //disable int TX

	}
}


UART0::UART0(uint32_t baudrate)
{
	SYSCON->SYSAHBCLKCTRL0 |= (1u<<7)   /* SWM   */
	                            | (1u<<18)  /* IOCON */
	                            | (1u<<14); /* USART0 */

	SWM0->PINASSIGN.PINASSIGN0 = (8 << 0) | (9 << 8);			// TX0 = P0.8	RX0 = P0.9

	USART0->CFG = 	(0)				// 0=DISABLE 1=ENABLE
					| (1 << 2)		// 0=7BITS 1=8BITS 2=9BITS
					| (0 << 4)		// 0=NOPARITY 2=PAR 3=IMPAR
					| (0 << 6)		// 0=1BITSTOP 1=2BITSTOP
					| (0 << 9)		// 0=NOFLOWCONTROL 1=FLOWCONTROL
					| (0 << 11)		// 0=ASINCRONICA 1=SINCRONICA
					| (0 << 14);	// 0=SLAVE 1=MASTER


	USART0->CTL = 0;

	USART0->INTENSET = (1 << 0);	//RX Y TX INTERRUPT


	UART0CLKSEL = 1;

	USART0->BRG = (FREQ_PRINCIPAL / baudrate) / 16;

	NVIC->ISER[0] = (1 << 3); /* enable interrupt */

	USART0->CFG |= (1);			// ENABLE USART0
}

UART0::~UART0() {
	// TODO Auto-generated destructor stub
}

void UART0::Send(uint8_t *Datos, uint32_t Tamanio)
{
	uint32_t i;

	if(0 == Tamanio)
		Tamanio = CADENAS_Strlen(Datos);

	for(i = 0 ; i < Tamanio ; i++)
		PushTx(Datos[i]);

	return;
}

//***************************************************************************************************************************
//	Sirve para calcular el tamaño de una cadena de caracteres ascii
//***************************************************************************************************************************
uint32_t UART0::CADENAS_Strlen(uint8_t *datos)
{
	uint32_t	i;

	for(i = 0 ; i < 0xffff ; i++)
	{
		if(!datos[i])
			return i;
	}

	return i;
}



void UART0::PushTx(uint8_t dato)
{
	FIFO.TX.Buffer[FIFO.TX.Indice_in] = dato;

	FIFO.TX.Indice_in ++;
	FIFO.TX.Indice_in %= UART0_TAMANIO_COLA_TX;

	if ( !(USART0->INTENSET & (1 << 2)) )
	{
		USART0->TXDAT = (uint8_t)PopTx();

		USART0->INTENSET = (1 << 2); // int tx
	}
}

int32_t UART0::PopTx( void )
{
	int32_t dato = -1;

	if ( FIFO.TX.Indice_in != FIFO.TX.Indice_out )
	{
		dato = (int32_t) FIFO.TX.Buffer[FIFO.TX.Indice_out];
//		UART0.TX.Buffer[UART0.TX.Indice_out] = 0;

		FIFO.TX.Indice_out ++;
		FIFO.TX.Indice_out %= UART0_TAMANIO_COLA_TX;
	}
	return dato;
}

void UART0::PushRx(uint8_t dato)
{
	FIFO.RX.Buffer[FIFO.RX.Indice_in] = dato;

	FIFO.RX.Indice_in ++;
	FIFO.RX.Indice_in %= UART0_TAMANIO_COLA_RX; // reinicia a 0 si alcanza UART0_TAMANIO_COLA_RX
	FIFO.RX.Esta_Leyendo = true;
}

bool UART0::Esta_Leyendo(void) {
	return FIFO.RX.Esta_Leyendo;
}

int32_t UART0::PopRx( void )
{
	int32_t dato = -1;

	if ( FIFO.RX.Indice_in != FIFO.RX.Indice_out )
	{
		dato = (int32_t) FIFO.RX.Buffer[FIFO.RX.Indice_out];
		FIFO.RX.Indice_out ++;
		FIFO.RX.Indice_out %= UART0_TAMANIO_COLA_RX; // reinicia a 0 si alcanza UART0_TAMANIO_COLA_RX
	} else {
		FIFO.RX.Esta_Leyendo = false;
	}

	return dato;
}

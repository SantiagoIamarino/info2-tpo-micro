/*
 * UART0.h
 *
 *  Created on: 12 ago. 2025
 *      Author: Gustavo Fresno
 */

#ifndef UART_DRIVER_UART0_H_
#define UART_DRIVER_UART0_H_

#define		UART0_TAMANIO_COLA_RX					300
#define		UART0_TAMANIO_COLA_TX					300

#define UART0CLKSEL                               *(( uint32_t  * )0x40048090u)
#define UART1CLKSEL                               *(( uint32_t  * )0x40048094u)
#define UART2CLKSEL                               *(( uint32_t  * )0x40048098u)
#define UART3CLKSEL                               *(( uint32_t  * )0x4004809Cu)
#define UART4CLKSEL                               *(( uint32_t  * )0x400480A0u)

typedef struct
{
	uint8_t						Esta_Leyendo;
	alignas(4) uint8_t 			Buffer[UART0_TAMANIO_COLA_RX]{};
	uint16_t					Indice_in, Indice_out;
}STRUCT_RX0;

typedef struct
{
	uint8_t						Esta_Escribiendo;
	alignas(4) uint8_t 			Buffer[UART0_TAMANIO_COLA_TX]{};
	uint16_t					Indice_in, Indice_out;
}STRUCT_TX0;

typedef struct
{
	STRUCT_RX0					RX;
	STRUCT_TX0					TX;
}UART0_Struct;

class UART0 {
private:

public:
	UART0_Struct	FIFO;
	UART0(uint32_t baudrate);
	virtual ~UART0();

	void Send(uint8_t *Datos, uint32_t Tamanio);
	void PushTx(uint8_t Dato);
	int32_t PopTx( void );
	void PushRx(uint8_t Dato);
	int32_t PopRx( void );
	bool Esta_Leyendo(void);

	uint32_t CADENAS_Strlen(uint8_t *datos);
};

extern "C" {
void UART0_IRQHandler (void);
}

#endif /* UART_DRIVER_UART0_H_ */


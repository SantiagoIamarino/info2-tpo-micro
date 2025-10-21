/*
 * PCCON.h
 *
 *  Created on: Oct 17, 2025
 *      Author: santiagoiamarino
 */

#ifndef DRIVERS_PC_CON_DRIVER_PCCON_H_
#define DRIVERS_PC_CON_DRIVER_PCCON_H_

#include <cstdio>
#include "tipos.h"
#include "LPC845.h"
#include "Hardware.h"
#include "GPIO.h"
#include "SYSTICK.h"
#include "CALLBACK.h"
#include "TIMER.h"
#include "UART0.h"
#include "GRAL.h"

struct SuenioCFG {
    uint8_t horas_suenio;
    bool alarma_on;
    bool luz_on;
};

class PC_CON {
public:
	using CallbackFn = void (*)(PC_CON* self);

	PC_CON();
	void init();
	virtual ~PC_CON();

	bool initiated = false;
	bool Leer_Resp(uint8_t* resp_a_buscar, bool debug);
	bool Leer_Resp_Con_Reintentos(uint8_t* resp_a_buscar, uint32_t max_intentos, bool debug);
	void Enviar_Comando(uint8_t* cmd);
	bool Obtener_Respuesta(uint8_t** buf, uint8_t resp_length);

	bool cfg_obtenida = false;
	bool ack_recibido = false;

	bool Obtener_Configuracion(SuenioCFG* cfg);
};

#endif /* DRIVERS_PC_CON_DRIVER_PCCON_H_ */

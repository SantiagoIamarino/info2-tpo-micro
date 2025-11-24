#include <cstdio>
#include "tipos.h"
#include "GPIO.h"
#include "UART0.h"
#include "GRAL.h"
#include "CALLBACK.h"
#include "TIMER.h"
#include "PCCON.h"
#include "cfg_maq_estados.h"
#include "actuadores.h"
#include "caidas.h"

extern UART0 Uart0;
extern PC_CON PC_CONNECTION;
extern MPU MPU_ACC;
extern MAX MAX_SENSOR;

bool caida_ok_detected = false;

void analizar_tramas(SuenioCFG* cfg){
	if(!PC_CONNECTION.Ready()) return; // en caso de que alguna comunicacion con la PC se este procesando

	int32_t b = Uart0.PopRx();

	if(PC_CONNECTION.Esperando_Resp_Caida()){
		resp_caida_maq_estados(b, &caida_ok_detected);
		if (caida_ok_detected) { // el usuario clickeo "Todo bien", parar actuadores y volver al estado inicial
			Uart0.Send((uint8_t*)"<ACK_CAIDA_OK>", 0);
			pararActuadores();

			caida_ok_detected = false;
			PC_CONNECTION.Caida_Todo_OK();
		}
	}

	cfg_maq_estados(b, cfg, true);
}

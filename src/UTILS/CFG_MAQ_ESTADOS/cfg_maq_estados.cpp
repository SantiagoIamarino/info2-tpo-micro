/*
 * cfg_maq_estados.cpp
 *
 *  Created on: Nov 8, 2025
 *      Author: santiagoiamarino
 */


#include "cfg_maq_estados.h"

// ---- helpers de impresión ----
static void uart_print_str(const char* s) {
    Uart0.Send((uint8_t*)s, 0);   // usa CADENAS_Strlen
}

static void uart_print_u8_2(uint8_t v) {   // 2 dígitos con cero a la izquierda
    Uart0.PushTx((uint8_t)('0' + (v/10)));
    Uart0.PushTx((uint8_t)('0' + (v%10)));
}

// ---- logger de la config ----
static void Log_SuenioCFG(const SuenioCFG* c) {
	uart_print_str("PF_ID=");
	uart_print_u8_2(c->profile_id);
    uart_print_str("; HS=");
    uart_print_u8_2(c->horas_suenio);
    uart_print_str("; ALARMA=");
    uart_print_str(c->alarma_on ? "TRUE" : "FALSE");
    uart_print_str("; LUZ=");
    uart_print_str(c->luz_on ? "TRUE" : "FALSE");
    uart_print_str("; HORA_LIMITE_SEG=");

    char buf[32];
	// convertir uint32_t a string
	int n = sprintf(buf, "%lu", (unsigned long)c->hora_limite_seg);
	Uart0.Send((uint8_t*)buf, n);

}

extern MAX MAX_SENSOR;
extern MPU MPU_ACC;
extern PC_CON PC_CONNECTION;
extern UART0 Uart0;

auto isDigit = [](int32_t b){ return (b >= '0' && b <= '9'); };

enum State : uint8_t {
		S_ESPERO_TRAMA = 0,
		S_CHECK_COMANDO,   // "CFG:PF_ID="  ó  "CFG_UPDATE:PF_ID="
		S_PF_ID,           // dos dígitos de PF_ID
		S_K_HORAS,         // ";HORAS_SUENIO="
		S_HORAS_2DIG,      // dos dígitos de horas
		S_K_ALARMA,        // ";ALARMA_ON="
		S_ALARMA_VAL,      // "0TRUE" | "FALSE"
		S_K_LUZ,           // ";LUZ_ON="
		S_LUZ_VAL,         // "TRUE" | "FALSE"
		S_K_HORA_LIM,	   // ;HORA_LIMITE=
		S_K_HORA_LIM_VAL,  // HH:MM:SS
		S_K_CHECKSUM,	   // ";CS="
		S_SUM_CHECK,       // checkear valor en hexa, suma de toda la trama menos <>
		S_WAIT_FINAL,      // '>'
		S_DONE,
		S_ERROR
};

const char* HEADER;
const char* HEADER_UPDT = "CFG_UPDATE:PF_ID=";
const char* K_HRS  		= ";HORAS_SUENIO=";
const char* K_ALR  		= ";ALARMA_ON=";
const char* K_LUZ  		= ";LUZ_ON=";
const char* K_HORA_LIM  = ";HORA_LIMITE=";
const char* K_CHKSUM  	= ";CS=";

State st = S_ESPERO_TRAMA;
bool cfg_msg_done  = false;
bool cfg_msg_error = false;

uint32_t idle_counter = 0;
static const uint32_t CFG_IDLE_MAX = 50000;

uint8_t index = 0;
uint8_t profile_id[2];
uint8_t horas[2];
uint8_t alarma_on[6];
uint8_t luz_on[6];
uint8_t hora_limite[9];
uint8_t cs_val[2];
uint8_t cs_calc = 0;
bool sensors_paused = false;

uint8_t n_dec;
uint8_t n_uni;


void cfg_maq_estados( int32_t b, SuenioCFG* cfg, bool es_update){
	// Ejemplos validos:
	// <CFG:PF_ID=01;HORAS_SUENIO=08;ALARMA_ON=TRUE;LUZ_ON=FALSE;HORA_LIMITE=HH:MM:SS;CS=NN>
	// <CFG_UPDATE:PF_ID=01;HORAS_SUENIO=08;ALARMA_ON=TRUE;LUZ_ON=FALSE;HORA_LIMITE=HH:MM:SS;CS=NN>

	if(b < 0) {
		if (st != S_ESPERO_TRAMA) {
			// Esta en medio de una trama, cuento inactividad
			idle_counter++;
			if (idle_counter > CFG_IDLE_MAX) {
				st = S_ESPERO_TRAMA;
				index = 0;
				idle_counter = 0;

				if (sensors_paused) {
					MAX_SENSOR.resume();
					MPU_ACC.resume();
					sensors_paused = false;
				}
			}
		} else {
			idle_counter = 0;
		}
		return;
	}

	if(st != S_ESPERO_TRAMA && st != S_WAIT_FINAL && st != S_DONE && st != S_ERROR && st != S_K_CHECKSUM && st != S_SUM_CHECK){
		// sumo para el checksum siempre que no sea principio, final, error, done o checksum
		cs_calc += (uint8_t)b;
	}


	switch (st) {

		case S_ESPERO_TRAMA: // <
			if(sensors_paused){ MAX_SENSOR.resume(); MPU_ACC.resume(); sensors_paused = false; }

			if (b == '<') { st = S_CHECK_COMANDO; index = 0; cs_calc = 0; }
			break;

		case S_CHECK_COMANDO: // "CFG:PF_ID="  ó  "CFG_UPDATE:PF_ID="
			if(!sensors_paused){ MAX_SENSOR.pause(); MPU_ACC.pause(); sensors_paused = true; }

			HEADER = es_update ? (char*)"CFG_UPDATE:PF_ID=" : (char*)"CFG:PF_ID=";

			if (b == HEADER[index]) {
				index++;
				if (HEADER[index] == '\0') { st = S_PF_ID; index = 0; }
			} else {
				// si se rompe el header, vuelvo a esperar '<'
				st = (b == '<') ? S_CHECK_COMANDO : S_ESPERO_TRAMA;
				index = 0;
			}
			break;

		case S_PF_ID:
			if (!isDigit(b)) { st = S_ESPERO_TRAMA; index = 0; break; }

			profile_id[index] = b;
			index++;

			if(index == 2) { st = S_K_HORAS; index = 0; }

			break;

		case S_K_HORAS:
			if (b == K_HRS[index]) {
				index++;
				if (K_HRS[index] == '\0') { st = S_HORAS_2DIG; index = 0; }
			} else {
				st = (b == '<') ? S_CHECK_COMANDO : S_ESPERO_TRAMA;
				index = 0;
			}
			break;

		case S_HORAS_2DIG: // ";HORAS_SUENIO="
			if (!isDigit(b)) { st = S_ESPERO_TRAMA; index = 0; break; }

			horas[index] = b;
			index++;

			if(index == 2) { st = S_K_ALARMA; index = 0; }

			break;

		case S_K_ALARMA: // ";ALARMA_ON="
			if (b == K_ALR[index]) {
				index++;
				if (K_ALR[index] == '\0') { st = S_ALARMA_VAL; index = 0; }
			} else {
				st = (b == '<') ? S_CHECK_COMANDO : S_ESPERO_TRAMA;
				index = 0;
			}
			break;

		case S_ALARMA_VAL: // "0TRUE" | "FALSE"
			alarma_on[index] = b;
			index++;

			if(index == 5) {
				alarma_on[5] = '\0';
				st = S_K_LUZ; index = 0;
			}
			break;

		case S_K_LUZ: // ";LUZ_ON="
			if (b == K_LUZ[index]) {
				index++;
				if (K_LUZ[index] == '\0') { st = S_LUZ_VAL; index = 0; }
			} else {
				st = (b == '<') ? S_CHECK_COMANDO : S_ESPERO_TRAMA;
				index = 0;
			}
			break;

		case S_LUZ_VAL: // "0TRUE" | "FALSE"
			luz_on[index] = b;
			index++;

			if(index == 5) {
				luz_on[5] = '\0';
				st = S_K_HORA_LIM; index = 0;
			}
			break;

		case S_K_HORA_LIM: // ";HORA_LIMITE=
			if (b == K_HORA_LIM[index]) {
				index++;
				if (K_HORA_LIM[index] == '\0') { st = S_K_HORA_LIM_VAL; index = 0; }
			} else {
				st = (b == '<') ? S_CHECK_COMANDO : S_ESPERO_TRAMA;
				index = 0;
			}
			break;

		case S_K_HORA_LIM_VAL: // HH:MM:SS
			hora_limite[index] = b;
			index++;

			if(index == 8) {
				hora_limite[8] = '\0';
				st = S_K_CHECKSUM; index = 0;
			}
			break;

		case S_K_CHECKSUM:
			if (b == K_CHKSUM[index]) {
				index++;
				if (K_CHKSUM[index] == '\0') { st = S_SUM_CHECK; index = 0; }
			} else {
				st = (b == '<') ? S_CHECK_COMANDO : S_ESPERO_TRAMA;
				index = 0;
			}
			break;

		case S_SUM_CHECK:
			cs_val[index] = (uint8_t)b;
			index++;

			if (index == 2) {
				index = 0;
				if(hex_a_dec(cs_val) == cs_calc) { // checksum correcto
					st = S_WAIT_FINAL;
				} else {
					st = S_ERROR;
				}
			}
			break;

		case S_WAIT_FINAL:
			if (b == '>') { st = S_DONE; }
			else { st = S_ERROR; }
			break;

		case S_DONE: {
			Uart0.Send((uint8_t*)"<CFG_ACK>", 0 );

			st = S_ESPERO_TRAMA;
			index = 0;

			//Profile ID
			n_dec = profile_id[0] - '0';
			n_uni = profile_id[1] - '0';
			cfg->profile_id = n_dec * 10 + n_uni;

			// Horas Sueño
			n_dec = horas[0] - '0';
			n_uni = horas[1] - '0';
			cfg->horas_suenio =  n_dec * 10 + n_uni;

			// Alarma
			if(STR_Comparar((uint8_t*)alarma_on, (uint8_t*)"0TRUE")) {
				cfg->alarma_on = true;
			} else if(STR_Comparar((uint8_t*)alarma_on, (uint8_t*)"FALSE")) {
				cfg->alarma_on = false;
			} else { st = S_ERROR; }

			// Luz
			if(STR_Comparar((uint8_t*)luz_on, (uint8_t*)"0TRUE")) {
				cfg->luz_on = true;
			} else if(STR_Comparar((uint8_t*)luz_on, (uint8_t*)"FALSE")) {
				cfg->luz_on = false;
			} else { st = S_ERROR; }

			// HORA_LIMITE convertir HH:MM:SS (restantes) a segundos, se ira restando con un timer
			int HH = (hora_limite[0] - '0') * 10 + (hora_limite[1] - '0');
			int MM = (hora_limite[3] - '0') * 10 + (hora_limite[4] - '0');
			int SS = (hora_limite[6] - '0') * 10 + (hora_limite[7] - '0');

			cfg->hora_limite_seg = (uint32_t)(HH * 3600 + MM * 60 + SS);

			Log_SuenioCFG(cfg);

			if(sensors_paused){ MAX_SENSOR.resume(); MPU_ACC.resume(); sensors_paused = false; }
			cfg_msg_done  = true;
			cfg_msg_error = false;

			break;
		}
		case S_ERROR:
			st = S_ESPERO_TRAMA;
			index = 0;
			cs_calc = 0;
			if (sensors_paused) {
				MAX_SENSOR.resume();
				MPU_ACC.resume();
				sensors_paused = false;
			}
		    cfg_msg_error = true;
		    cfg_msg_done  = false;
			break;

		default: break;
	}
}




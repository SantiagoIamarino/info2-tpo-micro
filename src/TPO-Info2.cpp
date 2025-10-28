#include "Defines.h"

Gpio LED_VERDE(PIN_LED_VERDE, OUTPUT);
MAX MAX_SENSOR;
MPU MPU_ACC;
PC_CON PC_CONNECTION;

SuenioCFG suenio_config;


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
    uart_print_str("\r\n");
}

int main(void)
{

	log_debug((uint8_t*)"Initializing...\r\n", 0);

	PC_CONNECTION.init();
	// obtiene configuracion desde la PC (horas_suenio, alarma_on, luz_on)
	if(!PC_CONNECTION.Obtener_Configuracion(&suenio_config)) {
		suenio_config = { 8, true, true }; // uso config default si falla
	}

	// log suenio_config
	Log_SuenioCFG(&suenio_config);

	MAX_SENSOR.init();
	MPU_ACC.init();

    while (1) {
    	suenio_maq_estados(&suenio_config);
    }
}

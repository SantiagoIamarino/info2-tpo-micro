#include "Defines.h"
#include "tramas.h"

MAX MAX_SENSOR;
MPU MPU_ACC;
PC_CON PC_CONNECTION;

SuenioCFG suenio_config;

GPIOF Pulsador(PIN_PULSADOR);
GPIOPWM Ledd(0, 22);
Gpio Buzzerr(0, 19, OUTPUT);

int main(void) {

	log_debug((uint8_t*) "Initializing...\r\n", 0);

	PC_CONNECTION.init();
	// obtiene configuracion desde la PC (horas_suenio, alarma_on, luz_on)
	if (!PC_CONNECTION.Obtener_Configuracion(&suenio_config)) {
		suenio_config = { 8, true, true }; // uso config default si falla
	}

	MAX_SENSOR.init();
	MPU_ACC.init();

	while (1) {
		suenio_maq_estados(&suenio_config);
		analizar_tramas(&suenio_config);
	}
}

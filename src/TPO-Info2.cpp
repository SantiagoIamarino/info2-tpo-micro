#include "Defines.h"

Gpio LED_VERDE(PIN_LED_VERDE, OUTPUT);
MAX MAX_SENSOR;
MPU MPU_ACC;
PC_CON PC_CONNECTION;

SuenioCFG suenio_config;


int main(void)
{

	log_debug((uint8_t*)"Initializing...\r\n", 0);

	MAX_SENSOR.init();
	MPU_ACC.init();
	PC_CONNECTION.init();

	// obtengo configuracion desde la PC (horas_suenio, alarma_on, luz_on)
	if(!PC_CONNECTION.Obtener_Configuracion(&suenio_config)) {
		suenio_config = { 8, true, true }; // uso config default si falla
	}

    while (1) {

    }
}

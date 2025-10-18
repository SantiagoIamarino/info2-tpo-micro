#include "Defines.h"

Gpio LED_VERDE(PIN_LED_VERDE, OUTPUT);
MAX MAX_SENSOR;
MPU MPU_ACC;
PC_CON PC_CONNECTION;



int main(void)
{

	log_debug((uint8_t*)"Initializing...\r\n", 0);

	MAX_SENSOR.init();
	MPU_ACC.init();
	PC_CONNECTION.init();

	// obtengo configuracion desde la PC (horas_suenio, alarma_on, luz_on)


    while (1) {

    }
}

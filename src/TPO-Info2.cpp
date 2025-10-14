#include "Defines.h"

UART0	Uart0(115200);
void tick();
TIMER t(10,3000,tick);

Gpio LED_VERDE(PIN_LED_VERDE, OUTPUT);
MAX MAX_SENSOR;
MPU MPU_ACC;


int main(void)
{

	log_debug((uint8_t*)"Initializing...\r\n", 0);

	//MAX_SENSOR.init();
	MPU_ACC.init();

    while (1) {



    }
}

void tick() {

	static uint8_t led=0;
	led^=1; LED_VERDE.Set(led);

	if(MPU_ACC.initiated) {
		MPU_ACC.read();
	}


}

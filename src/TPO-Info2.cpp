#include "Defines.h"

UART0	Uart0(9600);
void tick();
TIMER t(10,1000,tick);

Gpio LED_VERDE(PIN_LED_VERDE, OUTPUT);
MAX MAX_SENSOR;
MPU MPU_ACC;


int main(void)
{

	log_debug((uint8_t*)"Initializing...\r\n", 0);

    while (1) {

    }
}

void tick() {

	static uint8_t led=0;
	led^=1; LED_VERDE.Set(led);

	if(MAX_SENSOR.initiated) {
		log_debug((uint8_t*)"READING MAX...\r\n", 0);
		uint32_t red, ir;
		uint8_t avail=0;
		if (MAX_SENSOR.read(&red, &ir, &avail)) {
			char line[64];
			int n = snprintf(line, sizeof(line), "RED=%lu IR=%lu (avail=%u)\r\n",
							 (unsigned long)red, (unsigned long)ir, avail);
			if (n>0) log_debug((uint8_t*)line, n);
		}
	    else {
				log_debug((uint8_t*)"MAX read error\r\n", 0);
		}
	}

	if(MPU_ACC.initiated) {
		MPU_ACC.read();
	}


}

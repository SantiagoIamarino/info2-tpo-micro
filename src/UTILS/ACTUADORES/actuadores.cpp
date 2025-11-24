#include "actuadores.h"

extern MAX MAX_SENSOR;
extern MPU MPU_ACC;

GPIOPWM Led(0, 20);
Gpio Buzzer(0, 22, OUTPUT);
TIMER t_actuadores(10, 1, tick_actuadores);

static uint8_t pwm_luz_level = 1;
static uint16_t luz_fade_count = 0;
static uint16_t step_ms = 0;
static const uint32_t LUZ_FADE_DURATION_MS = 20000;


static uint32_t bz_counter = 0;
static uint8_t  bz_state = 0;

// Duraciones configurables
static const uint16_t BEEP_MS       = 100;   // duración beep
static const uint16_t BETWEEN_MS    = 50;   // silencio entre beep1 y beep2
static const uint16_t PAUSE_MS      = 800;   // silencio largo final


bool checkConfig = false;
bool actuadores_on = false;

extern SuenioCFG* suenio_cfg;

void luz_actuador(){
	luz_fade_count++;

	if (luz_fade_count >= step_ms) {
		Led.SetPWM(pwm_luz_level);
		luz_fade_count = 0;

		if (pwm_luz_level < 100) {
			pwm_luz_level++;
			Led.SetPWM(pwm_luz_level);
		}
	}
}

void alarma_actuador(){
	bz_counter++;

	switch (bz_state) {

		case 0: // ---- BEEP 1 ----
			Buzzer.Set(1);
			if (bz_counter >= BEEP_MS) {
				bz_counter = 0;
				bz_state = 1;
			}
			break;

		case 1: // ---- silencio entre beep1 y beep2 ----
			Buzzer.Set(0);
			if (bz_counter >= BETWEEN_MS) {
				bz_counter = 0;
				bz_state = 2;
			}
			break;

		case 2: // ---- BEEP 2 ----
			Buzzer.Set(1);
			if (bz_counter >= BEEP_MS) {
				bz_counter = 0;
				bz_state = 3;
			}
			break;

		case 3: // ---- silencio largo antes de repetir ----
			Buzzer.Set(0);
			if (bz_counter >= PAUSE_MS) {
				bz_counter = 0;
				bz_state = 0;   // volver a beep 1
			}
			break;
	}
}

void activarActuadores(bool _checkConfig){
	checkConfig = _checkConfig;
	step_ms = LUZ_FADE_DURATION_MS / 99;

	MPU_ACC.pause();
	MAX_SENSOR.pause();
	actuadores_on = true;
}

void pararActuadores(){
	bz_counter = 0;
	bz_state = 0;
	pwm_luz_level = 1;
	luz_fade_count = 0;
	step_ms = 0;

	Buzzer.Set(0);
	Led.SetPWM(0);

	actuadores_on = false;

	MPU_ACC.resume();
	MAX_SENSOR.resume();

}

void tick_actuadores(){
	if(!actuadores_on) return;

	if(!checkConfig || suenio_cfg->luz_on){
		luz_actuador();
	}

	if(!checkConfig || suenio_cfg->alarma_on){
		alarma_actuador();
	}
}

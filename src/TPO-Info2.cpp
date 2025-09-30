#include "Defines.h"

UART0	Uart0(9600);
void tick();
TIMER t(10,2500,tick);

Gpio LED_VERDE(PIN_LED_VERDE, OUTPUT);
//I2C I2C_ACC(10, 11, 100);
I2C I2C_ACC(10, 11, 100);

// Dirección I2C (7-bit)
static const uint8_t MAX_ADDR         = 0x57;

// Registros clave
#define MAX_ADDR          0x57
#define REG_INTR_STATUS_1 0x00
#define REG_INTR_ENABLE_1 0x02
#define REG_FIFO_WR_PTR   0x04
#define REG_OVF_COUNTER   0x05
#define REG_FIFO_RD_PTR   0x06
#define REG_FIFO_DATA     0x07
#define REG_FIFO_CONFIG   0x08
#define REG_MODE_CONFIG   0x09
#define REG_SPO2_CONFIG   0x0A
#define REG_LED1_RED      0x0C
#define REG_LED2_IR       0x0D
#define REG_LED3_IR       0x0D
#define REG_MULTI_LED_1   0x11   // SLOT1
#define REG_MULTI_LED_2   0x12   // SLOT2
#define REG_PART_ID       0xFF

static const uint8_t MPU_ADDR = 0x68;
static const uint8_t REG_PWR_MGMT_1 = 0x6B;
static const uint8_t REG_WHO_AM_I   = 0x75;
static const uint8_t REG_ACCEL_XOUT_H = 0x3B;

bool mpu_initiated = false;
bool max_initiated = true;

static void maxDumpRegs(void);

void delay(){
	for (volatile int i=0; i<30000; ++i) __NOP();
}

void log_debug(uint8_t* text, int length = 0) {
	Uart0.Send((uint8_t*)text, length);
}

static char* i16toa(int16_t v, char* p) {
    char tmp[6]; int i = 0; bool neg = (v < 0);
    if (neg) v = -v;
    do { tmp[i++] = '0' + (v % 10); v /= 10; } while (v);
    if (neg) *p++ = '-';
    while (i--) *p++ = tmp[i];
    *p = 0;
    return p;
}

static void log_acc(int16_t ax, int16_t ay, int16_t az) {
    char buf[40], *p = buf;
    *p++='A'; *p++='C'; *p++='C'; *p++=' '; *p++='X'; *p++='=';
    p = i16toa(ax, p);
    *p++=' '; *p++='Y'; *p++='='; p = i16toa(ay, p);
    *p++=' '; *p++='Z'; *p++='='; p = i16toa(az, p);
    *p++='\r'; *p++='\n';
    log_debug((uint8_t*)buf, (uint32_t)(p - buf));
}

bool mpuInit()
{

	log_debug((uint8_t*)"Initiating MPU\r\n", 0);

	for(int i = 0; i < 200; i++){
		// 1) Ver si responde en 0x68
		if (!I2C_ACC.scanOne(MPU_ADDR)) {
			log_debug((uint8_t*)"MPU no responde en 0x68\r\n", 0);
			delay();continue;
		}

		// 2) Salir de sleep: escribir 0x00 en PWR_MGMT_1
		if (!I2C_ACC.writeReg(MPU_ADDR, REG_PWR_MGMT_1, 0x00)) {
			log_debug((uint8_t*)"Error PWR_MGMT_1\r\n", 0);
			delay();continue;
		}

		// Pequeño delay
		delay();

		// 3) WHO_AM_I
		uint8_t who=0;
		if (!I2C_ACC.readReg(MPU_ADDR, REG_WHO_AM_I, who)) {
			log_debug((uint8_t*)"Error WHO_AM_I\r\n", 0);
			continue;
		}

		if (who == 0x68) {
			I2C_ACC.writeReg(MPU_ADDR, REG_PWR_MGMT_1, 0x00);
			log_debug((uint8_t*)"Init MPU OK\r\n", 0);
			mpu_initiated = true;
			return true;
		}
	}

	log_debug((uint8_t*)"Init MPU FAIL\r\n", 0);
	return false;

}



bool maxProbe()
{
    // ¿está en 0x57?
    if (!I2C_ACC.scanOne(MAX_ADDR)) {
        Uart0.Send((uint8_t*)"MAX not found at 0x57\r\n",0);
        return false;
    }

    // Leer PART ID (0xFF) -> 0x15
    uint8_t partid = 0;
    if (!I2C_ACC.readReg(MAX_ADDR, REG_PART_ID, partid)) {
        Uart0.Send((uint8_t*)"MAX read PARTID fail\r\n",0);
        return false;
    }
    char msg[32];
    int n = sprintf(msg, "MAX PARTID=0x%02X\r\n", partid);
    Uart0.Send((uint8_t*)msg, n);
    return (partid == 0x15);
}

static bool waitResetClear(void){
    // Esperar a que el bit RESET (0x40) en MODE_CONFIG se borre
    for (int i=0;i<200;i++){ uint8_t mc=0;
        if (I2C_ACC.readReg(MAX_ADDR, REG_MODE_CONFIG, mc) && !(mc & 0x40)) return true;
        delay();
    }
    return false;
}

bool maxInit(void)
{
    // Ver si responde
    if (!I2C_ACC.scanOne(MAX_ADDR)) { Uart0.Send((uint8_t*)"MAX 0x57 not found\r\n",0); return false; }

    // Reset
    if (!I2C_ACC.writeReg(MAX_ADDR, REG_MODE_CONFIG, 0x40)) return false;
    if (!waitResetClear()) { Uart0.Send((uint8_t*)"Reset bit stuck\r\n",0); }

    // FIFO: avg=4 (2<<5), rollover ON (1<<4), almostFull=0
    if (!I2C_ACC.writeReg(MAX_ADDR, REG_FIFO_CONFIG, (2<<5)|(1<<4)|0x00)) return false;

    // SPO2: ADC range=0 (4096nA), SR=100Hz (3<<2), PW=411us/18-bit (3)
    if (!I2C_ACC.writeReg(MAX_ADDR, REG_SPO2_CONFIG, (0<<5)|(3<<2)|3)) return false;

    // Corriente LEDs (subí para test)
    I2C_ACC.writeReg(MAX_ADDR, REG_LED1_RED, 0x50);   // ~12-17mA
    I2C_ACC.writeReg(MAX_ADDR, REG_LED2_IR,  0x50);
    I2C_ACC.writeReg(MAX_ADDR, REG_LED3_IR,  0x50);

    // Opción 1: SpO2 mode (RED+IR sin slots)
    // if (!I2C_ACC.writeReg(MAX_ADDR, REG_MODE_CONFIG, 0x03)) return false;

    // Opción 2: Multi-LED con slots (más compatible con MAX30105)
    // SLOT1=RED (0x01), SLOT2=IR (0x02), MODE=0x07
    I2C_ACC.writeReg(MAX_ADDR, REG_MULTI_LED_1, 0x02);
    I2C_ACC.writeReg(MAX_ADDR, REG_MULTI_LED_2, 0x03);
    if (!I2C_ACC.writeReg(MAX_ADDR, REG_MODE_CONFIG, 0x07)) return false;

    // Habilitar PPG_RDY en INT1
    I2C_ACC.writeReg(MAX_ADDR, REG_INTR_ENABLE_1, 0x40);

    // Limpiar punteros FIFO
    I2C_ACC.writeReg(MAX_ADDR, REG_FIFO_WR_PTR, 0x00);
    I2C_ACC.writeReg(MAX_ADDR, REG_OVF_COUNTER, 0x00);
    I2C_ACC.writeReg(MAX_ADDR, REG_FIFO_RD_PTR, 0x00);

    max_initiated=true;
    Uart0.Send((uint8_t*)"MAX init OK\r\n",0);
    maxDumpRegs(); // opcional para ver qué quedó
    return true;
}

static bool maxAvail(uint8_t *nSamples /*out*/) {
    uint8_t wr=0, rd=0;
    if (!I2C_ACC.readReg(MAX_ADDR, REG_FIFO_WR_PTR, wr)) return false;
    if (!I2C_ACC.readReg(MAX_ADDR, REG_FIFO_RD_PTR, rd)) return false;
    *nSamples = (wr - rd) & 0x1F;   // FIFO de 32 muestras
    return true;
}

void maxReadLoopOnce(void)
{
    // O bien chequear el flag
    uint8_t st=0;
    I2C_ACC.readReg(MAX_ADDR, REG_INTR_STATUS_1, st); // leerla la limpia
    // if (!(st & 0x40)) return; // si querés depender del flag

    uint8_t avail=0;
    if (!maxAvail(&avail)) { Uart0.Send((uint8_t*)"ptrs fail\r\n",0); return; }
    if (avail == 0) {
    	Uart0.Send((uint8_t*)"NADA AUN\r\n",0);
    	return;
    }

    // Consumí una muestra (RED+IR = 6 bytes)
    uint8_t buf[6];
    if (!I2C_ACC.readBytes(MAX_ADDR, REG_FIFO_DATA, buf, 6)) {
        Uart0.Send((uint8_t*)"FIFO read fail\r\n",0); return;
    }

    uint32_t red = ((uint32_t)buf[0]<<16) | ((uint32_t)buf[1]<<8) | buf[2];
    uint32_t ir  = ((uint32_t)buf[3]<<16) | ((uint32_t)buf[4]<<8) | buf[5];
    red &= 0x3FFFF; ir &= 0x3FFFF;

    char line[48];
    int n = sprintf(line, "RED=%lu IR=%lu (avail=%u)\r\n",
                    (unsigned long)red, (unsigned long)ir, avail);
    Uart0.Send((uint8_t*)line, n);
}

static void maxDumpRegs(void) {
    struct { uint8_t reg; const char* name; } r[] = {
        {0x09,"MODE_CONFIG"}, {0x0A,"SPO2_CONFIG"}, {0x08,"FIFO_CONFIG"},
        {0x04,"FIFO_WR_PTR"}, {0x06,"FIFO_RD_PTR"}, {0x05,"OVF_COUNTER"},
        {0x0C,"LED1_RED"}, {0x0D,"LED2_IR"}, {0x11,"SLOT1"}, {0x12,"SLOT2"},
        {0x02,"INTR_EN1"}, {0x00,"INTR_ST1"}, {0xFF,"PART_ID"}
    };
    for (unsigned i=0;i<sizeof(r)/sizeof(r[0]);++i) {
        uint8_t v=0;
        if (I2C_ACC.readReg(0x57, r[i].reg, v)) {
            char line[40]; int n=sprintf(line,"%s(0x%02X)=0x%02X\r\n", r[i].name, r[i].reg, v);
            Uart0.Send((uint8_t*)line,n);
        }
    }
}

int main(void)
{

	//mpuInit();
	log_debug((uint8_t*)"Initializing...\r\n", 0);

	for (volatile int i=0; i<100; ++i) {

		if (maxProbe()) {
			log_debug((uint8_t*)"maxProbe WORKED...\r\n", 0);
			maxInit();
			break;
		}
		log_debug((uint8_t*)"maxProbe failed, trying again...\r\n", 0);
	    delay();
	 }


    while (1) {

    }
}

void tick() {


	static uint8_t led=0;
	led^=1; LED_VERDE.Set(led);

	if(!max_initiated) {
		log_debug((uint8_t*)"MAX NOT INITIATED, SKIPPING...\r\n", 0);
		return;
	} else {
		log_debug((uint8_t*)"READING MAX...\r\n", 0);
		maxReadLoopOnce();
	}


	/*uint8_t buf[6];
	if (I2C_ACC.readBytes(MPU_ADDR, REG_ACCEL_XOUT_H, buf, 6)) {
		int16_t ax = (int16_t)((buf[0] << 8) | buf[1]);
		int16_t ay = (int16_t)((buf[2] << 8) | buf[3]);
		int16_t az = (int16_t)((buf[4] << 8) | buf[5]);

		log_acc(ax, ay, az);
	} else {
		log_debug((uint8_t*)"ACC read error\r\n", 0);
	}*/
}

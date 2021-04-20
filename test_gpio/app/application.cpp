#include <SmingCore.h>

Timer procTimer;
bool state = true;
uint8_t gpio = 2;

uint8_t next[] {
	4, // 0
	0, // 1
	0, // 2
	0, // 3
	5, // 4
	16,// 5
	0, // 6
	0, // 7
	0, // 8
	0, // 9
	0, // 10
	0, // 11
	0, // 12
	0, // 13
	0, // 14
	0, // 15
	2, // 16
};

void blink()
{
	digitalWrite(gpio, state);

	if(state) {
		Serial.printf("gpio %2d on\n", gpio);
	} else {
		Serial.printf("gpio %2d off\n", gpio);
		gpio = next[gpio];
	}
	state = !state;
}

void init()
{
    WifiAccessPoint.enable(false);

	Serial.begin(115200);
	// Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Allow debug output to serial

	for(int i=0; i<=16; i++) {
		pinMode(i, OUTPUT);
	}
	gpio = 2;
	procTimer.initializeMs(1000, blink).start();
}

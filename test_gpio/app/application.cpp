#include <SmingCore.h>

Timer procTimer;
bool state = true;
uint8_t gpio = 2;

uint8_t next[] {
	4, // 0
  255, // 1
	0, // 2
  255, // 3
	5, // 4
   16, // 5
  255, // 6
  255, // 7
  255, // 8
  255, // 9
  255, // 10
  255, // 11
  255, // 12
  255, // 13
  255, // 14
  255, // 15
	2, // 16
};

void gpio_shift()
{
	digitalWrite(gpio, state);

	if(state) {
		gpio = next[gpio];
	}
	state = !state;
}

void gpio_init()
{
	Serial.begin(115200);
	Serial.systemDebugOutput(true); // Allow debug output to serial
	Serial.println("init pins for output");
	for(int i=0; i<=16; i++) {
		if(next[i] < 17) {
			pinMode(next[i], OUTPUT);
		}
	}
	gpio = 2;
	Serial.println("start timer");
	procTimer.initializeMs(100, gpio_shift).start();
	Serial.println("init done!");
}


#define LED_PIN 2 // GPIO2

void blink()
{
	digitalWrite(LED_PIN, state);
	state = !state;
}

void blink_init()
{
	pinMode(LED_PIN, OUTPUT);
	procTimer.initializeMs(1000, blink).start();
}

void init() {
	WifiAccessPoint.enable(false);
	gpio_init();
}
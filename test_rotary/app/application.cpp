#include <SmingCore.h>
#include <Libraries/Adafruit_ILI9341/Adafruit_ILI9341.h>
#include <Libraries/Adafruit_GFX/BMPDraw.h>
#include "grid.h"
#include "rotary.h"

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "jukebox" // Put your SSID and password here
#define WIFI_PWD "Kieslowski"
#endif

#define VERSION "v0.4"
#define DISABLE_WIFI
#define ROTARY_POLL_US 250

#define LED_PIN 2 // GPIO2

#define SWITCH_PIN 3  // GPIO3 = RX
#define ROT_A_PIN 10  // GPIO10 = SD3
#define ROT_B_PIN 2   // GPIO2 = D4

/*
Pinout display:
MISO GPIO12 (oransj -> 7)
MOSI GPIO13 (blå    -> 6)
CLK GPIO14  (grønn  -> 8)
CS GPIO15   (hvit   -> 5)
DC GPIO5    (lilla  -> 14)
RST GPIO4   (grå    -> 13  )

VCC (brun)
GND (svart)
LED, bakgrunnslys (gul)
*/

Adafruit_ILI9341 tft;

//Timer guiTimer;
Timer pollTimer;
Grid *grid;

String sensors[] = { "", "Switch pressed", "Rotary value", "Rotary direction", NULL};
String values[]  = {"value", NULL};

void splashScreen() {
		// text display tests
	tft.setTextSize(2);

	tft.setTextColor(ILI9341_GREEN);
	tft.setCursor(0, 0);
	tft.setCursor(60, 60);
	tft.print("Rotary switch demo");
	tft.setCursor(60, 80);
	tft.print(VERSION);
	tft.setTextColor(ILI9341_WHITE);
	tft.setCursor(60, 120);
	tft.print("rrn");
}

void initDisplay() {
	tft.begin();
	tft.fillScreen(0);
	tft.setRotation(1);

	splashScreen();
	delay(2000);
}

void initGrid() {
	grid = new Grid(&tft, 5);
	grid->init(sensors, values);
}

int swcount=0;

void updateRotValue(Rotary* r, int direction) {
	switch(direction) {
		case -1:
			grid->updatecell(0, 1, String(r->getValue()));
			grid->updatecell(0, 2, "Down");
			break;
		case 0:
			grid->updatecell(0, 0, String(++swcount));
			break;
		case 1:
			grid->updatecell(0, 1, String(r->getValue()));
			grid->updatecell(0, 2, "Up");
			break;
	}
}

Rotary rotary(ROT_A_PIN, ROT_B_PIN, SWITCH_PIN);

void initRotary() {
	rotary.registerCallback(updateRotValue);
}

void poll() {
	rotary.poll();
}

void init()
{
	Serial.begin(115200);
	// Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Allow debug output to serial

	WifiAccessPoint.enable(false);
#ifndef DISABLE_WIFI
	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiStation.enable(true);
#endif

	Serial.println("init display");
	initDisplay();
	initGrid();
	Serial.println("init rotary polling");
	initRotary();
	//initPinsPoll();
	pollTimer.initializeUs(ROTARY_POLL_US, poll).start();
	// Serial.println("start timer");
	//guiTimer.initializeMs(1000, updateStatus).start();
}



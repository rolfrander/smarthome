#include <SmingCore.h>
#include <Libraries/Adafruit_ILI9341/Adafruit_ILI9341.h>
#include <Libraries/Adafruit_GFX/BMPDraw.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "jukebox" // Put your SSID and password here
#define WIFI_PWD "Kieslowski"
#endif

#define DISABLE_WIFI

#define LED_PIN 2 // GPIO2

#define SWITCH_PIN 3  // GPIO3 = RX
#define ROT_A_PIN 10  // GPIO10 = SD3
#define ROT_B_PIN 2   // GPIO2 = D4

/*
Pinout:
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

bool state = true;

bool state_a = false;
bool state_b = false;
bool state_sw = false;

String txt_a_on  = "ROT-A: On ";
String txt_a_off = "ROT-A: Off";
String txt_b_on  = "ROT-B: On ";
String txt_b_off = "ROT-B: Off";
String txt_sw_on  = "Switch pressed";
String txt_sw_off = "Switch neutral";

void blink()
{
	digitalWrite(LED_PIN, state);
	state = !state;
}


Timer guiTimer;
Timer procTimer;

int counter=0;

void clearAndWrite(int x, int y, int textSize, int textColor, String text) {
	tft.setTextSize(textSize);
	tft.setTextColor(textColor);
	tft.fillRect(x, y, text.length()*textSize*6, textSize*8, ILI9341_DARKGREY);
	tft.setCursor(x, y);
	tft.print(text);
}

void displayStatus()
{
	String status = String(counter);
	int line = 0;
	clearAndWrite(15, 15 + line*30, 3, ILI9341_GREEN, status);
	line++;
	clearAndWrite(15, 15 + line*30, 3, ILI9341_WHITE, state_sw ? txt_sw_on : txt_sw_off);
	line++;
	clearAndWrite(15, 15 + line*30, 3, ILI9341_WHITE, state_a ? txt_a_on : txt_a_off);
	line++;
	clearAndWrite(15, 15 + line*30, 3, ILI9341_WHITE, state_b ? txt_b_on : txt_b_off);
}

void initDisplay() {
	// text display tests
	tft.begin();
	tft.fillScreen(0);
	tft.setRotation(1);
	tft.setTextSize(2);

	tft.setTextColor(ILI9341_GREEN);
	tft.setCursor(0, 0);
	tft.setCursor(60, 60);
	tft.print("Rotary switch demo");
	tft.setCursor(60, 80);
	tft.print("v0.11");
	tft.setTextColor(ILI9341_WHITE);
	tft.setCursor(60, 120);
	tft.print("rrn");
	delay(2000);
	tft.fillScreen(0);
}

void initPins() {
	pinMode(SWITCH_PIN, INPUT_PULLUP);
	pinMode(ROT_A_PIN, INPUT_PULLUP);
	pinMode(ROT_B_PIN, INPUT_PULLUP);
}

void updateStatus() {
	counter++;
	state_sw= digitalRead(SWITCH_PIN);
	state_a = digitalRead(ROT_A_PIN);
	state_b = digitalRead(ROT_B_PIN);
	displayStatus();
}

void init()
{
//	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
//	Serial.systemDebugOutput(true); // Allow debug output to serial

#ifndef DISABLE_WIFI
	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiStation.enable(true);
	WifiAccessPoint.enable(false);
#endif

	initDisplay();
	initPins();
	guiTimer.initializeMs(1000, updateStatus).start();
}



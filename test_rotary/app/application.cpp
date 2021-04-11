#include <SmingCore.h>
#include <Libraries/Adafruit_ILI9341/Adafruit_ILI9341.h>
#include <Libraries/Adafruit_GFX/BMPDraw.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "jukebox" // Put your SSID and password here
#define WIFI_PWD "Kieslowski"
#endif

#define VERSION "v0.3"
#define DISABLE_WIFI

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

Timer guiTimer;

String events[] = {"Counter", NULL};
String inputs[] = {"Input", "button", "rot-A", "rot-B", "rotation", NULL};

#define CNT_BUT 0
#define CNT_ROT_A 1
#define CNT_ROT_B 2
#define CNT_VAL 3

const int numCounters = 4;
int counters[] = { 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};


const inline int maxInt(int a, int b) {
	return (a>b) ? a : b;
}

const inline int minInt(int a, int b) {
	return (a<b) ? a : b;
}

class Grid {
	private:
		int margin;
		int firstColWith;
		int colWith;
		int textSize;
		Adafruit_GFX *gfx;

		void drawCell(int col, int row, String txt, int foreground, int background) {
			this->gfx->setTextSize(this->textSize);
			this->gfx->setTextColor(foreground);
			int x = this->margin;
			if(col > 0) { x += firstColWith; }
			if(col > 1) { x += colWith * (col-1); }
			int y = this->margin + row * textSize * 8;
			int width = (col == 0 ? firstColWith : colWith) * textSize * 6;
			int height = textSize * 8;
			this->gfx->fillRect(x, y, width, height, background);
			this->gfx->setCursor(x, y);
			this->gfx->print(txt);
		}

	public:
		Grid(Adafruit_GFX *_gfx, int margin) {
			this->gfx = _gfx;
			this->margin = margin;
		}

		void init(String leftCol[], String topRow[]) {
			this->gfx->fillScreen(ILI9341_BLACK);

			int maxLeftCol = 0;
			int rows = 0;
			for(int i=0; leftCol[i]; i++) {
				maxLeftCol = maxInt(maxLeftCol, leftCol[i].length()+1);
				rows++;
			}
			int maxTopRow = 0;
			int cols = 0; // this does not include leftmost column
			for(int i=0; topRow[i]; i++) {
				maxTopRow = maxInt(maxTopRow, topRow[i].length()+1);
				cols++;
			}
			int maxCharWith = (gfx->width() - this->margin*2) / (maxLeftCol + maxTopRow*cols);
			int maxCharHeight = (gfx->height() - this->margin*2) / rows;
			this->textSize = minInt(maxCharWith / 6, maxCharHeight / 8);

			this->firstColWith = maxLeftCol * this->textSize * 6;
			this->colWith = maxTopRow * this->textSize * 6;

			for(int i=0; i<rows; i++) {
				this->drawCell(0, i, leftCol[i], ILI9341_WHITE, ILI9341_DARKGREY);
			}
			for(int i=0; i<cols; i++) {
				this->drawCell(i+1, 0, topRow[i], ILI9341_YELLOW, ILI9341_BLUE);
			}
		}

		void updatecell(int col, int row, String text) {
			this->drawCell(col+1, row+1, text, ILI9341_GREEN, ILI9341_BLACK);
		}
};

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

Grid *grid;

void initDisplay() {
	tft.begin();
	tft.fillScreen(0);
	tft.setRotation(1);

	splashScreen();
	delay(2000);
}

void initGrid() {
	grid = new Grid(&tft, 5);
	grid->init(inputs, events);
}

bool pendingUpdate[] = {false, false, false, false};

void clearPending(void *pending) {
	*(bool *)pending = false;
}

SimpleTimer clearTimers[] {
	SimpleTimer(), 
	SimpleTimer(), 
	SimpleTimer(), 
	SimpleTimer()
};

void initTimers() {
	for(uint32_t i=0; i<numCounters; i++) {
		clearTimers[i].initializeMs(500, clearPending, &pendingUpdate[i]);
	}
}

void doUpdateCounter(uint32_t c) {
	counters[c]++;
	grid->updatecell(0, c, String(counters[c]));
	if(c == CNT_BUT) {
		for(int i=1; i<numCounters; i++) {
			counters[i] = 0;
			grid->updatecell(0, i, "0");
		}
	}

	if(c == CNT_ROT_A) {
		if(digitalRead(ROT_B_PIN)) { 
			counters[CNT_VAL]++;
		} else {
			counters[CNT_VAL]--;
		}
		grid->updatecell(0, CNT_VAL, String(counters[CNT_VAL]));
	}
	// pendingUpdate[c] = false;
	clearTimers[c].start(false);
}

void updateCounter(uint32_t c) {
	if(!pendingUpdate[c]) {
		pendingUpdate[c] = true;
		System.queueCallback(doUpdateCounter, c);
	}
}

void IRAM_ATTR intSwitch() { updateCounter(CNT_BUT);   };
void IRAM_ATTR intRotA()   { updateCounter(CNT_ROT_A); };
void IRAM_ATTR intRotB()   { updateCounter(CNT_ROT_B); };

void initPins() {

	// note to self: only one interrupt per pin
	attachInterrupt(SWITCH_PIN, intSwitch, FALLING);
	attachInterrupt(ROT_A_PIN, intRotA, FALLING);
	attachInterrupt(ROT_B_PIN, intRotB, FALLING);

	// attachInterrupt sets pinMode, so this must be reset after interrupt setup
	pinMode(SWITCH_PIN, INPUT_PULLUP);
	pinMode(ROT_A_PIN, INPUT_PULLUP);
	pinMode(ROT_B_PIN, INPUT_PULLUP);

}

void updateStatus() {
	for(int i=0; i<numCounters; i++) {
		grid->updatecell(0, i, String(counters[i]));
	}
}

int pin_a[320];
int pin_b[320];
int count=0;
Timer pollTimer;
int offset = 0;
int mark_a = -1;
int mark_b = -1;

void markA() {
	mark_a = count;
}

void markB() {
	mark_b = count;
}

void initPinsPoll() {
	attachInterrupt(ROT_A_PIN, markA, FALLING);
	attachInterrupt(ROT_B_PIN, markB, FALLING);

	pinMode(SWITCH_PIN, INPUT_PULLUP);
	pinMode(ROT_A_PIN, INPUT_PULLUP);
	pinMode(ROT_B_PIN, INPUT_PULLUP);
}

void graph(Adafruit_GFX* gfx, int offset, int data[], int count) {
	int m = minInt(count, gfx->width());
	int16_t y = offset + 7;
	for(int16_t x=0; x<m; x++) {
		gfx->drawPixel(x, y-data[x]*5, ILI9341_WHITE);
	}
}

void initGraphs() {
	tft.fillScreen(ILI9341_BLACK);
}

void drawGraphs(void *) {
	if(offset == 0) {
		tft.fillScreen(ILI9341_BLACK);
	}
	if(mark_a > 0) {
		tft.drawLine(mark_a, offset+1, mark_a, offset+8, ILI9341_PINK);
	}
	graph(&tft, offset, pin_a, 320);
	if(mark_b > 0) {
		tft.drawLine(mark_b, offset+11, mark_b, offset+18, ILI9341_YELLOW);
	}
	graph(&tft, offset+10, pin_b, 320);
	mark_a = -1;
	mark_b = -1;
	offset += 25;
	if(offset+10 > tft.height()) {
		offset = 0;
	}
}

void poll() {
	bool a = !digitalRead(ROT_A_PIN);
	bool b = !digitalRead(ROT_B_PIN);
	if(a || b || (count>0)) {
		count++;
		pin_a[count] = a ? 1 : 0;
		pin_b[count] = b ? 1 : 0;
	}
	if(count >= 319) {
		count = 0;
		pin_a[0] = 0;
		pin_b[0] = 0;
		System.queueCallback(drawGraphs);
	}
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Allow debug output to serial

#ifndef DISABLE_WIFI
	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiStation.enable(true);
	WifiAccessPoint.enable(false);
#endif

	// Serial.println("init display");
	initDisplay();
	initGraphs();
	// initGrid();
	// Serial.println("init debounce timers");
	// initTimers();
	Serial.println("init pins and interrupts");
	// initPins();
	initPinsPoll();
	pollTimer.initializeUs(500, poll).start();
	// Serial.println("start timer");
	//guiTimer.initializeMs(1000, updateStatus).start();
}



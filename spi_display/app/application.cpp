#include <SmingCore.h>
#include <Wire.h>
#include <HSPI/Controller.h>
#include <HSPI/Device.h>
#include <HSPI/Common.h>

/*
Pinout display koblet til geekreit:
MISO SD0 (oransj -> 8H)
MOSI SD1 (blå    -> 10H)
CLK  CLK (grønn  -> 7H)
CS   ~D3/GPIO0  (hvit   -> 12V)
DC   ~D1/GPIO5   (lilla  -> 14V)
RST  ~D2/GPIO4   (grå    -> 13V)

VCC (brun)
GND (svart)
LED, bakgrunnslys (gul)
*/

#define PIN_DC    5
#define PIN_RESET 4

#define PINSET_OVERLAP_HSPI_CS 0 // = GPIO15 = TXD2 = ~D8
#define PINSET_OVERLAP_SPI_CS1 1 // = GPIO1 = TXD1 = ~D4
#define PINSET_OVERLAP_SPI_CS2 2 // = GPIO0 = UART0_TXD = ~D3

namespace HSPI {
    class ILI9341_SPI: public Device {
    public:
        ILI9341_SPI(Controller& controller): Device(controller) {}

        IoModes getSupportedIoModes() const override {
            return IoMode::SPI;
        }
    };
} // namespace HSPI

HSPI::Controller spi;
HSPI::ILI9341_SPI screen(spi);

Timer procTimer;
bool state = true;

void dumpBuffer(String debugText, uint8* buffer, uint16 length) {
    Serial.print(debugText);
    for(int i=0; i<length; i++) {
        Serial.printf("%0x ", buffer[i]);
    }
    Serial.println();
}

void close() {
    Serial.println("close");
    screen.end();
    spi.end();
}

void IRAM_ATTR print_request(HSPI::Request& r) {
    Serial.println("After:");
    Serial.printf("Command : %0xh\n", r.cmd);
    Serial.printf("Data in : %0x (%d) -> %d\n",  r.in.data32, r.in.length, r.in.isPointer);
    Serial.printf("Data out: %0x (%d) -> %d\n",  r.out.data32,r.out.length,r.out.isPointer);
}

void IRAM_ATTR request_complete(HSPI::Request& r) {
    print_request(r);
//    System.queueCallback(close);

}

void init()
{
    WifiAccessPoint.enable(false);

	Serial.begin(115200);
	// Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Allow debug output to serial

    pinMode(PIN_DC, OUTPUT);
    pinMode(PIN_RESET, OUTPUT);

    // reset display
    digitalWrite(PIN_RESET, 1);
    Serial.println("reset");
    digitalWrite(PIN_RESET, 0);
    delay(50);
    digitalWrite(PIN_RESET, 1);
    Serial.println("reset done");
    delay(500);
    // init controller
    spi.begin();

    // assign device to CS-pin
    screen.begin(HSPI::PinSet::overlap, PINSET_OVERLAP_SPI_CS2);
    screen.setSpeed(10000000U);
    screen.setBitOrder(MSBFIRST);
    screen.setClockMode(HSPI::ClockMode::mode0);

    HSPI::Request r1, r2;
    r1.device = &screen;
    r1.setCommand(0x0d3, 8); // read display identification information
    r1.out.clear();
    r1.in.clear();

    r2.setCommand(0, 0);
    r2.in.set32(0, 3);

    digitalWrite(PIN_DC, 0);
    screen.execute(r1);
    digitalWrite(PIN_DC, 1);
    screen.execute(r2);

    print_request(r1);
    print_request(r2);
}

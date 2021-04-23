#include <SmingCore.h>
#include <Wire.h>
#include <HSPI/Controller.h>
#include <HSPI/Device.h>
#include <HSPI/Common.h>

/*
Pinout display koblet til geekreit:
MISO SD0        (oransj -> 8H)
LED             (gul    -> +)
CLK  CLK        (grønn  -> 7H)
MOSI SD1        (blå    -> 10H)
DC   ~D1/GPIO5  (lilla  -> 14V)
RST  ~D2/GPIO4  (grå    -> 13V)
CS   ~D3/GPIO0  (hvit   -> 12V)
GND             (svart  -> -)
VCC             (brun   -> +)

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
    Serial.println("*** close");
    screen.end();
    spi.end();
    Serial.println("*** done");
}

void IRAM_ATTR print_request(String txt, HSPI::Request& r) {
    Serial.println(txt);
    if(r.cmdLen > 0)     { Serial.printf("Command : %02X\n", r.cmd); }
    if(r.addrLen > 0)    { Serial.printf("Address : %0x (%d)\n", r.addr, r.addrLen);}
    if(r.in.length > 0)  { Serial.printf("Data in : %0x (%d)\n", r.in.data32, r.in.length); }
    if(r.out.length > 0) { Serial.printf("Data out: %0x (%d)\n", r.out.data32,r.out.length); }
}

void IRAM_ATTR request_complete(HSPI::Request& r) {
    System.queueCallback(close);
}

void testsequence()
{
    Serial.println("****************************************");
    Serial.println("*** start");
    pinMode(PIN_DC, OUTPUT);
    pinMode(PIN_RESET, OUTPUT);

    // reset display
    digitalWrite(PIN_RESET, 1);
    digitalWrite(PIN_RESET, 0);
    delay(50);
    digitalWrite(PIN_RESET, 1);
    delay(500);
    // init controller
    spi.begin();

    // assign device to CS-pin
    screen.begin(HSPI::PinSet::overlap, PINSET_OVERLAP_SPI_CS2);
    screen.setSpeed(1000000U);
    screen.setBitOrder(MSBFIRST);
    screen.setClockMode(HSPI::ClockMode::mode0);
    screen.setIoMode(HSPI::IoMode::SPI);

    HSPI::Request r1, r2;
    r1.device = &screen;
    //r1.out.clear();
    r1.setCommand(0xc, 8);
    //r1.out.set8(0x0);
    r1.in.clear();
    print_request("Request 1", r1);
    digitalWrite(PIN_DC, 0);
    screen.execute(r1);
    print_request("Request 1 done", r1);

    //r2.setCommand(0xd3, 8);
    r2.setCommand(0, 0);
    r2.in.set32(0x0, 4);
    r2.out.clear();
    r2.callback = request_complete;
    print_request("Request 2", r2);
    digitalWrite(PIN_DC, 0);
    screen.execute(r2);
    print_request("Request 2 done", r2);
}

void init() {
	Serial.begin(115200);
	// Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Allow debug output to serial

    WifiAccessPoint.enable(false);
    procTimer.initializeMs(5000, testsequence).start();
}
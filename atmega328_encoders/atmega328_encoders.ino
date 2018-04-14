#include <Wire.h>

//#define DEBUG
#ifdef DEBUG
  #define BAUD 115200
  #define dbPrint(x) Serial.print(x)
  #define dbPrintln(x) Serial.println(x)
#else
  #define dbPrint(x)
  #define dbPrintln(x)
#endif

#define I2C_ADDRESS 40

const int QEM [16] = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0};

volatile uint16_t lastRead;

volatile int32_t counts[6] = {0};

uint16_t lastCount = 0;

void setup() {

  #ifdef DEBUG
    Serial.begin(BAUD);
  #endif
  
  // set all encoder pins to input
  for (int i = 0; i < 12; i++)
    pinMode(i, INPUT);

  // set up interrupts
  PCIFR |= (1 << PCIF0); // clear PCINT[7:0] flag
  PCIFR |= (1 << PCIF2); // clear PCINT[23:16] flag
  
  PCICR |= (1 << PCIE0); // enable PCINT[7:0]
  PCICR |= (1 << PCIE2); // enable PCINT[23:16]
  
  PCMSK0 = 0b00001111; // mask to enable PCINT on (D8-D11)
  PCMSK2 = 0b00111111; // enable PCINT on (D0-D7)

  sei(); // enable global interrupts to be safe

  // setup I2C
  Wire.begin(I2C_ADDRESS);
  Wire.onRequest(requestEvent);
}

// pass both ISRs to updateCounts()
ISR(PCINT0_vect) { updateCounts(); }
ISR(PCINT2_vect) { updateCounts(); }

void loop() {
//    if (millis() - lastCount > 500) {
//      dbPrint("Counts: ");
//      dbPrint(counts[5]);
//      dbPrint(",");
//      dbPrint(counts[4]);
//      dbPrint(",");
//      dbPrint(counts[3]);
//      dbPrint(",");
//      dbPrint(counts[2]);
//      dbPrint(",");
//      dbPrint(counts[1]);
//      dbPrint(",");
//      dbPrintln(counts[0]);
//      lastCount = millis();
//    }
}

// shared ISR for PCINT0 and PCINT1
void updateCounts()
{
  // quickly read all pins
  uint16_t newRead = PIND | ( (PINB & 0x0F) << 8 );
  
  // mask out pins that have not changed
  uint16_t change = newRead ^ lastRead;

  for (int i = 0; i < 6; ++i)
  {
    // update count based on old and new readings
    counts[i] += QEM [ ( (lastRead >> (2 * i) ) & 0b11) * 4 + ( (newRead >> (2 * i) & 0b11) ) ];
  }

  // update lastReading
  lastRead = newRead;
}

void requestEvent()
{
  byte data[4];
  int32_t value;
  
  for (int i = 0; i < 6; i++)
  {
    value = counts[i];
  
    // put value into 4 byte array
    data[0] = (value >> 24) & 0xFF;
    data[1] = (value >> 16) & 0xFF;
    data[2] = (value >> 8) & 0xFF;
    data[3] = value & 0xFF;
  
    // write 4 bytes of data to I2C
    Wire.write(data, 4);
    dbPrint("Data[");
    dbPrint(i);
    dbPrint("]: ");
    dbPrint(data[3]);
    dbPrint(",");
    dbPrint(data[2]);
    dbPrint(",");
    dbPrint(data[1]);
    dbPrint(",");
    dbPrintln(data[0]);
  }
}

#include <Wire.h>

//#define DEBUG_REQUEST
//#define DEBUG
#ifdef DEBUG
  #define BAUD 115200
  #define dbPrint(x) Serial.print(x)
  #define dbPrintln(x) Serial.println(x)
#else
  #define dbPrint(x)
  #define dbPrintln(x)
#endif

#define I2C_ADDRESS 0x28
#define ENC_COUNT 6       // Number of encoders

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
    if (millis() - lastCount > 500) {
      dbPrint("Counts: ");
      dbPrint(counts[5]);
      dbPrint(",");
      dbPrint(counts[4]);
      dbPrint(",");
      dbPrint(counts[3]);
      dbPrint(",");
      dbPrint(counts[2]);
      dbPrint(",");
      dbPrint(counts[1]);
      dbPrint(",");
      dbPrintln(counts[0]);
      lastCount = millis();
    }
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

// If we are debugging the request, use a generic request event
#ifdef DEBUG_REQUEST
void requestEvent()
{
  byte data[4];
  data[0] = 0xF0;
  data[1] = 0x0F;
  data[2] = 0xF0;
  data[3] = 0x0F;
  Wire.write(data, 4);
}

// If we aren't debugging, use the regular request event
#else
void requestEvent()
{
  int32_t value;
  byte sendBuffer[ENC_COUNT*sizeof(value)];
  
  for (int i = 0; i < ENC_COUNT*sizeof(value); i++)
  {
    value = counts[i];

    /*
     * Just because I'm obnoxious, there is another way to convert
     * the counts array to an array of bytes. This would, however
     * be potentially dangerous since it relies on the compiler
     * acting in a certain way. Anyways, the way to do it would be
     * something like this:
     * int36_t counts[6];
     * ... Counts gets filled somewhere ... 
     * uint8_t* value;
     * value = &counts;
     * Now, value points to the counts array and we could use the 
     * Wire.write command to send 24 bytes. This may not work
     * because the compiler might see this as unsafe (which it is)
     * since we never defined the memory space available to the 
     * value pointer. More of food for thought than something that
     * should actually be implemented. Also, depending on whether
     * data is stored as big or little endian, the data may come
     * out mixed up.
     */
  
    // Put value into a 24 byte array
    sendBuffer[i*4+0] = (value >> 24) & 0xFF;
    sendBuffer[i*4+1] = (value >> 16) & 0xFF;
    sendBuffer[i*4+2] = (value >> 8) & 0xFF;
    sendBuffer[i*4+3] = value & 0xFF;
  }

  //Write the buffer to the I2C line
  Wire.write(sendBuffer, ENC_COUNT*sizeof(value));

  // If debugging, also print the data to serial
  #ifdef DEBUG
    for (int i = 0; i < ENC_COUNT*sizeof(value); i++) 
    {
      dbPrint("Data[");
      dbPrint(i);
      dbPrint("]: ");
      dbPrintln(sendBuffer[i]);
    }
  #endif
}
#endif

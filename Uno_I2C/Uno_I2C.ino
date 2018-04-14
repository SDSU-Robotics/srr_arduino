#include <Wire.h>

#define I2C_ADDRESS 40
const int joystickPin = A0;

uint32_t value = 0;
byte data[4];

void setup() {
  Wire.begin(I2C_ADDRESS);       // join i2c bus with address I2C_ADDRESS
  Wire.onRequest(requestEvent);  // register event
}

void loop() {
  value = analogRead(joystickPin);
}

// function that executes whenever data is requested by master
// this function is registered as an event, see setup()
void requestEvent()
{
  // put value into 4 byte array
  data[0] = (value >> 24) & 0xFF;
  data[1] = (value >> 16) & 0xFF;
  data[2] = (value >> 8) & 0xFF;
  data[3] = value & 0xFF;

  // write 4 bytes of data to I2C
  Wire.write(data, 4);
}

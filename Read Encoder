int32_t CHA;
int32_t CHB;
int32_t master_count;
#define MOTOR_DIR 1

// Volatile variables for the ISR
volatile int portState = 0;
volatile int16_t encoderTicks = 0;
volatile bool changedTick = 0;

void setup() { 
  Serial.begin(9600); 
  pinMode(CHA, INPUT);
  pinMode(CHB, INPUT);
  Serial.print (master_count);
  attachInterrupt(0, ISR_A, RISING);  
  attachInterrupt(0, ISR_B, RISING);
  // interrupt 0 digital pin 2 positive edge trigger
}

void ISR_A() {
  portState = PIND & B11;
  switch ( portState ) {
    case B00:
    case B11:
      #if MOTOR_DIR
        encoderTicks += 1;
      #else
        encoderTicks -= 1;
      #endif
      break;
    case B01:
    case B10:
      #if MOTOR_DIR
        encoderTicks -= 1;
      #else
        encoderTicks += 1;
      #endif
      break;
  }
  changedTick = 1;
}
void ISR_B() {
  portState = PIND & B11;
  switch ( portState ) {
    case B00:
    case B11:
      #if MOTOR_DIR
        encoderTicks -= 1;
      #else
        encoderTicks += 1;
      #endif
      break;
    case B01:
    case B10:
      #if MOTOR_DIR
        encoderTicks += 1;
      #else
        encoderTicks -= 1;
      #endif
      break;
  }
  changedTick = 1;
}

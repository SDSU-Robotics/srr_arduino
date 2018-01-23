#define echoPin 2                                     // Pin 2 Echo input
#define intID 0                                       // Interrupt id for echo pulse

const int trigPins[8] = { 3, 4, 5, 6, 7, 8, 9, 10 };

volatile long echo_start = 0;                         // Records start of echo pulse 
volatile long echo_end = 0;                           // Records end of echo pulse
volatile long echo_duration = 0;                      // Duration - difference between end and start

unsigned int values[8];
int lastRead;

void setup() 
{
  for (int i = 0; i < 8; ++i)        // set trigger pin outputs
    pinMode(trigPins[i], OUTPUT);

  pinMode(echoPin, INPUT);                            // Echo pin set to input
  
  attachInterrupt(intID, echo_interrupt, CHANGE);  // Attach interrupt to the sensor echo input

  Serial.begin(9600);
}

void loop()
{
  for (int i = 0; i < 8; i++)
  {
    trigger_pulse(trigPins[i]);
    delay(15); // wait 15 ms to make sure echo is back
    
    if (echo_duration != lastRead)
    {
      values[i] = echo_duration;
      lastRead = echo_duration;
    }
    else // did not receive new pulse
      values[i] = -1;
  }
  
  for (int i = 0; i < 8; i++)
  {
    Serial.print(values[i]);
    Serial.print("\t");
  }
  Serial.print("\n");
}


void trigger_pulse(int pin)
{
  digitalWrite(pin, HIGH);
  delayMicroseconds(10);
  digitalWrite(pin, LOW);
}

void echo_interrupt()
{
  switch (digitalRead(echoPin))                     // Test to see if the signal is high or low
  {
    case HIGH:                                      // High so must be the start of the echo pulse
      echo_end = 0;                                 // Clear the end time
      echo_start = micros();                        // Save the start time
      break;
      
    case LOW:                                       // Low so must be the end of hte echo pulse
      echo_end = micros();                          // Save the end time
      echo_duration = echo_end - echo_start;        // Calculate the pulse duration
      break;
  }
}
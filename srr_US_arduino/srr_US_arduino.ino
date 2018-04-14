#include <ros.h>

#include <std_msgs/MultiArrayLayout.h>
#include <std_msgs/MultiArrayDimension.h>

#include <std_msgs/UInt16MultiArray.h>//MultiArray.h>

ros::NodeHandle nh;
std_msgs::UInt16MultiArray US_msg;
unsigned short data_space[8];

ros::Publisher US_raw_pub("US_raw", &US_msg);

#define echoPin 2                                     // Pin 2 Echo input
#define intID 0                                       // Interrupt id for echo pulse

const int trigPins[8] = { 3, 4, 5, 6, 7, 8, 9, 10 };

volatile long echo_start = 0;                         // Records start of echo pulse 
volatile long echo_end = 0;                           // Records end of echo pulse
volatile long echo_duration = 0;                      // Duration - difference between end and start

int lastRead;

void setup() 
{
  for (int i = 0; i < 8; ++i)        // set trigger pin outputs
    pinMode(trigPins[i], OUTPUT);

  pinMode(echoPin, INPUT);                            // Echo pin set to input
  
  attachInterrupt(intID, echo_interrupt, CHANGE);  // Attach interrupt to the sensor echo input

  nh.getHardware()->setBaud(57600);
  nh.initNode();
  US_msg.data_length = 8;
  US_msg.data = data_space; // allocate space for data
  
  nh.advertise(US_raw_pub);
}

void loop()
{
  for (int i = 0; i < 8; i++)
  {
    trigger_pulse(trigPins[i]);
    delay(15); // wait 15 ms to make sure echo is back
    
    if (echo_duration != lastRead)
    {
      US_msg.data[i] = echo_duration;
      lastRead = echo_duration;
    }
    else // did not receive new pulse
      US_msg.data[i] = -1;
  }
  
  US_raw_pub.publish( &US_msg );
  nh.spinOnce(); // necessary to establish link
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

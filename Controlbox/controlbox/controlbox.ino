#include <LiquidCrystal.h>

#define speed_pot 3   // Defines pin for speed setting pot

// Loop variables
unsigned long loop_start_time = 0;
const int loop_period = 50;
 
char drive_direction = 'F';   // This variable needs setting depending on switch position - F, R, or N

// Sent variables
char state = 'a';
char horn = 'h';
int desired_speed = 0;    // Filtered speed setpoint

void setup() 
{
  Serial.begin(9600);
}

void loop() 
{
  loop_start_time = millis();

  // Put rest of main code here - read switches, change state and horn
  
  //

  desired_speed = map(analogRead(speed_pot),0,1023,0,150);    // maps analog value from pot to corresponding speed setpoint (150 = 15kmph)
  if(drive_direction == 'R')    // gives a negative value to indicate direction is backwards
  {
    desired_speed = - desired_speed;
  }
    
  while(millis() - loop_start_time < loop_period) // Fixes loop time to 50ms
  {
  }
  send_command();   // Sends command word to the loco arduino
}

void send_command()
{
  Serial.write(desired_speed);
  Serial.write(state);
  Serial.write(horn);
}

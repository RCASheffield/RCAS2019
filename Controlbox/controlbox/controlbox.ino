#include <LiquidCrystal.h>

#define deadman_switch 6
#define drive_switch_1 7
#define drive_switch_2 8
#define mode_switch_1 9
#define mode_switch_2 12
#define horn_switch 14
#define regen_switch 15
#define auto_switch 16 
#define speed_pot 3   // Defines pin for speed setting pot

// Loop variables
unsigned long loop_start_time = 0;
const int loop_period = 50;
 
char drive_direction = 'F';   // This variable needs setting depending on switch position - F, R, or N

// Sent variables
char state = 'a';
char horn = 'h';
int desired_speed = 0;    // speed setpoint

void setup() 
{
  pinMode(deadman_switch,INPUT_PULLUP);
  pinMode(drive_switch_1,INPUT_PULLUP);
  pinMode(drive_switch_2,INPUT_PULLUP);
  pinMode(mode_switch_1,INPUT_PULLUP);
  pinMode(mode_switch_2,INPUT_PULLUP);
  pinMode(horn_switch,INPUT_PULLUP);
  pinMode(regen_switch,INPUT_PULLUP);
  pinMode(auto_switch,INPUT_PULLUP);
  Serial.begin(9600);
  LiquidCrystal lcd(11, 10, 5, 4, 3, 2);
  lcd.begin(16,2);    // need to add code to display: desired speed, actual speed, battery voltage, capacitor voltage, pwm value
}

void loop() 
{
  loop_start_time = millis();

  switch_states();

  set_horn();
  
  set_speed();
  
  send_command();   // Sends command word to the loco arduino
  
  while(millis() - loop_start_time < loop_period) // Fixes loop time to 50ms
  {
  }
}

void send_command()
{
  Serial.write(desired_speed);
  Serial.write(state);
  Serial.write(horn);
}

void set_horn()
{
  if(digitalRead(horn_switch) == LOW)
  {
    horn = 'H';
  }
  else
  {
    horn = 'h';
  }
}

void set_speed()
{
  desired_speed = map(analogRead(speed_pot),0,1023,0,150);    // maps analog value from pot to corresponding speed setpoint (150 = 15kmph)
  if(drive_direction == 'R')    // gives a negative value to indicate direction is backwards
  {
    desired_speed = - desired_speed;
  }
  if(drive_direction == 'N')
  {
    desired_speed = 0;
  }
}

void switch_states()
{
    if(digitalRead(deadman_switch)==HIGH)   // deadman released
  {
    state = 'a';
  }
  else  // deadman pressed
  {
    if(digitalRead(drive_switch_1)==LOW)  // forward
    {
      drive_direction == 'F';
    }
    else if(digitalRead(drive_switch_2)==LOW)  // forward
    {
      drive_direction == 'R';
    }
    else    // neutral
    {
      drive_direction == 'N';
      state = 'b';
    }
    if(drive_direction != 'N')    // forward or backward
    {
      if(digitalRead(regen_switch) == HIGH) // regen off
      {
        if(digitalRead(auto_switch)==HIGH)  // autostop off
        {
          state = 'c';
        }
        else  // autostop on
        {
          state = 'd';
        }
      }
      else  // regen on
      {
        if(digitalRead(mode_switch_1) == LOW)  // drive mode
        {
          state = 'e';
        }
        else if(digitalRead(mode_switch_2) == LOW)  // collect mode
        {
          state = 'f';
        }
        else  // use mode
        {
          state = 'g';
        }
      }
    }
  }
}

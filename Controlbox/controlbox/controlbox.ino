#include <LiquidCrystal.h>

#define deadman_switch 6
#define drive_switch_1 7
#define drive_switch_2 8

#define horn_switch 14

#define auto_switch 16 
#define speed_pot 3   // Defines pin for speed setting pot

LiquidCrystal lcd(11, 10, 5, 4, 3, 2);

// Loop variables
unsigned long loop_start_time = 0;
const int loop_period = 50;
 
// Sent variables
char autostop = 'a';    // a - off, A - on
char drive = 'S'    // S - stop, N - neutral, F - forward, R - reverse
char horn = 'h';    // h - off, H - on
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
  
  lcd.begin(20,4);
}

void loop() 
{
  loop_start_time = millis();

  if(digitalRead(horn_switch) == LOW)
  {
    horn = 'H';
  }
  else
  {
    horn = 'h';
  }
  
  if(digitalRead(auto_switch) == LOW)
  {
    autostop = 'a';
  }
  else
  {
    autostop = 'A';
  }

  if(digitalRead(deadman_switch)==HIGH)   // deadman released
  {
    drive = 'S';
  }
  else
  {
     if(digitalRead(drive_switch_1)==LOW)  // forward
    {
      drive == 'F';
    }
    else if(digitalRead(drive_switch_2)==LOW)  // forward
    {
      drive == 'R';
    }
    else    // neutral
    {
      drive == 'N';
    }
  }

  desired_speed = map(analogRead(speed_pot),0,1023,0,150);    // maps analog value from pot to corresponding speed setpoint (150 = 15kmph)

//  lcd.setCursor(0,0);
//  lcd.print(desired_speed);
//  lcd.print('     ');
//  lcd.setCursor(0,1);
//  lcd.print(horn);
//  lcd.setCursor(0,2);
//  lcd.print(state);

  //  Serial.write(desired_speed);
  //  Serial.write(state);
  //  Serial.write(horn);

  Serial.print(desired_speed);
  Serial.print(horn);
    
  while(millis() - loop_start_time < loop_period) // Fixes loop time to 50ms
  {
  }
  
}

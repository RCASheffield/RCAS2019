#include <LiquidCrystal.h>

#define deadman_switch 6
#define drive_switch_1 7
#define drive_switch_2 8
#define horn_switch 14
#define auto_switch 16 
#define speed_pot 3   // Defines pin for speed setting pot
#define speed_pot 0   // Defines pin for speed setting pot

LiquidCrystal lcd(11, 10, 5, 4, 3, 2);
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);

// Loop variables
unsigned long loop_start_time = 0;
const int loop_period = 25;
unsigned long time_last_comms_sent = 0;
const int send_period = 40;
unsigned long lcd_update_time = 0;
const int lcd_update_period = 500;
 
// Sent variables
byte desired_speed = 0;    // speed pot value
byte setpoint = 0;    // ramped speed setpoint
char horn = 'h';    // h - off, H - on
char drive = 'S';    // S - stop, N - neutral, F - forward, R - reverse
char autostop = 'a';    // a - off, A - on

byte actual_speed = 0;
float actual_speed_kmph = 0;
float desired_speed_kmph = 0;
byte pwm = 0;
byte pwm_percent = 0;

void setup() 
{
  Serial.begin(115200);
  pin_definitions();
  lcd_setup();
}

void loop() 
{
  loop_start_time = millis();

  if(millis() - time_last_comms_sent > send_period)
  {
    send_comms();
    time_last_comms_sent = millis();
  }
  
  setpoint = ramp_input(); // Sets desired speed to filtered setpoint based on speed pot setting
  
  switch_states();    // Reads swithces and sets sent variables accordingly

  receive_comms();

  if(millis() - lcd_update_time > lcd_update_period)
  {
    lcd_update();
    lcd_update_time = millis();
  }
       
  while(millis() - loop_start_time < loop_period) // Fixes loop time loop period in ms
  {
  }
  
}

void send_comms()
{
  Serial.print(',');
  Serial.print(char(setpoint));
  Serial.print(horn);
  Serial.print(drive);
  Serial.print(autostop); 
}

byte ramp_input()   // Ramps input to avoid large step inputs to the control system on the loco
{
  desired_speed = map(analogRead(speed_pot),0,1023,0,150);    // maps analog value from pot to corresponding speed setpoint (150 = 15kmph)
  if (desired_speed < setpoint)
  {
    setpoint++;
    return setpoint;
  }
  else if (desired_speed > setpoint)
  {
    setpoint--;
    return setpoint;
  }
  else
  {
    return setpoint;
  }
}

void switch_states()    // Reads switches and sets output variables
{
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
    autostop = 'A';
  }
  else
  {
    autostop = 'a';
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
}

void pin_definitions()
{
  pinMode(deadman_switch,INPUT_PULLUP);
  pinMode(drive_switch_1,INPUT_PULLUP);
  pinMode(drive_switch_2,INPUT_PULLUP);
  pinMode(horn_switch,INPUT_PULLUP);
  pinMode(auto_switch,INPUT_PULLUP);
}

void receive_comms()
{
  while(Serial.available() > 0)
  {
    Serial.find(',');
    while(Serial.available() < 2)
    {
    }
    actual_speed = byte(Serial.read());
    pwm = byte(Serial.read());
  }
}

void lcd_setup()
{
  lcd.begin(20,4);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Speed:      kmph");
  lcd.print("Speed:         kmph");
  lcd.setCursor(0,1);
  lcd.print("Setpoint:      kmph");
  lcd.setCursor(0,2);
  lcd.print("Mode:");
  lcd.setCursor(0,3);
  lcd.print("Auto:     PWM:     %");
}

void lcd_update()
{
  actual_speed_kmph = actual_speed / 10.0;
  lcd.setCursor(8,0);
  lcd.setCursor(10,0);
  lcd.print("    ");
  lcd.setCursor(8,0);
  lcd.setCursor(10,0);
  lcd.print(actual_speed_kmph,1);
  
  desired_speed_kmph = desired_speed / 10.0;
  lcd.setCursor(11,1);
  lcd.setCursor(10,1);
  lcd.print("    ");
  lcd.setCursor(11,1);
  lcd.setCursor(10,1);
  lcd.print(desired_speed_kmph,1);
  
  lcd.setCursor(7,2);
  lcd.setCursor(6,2);
  switch (drive)
  {
    case 'S':
    lcd.print("Stopped");
    break;
    
    case 'N':
    lcd.print("Neutral");
    break;

    case 'F':
    lcd.print("Forward");
    break;

    case 'R':
    lcd.print("Reverse");
    break;
  }

  lcd.setCursor(7,3);
  lcd.setCursor(6,3);
  switch (autostop)
  {
    case 'a':
    lcd.print("Off");
    break;
    
    case 'A':
    lcd.print("On");
    break;
  }

  pwm_percent = map(pwm,0,255,0,100);
  lcd.setCursor(16,3);
  lcd.setCursor(15,3);
  lcd.print("   ");
  lcd.setCursor(16,3);
  lcd.setCursor(15,3);
  lcd.print(pwm_percent);
}

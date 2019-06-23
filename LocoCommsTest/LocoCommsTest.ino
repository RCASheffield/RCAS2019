
#include <LiquidCrystal.h>

// LCD Display Setup
LiquidCrystal lcd(50, 51, 49, 48, 47, 46);

// Recieved Variables
byte desired_speed = 0;
char drive = 'S';
char horn = 'h';
char autostop = 'a';

// State and PWM variables
char prev_drive = 'S';
char prev_horn = 'h';
char prev_autostop = 'a';

float desired_speed_kmph = 0;

byte pwm = 128;   // Output to MOSFET drivers
long integral_error = 0;
byte setpoint = 0;

// Sent Variables
byte actual_speed = 0;
int battery_voltage = 48;

// Loop Variables
long loop_start_pulse_count;
long loop_pulse_count;

// Timing Variables
unsigned long loop_start_time = 0;
const unsigned long loop_period = 10;   // sets loop period and hence sample rate
unsigned long time_last_comms_received = 0;
const unsigned long timeout_period = 95;  // sets timeout for loss of communications
unsigned long time_last_comms_sent = 0;
const unsigned long send_period = 500;     // sets data send interval
unsigned long lcd_update_time = 0;
const unsigned long lcd_update_period = 1000;

void setup()
{
  Serial.begin(9600);   // Must match controller arduino
  lcd.begin(16, 2);
  lcd.clear();
}
void loop()
{
  loop_start_time = millis();
  
  receive_comms();
  
  if (millis() - time_last_comms_received > timeout_period)  // shutdown loco if communication is lost
  {
    drive = 'S';
    horn = 'h';
    autostop = 'a';
    desired_speed = 0;
  }
//  Serial.print(desired_speed);
//  Serial.print(horn);
//  Serial.print(drive);
//  Serial.print(autostop);
  
  if (millis() - lcd_update_time > lcd_update_period)
  {
    desired_speed_kmph = desired_speed / 10.0;
    lcd.setCursor(3,0);
    lcd.print(desired_speed_kmph,1);
    lcd.write("    ");
  }
  
  if (horn != prev_horn)
  {
    switch(horn)
    {
      case 'h':
      lcd.setCursor(0,0);
      lcd.write("h");
      break;
      
      case 'H':
      lcd.setCursor(0,0);
      lcd.write("H");
      break;

      default:
      lcd.setCursor(0,0);
      lcd.write("E");
      break;
    }
  }

  if (drive != prev_drive)
  {
    switch(drive)
    {
      case 'S':
      lcd.setCursor(0,1);
      lcd.write("Stopped");
      break;
      
      case 'N':
      lcd.setCursor(0,1);
      lcd.write("Neutral");
      break;

      case 'F':
      lcd.setCursor(0,1);
      lcd.write("Forward");
      break;

      case 'R':
      lcd.setCursor(0,1);
      lcd.write("Reverse");
      break;

      default:
      lcd.setCursor(0,1);
      lcd.write("Error  ");
      break;
    }
  }

  if (autostop != prev_autostop)
  {
    switch(autostop)
    {
      case 'a':
      lcd.setCursor(1,0);
      lcd.write("a");
      break;
      
      case 'A':
      lcd.setCursor(1,0);
      lcd.write("A");
      break;

      default:
      lcd.setCursor(1,0);
      lcd.write("E");
      break;
    }
  }
  
  if (millis() - time_last_comms_sent > send_period)  // sends communications at fixed period
  {
    send_comms();
  }
  
  prev_drive = drive;
  prev_horn = horn;
  prev_autostop = autostop;
  while (millis() - loop_start_time < loop_period)  // fixes loop length
  {
  }
  Serial.println(millis() - loop_start_time);
}

void receive_comms()
{
  while(Serial.available() > 0)
  {
    Serial.find(',');
    while(Serial.available() < 4)
    {
      
    }
    desired_speed = byte(Serial.read());
    horn = Serial.read();
    drive = Serial.read(); 
    autostop = Serial.read();  
    time_last_comms_received = millis();
  }
}

void send_comms()
{
//  Serial.write('s');
//  Serial.write(actual_speed);
//  Serial.write('v');
//  Serial.write(battery_voltage);
//  Serial.write('p');
//  Serial.write(pwm);
//  time_last_comms_sent = millis();
}


#include <LiquidCrystal.h>

// Output Pin Definitions
#define safety_pin 10
#define brakes_pin 9
#define H1A 6
#define H1B 8
#define H2A 5
#define H2B 7
#define pwm_pin 3o
#define horn_pin  4

// Analog Input Pin Definitions
#define battery_voltage_pin 2

#define autostop_pin  13  // IR sensor    // THIS PIN NEEDS CONNECTING TO/SWAPPING FOR PIN 19 FOR INTERRUPT TO WORK
#define autostop_pin_interrupt  19
// #define encoder_1a  25   // THIS PIN NEEDS CONNECTING TO/SWAPPING FOR PIN 18 FOR INTERRUPT TO WORK
#define encoder_1a_interrupt  18
#define encoder_1b  26
#define encoder_1i  27  // Index

// LCD Display Setup
LiquidCrystal lcd(50, 51, 49, 48, 47, 46);

// Recieved Variables
byte desired_speed = 0;
char drive = 'S';
char horn = 'h';
char autostop = 'a';

// Previous State Variables
char prev_drive = 'S';
char prev_horn = 'h';
char prev_autostop = 'a';

float desired_speed_kmph = 0;

// Interrupt Variables
volatile boolean autostop_flag = 0;
volatile long pulse_count = 0;

// Output to motor controller computation variables
byte pwm = 0; 
unsigned int control_output = 0;
byte error = 0;  
long integral_error = 0;
const float kp = 3;
const float ki = 0.005;

// Sent Variables
int actual_speed = 0;
float actual_speed_kmph = 0;
int battery_voltage = 48;

// Loop and Timing Variables
long loop_start_pulse_count = 0;
long loop_pulse_count = 01;
unsigned long loop_start_time = 0;
const byte loop_period = 10;   // sets loop period and hence sample rate
unsigned long time_last_comms_received = 0;
const byte timeout_period = 95;  // sets timeout for loss of communications
unsigned long time_last_comms_sent = 0;
const int send_period = 500;     // sets data send interval
unsigned long lcd_update_time = 0;
const int lcd_update_period = 1000;
unsigned long bridge_low_time = 0;
const int bridge_dead_time = 200;
boolean bridge_switch_flag = 0;
volatile unsigned long auto_start_pulse_count = 0;

void setup()
{
  pin_definitions();
  digitalWrite(safety_pin, HIGH); //Safety relay
  pinMode(autostop_pin_interrupt, INPUT);
  pinMode(autostop_pin, INPUT);
  
  Serial.begin(115200);   // M ust match controller arduino
  lcd.begin(16, 2);
  lcd.clear();
  attachInterrupt(digitalPinToInterrupt(encoder_1a_interrupt), read_encoder1, RISING);
}
void loop()
{
  loop_start_time = millis();
  loop_start_pulse_count = pulse_count;
  actual_speed = 2.20893 * loop_pulse_count/2;  //the 2 is a bodge// constant = 0.2650872/(G*T) where G is the gear ratio between the encoder and the wheel and T is the loop period. Outputs speed such that 150 = 15km/h

  receive_comms();
  
  if (millis() - time_last_comms_received > timeout_period)  // shutdown loco if communication is lost
  {
    drive = 'S';
    horn = 'h';
    autostop = 'a';
    desired_speed = 0;
  }
  
  if (millis() - lcd_update_time > lcd_update_period)   // lcd update speed information
  {
    desired_speed_kmph = desired_speed / 10.0;
    lcd.setCursor(3,0);
    lcd.print(desired_speed_kmph,1);

    actual_speed_kmph = actual_speed / 10.0;
    lcd.print("    ");
    lcd.setCursor(12,0);
    lcd.print(actual_speed_kmph,1);
    lcd.print("    ");
    
  }

  change_outputs();

//  if(autostop_flag == 1)
//  {
//    set_pwm(ref
//  }
//  else
//  {
    set_pwm(desired_speed);
//  }
  
  //analogWrite(pwm_pin,pwm); 
  analogWrite(pwm_pin,map(desired_speed,0,150,0,255)); 
  
  if (millis() - time_last_comms_sent > send_period)  // sends communications at fixed period
  {
    send_comms();
    time_last_comms_sent = millis();
  }
  
  while (millis() - loop_start_time < loop_period)  // fixes loop length
  {
  }
  
  loop_pulse_count = pulse_count - loop_start_pulse_count;
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

void change_outputs()
{
    if (horn != prev_horn)
  {
    switch(horn)
    {
      case 'h':
      lcd.setCursor(0,0);
      lcd.write("h");
      digitalWrite(horn_pin, LOW);
      break;
      
      case 'H':
      lcd.setCursor(0,0);
      lcd.write("H");
      digitalWrite(horn_pin, HIGH);
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
      digitalWrite(brakes_pin,LOW);
      digitalWrite(H1A,LOW);
      digitalWrite(H1B,LOW);
      digitalWrite(H2A,LOW);
      digitalWrite(H2B,LOW);
      break;
      
      case 'N':
      lcd.setCursor(0,1);
      lcd.write("Neutral");
      digitalWrite(brakes_pin,HIGH);
      digitalWrite(H1A,LOW);
      digitalWrite(H1B,LOW);
      digitalWrite(H2A,LOW);
      digitalWrite(H2B,LOW);
      break;

      case 'F':
      lcd.setCursor(0,1);
      lcd.write("Forward");
      digitalWrite(brakes_pin,HIGH);
      digitalWrite(H1A,LOW);
      digitalWrite(H1B,LOW);
      digitalWrite(H2A,LOW);
      digitalWrite(H2B,LOW);
      bridge_low_time = millis();
      bridge_switch_flag = 1;
      break;

      case 'R':
      lcd.setCursor(0,1);
      lcd.write("Reverse");
      digitalWrite(brakes_pin,HIGH);
      digitalWrite(H1A,LOW);
      digitalWrite(H1B,LOW);
      digitalWrite(H2A,LOW);
      digitalWrite(H2B,LOW);
      bridge_low_time = millis();
      bridge_switch_flag = 1;
      break;

      default:
      lcd.setCursor(0,1);
      lcd.write("Error  ");
      digitalWrite(brakes_pin,LOW);
      break;
    }
  }

  if(bridge_switch_flag == 1)
  {
    if(millis() - bridge_low_time > bridge_dead_time)
    {
      bridge_switch_flag = 0;
      switch (drive)
      {
        case 'F':
        digitalWrite(H1A,HIGH);
        digitalWrite(H2B,HIGH);
        break;

        case 'R':
        digitalWrite(H1B,HIGH);
        digitalWrite(H2A,HIGH);
        break;
      }
    }
  }

  if (autostop != prev_autostop)
  {
    autostop_flag = 0;
    switch(autostop)
    {
      case 'a':
      lcd.setCursor(1,0);
      lcd.write("a");
      detachInterrupt(digitalPinToInterrupt(autostop_pin_interrupt));   // Prevents autostop when not in autostop state
      break;
      
      case 'A':
      lcd.setCursor(1,0);
      lcd.write("A");
      attachInterrupt(digitalPinToInterrupt(autostop_pin_interrupt), autostop_ISR, RISING);
      break;

      default:
      lcd.setCursor(1,0);
      lcd.write("E");
      detachInterrupt(digitalPinToInterrupt(autostop_pin_interrupt));   // Prevents autostop when not in autostop state
      break;
    }
  }
  prev_drive = drive;
  prev_horn = horn;
  prev_autostop = autostop;
}

void send_comms()
{
  Serial.print(',');
  Serial.print(char(actual_speed));
  Serial.print(char(pwm));
}

void read_encoder1() // increments/decrements pulse count (position) depending on direction of rotation
{
  pulse_count++;
}

void autostop_ISR()
{
  auto_start_pulse_count = pulse_count;
  autostop_flag = 1;
  lcd.setCursor(14,1);
  lcd.print("EN");
  detachInterrupt(digitalPinToInterrupt(autostop_pin_interrupt));   // Prevents autostop triggering again
}

void set_pwm(int requested)
{
  error = requested - actual_speed;
//  if(actual_speed == 0)
//  {
//    integral_error = 0;
//  }
  control_output = kp * error + ki * integral_error;
//  if (control_output > 255)
//  {
//    integral_error = (255 - kp * error) / ki; // anti-windup measure
//    pwm = 255;
//    return;
//  }
//  else if (control_output < 0)
//  {
//    integral_error = (0 - kp * error) / ki; // anti-windup measure
//    pwm = 0;
//    return;
//  }
//  else
//  {
    integral_error = integral_error + error;
    pwm = control_output;
    return;
//  }
}

void pin_definitions()
{
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
}

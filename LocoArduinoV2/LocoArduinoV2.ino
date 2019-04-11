
#include <LiquidCrystal.h>

// Output Pin Definitions
const int sd_pin = 2;
const int pwm_pin = 3;
const int horn_pin = 4;
const int brakes_pin = 5;
const int power_pin = 6;
const int cap_pin = 7;
const int rheo_pin = 8;
const int spare_pin_1 = 9;
const int spare_pin_2 = 10;

// Analog Input Pin Definitions
const int battery_voltage_pin = 2;
const int cap_voltage_pin = 3;
const int current_sensor_pin = 4;
const int temp_sensor_pin = 5;

// LCD Display Setup
//LiquidCrystal lcd(50, 51, 49, 48, 47, 46);    // FOR ACTUAL LCD
LiquidCrystal lcd(12, 11, 10, 9, 8, 7);  // FOR TESTING

// Digital Input Pin Definitions
const int autostop_pin = 13;  // IR sensor
const int encoder_1a = 25;    
const int encoder_1b = 26;
const int encoder_1i = 27;    // Index
const int encoder_2a = 28;
const int encoder_2b = 29;
const int encoder_2i = 30;    // Index

// Output Variables
boolean brakes; // BRAKES relay
boolean power;  // POWER ISOLATE relay
boolean cap;   // CAP ISOLATE relay
boolean rheo; // RHEO relay
boolean sd;  // Driver shutdown pins
int pwm = 128;   // Output to MOSFET drivers

// Recieved Variables
int desired_speed = 0;
char state = 'a';
char horn = 'h';

char current_state = 'a';

// Sent Variables
int actual_speed = 0;
int battery_voltage = 48;
int cap_voltage = 0;

// Timing Variables
unsigned long loop_start_time = 0;
const unsigned long loop_period = 50;   // sets loop period and hence sample rate
unsigned long time_last_comms_received = 0;
const unsigned long timeout_period = 1000;  // sets timeout for loss of communications
unsigned long time_last_comms_sent = 0;
const unsigned long send_period = 5000;     // sets data send interval

// Temperature Threshold
const int temp_threshold = 150;   // temp ~0.4883 * reading from pin. Hence threshold of 150 is approx 75 degrees C.

void setup() 
{
  // put your setup code here, to run once:
  Serial.begin(9600);
  lcd.begin(16,2);
}

void loop() 
{
    loop_start_time = millis();
    
    receive_comms();    // check for new commands
    if(millis() - time_last_comms_received > timeout_period || analogRead(temp_sensor_pin) > temp_threshold)   // shutdown loco if communication is lost or in case of overheating
    {
      state = 'a';
    }
    
    if(state != current_state)    // change variables only on state change within state change function
    {
      state_change();
      current_state = state;
    }

    set_horn();

    
    
    // display information on LCD
    
    lcd.setCursor(0,1);
    lcd.print(desired_speed);
    lcd.print("     ");

    



    if(millis() - time_last_comms_sent > send_period)   // sends communications at fixed period
    {
      send_comms();
    }
    
    while(millis() - loop_start_time < loop_period)   // fixes loop length
    {
    }
}

void receive_comms()
{
  while (Serial.available()>0)    // Command word: speed,state,horn e.g. 500bh - speed 500, state b, horn off; 0aH - speed 0, state a, horn on; etc.
  {
    desired_speed = Serial.parseInt();
    state = Serial.read();
    horn = Serial.read();
    time_last_comms_received = millis();
  }
}

void state_change()   // Runs once on state change
{
  lcd.setCursor(15,1);
  lcd.print(state);
  switch(state)
  {
    case 'a': // Neutral, parking brake on
      
    break;

    case 'b':   // Neutral (shutdown low)
      
    break;
    
    case 'c':   // Standard, forward/backward drive, speed set by controller
      
    break;

    case 'd':   // Standard, forward/backward drive, speed set by controller, but enters auto stop when signal recieved
     
    break;

    case 'e':   // Forward/backward drive, speed set by controller, but capacitor disconnected and emptied
      
    break;

    case 'f':   // Automatic regen collection, battery disconnected, slows train down to stop within required distance
      
    break;

    case 'g':   // Forward/backward drive, speed set by controller, batteries disconnected, power from capacitor
      
    break;
  }
}

void set_horn()   // turns horn on or off
{
  if(horn == 'H')   
    {
      digitalWrite(horn_pin,HIGH);
    }
  else
    {
      digitalWrite(horn_pin,LOW);
    }
}

void send_comms()
{
  Serial.write('s');
  Serial.write(actual_speed);
  Serial.write('v');
  Serial.write(battery_voltage);
  Serial.write('c');
  Serial.write(cap_voltage);
  Serial.write('p');
  Serial.write(pwm);
  time_last_comms_sent = millis();
}


#include <LiquidCrystal.h>

// Output Pin Definitions
#define sd_pin  2
#define pwm_pin 3
#define horn_pin  4
#define brakes_pin 5
#define power_pin 6
#define cap_pin 7
#define rheo_pin  8
#define spare_pin_1 9
#define spare_pin_2 10

// Analog Input Pin Definitions
#define battery_voltage_pin 2
#define cap_voltage_pin 3
#define current_sensor_pin  4
#define temp_sensor_pin 5

// LCD Display Setup
//LiquidCrystal lcd(50, 51, 49, 48, 47, 46);    // FOR ACTUAL LCD
LiquidCrystal lcd(12, 11, 10, 9, 8, 7);  // FOR TESTING

// Digital Input Pin Definitions
#define autostop_pin  13  // IR sensor    // THIS PIN NEEDS CONNECTING TO/SWAPPING FOR PIN 19 FOR INTERRUPT TO WORK
#define autostop_pin_interrupt  19
#define encoder_1a  25   // THIS PIN NEEDS CONNECTING TO/SWAPPING FOR PIN 18 FOR INTERRUPT TO WORK
#define encoder_1a_interrupt  18
#define encoder_1b  26
#define encoder_1i  27  // Index
#define encoder_2a  28
#define encoder_2a_interrupt  21
#define encoder_2b  29
#define encoder_2i  30   // Index

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

volatile boolean autostop_mode = 0;
volatile long pulse_count = 0;

long loop_start_pulse_count;
long loop_pulse_count;

// Sent Variables
int actual_speed = 0;
int battery_voltage = 48;
int cap_voltage = 0;

// Timing Variables
unsigned long loop_start_time = 0;
const unsigned long loop_period = 10;   // sets loop period and hence sample rate
unsigned long time_last_comms_received = 0;
const unsigned long timeout_period = 100;  // sets timeout for loss of communications
unsigned long time_last_comms_sent = 0;
const unsigned long send_period = 500;     // sets data send interval

// Temperature Threshold
const int temp_threshold = 150;   // temp ~0.4883 * reading from pin. Hence threshold of 150 is approx 75 degrees C.

void setup() 
{
  Serial.begin(9600);
  TCCR3B = (TCCR3B & B11111000) | B00000001;    // 32KHz PWM frequency
  attachInterrupt(digitalPinToInterrupt(encoder_1a_interrupt), read_encoder1, RISING);
  // attachInterrupt(digitalPinToInterrupt(encoder_2a_interrupt), read_encoder2, RISING);   // Use if encoder 1 is faulty
  lcd.begin(16,2);
}

void loop() 
{
    loop_start_pulse_count = pulse_count;
    loop_start_time = millis();

    actual_speed = 2.20893*loop_pulse_count;    // constant = 0.2650872/(G*T) where G is the gear ratio between the encoder and the wheel and T is the loop period. Outputs speed such that 150 = 15km/h
     
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

    if (state == 'c' || (state == 'd' && autostop_mode == 0) || state == 'e' || state == 'g')
    {
      // pwm set by control system to regulate speed
    }
    else if(state == 'd' && autostop_mode == 1)
    {
      // set pwm to achieve correct position
    }
    else if (state == 'f')
    {
      // Who knows how regen works...
      // try to keep current flowing into the capacitor for as long as possible
    }
    
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

    loop_pulse_count = pulse_count - loop_start_pulse_count;
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
  detachInterrupt(digitalPinToInterrupt(autostop_pin_interrupt));   // Prevents autostop when not in autostop state
  autostop_mode = 0;    // Resets allowing autostop to be entered again
  switch(state)
  {
    case 'a': // Neutral, parking brake on
      digitalWrite(sd_pin,LOW);   // MOSFETS off
      digitalWrite(brakes_pin,LOW);   // Brakes on
      digitalWrite(power_pin,LOW);    // Batteries disconnected
      digitalWrite(cap_pin,LOW);    // Capacitor isolated from motors
      if(current_state = 'f')
      {
        digitalWrite(rheo_pin,HIGH);   // Prevents capacitor discharging if deadman is released when the previous state is regen-collect.
      }
      else
      {
        digitalWrite(rheo_pin,LOW);   // Rheo resistors connected to capacitor
      }
      digitalWrite(spare_pin_1,LOW);    // Resistors connected to ground to discharge capacitor
      pwm = 128;
    break;

    case 'b':   // Neutral (shutdown low)
      digitalWrite(sd_pin,LOW);   // MOSFETS off
      digitalWrite(brakes_pin,HIGH);   // Brakes off
      digitalWrite(power_pin,LOW);    // Batteries disconnected
      digitalWrite(cap_pin,LOW);    // Capacitor isolated from motors
      digitalWrite(rheo_pin,LOW);   // Rheo resistors connected to capacitor
      digitalWrite(spare_pin_1,LOW);    // Resistors connected to ground to discharge capacitor
      pwm = 128;
    break;
    
    case 'c':   // Standard, forward/backward drive, speed set by controller
      digitalWrite(sd_pin,HIGH);   // MOSFETS set by PWM
      digitalWrite(brakes_pin,HIGH);   // Brakes off
      digitalWrite(power_pin,HIGH);    // Batteries connected
      digitalWrite(cap_pin,LOW);    // Capacitor isolated from motors
      digitalWrite(rheo_pin,LOW);   // Rheo resistors connected to capacitor
      digitalWrite(spare_pin_1,LOW);    // Resistors connected to ground to discharge capacitor
    break;

    case 'd':   // Standard, forward/backward drive, speed set by controller, but enters auto stop when signal recieved
      digitalWrite(sd_pin,HIGH);   // MOSFETS set by PWM
      digitalWrite(brakes_pin,HIGH);   // Brakes off
      digitalWrite(power_pin,HIGH);    // Batteries connected
      digitalWrite(cap_pin,LOW);    // Capacitor isolated from motors
      digitalWrite(rheo_pin,LOW);   // Rheo resistors connected to capacitor
      digitalWrite(spare_pin_1,LOW);    // Resistors connected to ground to discharge capacitor
      attachInterrupt(digitalPinToInterrupt(autostop_pin_interrupt), autostop, RISING);
    break;

    case 'e':   // Forward/backward drive, speed set by controller, but capacitor disconnected and emptied
      digitalWrite(sd_pin,HIGH);   // MOSFETS set by PWM
      digitalWrite(brakes_pin,HIGH);   // Brakes off
      digitalWrite(power_pin,HIGH);    // Batteries connected
      digitalWrite(cap_pin,LOW);    // Capacitor isolated from motors
      digitalWrite(rheo_pin,LOW);   // Rheo resistors connected to capacitor
      digitalWrite(spare_pin_1,LOW);    // Resistors connected to ground to discharge capacitor
    break;

    case 'f':   // Automatic regen collection, battery disconnected, slows train down to stop within required distance
      digitalWrite(sd_pin,HIGH);   // MOSFETS set by PWM
      digitalWrite(brakes_pin,HIGH);   // Brakes off
      digitalWrite(power_pin,LOW);    // Batteries disconnected
      digitalWrite(cap_pin,LOW);    // Capacitor isolated from motors
      digitalWrite(rheo_pin,LOW);   // Rheo resistors connected to capacitor
      digitalWrite(spare_pin_1,HIGH);    // Resistors connected to motors to charge capacitor
    break;

    case 'g':   // Forward/backward drive, speed set by controller, batteries disconnected, power from capacitor
      digitalWrite(sd_pin,HIGH);   // MOSFETS set by PWM
      digitalWrite(brakes_pin,HIGH);   // Brakes off
      digitalWrite(power_pin,LOW);    // Batteries diconnected
      digitalWrite(cap_pin,HIGH);    // Capacitor connected directly to motors
      digitalWrite(rheo_pin,HIGH);   // Rheo resistors disconnected from capacitor
      digitalWrite(spare_pin_1,LOW);    // Resistors connected to ground - does nothing
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

void read_encoder1() // increments/decrements pulse count (position) depending on direction of rotation
{
  if(digitalRead(encoder_1b)==HIGH)
  {
    pulse_count++;
  }
  else
  {
    pulse_count--;
  }
}
  
/* Uncomment if encoder 1 does not work
void read_encoder2() // increments/decrements pulse count (position) depending on direction of rotation
{
  if(digitalRead(encoder_2b)==HIGH)
  {
    pulse_count++;
  }
  else
  {
    pulse_count--;
  }
}
*/

void autostop()
{
  pulse_count = 0;
  autostop_mode = 1;
  detachInterrupt(digitalPinToInterrupt(autostop_pin_interrupt));   // Prevents autostop triggering again
}

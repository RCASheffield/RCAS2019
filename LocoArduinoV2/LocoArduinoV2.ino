
#include <LiquidCrystal.h>

// Output Pin Definitions
#define safety_pin 10
#define brakes_pin 9
#define 1A 6
#define 1B 8
#define 2A 5
#define 2B 7
#define pwm_pin 3

#define horn_pin  4

// Analog Input Pin Definitions
#define battery_voltage_pin 2
// #define temp_sensor_pin 5

// LCD Display Setup
LiquidCrystal lcd(50, 51, 49, 48, 47, 46);

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

// Recieved Variables
int desired_speed = 0;
char state = 'a';
char horn = 'h';

// State and PWM variables
char current_state = 'a';
int pwm = 128;   // Output to MOSFET drivers
long integral_error = 0;
int setpoint = 0;

// Interrupt Variables
volatile boolean autostop_mode = 0;
volatile long pulse_count = 0;

// Sent Variables
int actual_speed = 0;
int battery_voltage = 48;
int cap_voltage = 0;

// Temperature
const int temp_threshold = 150;   // temp ~0.4883 * reading from pin. Hence threshold of 150 is approx 75 degrees C.
int temperature = 50;

// Other Measured Values
int current_reading = 0;

// Loop Variables
long loop_start_pulse_count;
long loop_pulse_count;

// Timing Variables
unsigned long loop_start_time = 0;
const unsigned long loop_period = 10;   // sets loop period and hence sample rate
unsigned long time_last_comms_received = 0;
const unsigned long timeout_period = 100000000000;  // sets timeout for loss of communications
unsigned long time_last_comms_sent = 0;
const unsigned long send_period = 500;     // sets data send interval

void setup()
{
  pin_definitions();
  digitalWrite(safety_pin, HIGH); //Safety relay
  Serial.begin(9600);   // Must match controller arduino
  //TCCR3B = (TCCR3B & B11111000) | B00000001;    // 31.25 KHz PWM frequency
  //attachInterrupt(digitalPinToInterrupt(encoder_1a_interrupt), read_encoder1, RISING);    // Setup interrupt for encoder 1
  // attachInterrupt(digitalPinToInterrupt(encoder_2a_interrupt), read_encoder2, RISING);   // Use if encoder 1 is faulty
  lcd.begin(16, 2);
}

void loop()
{
  loop_start_pulse_count = pulse_count;
  loop_start_time = millis();

  actual_speed = 2.20893 * loop_pulse_count;  // constant = 0.2650872/(G*T) where G is the gear ratio between the encoder and the wheel and T is the loop period. Outputs speed such that 150 = 15km/h

  receive_comms();    // check for new commands
  if (millis() - time_last_comms_received > timeout_period || temperature > temp_threshold)  // shutdown loco if communication is lost or in case of overheating
  {
    state = 'a';
  }

  if (state != current_state)   // change variables only on state change within state change function
  {
    lcd.clear();
    state_change();
    current_state = state;
  }

  set_horn();


  if (state == 'c' || (state == 'd' && autostop_mode == 0) || state == 'e' || state == 'g')   // pwm set by control system to regulate speed
  {
    analogWrite(pwm_pin, desired_speed + 127);
    //pwm = set_pwm(desired_speed,actual_speed);
    //analogWrite(pwm_pin,pwm);
  }
  else if (state == 'd' && autostop_mode == 1)
  {
    // set pwm to achieve correct position
  }
  
  lcd.setCursor(0, 0);
  lcd.print(desired_speed);
  lcd.print("     ");
  lcd.setCursor(0, 1);
  lcd.print(state);
  Serial.print(state);
  Serial.print(desired_speed);
  Serial.println();


  if (millis() - time_last_comms_sent > send_period)  // sends communications at fixed period
  {
    // battery_voltage = analogRead(battery_voltage_pin);
    //   cap_voltage = analogRead(cap_voltage_pin);
    //temperature = analogRead(temp_sensor_pin);
    // send_comms();
  }

  while (millis() - loop_start_time < loop_period)  // fixes loop length
  {
  }

  loop_pulse_count = pulse_count - loop_start_pulse_count;
}

void receive_comms()
{
  while (Serial.available() > 0)  // Command word: speed,state,horn e.g. 500bh - speed 500, state b, horn off; 0aH - speed 0, state a, horn on; etc.
  {
    desired_speed = Serial.parseInt();
    state = Serial.read();
    horn = Serial.read();
    time_last_comms_received = millis();
  }
}

void state_change()   // Runs once on state change
{
  lcd.setCursor(15, 1);
  lcd.print(state);
  detachInterrupt(digitalPinToInterrupt(autostop_pin_interrupt));   // Prevents autostop when not in autostop state
  autostop_mode = 0;    // Resets allowing autostop to be entered again
  integral_error = 0;
  setpoint = 0;
  switch (state)
  {
    case 'a': // Neutral, parking brake on
      digitalWrite(brakes_pin, LOW);  // Brakes on
      digitalWrite(power_pin, LOW);   // Batteries disconnected
      //digitalWrite(cap_pin, LOW);   // Capacitor isolated from motors
      if (current_state = 'f')
      {
        digitalWrite(rheo_pin, HIGH);  // Prevents capacitor discharging if deadman is released when the previous state is regen-collect.
      }
      else
      {
        digitalWrite(rheo_pin, LOW);  // Rheo resistors connected to capacitor
      }
      digitalWrite(spare_pin_1, LOW);   // Resistors connected to ground to discharge capacitor
      pwm = 128;
      break;

    case 'b':   // Neutral (shutdown low)
      digitalWrite(sd_pin, LOW);  // MOSFETS off
      digitalWrite(brakes_pin, HIGH);  // Brakes off
      digitalWrite(power_pin, LOW);   // Batteries disconnected
      digitalWrite(cap_pin, LOW);   // Capacitor isolated from motors
      digitalWrite(rheo_pin, LOW);  // Rheo resistors connected to capacitor
      digitalWrite(spare_pin_1, LOW);   // Resistors connected to ground to discharge capacitor
      pwm = 128;
      break;

    case 'c':   // Standard, forward/backward drive, speed set by controller
      digitalWrite(sd_pin, HIGH);  // MOSFETS set by PWM
      digitalWrite(brakes_pin, HIGH);  // Brakes off
      digitalWrite(power_pin, HIGH);   // Batteries connected
      digitalWrite(cap_pin, LOW);   // Capacitor isolated from motors
      digitalWrite(rheo_pin, LOW);  // Rheo resistors connected to capacitor
      digitalWrite(spare_pin_1, LOW);   // Resistors connected to ground to discharge capacitor
      break;

    case 'd':   // Standard, forward/backward drive, speed set by controller, but enters auto stop when signal recieved
      digitalWrite(sd_pin, HIGH);  // MOSFETS set by PWM
      digitalWrite(brakes_pin, HIGH);  // Brakes off
      digitalWrite(power_pin, HIGH);   // Batteries connected
      digitalWrite(cap_pin, LOW);   // Capacitor isolated from motors
      digitalWrite(rheo_pin, LOW);  // Rheo resistors connected to capacitor
      digitalWrite(spare_pin_1, LOW);   // Resistors connected to ground to discharge capacitor
      attachInterrupt(digitalPinToInterrupt(autostop_pin_interrupt), autostop, RISING);
      break;

    default:    // In case of error
      digitalWrite(sd_pin, LOW);  // MOSFETS off
      digitalWrite(brakes_pin, LOW);  // Brakes on
      digitalWrite(power_pin, LOW);   // Batteries disconnected
      digitalWrite(cap_pin, LOW);   // Capacitor isolated from motors
      digitalWrite(rheo_pin, LOW);  // Rheo resistors connected to capacitor
      digitalWrite(spare_pin_1, LOW);   // Resistors connected to ground to discharge capacitor
      pwm = 128;
      break;
  }
}

void set_horn()   // turns horn on or off
{
  if (horn == 'H')
  {
    digitalWrite(horn_pin, HIGH);
  }
  else
  {
    digitalWrite(horn_pin, LOW);
  }
}

void send_comms()
{
  Serial.write('s');
  Serial.write(actual_speed);
  Serial.write('v');
  Serial.write(battery_voltage);
  Serial.write('p');
  Serial.write(pwm);
  time_last_comms_sent = millis();
}

void read_encoder1() // increments/decrements pulse count (position) depending on direction of rotation
{
  if (digitalRead(encoder_1b) == HIGH)
  {
    pulse_count++;
  }
  else
  {
    pulse_count--;
  }
}

void autostop()
{
  pulse_count = 0;
  autostop_mode = 1;
  detachInterrupt(digitalPinToInterrupt(autostop_pin_interrupt));   // Prevents autostop triggering again
}

int set_pwm(int requested, int actual)
{
  const float kp = 0.8;
  const float ki = 0.02;
  const float alpha = 0.04;    // Averaging filter constant
  int control_signal = 0;
  int error = 0;
  setpoint = alpha * requested + (1 - alpha) * setpoint; // averaging filter to prevent large step changes in input
  error = setpoint - actual;
  control_signal = 128 + kp * error + ki * integral_error;
  if (control_signal > 255)
  {
    integral_error = (requested - 128 - kp * error) / ki; // anti-windup measure
    return 255;
  }
  else if (control_signal < 0)
  {
    integral_error = (requested - 128 - kp * error) / ki; // anti-windup measure
    return 0;
  }
  else
  {
    integral_error = integral_error + error;
    return control_signal;
  }
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

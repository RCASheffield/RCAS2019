
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
boolean deadman = 1;
char drive;
boolean regen;
char mode;
boolean auto_stop;
int desired_speed;

// Sent Variables
int actual_speed = 0;
int battery_voltage = 48;
int cap_voltage = 0;

// Timing Variables
boolean exit_flag;
boolean first_loop;
const unsigned long timeout = 10000;
unsigned long deadman_time = 0;
const unsigned long data_refresh_time = 250;
unsigned long send_time = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  common_function();
  lcd.begin(16,2);
  lcd.clear();
  lcd.setCursor(0,1);
  lcd.print("Initialising...");
}

void loop() {
  exit_flag=0;
  first_loop = 0;
  lcd.clear();
  lcd.setCursor(0,1);
  while(exit_flag==0&&drive=='N'){  // Neutral, shutdown pin low
    if(first_loop == 0){  // Code here executes once
      lcd.print("Neutral");
      first_loop = 1;
    }                     // Code here repeats
  common_function();
  
  }
  
  
  while(exit_flag==0&&drive!='N'){
    while(exit_flag==0&&regen==0){
      while(exit_flag==0&&auto_stop==0){
      // Standard, forward/backward drive, speed set by controller
      common_function();
      
      }
      while(exit_flag==0&&auto_stop==1){
      // Standard, forward/backward drive, speed set by controller, but enters auto stop when signal recieved
      common_function();
      
      }
    }
    while(exit_flag==0&&regen==1){
      while(exit_flag==0&&mode=='D'){
      // Forward/backward drive, speed set by controller, but capacitor disconnected and emptied
      common_function();
      
      }
      while(exit_flag==0&&mode=='C'){
      // Automatic regen collection, battery disconnected, slows train down to stop within required distance
      common_function();
      
      }
      while(exit_flag==0&&mode=='U'){
      // Forward/backward drive, speed set by controller, batteries disconnected, power from capacitor
      common_function();
      
      }
    }
  }
}

void common_function(){
  communications();
  while((millis()-deadman_time) > timeout || deadman == 0){ // deadman timeout
    // Neutral, parking brake on
    lcd.clear();
    lcd.setCursor(0,1);
    lcd.print("Emergency");
    communications(); // will continue in shutdown mode until deadman is pressed again
  }
}

void communications(){
  // Received data
  char incoming_byte;
  while (Serial.available()>0){
    incoming_byte = Serial.read();
    if (incoming_byte=='X'){
      deadman = 1;
      deadman_time = millis();
    }
    if (incoming_byte == 'x'){
      deadman = 0;
    }
    if(incoming_byte=='h'){
      digitalWrite(horn_pin,0);
    }
    if(incoming_byte=='H'){
      digitalWrite(horn_pin,1);
    }
    if(incoming_byte=='N'){
      drive = 'N';
      exit_flag = 1;
    }
    if(incoming_byte=='F'){
      drive = 'F';
      exit_flag = 1;
    }
    if(incoming_byte=='B'){
      drive = 'B';
      exit_flag = 1;
    }
    if(incoming_byte=='r'){
      regen = 0;
      exit_flag = 1;
    }
    if(incoming_byte=='R'){
      regen = 1;
      exit_flag = 1;
    }
    if(incoming_byte=='D'){
      mode = 'D';
      exit_flag = 1;
    }
    if(incoming_byte=='C'){
      mode = 'C';
      exit_flag = 1;
    }
    if(incoming_byte=='U'){
      mode = 'U';
      exit_flag = 1;
    }
    if(incoming_byte=='a'){
      auto_stop = 0;
      exit_flag = 1;
    }
    if(incoming_byte=='A'){
      auto_stop = 1;
      exit_flag = 1;
    }
    if(incoming_byte=='s'){
      desired_speed = Serial.parseInt();
    }
    
  }
  // Send data
    if((millis()-send_time) > data_refresh_time){
      Serial.write('s');
      Serial.write(actual_speed);
      Serial.write('v');
      Serial.write(battery_voltage);
      Serial.write('c');
      Serial.write(cap_voltage);
      Serial.write('p');
      Serial.write(pwm);
      send_time = millis();
    }
}

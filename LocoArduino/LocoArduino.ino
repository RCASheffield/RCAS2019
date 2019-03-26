// Output pin definitions
const int horn_pin = 4;

// Output Variables
boolean brakes; // BRAKES relay
boolean power;  // POWER ISOLATE relay
boolean cap;   // CAP ISOLATE relay
boolean rheo; // RHEO relay
boolean sd;  // Driver shutdown pins
int pwm;   // Output to MOSFET drivers

// Recieved Variables
boolean deadman;
char drive;
boolean regen;
char mode;
boolean auto_stop;
int desired_speed;

int state;
boolean exit_flag;

const int timeout = 100;
unsigned long deadman_time;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  common_function();
}

void loop() {
  exit_flag=0;
  while(exit_flag==0&&drive=='N'){
  // Neutral, shutdown pin low
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
  while((millis()-deadman_time) > timeout){ // deadman timeout
    // Neutral, parking brake on
    communications(); 
  }
}

void communications(){
  // Received data
  while (Serial.available()>0){
    incoming_byte = Serial.read();
    if (incoming_byte=='X'){
      deadman = 1;
      deadman_time = millis();
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
    // Send data
    
}

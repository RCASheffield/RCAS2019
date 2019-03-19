
// Output Variables
boolean brakes; // BRAKES relay
boolean power;  // POWER ISOLATE relay
boolean cap;   // CAP ISOLATE relay
boolean rheo; // RHEO relay
boolean sd;  // Driver shutdown pins
int pwm;   // Output to MOSFET drivers

// Recieved Variables
boolean deadman;
boolean horn; // Horn drive transistor (recieved directly)
char drive;
boolean regen;
char mode;
boolean auto_stop;
int desired_speed;

int state;

void state_activate (int state){
  if (state == 1 ){
    brakes = 0;
    power = 1;
    cap = 1;
    rheo = 1;
  }
  if (state == 2 ){
    brakes = 1;
    power = 1;
    cap = 1;
    rheo = 1;
  }
}
void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:
 if (deadman == 0){
  //Neutral & Parking brake on (STATE 1)
  state_activate (1);
 }
 else {
  if (drive == N) {
    //Neutral (Shutdown low) (STATE 2)
    state_activate (2);
  }
  else (
 }
}

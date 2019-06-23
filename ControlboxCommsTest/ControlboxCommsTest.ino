// Loop variables
unsigned long loop_start_time = 0;
const int loop_period = 40;
 
// Sent variables
int desired_speed = 0;    // speed setpoint
char horn = 'h';    // h - off, H - on
char drive = 'S';    // S - stop, N - neutral, F - forward, R - reverse
char autostop = 'a';    // a - off, A - on

char command[4];

void setup() 
{
  Serial.begin(9600);
  
}

void loop() 
{
  loop_start_time = millis();
  desired_speed = 150;
  Serial.print(',');
  Serial.print(char(desired_speed));
  Serial.print('H');
  Serial.print('F');
  Serial.print('A');    
  while(millis() - loop_start_time < loop_period) // Fixes loop time to 50ms
  {
  }
  
}

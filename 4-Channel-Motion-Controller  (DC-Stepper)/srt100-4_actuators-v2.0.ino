// Stepper microstepping settings :  3200
// M1 is at the front right, M2 at the rear and M3 at the front left
// totalstep = 49000
// v2.0 Licence CC BY NC SA
// www.lebois-racing.fr/en

#define RelayPin  A0 //used to control the PSU thougth a relay

// WIRING // IT CAN'T BE CHANGED
#define StepPin1  2 //Step or pulse pin, check how DirectionManager is written.
#define DirPin1   3
#define StepPin2  4
#define DirPin2   5
#define StepPin3  6
#define DirPin3   7
#define StepPin4  8
#define DirPin4   9


#define pulseDelay 6 // mostly set the speed of the actuator : the lower the faster. Don't go below 6 or it will fail.
#define directionDelay 6

byte buffer           = 0 ;    // It takes the value of the serial data
byte buffercount      = 0 ;    // To count where we are in the serial datas
byte commandbuffer[8] = {0};   // To stock the serial datas in the good order.   

unsigned m1Target = 0,   m2Target = 0,   m3Target = 0,   m4Target = 0; 
unsigned m1Position = 0, m2Position = 0, m3Position = 0, m4Position = 0;
byte pulseM1 = B00000000, pulseM2 = B00000000, pulseM3 = B00000000, pulseM4 = B00000000;
int  dir1 = 1, dir2 = 1, dir3 = 1, dir4 = 1;         //Will be used to set the motors direction
byte dirChange = 0;
bool noData = true;

void setup() {
  Serial.begin(921600);    //To communicate with FlyPT. In FlyPT, put that number in "Serial speed".

  pinMode(StepPin1, OUTPUT);
  pinMode(DirPin1, OUTPUT);
  digitalWrite(DirPin1, HIGH);

  pinMode(StepPin2, OUTPUT);
  pinMode(DirPin2, OUTPUT);
  digitalWrite(DirPin2, HIGH);

  pinMode(StepPin3, OUTPUT);
  pinMode(DirPin3, OUTPUT);
  digitalWrite(DirPin3, HIGH);  

  pinMode(StepPin4, OUTPUT);
  pinMode(DirPin4, OUTPUT);
  digitalWrite(DirPin4, HIGH);  
  
  pinMode(RelayPin, OUTPUT);
  digitalWrite(RelayPin, LOW);       //LOW disable the  psu
  startPSU();
}

void loop() {
  SerialReaderC();                     //Get the datas from Simtools
  moveMotorC();
}

void SerialReaderC() {       // This function is the work of Sirnoname. Simtools output : P<Axis1a><Axis2a>, Data bits : 10 bits, Parity : None, stop bits : 1
  while (Serial.available())
  {
    if (buffercount == 0)
    {
      buffer = Serial.read();

      if (buffer != 'P') {
        controlPSU();
        buffercount = 0; // "P" is the marquer. If we read P, the next data is Motor1
      } else {
        buffercount = 1;
      }
    }
    else   //  if(buffercount>=1)
    {
      buffer = Serial.read();
      commandbuffer[buffercount-1] = buffer; // The first value next to "P" is saved in commandbuffer in the place "buffercount"
      buffercount++;
      if (buffercount >= 9) {
          m1Target = commandbuffer[0] * 256 + commandbuffer[1];
          m2Target = commandbuffer[2] * 256 + commandbuffer[3];
          m3Target = commandbuffer[4] * 256 + commandbuffer[5];
          m4Target = commandbuffer[6] * 256 + commandbuffer[7];
          buffercount = 0;
        break;
      }
    }
  }
}
void moveMotorC() {
  directionManagerC();
  singleStepC();
}

void directionManagerC() { 
  if ((m1Target < m1Position) && (dir1 == 1)) {
    PORTD &= B11110111;
    dir1 = -1;
    dirChange = 1;
  }

  if ((m1Target > m1Position ) && (dir1 == -1)) {
    PORTD |= B00001000;
    dir1 = 1;
    dirChange = 1;
  }

  if ((m2Target < m2Position) && (dir2 == 1)) {
    PORTD &= B11011111;
    dir2 = -1;
    dirChange = 1;
  }
  
  if ((m2Target > m2Position ) && (dir2 == -1)) {
    PORTD |= B00100000;
    dir2 = 1;
    dirChange = 1;
  }

  if ((m3Target < m3Position) && (dir3 == 1)) {
    PORTD &= B01111111;
    dir3 = -1;
    dirChange = 1;
  }
  
  if ((m3Target > m3Position ) && (dir3 == -1)) {
    PORTD |= B10000000;
    dir3 = 1;
    dirChange = 1;
  }

  if ((m4Target < m4Position) && (dir4 == 1)) {
    PORTB &= B11111101;
    dir4 = -1;
    dirChange = 1;
  }
  
  if ((m4Target > m4Position ) && (dir4 == -1)) {
    PORTB |= B00000010;
    dir4 = 1;
    dirChange = 1;
  }
  
  if (dirChange == 1) {
    delayMicroseconds(directionDelay);
    dirChange = 0;
  }
}

void singleStepC() {
  if (m1Target != m1Position) {
    pulseM1 = B00000100;
    m1Position += dir1;
  }
  else{pulseM1 = B00000000;}
  
  if (m2Target != m2Position) {
    pulseM2 = B00010000;
    m2Position += dir2;
  }
  else{pulseM2 = B00000000;}
    
  if (m3Target != m3Position) {
        pulseM3 = B01000000;
    m3Position += dir3;
  }
  else{pulseM3 = B00000000;}

  if (m4Target != m4Position) {
        pulseM4 = B00000001;
    m4Position += dir4;
  }
  else{pulseM4 = B00000000;}
    
  if((pulseM1 == B00000000) &&(pulseM2 == B00000000)&&(pulseM3 == B00000000)&& (pulseM4 == B00000000)){return;}
      PORTD |= pulseM1 | pulseM2 | pulseM3 | pulseM4;
      PORTB |= pulseM4;
  delayMicroseconds(pulseDelay);
      PORTD &= B10101011;
      PORTB &= B11111110;
}


void startPSU() {
  digitalWrite(RelayPin, LOW);
  while (noData == true) {
    if (Serial.available()) {
      if (Serial.read() == 'A'){
        if (Serial.read() == 'C'){
      noData = false;
      digitalWrite(RelayPin, HIGH);}
    }
    }
  }
}

void controlPSU() {
  if (buffer == 'E') {
          buffer = Serial.read();
                buffer = Serial.read();
                      buffer = Serial.read();
                            buffer = Serial.read();
                                  buffer = Serial.read();
    noData = true;
    digitalWrite(RelayPin, LOW);
    startPSU();
  }  }

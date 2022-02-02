// Arduino-board : Leonardo
// Stepper driver: HBS86H
// Stepper microstepping settings :  3200
// M1 : Surge
// totalstep = 49000


// WIRING // IT CAN'T BE CHANGED
#define StepPin1  2 //Step or pulse pin, check how DirectionManager is written.
#define DirPin1   3

#define RelayPin  A0 //used to control the PSU thougth a relay

#define pulseDelay 6 // mostly set the speed of the actuator : the lower the faster. Don't go below 6 or it will fail.
#define directionDelay 6

byte buffer           = 0 ;    // It takes the value of the serial data
byte buffercount      = 0 ;    // To count where we are in the serial datas
byte commandbuffer[8] = {0};   // To stock the serial datas in the good order.   

unsigned m1Target = 0; 
unsigned m1Position = 0;
byte pulseM1 = B00000000;
int  dir1 = 1;         //Will be used to set the motors direction
byte dirChange = 0;
bool noData = true;

void setup() {
  Serial.begin(921600);    //To communicate with FlyPT. In FlyPT, put that number in "Serial speed".

  pinMode(StepPin1, OUTPUT);
  pinMode(DirPin1, OUTPUT);
  digitalWrite(DirPin1, HIGH);
  
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
}

void singleStepC() {
  if (m1Target != m1Position) {
    pulseM1 = B00000100;
    m1Position += dir1;
  }
  else{pulseM1 = B00000000;}
    
  if((pulseM1 == B00000000) ){return;}
      PORTD |= pulseM1; 
  delayMicroseconds(pulseDelay);
      PORTD &= B10101011;
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
  }  
  }
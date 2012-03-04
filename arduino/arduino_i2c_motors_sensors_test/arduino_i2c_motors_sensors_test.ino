 
#include <Servo.h> 
#include <Wire.h>

//Arduino's Address
#define I2C_SLAVE_ADDRESS 0x10

//Motor declarations
Servo motorLeft;
Servo motorRight;
//Servo motorBottom;

const int pinLeftMotor = 10;
const int pinRightMotor = 11;
//const int pinBottomMotor = 9;

// pin 11 and 18 are control lines to reverse left and right motors

//Sonar declarations 
//A0---> digital pin 3
//A1---> digital pin 4
//A2---> digital pin 8
//A3---> digital pin 7
//A4---> digital pin ???


int sonarFrontValue = 0;
int sonarLeftValue = 0;
int sonarRightValue = 0;
int sonarBottomValue = 0;

int sonarFrontAnalogPin = 0;
int sonarLeftAnalogPin = 1;
int sonarRightAnalogPin = 2;
int sonarBottomAnalogPin = 3;

int sonarFrontCS = 3;
int sonarLeftCS = 4;
int sonarRightCS = 8;
int sonarBottomCS = 7;

int digitalPinShift = 3; // pin number at which sonar row starts, 0,1 are reserved for RX,TX
int analogPinShift = 0; //analog pins start at A0
int sonarChipSelectArray[5] = {3,4,8,7,0};

int sonarSum = 0;
int sonarMotorThreshold = 50;

boolean isMotorOn = false;

/*
  The Arduino as an I2C slave.
  Echo back data received.
*/

// BUFFER_LENGTH comes from Wire.h
// It is the default Arduino I2C size limit for a single transfer.
// Larger transfers need to be broken up.
// Not doing that here.
uint8_t buff[BUFFER_LENGTH];

int pos;

// If the buffer overflows, reset to zero ;-)
void onRecieveHandler(int numBytes)
{
    for (int i = 0; i < numBytes; i++) {
        buff[pos] = Wire.read();
        pos++;

        if (pos >= BUFFER_LENGTH)
            pos = 0; 
    }
}

void onRequestHandler()
{
    if (!pos)
        return;
        
    //for(int i = 0; i < pos; i++)
    //    buff[i] = toupper(buff[i]);    
        
    Wire.write(buff, pos);
    pos = 0; 
}

/*

  With power means, enable pins 2 and 3 for ground and
  power respectively. Puts power close to the 4,5 pins
  used for I2C. Set withPower=0 if you don't want this.

*/

void begin_i2c(int withPower)
{
    byte pwrpin = PORTC3;
    byte gndpin = PORTC2;
  
    if (withPower) {
        DDRC |= _BV(pwrpin) | _BV(gndpin);  // make outputs
        PORTC &=~ _BV(gndpin);
        PORTC |=  _BV(pwrpin);
        delay(100);
    }
  
    Wire.begin(I2C_SLAVE_ADDRESS);
    
    Wire.onReceive(onRecieveHandler);
    Wire.onRequest(onRequestHandler);
}

//Setup
void setup() 
{ 
  //Serial setup
  Serial.begin(9600);

  //I2C Setup
  digitalWrite(SDA, 0);
  digitalWrite(SCL, 0);
  begin_i2c(0);
  
  // Setting all sonars to NON - range mode --> LOW == off
  pinMode(sonarFrontCS,OUTPUT); 
  digitalWrite(sonarFrontCS,LOW);

  pinMode(sonarLeftCS,OUTPUT); 
  digitalWrite(sonarLeftCS,LOW); 

  pinMode(sonarRightCS,OUTPUT); 
  digitalWrite(sonarRightCS,LOW); 

  pinMode(sonarBottomCS,OUTPUT); 
  digitalWrite(sonarBottomCS,LOW); 

  //Motor setup
  //Attaching motors to digital pins (PWM pins only)
  motorLeft.attach(pinLeftMotor);
  motorRight.attach(pinRightMotor);
  //motorBottom.attach(pinBottomMotor);

  Serial.println("Motors are attached");
  digitalWrite(13, HIGH);
  delay(1000);
  digitalWrite(13, LOW);

  
  //Motor arming sequence
  motorLeft.write(10);
  motorRight.write(10);
  //motorBottom.write(10);

  //wait for motors to ARM.
  delay(2000);

  //Setup complete, turn on LED to notify
  pinMode(13,OUTPUT);
  digitalWrite(13, HIGH);
  Serial.println("Motors are ARMED");
} 


void readSonars()
{
  //Handle sonars
  //Unroled loop version
  
  //activate sonar  
  int i = 0;
  
  digitalWrite(sonarFrontCS, HIGH);
  // give it 42 us, it actually only needs 20us
  delayMicroseconds(22);
  // read the sonar value
  sonarFrontValue = analogRead(sonarFrontAnalogPin);
  // shut of the sonar
  digitalWrite(sonarFrontCS, LOW);
  //cleanup time
  delay(100);
  
  i++;
  
  digitalWrite(sonarLeftValue, HIGH);
  // give it 42 us, it actually only needs 20us
  delayMicroseconds(22);
  // read the sonar value
  sonarLeftValue = analogRead(sonarLeftAnalogPin);
  // shut of the sonar
  digitalWrite(sonarLeftValue, LOW);
  //cleanup time
  delay(100);
  
  i++;
  
  digitalWrite(sonarRightValue, HIGH);
  // give it 42 us, it actually only needs 20us
  delayMicroseconds(22);
  // read the sonar value
  sonarRightValue = analogRead(sonarRightAnalogPin);
  // shut of the sonar
  digitalWrite(sonarRightValue, LOW);
  // cleanup time
  delay(100);
  
  i++;
  
  digitalWrite(sonarBottomValue, HIGH);
  // give it 42 us, it actually only needs 20us
  delayMicroseconds(22);
  // read the sonar value
  sonarBottomValue = analogRead(sonarBottomAnalogPin);
  // shut of the sonar
  digitalWrite(sonarBottomValue, LOW);
  // cleanup time
  delay(100);
  
  i++;
  
}
  
void writeOutSonars()
{
  Serial.print("Sonar Front: ");
  Serial.print(sonarFrontValue);
  Serial.print("\n");
  
  Serial.print("Sonar Left: ");
  Serial.print(sonarLeftValue);
  Serial.print("\n");
  
  Serial.print("Sonar Right: ");
  Serial.print(sonarRightValue);
  Serial.print("\n");
  
  Serial.print("Sonar Bottom: ");
  Serial.print(sonarBottomValue);
  Serial.print("\n");
}
  

// Main loop
int sweepMin = 55;
int sweepMax = 60;
void loop() 
{
  
  readSonars();
  writeOutSonars();
   
  
  Serial.print("SonarSum: ");
  Serial.println(sonarSum);
  Serial.print("Motor Status: ");
  
  if(sonarFrontValue < sonarMotorThreshold && isMotorOn == false)
  {
   motorRight.write(60); 
   Serial.println("Active");
   isMotorOn = true;
  }
  else
  {
    motorRight.write(0);
    Serial.println("Off");
    isMotorOn = false;
  }
  
  //delimiter
  Serial.println("\n----------\n");  
}

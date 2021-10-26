/*
 *  Project:      HueHeader
 *  Description:  Hue light is turn on, displays colors of HueRainbow array const values.
 *                                                        and encoder changes brightness
 *  Authors:      Pedro Sanchez
 *  Date:         Oct 25 2021
 */
 
#include <SPI.h>
#include <Ethernet.h>
#include <mac.h>
#include <hue.h>
#include <Encoder.h>
#include <OneButton.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>


Adafruit_MPU6050 mpu;

// The "c" pin on the encoder is connected to GND
const int encoderRed = 3;   // OUTPUT from the Teensy
const int encoderGreen = 4;  // OUTPUT from the Teensy
const int encoderSW = 5; // SW means the  switch  Note: it is an INPUT to the Teensy

const int JOYSTICK = 7;
const int JOYSTICK_RX = A2;
const int JOYSTICK_RY = A3;

int joystickValue = 0;

const int BUTTONPIN = 23;
int  totalColors = sizeof(HueRainbow)/sizeof(HueRainbow[0]);
bool isOFF = true;
const int LIGHT = 2;
const int activeLOW = true;
const int pullUP = true;

Encoder myEnc(1,2);
OneButton myButton(BUTTONPIN , activeLOW , pullUP);

int encoderPosition;
int brightness;

void setup()
{


  // JoyStick
  
  pinMode(JOYSTICK, INPUT_PULLUP);

  Serial.begin(115200);
  while (!Serial);

  // MPU6050

  Serial.printf("Adafruit MPU6050 test!\n");

  // Try to initialize!
  if (!mpu.begin()) {
    Serial.printf("Failed to find MPU6050 chip\n");
    while (1) {
      delay(10);
    }
  }
  Serial.printf("MPU6050 Found!\n");  

  setupMPU6050();

  /*
  // Ethernet
  pinMode(10, OUTPUT);
  digitalWrite(10, HIGH);
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);

  // Encoder
  pinMode(encoderRed, OUTPUT);      // OUTPUT from the Teensy
  pinMode(encoderGreen, OUTPUT);    // OUTPUT from the Teensy
  pinMode(encoderSW, INPUT_PULLUP); // SW means the  switch  Note: it is an INPUT to the Teensy

  digitalWrite(encoderRed, HIGH);
  encoderPosition = myEnc.read();   // First Time the encoder is read 
//  previousPosition = encoderPosition;
  Serial.printf("Initial Econder position =  %d\n\n", encoderPosition);

  // Button
  myButton.attachClick(turn_OFF_or_ON);
  myButton.setClickTicks(250); 

  Serial.printf("Started Hue V1 with %i colors in the HueRainbow  \n", totalColors);


  Ethernet.begin(mac);
  delay(200);          //ensure Serial Monitor is up and running           
  printIP();
  Serial.printf("LinkStatus: %i  \n",Ethernet.linkStatus());
 */ 
}

void loop() {

  // JoyStick


//  joystickValue = analogRead(JOYSTICK_RX);       // read X axis value [0..1023]
//  Serial.printf("X: %04i", joystickValue);
//
//  joystickValue = analogRead(JOYSTICK_RY);       // read Y axis value [0..1023]
//  Serial.printf(" | Y: %04i", joystickValue);
//
//  joystickValue = digitalRead(JOYSTICK);        // read Button state [0,1]
//  Serial.printf(" | Button: %i\n", joystickValue);
//
//  delay(100);

  // MPU6050
  
  readMPU6050();

  
 /* 
  myButton.tick();
  // setHue function needs 5 parameters
  // int bulb - this is the bulb number
  //  bool activated - true for bulb on, false for off
  // int color - Hue color from hue.h
  //  int - brightness - from 0 to 255 
  //  int - saturation - from 0 to 255
  //

  encoderPosition = myEnc.read();
  if (encoderPosition<10) { 
    encoderPosition=10;
    myEnc.write(10);    
  }
  else {
    if (encoderPosition>255) {
       encoderPosition=255;
       myEnc.write(255);    
    }
  }
     brightness = encoderPosition;
*/
}

void setupMPU6050() {

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  Serial.printf("Accelerometer range set to: ");
  switch (mpu.getAccelerometerRange()) {
  case MPU6050_RANGE_2_G:
    Serial.printf("+-2G\n");
    break;
  case MPU6050_RANGE_4_G:
    Serial.printf("+-4G\n");
    break;
  case MPU6050_RANGE_8_G:
    Serial.printf("+-8G\n");
    break;
  case MPU6050_RANGE_16_G:
    Serial.printf("+-16G\n");
    break;
  }
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  Serial.printf("Gyro range set to: \n");
  switch (mpu.getGyroRange()) {
  case MPU6050_RANGE_250_DEG:
    Serial.printf("+- 250 deg/s\n");
    break;
  case MPU6050_RANGE_500_DEG:
    Serial.printf("+- 500 deg/s\n");
    break;
  case MPU6050_RANGE_1000_DEG:
    Serial.printf("+- 1000 deg/s\n");
    break;
  case MPU6050_RANGE_2000_DEG:
    Serial.printf("+- 2000 deg/s\n");
    break;
  }

  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  Serial.printf("Filter bandwidth set to: ");
  switch (mpu.getFilterBandwidth()) {
  case MPU6050_BAND_260_HZ:
    Serial.printf("260 Hz\n");
    break;
  case MPU6050_BAND_184_HZ:
    Serial.printf("184 Hz\n");
    break;
  case MPU6050_BAND_94_HZ:
    Serial.printf("94 Hz\n");
    break;
  case MPU6050_BAND_44_HZ:
    Serial.printf("44 Hz\n");
    break;
  case MPU6050_BAND_21_HZ:
    Serial.printf("21 Hz\n");
    break;
  case MPU6050_BAND_10_HZ:
    Serial.printf("10 Hz\n");
    break;
  case MPU6050_BAND_5_HZ:
    Serial.printf("5 Hz\n");
    break;
  }

  Serial.printf("\n");
  delay(100);
  
}

void readMPU6050() {
  /* Get new sensor events with the readings */
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  /* Print out the values */
  Serial.printf("Acceleration X: %.2f, Y: %.2f, Z: %.2f m/s^2\n", a.acceleration.x, a.acceleration.y, a.acceleration.z);

  Serial.printf("Rotation X: %.2f, Y: %.2f, , Z: %.2f rad/s\n", g.gyro.x, g.gyro.y, g.gyro.z );

  Serial.printf("Temperature: %.2f degC\n\n", temp.temperature);

  delay(500);  
}


void printIP() {
  Serial.printf("My IP address: ");
  for (byte thisByte = 0; thisByte < 3; thisByte++) {
    Serial.printf("%i.",Ethernet.localIP()[thisByte]);
  }
  Serial.printf("%i\n",Ethernet.localIP()[3]);
}

void turn_OFF_or_ON() {
//  Serial.printf("Pressed the buttom before: %d \n", isOFF);
  isOFF = !isOFF;
//  Serial.printf("brightness %i\n", brightness);
  if (!isOFF) { // if it's on and clicked for OFF
    for (int i=0; i<totalColors; i++) {
       setHue(LIGHT, true, HueRainbow[i], brightness, 255);
       delay(1000);
    }
  }
  else { // turn on rainbow
    delay(1000);
    setHue(LIGHT, false, 0, 0, 0);
  }
//  Serial.printf("Pressed the buttom after: %d \n", isOFF);
}

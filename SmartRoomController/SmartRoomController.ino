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

bool joystickButtonState  = false;
bool joystickClicked   = false;

int  previousButtonState = joystickButtonState;
int  previousX = 730;
int  previousY = 750;
bool p_xMoved = false;
bool p_yMoved = false;
int directionXY = 6; // Still
int p_directionXY = directionXY;


const int BUTTONPIN = 23;
int  totalColors = sizeof(HueRainbow)/sizeof(HueRainbow[0]);
bool isOFF = true;
const int LIGHT = 2;
const int activeLOW = true;
const int pullUP = true;

Encoder myEnc(1,2);
OneButton myButton(BUTTONPIN , activeLOW , pullUP);

enum Direction {    // Joystick movements
//  0      1    2   3      4       5          6         7          8
  Still, Left, Up, Right, Down, Left_Down, Left_Up, Right_Down, Right_Up
};

int encoderPosition;
int brightness;

void setup()
{


  // JoyStick
  
  pinMode(JOYSTICK, INPUT_PULLUP);

  Serial.begin(115200);
  while (!Serial);


  // Ethernet
  pinMode(10, OUTPUT);
  digitalWrite(10, HIGH);
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);
  Ethernet.begin(mac);
  delay(200);          //ensure Serial Monitor is up and running           
  printIP();
  Serial.printf("LinkStatus: %i  \n",Ethernet.linkStatus());

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


//  Ethernet.begin(mac);
//  delay(200);          //ensure Serial Monitor is up and running           
//  printIP();
//  Serial.printf("LinkStatus: %i  \n",Ethernet.linkStatus());
 

 Serial.printf("\n\n\nButton state: %d\n\n", joystickButtonState);
   Serial.printf(" *************************   Direction: %d\n\n\n      ", directionXY);

}

void loop() {

  //                  JoyStick Section
  readJoyStick();
  if (previousButtonState != joystickButtonState) { // joystick Button was clicked
     if (joystickButtonState) {
       HuesON(true);
    } else {
       HuesON(false);
    }   
    previousButtonState = joystickButtonState;
  }

  if (directionXY != p_directionXY) {
    doJoystickActions(); 
    p_directionXY = directionXY; 
  }


  // MPU6050
  
//  readMPU6050();

  
 
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

}


void doJoystickActions() {
  
  
  switch (directionXY) {
    case Still:

    break;
    case Up:
  Serial.printf(" *************************   UP Direction: %d\n      ", directionXY);

    break;
    case Down:
  Serial.printf(" *************************   DOWN Direction: %d\n      ", directionXY);

    break;
    case Left:
  Serial.printf(" *************************   LEFT Direction: %d\n      ", directionXY);

    break;
    case Right:
  Serial.printf(" *************************   RIGHT Direction: %d\n      ", directionXY);

    break;
    case Left_Up:
  Serial.printf(" *************************   LEFT UP Direction: %d\n      ", directionXY);

    break;
    case Left_Down:
  Serial.printf(" *************************   LEFT DOWN Direction: %d\n      ", directionXY);

    break;
    case Right_Up:
  Serial.printf(" *************************   RIGHT UP Direction: %d\n      ", directionXY);

    break;
    case Right_Down:
  Serial.printf(" *************************   RIGHT DOWN Direction: %d\n      ", directionXY);

    break;
  }
}

void HuesON(bool state) {
  Serial.printf(" >>>>>>>>>>>>>>>>>>>>>>>>>    Button state: %d\n      ", joystickButtonState);
  if (state) { // Turn ON all Hues
    
  }
  else {       // Turn OFF all Hues
    
  }
}

// bool p_xMoved = false;
// bool p_yMoved = false;

void readJoyStick() {

  int x, y, buttonValue;
  bool xMoved = false;
  bool yMoved = false;
  
  
  x = analogRead(JOYSTICK_RX);       // read X axis value [0..1023]
 // Serial.printf("X: %04i", x);

  y = analogRead(JOYSTICK_RY);       // read Y axis value [0..1023]
//  Serial.printf(" | Y: %04i", y);

//int  previousX = 730;
//int  previousY = 750;

  if (abs(x-previousX)>25) { // x moved
    xMoved = true;
    // p_xMoved = xMoved;

  }

  if (abs(y-previousX)>25) { // y moved
    yMoved = true;
    //  p_yMoved = yMoved;
    
  }  

  //               Assign movement direction
  if (xMoved) {
    if (yMoved) {
       // both moved
       if (x>900) { // right side
          if (y>900) {
            directionXY = Right_Down;
          }
          else {
            if (y<500) {
              directionXY = Right_Up;
            }
          }
       }
       else {
        if (x<100) {
          if (y<100) {
            directionXY = Left_Up;
          } 
          else {
            if (y>900) {
              directionXY = Left_Down;
            }
          }
        }
       }
  
    } 
    else {
      // only x movement
      if (x<100) directionXY = Left;
      if (x>950) directionXY = Right;
    }
  } 
  else { 
    if (yMoved) {
      // only y movement
      if (y<100) directionXY = Up;
      if (y>900) directionXY = Down;
    }
    else { // No movement
      directionXY = Still;
    }
   
  }

  
  

  buttonValue = digitalRead(JOYSTICK);        // read Button state [0,1]
//  Serial.printf(" | Button: %i\n", buttonValue);
  if (!joystickClicked) {
    if (buttonValue == 0) { // button was clicked
      joystickClicked = true;
    }
  }
  else {
    if (buttonValue == 1)  {
      joystickButtonState = !joystickButtonState;
      joystickClicked = false;
    }
  }

  delay(100);

}

void setupMPU6050() {

  // acceleration 1g(A single G-force 1g is equivalent to gravitational acceleration 9.8 m/s2)
  
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

void leftAction() {
  
}

void rightAction() {
  
}

void upAction() {
  
}

void downAction() {
  
}

void flat() {
  
}

void tilted() {
  
}

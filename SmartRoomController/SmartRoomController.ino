/*
 *  Project:      Smart Room Controller
 *  Description:  Hue light is turn on, displays colors of HueRainbow array const values.
 *                                                        and encoder changes brightness
 *  Authors:      Pedro Sanchez
 *  Date:         Oct 25 2021
 */
 
#include <SPI.h>
#include <Ethernet.h>
#include <WemoObj.h>
#include <mac.h>
#include <hue.h>
#include <Encoder.h>
#include <OneButton.h>
#include <Wire.h>
#include <IoTTimer.h>

#include <Adafruit_SSD1306.h>
#include <Adafruit_BME280.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     4 // Reset pin # 4 (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///<= See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
// Define BME280 object temperature, humidity, pressure
Adafruit_BME280 bme; //this is for I2C device
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

enum Direction {    // Joystick movements
  Still, Left, Up, Right, Down, Left_Down, Left_Up, Right_Down, Right_Up
};


// Ethernet
EthernetClient client;
bool status;   //user to ensure port openned correctly


// WEMO
WemoObj wemoObj;
bool isWemoOFF[5]  = {true, true, true, true, true};
bool checkTimer[5] = {false, false, false, false, false};
IoTTimer wemoTimer[2]; // 2 wemo objects to control
IoTTimer ipTimer;

char message[40];

// ENCODER :    The "c" pin on the encoder is connected to GND
const int encoderRed   = 3;   // OUTPUT from the Teensy
const int encoderGreen = 6;  // OUTPUT from the Teensy
const int encoderSW    = 5; // SW means the  switch  Note: it is an INPUT to the Teensy

// BUZZER
const int BUZZER = 21; // A7

// JOYSTICK
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

// Button
const int BUTTONPIN = 23;


//const char colorName[3][6] = {"yellow", "blue", "green"};
int  currentColor = HueYellow;
bool isOFF = true;
bool isOnline = false;
const int LIGHT = 2;
const int activeLOW = true;
const int pullUP = true;
OneButton myButton(BUTTONPIN , activeLOW , pullUP);

// Encoder
Encoder myEnc(1,2);
int encoderPosition = 150;
int brightness = 200; // Starting brightness
int p_encoderPosition = brightness;


// MPU6050
const int MPU_addr=0x68;
int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;
int minVal=265;
int maxVal=402;
double x, y, z;
double p_y=0.0;

void setup()
{

  //                 OLED  DISPLAY
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }  
  display.clearDisplay();display.display();  
  display.display(); // Display initial logo
  delay(500);       // Pause for 500 milliseconds
  
  display.cp437(true);         // Use full 256 char 'Code Page 437' font

  welcome_display();

  //             Buzzer
  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, LOW);
  digitalWrite(BUZZER, HIGH);
  delay(500);
  digitalWrite(BUZZER, LOW);

  //            JoyStick
  
  pinMode(JOYSTICK, INPUT_PULLUP);

  Serial.begin(115200);
  while (!Serial);
  Serial.printf("\n\n\n\n\n\nStarted SMART ROOM CONTROLLER V1 by Pedro Daniel Sanchez  \n");



  //            MPU6050

  Wire.begin();
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);

  //           Encoder
  pinMode(encoderRed, OUTPUT);      // OUTPUT from the Teensy
  pinMode(encoderGreen, OUTPUT);    // OUTPUT from the Teensy
  pinMode(encoderSW, INPUT_PULLUP); // SW means the  switch  Note: it is an INPUT to the Teensy

  myEnc.write(encoderPosition);   // First Time set to default initialization value
  p_encoderPosition = encoderPosition;

  // Button
  turn_LIGHTS_OFF();
  myButton.attachClick(turn_LIGHTS_OFF_or_ON);
  myButton.attachDoubleClick(turn_LIGHTS_OFF);
  myButton.setClickTicks(250); 

  //      Ethernet
  pinMode(10, OUTPUT);
  digitalWrite(10, HIGH);
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);

  digitalWrite(encoderRed, HIGH); 

  Serial.printf("\n #### 1\n");
  Ethernet.begin(mac);  
  Serial.printf(" #### 2\n");
  readIP();         
  Serial.printf(" #### 3\n");
  ipTimer.startTimer(5000); // Automatic checking Ethernet port status

  Serial.printf(" #########  End of Setup\n\n\n      ");

}

void loop() {
  myButton.tick();

  //                  JoyStick Section
  
  readJoyStick();
  if (previousButtonState != joystickButtonState) { // joystick Button was clicked
     if (joystickButtonState) {
       displayTilt(true);
    } else {
       displayTilt(false);
    }   
    previousButtonState = joystickButtonState;
  }

  if (directionXY != p_directionXY) {
    doJoystickActions(); 
    p_directionXY = directionXY; 
  }

  //              ENCODER
  
  encoderPosition = myEnc.read();
  
  if (isOFF) { // if Hues are off don't move the brightness
    myEnc.write(p_encoderPosition);
    encoderPosition = p_encoderPosition;
  }
  
  if (encoderPosition<40) { 
    encoderPosition=40;
    p_encoderPosition = encoderPosition;
    myEnc.write(encoderPosition);    
  }
  else {
    if (encoderPosition>255) {
       encoderPosition=255;
       p_encoderPosition = encoderPosition;
       myEnc.write(encoderPosition);    
    }
  }

  if (abs(p_encoderPosition - encoderPosition) > 20 ) { // only is pos changed 
    brightness = encoderPosition;
    p_encoderPosition = encoderPosition;
      for (int j=1; j<=6; j++) {
        setHue(j, true, currentColor, brightness, 255);         
      }
    
  }

  // Automatically check ETHERNET connection to router every 5 seconds
  
  if (ipTimer.isTimerReady()) {
       Ethernet.begin(mac);
       readIP();
       ipTimer.startTimer(5000);
  }
}


  void turn_OFF_or_ON_Wemo(int wemo) {
    
    isWemoOFF[wemo]=!isWemoOFF[wemo];
    if ( isWemoOFF[wemo]) { // if it's on and clicked for OFF
      wemoObj.switchOFF(wemo);
    }
    else { // turn on the wemo switch if switch was off
      wemoObj.switchON(wemo);
    }
  }


void  welcome_display() {
  // textSize 1 = 28 chars/line
  // textSize 2 = 10 chars/line
  sprintf(message, "%s","\n  Smart\n   Room\nController\n");
  myDrawText(message,0,2,false);
//  sprintf(message, "%s","\n12345678901234567890\n");
//  myDrawText(message,0,2, false);

//  sprintf(message, "Se%cor %s \n%s", 0xA4,"Pedro Sanchez", "11/27" );
//  myDrawText(message,0,1, false);
  
  //sprintf(message, "    SE%cOR\n\n %s", 165,"   JESUS\n\n mi  DIOS\n\n" );
 // myDrawText(message,0,1);
//  sprintf(message, "    SE%cOR\n\n %s", 165,"   JESUS\n\n mi  DIOS\n\n" );
//  myDrawText(message,3,1);
//  sprintf(message, "    SE%cOR\n\n %s", 165,"   JESUS\n\n mi  DIOS\n\n" );
//  myDrawText(message,1,1);  
//  sprintf(message, "    SE%cOR\n\n %s", 165,"   JESUS\n\n mi  DIOS\n\n" );
//  myDrawText(message,2,1);
//  sprintf(message, "    SE%cOR\n\n %s", 165,"   JESUS\n\n mi  DIOS\n\n" );
//  myDrawText(message,1,1);    
}

void myDrawText(char *text, int rotation, int size, bool clearScreen) {
  
  //  size = sizeof(text)/sizeof(text[0]);
  // Serial.printf("size=%i string=%s\n",size, text);
  
  if (clearScreen) {
    display.clearDisplay();
  }

  display.setTextSize(size);      
  display.setTextColor(SSD1306_WHITE); // Draw white text
  
  display.setRotation(rotation);
  display.setCursor(0, 0);     // Start at top-left corner
  
  display.printf("%s", text);

  display.display();
  delay(10000);
}

void doJoystickActions() {
  
  
  switch (directionXY) {
    case Still:   // Display status
           display.clearDisplay();
           display.setTextSize(1);      // Normal 1:1 pixel scale
           display.setTextColor(SSD1306_WHITE); // Draw white text
           display.setCursor(0, 0);     // Start at top-left corner
           display.printf("U  D  L  R  LU LD RU RD\n");
           for (int wemoNum = 0; wemoNum<5; wemoNum++) {
              display.printf("%2s ",isWemoOFF[wemoNum]?"-  ":"ON ");
           }
           display.setTextSize(2);      // Normal 1:1 pixel scale
           if (currentColor==HueYellow) {
              display.printf("\n\nYELLOW ");
           } 
           else if (currentColor==HueBlue) {
              display.printf("\n\n  BLUE");
           }
           else {
              display.printf("\n\nGREEN");
           }
           

           display.display();
    break;
    case Up:
      Serial.printf(" ^   UP Direction: %d\n      ", directionXY);      
      turn_OFF_or_ON_Wemo(0);
   break;
    case Down:
      Serial.printf(" V   DOWN Direction: %d\n      ", directionXY);
      turn_OFF_or_ON_Wemo(1);

    break;
    case Left:
      Serial.printf(" <   LEFT Direction: %d\n      ", directionXY);
      turn_OFF_or_ON_Wemo(2);

    break;
    case Right:
      Serial.printf(" >   RIGHT Direction: %d\n      ", directionXY);
      turn_OFF_or_ON_Wemo(3);

    break;
    case Left_Up:
      Serial.printf(" < ^  LEFT UP Direction: %d\n      ", directionXY);
      turn_OFF_or_ON_Wemo(4);

    break;
    case Left_Down:
      Serial.printf(" < v LEFT DOWN Direction: %d\n      ", directionXY);
      currentColor=HueYellow;

    break;
    case Right_Up:
      Serial.printf(" > ^ RIGHT UP Direction: %d\n      ", directionXY);
      currentColor=HueBlue;
             
    break;
    case Right_Down:
      Serial.printf(" > V RIGHT DOWN Direction: %d\n      ", directionXY);
      currentColor=HueGreen;
    break;
  }
}

void  displayTilt(bool state) {
  Serial.printf(" >>>>>>>>>>>>>>>>>>>>>>>>>    Button state: %d\n      ", joystickButtonState);
  if (state) { // Display Tilt
    readMPU6050();
    display.setTextSize(3);      // Normal 1:1 pixel scale
    display.setTextColor(SSD1306_WHITE); // Draw white text
    display.clearDisplay();
    display.setCursor(0, 0);
    display.printf(" Tilt\n%.1f %c\n", y, 0xF8);
    display.display();   
    //state=false;

  }
  else {       // Hide Tilt
    display.clearDisplay();
    display.display();
    delay(100); 
    //delay(2000);
    directionXY = Still;  // Show status
    doJoystickActions(); 

  }
}


void readJoyStick() {

  int x, y, buttonValue;
  bool xMoved = false;
  bool yMoved = false;
  
  
  x = analogRead(JOYSTICK_RX);       // read X axis value [0..1023]

  y = analogRead(JOYSTICK_RY);       // read Y axis value [0..1023]

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


void readMPU6050() { // using y on device for tilt measure
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr,14,true);
  AcX=Wire.read()<<8|Wire.read();
  AcY=Wire.read()<<8|Wire.read();
  AcZ=Wire.read()<<8|Wire.read();
  int xAng = map(AcX,minVal,maxVal,-90,90);
  int yAng = map(AcY,minVal,maxVal,-90,90);
  int zAng = map(AcZ,minVal,maxVal,-90,90);
   
  x= RAD_TO_DEG * (atan2(-yAng, -zAng)+PI);
  y= RAD_TO_DEG * (atan2(-xAng, -zAng)+PI);
  z= RAD_TO_DEG * (atan2(-yAng, -xAng)+PI);
 
  Serial.printf("Tilt %.1f %c\n", y, 0xF8);


  if (abs(y - p_y)>4) {
    display.setTextSize(3);      // Normal 1:1 pixel scale
    display.setTextColor(SSD1306_WHITE); // Draw white text
    display.clearDisplay();
    display.setCursor(0, 0);
    display.printf(" Tilt\n%.1f %c\n", y, 0xF8);
    display.display();
    p_y = y;
  }
delay(400);
}

void readIP() {
  if (Ethernet.localIP()[0]==0 && Ethernet.localIP()[1]==0 &&
      Ethernet.localIP()[2]==0 && Ethernet.localIP()[3]==0) {
       Serial.printf("ERROR:  NO IP ADDRESS SET\n");

       sprintf(message, "%s","\n    NO\n INTERNET\n");
       myDrawText(message,0,2,true);
       
       isOnline = false;
       digitalWrite(encoderRed, HIGH);
       digitalWrite(encoderGreen, LOW);
   } 
   else 
   {
      if (!isOnline) {
        sprintf(message, "%s","\n  READY !\n");
        myDrawText(message,0,2,true);
      }
      isOnline = true;
      digitalWrite(encoderRed, LOW);
      digitalWrite(encoderGreen, HIGH);
      Serial.printf("MY IP address: ");
      for (byte thisByte = 0; thisByte <= 3; thisByte++) {
        Serial.printf("%i.",Ethernet.localIP()[thisByte]);
      }    
      Serial.printf("\n");
      //digitalWrite(encoderRed, LOW);
      //digitalWrite(encoderGreen, HIGH);
      delay(10);
   }
}

void turn_LIGHTS_OFF_or_ON() {
  isOFF = !isOFF;
  if (!isOFF) { // turn lights ON
      for (int j=1; j<=6; j++) {
        setHue(j, true, currentColor, brightness, 255);         
      }
  }
  else { // turn lights off
      for (int j=1; j<=6; j++) {
        setHue(j, false, 0, 0, 0);
      }

  }
}

void turn_LIGHTS_OFF() {

  Serial.printf("======== 1   turn_lights_OFF: isOFF = %d  brightness %i\n", isOFF, brightness);

  for (int j=1; j<=6; j++) {
     setHue(j, false, 0, 0, 0);
  }
  isOFF=true;
  Serial.printf("======== 2   turn_lights_OFF: isOFF = %d  brightness %i\n", isOFF, brightness);

}

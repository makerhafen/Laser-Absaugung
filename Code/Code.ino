#include <Wire.h>
#include "Adafruit_TCS34725.h"
#include <Servo.h>

const int SERVO_PIN = 9;  // pin of servo for finger
const int VALVE_PIN = 10; // pin for pressure valve

const float VENTILATION_ON_TIME = 45 * 1000.0; 
const float PRESSURE_ON_TIME    = 10 * 1000.0; 
const int COLORDIFF = 30;
const int SERVO_FINGER_UP_POSTITION   = 50;
const int SERVO_FINGER_DOWN_POSTITION = 10;

bool ventilation_is_active = false;
unsigned long  last_millis_green_on = 0;
unsigned long  timediff = 0;
float red, green, blue;

Servo servo_finger; 

Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);

void setup() {
  Serial.begin(9600);
  pinMode(VALVE_PIN, OUTPUT);    // sets the digital pin 13 as output
  while(true){ // init rbg sensor
    if (tcs.begin()) {
      Serial.println("Found sensor");
      break;
    } else {
      Serial.println("No TCS34725 found ... check your connections");
      delay(1000);
    }
  }
  servo_finger.attach(SERVO_PIN); // setup servo
  servo_finger.write(SERVO_FINGER_UP_POSTITION); // to default up position 
  last_millis_green_on = millis() - 120000L;
  delay(1000);      
}

void finger_click(){
  servo_finger.write(SERVO_FINGER_DOWN_POSTITION);  
  delay(400); 
  servo_finger.write(SERVO_FINGER_UP_POSTITION);   
  delay(500); 
}

void read_rgb(){
  tcs.setInterrupt(false);  
  delay(100);  // takes 50ms to read, delay some more so we read 10 times a second
  tcs.getRGB(&red, &green, &blue);
  tcs.setInterrupt(true); 
  Serial.print("R:\t");   Serial.print(int(red)); 
  Serial.print("\tG:\t"); Serial.print(int(green)); 
  Serial.print("\tB:\t"); Serial.print(int(blue));
  Serial.print("\n");
}

void loop() {
  read_rgb();

  if(green > red + COLORDIFF && green > blue + COLORDIFF){ // LED is green
    last_millis_green_on = millis();
  }

  if(red > green + COLORDIFF && red > blue + COLORDIFF){ // LED is red, stop 
    last_millis_green_on = 0;
  }

  timediff = millis() - last_millis_green_on;
   
  if(timediff > VENTILATION_ON_TIME ){// green led is or was on in the last VENTILATION_ON_TIME seconds, enable ventilation if not active
    if(ventilation_is_active == true){
      Serial.println("disable ventilation");
      finger_click();
      ventilation_is_active = false;
    }
  }else{
    if(ventilation_is_active == false){
      Serial.println("enable ventilation");
      finger_click();
      ventilation_is_active = true;
    }    
  }
  
  if(timediff > PRESSURE_ON_TIME){// green led is or was on in the last VENTILATION_ON_TIME seconds, enable ventilation if not active
    digitalWrite(VALVE_PIN, LOW);
  }else{
     digitalWrite(VALVE_PIN, HIGH);
  }
 
}

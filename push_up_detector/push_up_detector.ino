#include <TFT.h> // Hardware-specific library
#include <SPI.h>

// macro for TFT
#define CS   10
#define DC   9
#define RESET  8  

// defines pins numbers
int buzzPin = 2;
int trigPin = 3;
int echoPin = 4;

// defines variables
// the variables for ultrasonic sensor
long duration;
int distance;

// count the number of push-up
int count = 0;

// the variables for push up detection
int flag1 = 1;
int flag2 = 1;

// Initialize screen object
TFT screen = TFT(CS, DC, RESET);

char charBuf[50];

void setup() {
  // put your setup code here, to run once:

  // Ultrasonic sensor connection configuration
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // buzz connection configuration
  pinMode(buzzPin, OUTPUT);

  // initialize the screen
  screen.begin();  
  screen.background(255, 255, 255);  // clear the screen with black
  screen.stroke(0, 0, 0);
  screen.text("Count:", 30, 30);
  String string = String(count);
  string.toCharArray(charBuf, 50);
  screen.text(charBuf, 70, 30);

  Serial.begin(9600); // Starts the serial communication
}

void loop() {
  // put your main code here, to run repeatedly:
  distance = ultrasonic_sensor_data (trigPin, echoPin);

  Serial.println(distance);

  if (distance < 20){
    flag1 = 0;
    flag2 = 0; 
  }
  else {
    flag2 = 1;
    if (flag2 != flag1){
      flag1 = 1;

      // buzz
      tone(buzzPin, 1000); // Send 1KHz sound signal
      delay(50);        
      noTone(buzzPin);     // Stop sound
      delay(50);

      // display the updated count  
      screen.stroke(255, 255, 255);
      screen.text(charBuf, 70, 30);
      count = count + 1;
      screen.stroke(0, 0, 0);
      String string = String(count);
      string.toCharArray(charBuf, 50);
      screen.text(charBuf, 70, 30);
     
    }
  }

  delay(100);
}

int ultrasonic_sensor_data (int trigPin, int echoPin) {
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  
  // Sets the trigPin on HIGH state for 10 micro seconds to ultrasonic sound
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  
  // Calculating the distance measured in centimeter
  distance = duration * 0.034 / 2;

  return distance;
}


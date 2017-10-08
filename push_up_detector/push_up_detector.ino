#include <TFT.h> // Hardware-specific library
#include <SPI.h>

class Btn {
private:
  volatile byte state = LOW;
  //storing the button state for short press mode
  volatile byte state_short = LOW;
  //storing the button state for long press mode
  volatile byte state_long = LOW;
  
  byte mPin;
  unsigned long mLongPressMills;
  
  //stores the time each button went high or low
  volatile unsigned long current_high;
  volatile unsigned long current_low;

public:
  Btn(byte pin, unsigned long long_press_mills):mPin(pin), mLongPressMills(long_press_mills){
  }

  void Setup(){
    pinMode(mPin, INPUT);
  }

  void OnInterrupt(){
    //if this is true the button was just pressed down
    if(digitalRead(mPin) == LOW)
    {
      //note the time the button was pressed
      current_high = millis();
      state = HIGH;
      state_short = LOW;
      state_long = LOW;
    }
    //if no button is high one had to be released. The millis function will increase while a button is hold down the loop function will be cycled (no change, so no interrupt is active) 
     if(digitalRead(mPin) == HIGH && state == HIGH)
    {
      current_low = millis();
      if((current_low - current_high) > 50 && (current_low - current_high) < mLongPressMills)
      {
        state_short = HIGH;
        state = LOW;
      }
      else if((current_low - current_high) >= mLongPressMills && (current_low - current_high) < 20000)
      {
        state_long = HIGH;
        state = LOW;
      }
    }
  }

  boolean IsLongClicked(){
    if(state_long == HIGH){
      state_long = LOW;
      return true;
    }
    return false;
  }

  boolean IsShortClicked(){
    if(state_short == HIGH){
      state_short = LOW;
      return true;
    }
    return false;
  }
};

// macro for TFT
#define CS   10
#define DC   9
#define RESET  8  

// defines pins numbers
int buzzPin = 5;
int trigPin = 6;
int echoPin = 7;
int mainBtnPin = 2; // interrupt can only be triggered on Pin 2 and 3

// defines buttons
Btn mainBtn(mainBtnPin, 2000);

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

// start flag
boolean start = false;

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

  // configure button
  mainBtn.Setup();
  attachInterrupt(digitalPinToInterrupt(mainBtnPin), mainBtnISR, CHANGE);

  Serial.begin(9600); // Starts the serial communication
}

void loop() {
  // put your main code here, to run repeatedly:
  if (start){
  distance = ultrasonic_sensor_data (trigPin, echoPin);

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

  Serial.println(start);
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

void mainBtnISR(){
  mainBtn.OnInterrupt();

  if(mainBtn.IsShortClicked()){
    start = !start;
    if(!start){
      // clear the count
      screen.stroke(255, 255, 255);
      screen.text(charBuf, 70, 30);
      count = 0;
      screen.stroke(0, 0, 0);
      String string = String(count);
      string.toCharArray(charBuf, 50);
      screen.text(charBuf, 70, 30);
    }
  }
}


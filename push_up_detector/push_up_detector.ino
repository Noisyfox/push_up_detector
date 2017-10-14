#include <EEPROM.h>

#include <WiFiEsp.h>
#include <WiFiEspClient.h>
#include <WiFiEspServer.h>
#include <WiFiEspUdp.h>

#include <SoftwareSerial.h>

#include <TFT.h> // Hardware-specific library
#include <SPI.h>

#define DEBUG

#ifdef DEBUG
#define DBG(message)    Serial.print(message)
#define DBGW(message)    Serial.write(message)
#else
#define DBG(message)
#define DBGW(message)
#endif // DEBUG

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
#define PIN_BUZZ 5
#define PIN_TRIG 6
#define PIN_ECHO 7
#define PIN_BTN_MAIN 2 // interrupt can only be triggered on Pin 2 and 3
#define PIN_WIFI_RX 3
#define PIN_WIFI_TX 4

// defines buttons
Btn mainBtn(PIN_BTN_MAIN, 2000);

#define WiFiSSIDLen 16
#define WiFiPSWLen 16
#define ADDR_SSID 0
#define ADDR_PSW WiFiSSIDLen

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

// WiFi serial
SoftwareSerial wifiSerial(PIN_WIFI_RX, PIN_WIFI_TX);

enum EState{
  s_init,
  s_stopped,
  s_counting,
  s_pairing
};

// start flag
volatile EState state = s_init;
volatile EState next_state = s_stopped;

void error(String msg){
  screen.background(255, 255, 255);
  screen.stroke(0, 0, 0);
  msg.toCharArray(charBuf, 50);
  screen.text(charBuf, 10, 30);
  while(1);
}

void setup() {
  // put your setup code here, to run once:
  randomSeed(analogRead(12));

  // Ultrasonic sensor connection configuration
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);

  // buzz connection configuration
  pinMode(PIN_BUZZ, OUTPUT);

  // initialize the screen
  screen.begin();

  // configure button
  mainBtn.Setup();
  attachInterrupt(digitalPinToInterrupt(PIN_BTN_MAIN), mainBtnISR, CHANGE);

  Serial.begin(9600); // Starts the serial communication

  // Init WiFi module
  wifiSerial.begin(9600);
  WiFi.init(&wifiSerial);
  if(WiFi.status() == WL_NO_SHIELD){
    DBG(F("Module have no response.\n\r"));
    error(F("WiFi reset failed."));
  } else {
    DBG(F("Module is ready.\n\r"));
  }
}

void loop() {
  EState currentState = state;
  EState nextState = next_state;
  state = nextState;
  
  if(currentState != nextState){
    switch(currentState){
      case s_init:
        ExitInit();
        break;
    }
    switch(nextState){
      case s_stopped:
        EnterStopped();
        break;
      case s_counting:
        EnterCounting();
        break;
      case s_pairing:
        EnterPairing();
        break;
    }
  }

  switch(nextState){
    case s_init:
      next_state = s_stopped;
      break;
    case s_counting:
      OnCounting();
      break;
    case s_pairing:
      OnPairing();
      break;
  }

  // Bridge usb serial & wifi shield
  while(wifiSerial.available() > 0){
    Serial.write(wifiSerial.read());
  }
  while(Serial.available() > 0){
    wifiSerial.write(Serial.read());
  }
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
    switch(state){
      case s_init:
      case s_pairing:
        break;
      case s_stopped:
        next_state = s_counting;
        break;
      case s_counting:
        next_state = s_stopped;
        break;
    }
  } else if(mainBtn.IsLongClicked()){
    switch(state){
      case s_init:
      case s_pairing:
      case s_counting:
        break;
      case s_stopped:
        next_state = s_pairing;
        break;
    }
  }

//  Serial.println(start);
}

void ExitInit(){
  // Setup WiFi to station mode and connect to saved AP
  WiFi.reset();
}

void EnterStopped(){
  screen.background(255, 255, 255);
  screen.stroke(0, 0, 0);
  screen.text("Stopped.", 30, 30);
}

void EnterCounting(){
  // Clear count number
  count = 0;
  
  screen.background(255, 255, 255);  // clear the screen with white
  screen.stroke(0, 0, 0);
  screen.text("Count:", 30, 30);
  String string = String(count);
  string.toCharArray(charBuf, 50);
  screen.text(charBuf, 70, 30);
}

void OnCounting(){
    distance = ultrasonic_sensor_data (PIN_TRIG, PIN_ECHO);

    if (distance < 20){
      flag1 = 0;
      flag2 = 0; 
    }
    else {
      flag2 = 1;
      if (flag2 != flag1){
        flag1 = 1;
  
        // buzz
        tone(PIN_BUZZ, 1000); // Send 1KHz sound signal
        delay(50);        
        noTone(PIN_BUZZ);     // Stop sound
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

void EnterPairing(){
  screen.background(255, 255, 255);
  screen.stroke(0, 0, 0);
  screen.text("Pairing...", 20, 30);
  
  // Setup wifi as AP mode and open a port for listening.
  String AP_name = "PUSHUP_";
  for(int i = 0; i < 4; i++){
    AP_name = AP_name + String(random(10));
  }
  AP_name.toCharArray(charBuf, 50);
  if(WiFi.beginAP(charBuf) != WL_CONNECTED){
    error(F("Config AP failed."));
  }
  ("SSID:" + AP_name).toCharArray(charBuf, 50);
  screen.text(charBuf, 20, 50);
}

boolean MatchCommand(const char* cmd, char** input){
  if(strstr(*input, cmd) == *input){
    *input += strlen(cmd);
    return true;
  }

  return false;
}

void EEPROM_WriteStr(int addr, const char* str, int maxLen){
  int len = strlen(str);
  if(len > maxLen)len = maxLen;

  for(int i = 0; i < len; i++){
    EEPROM.write(addr + i, str[i]);
  }
  
  EEPROM.write(addr + len, '\0');

  DBG(String(str));
//  DBG("Saved at address
}

void EEPROM_ReadStr(int addr, char* str, int maxLen){
  int i;
  for(i = 0; i < maxLen; i++) {
    char c = EEPROM.read(addr + i);
    if(c == '\0')break;
    str[i] = c;
  }
  str[i] = '\0';
}

void OnPairing(){
  // Read socket and process command from mobile application
  WiFiEspServer server(80);
  server.begin();
  while(1){
    WiFiEspClient client = server.available();
    if(!client){
      continue;
    }
    char command[201];
    int index = 0;
    while (client.connected()) {
      while(client.available()){
        char c = client.read();
        if(c == '\r' && index > 0 && command[index-1] == '\n') {
          command[index - 1] = '\0';
          index = 0;
          
          // Execute command
          char* cp = command;
          if(strncmp(cp, "RST", 200) == 0){
            client.println(F("OK"));
            next_state = s_init;
            return;
          } else if(MatchCommand("APSSID=", &cp)){ // Set AP SSID
            int len = strlen(cp);
            if(len == 0 || len > WiFiSSIDLen - 1){
              client.println(F("ERROR: Wrong size."));
            } else {
              EEPROM_WriteStr(ADDR_SSID, cp, WiFiSSIDLen - 1);
              client.println(F("OK"));
            }
          } else if(MatchCommand("APPSW=", &cp)){ // Set AP password, if no password, send command "APPSW=".
            int len = strlen(cp);
            if(len > WiFiPSWLen - 1){
              client.println(F("ERROR: Wrong size."));
            } else {
              EEPROM_WriteStr(ADDR_PSW, cp, WiFiPSWLen - 1);
              client.println(F("OK"));
            }
          } else {
            client.println(F("ERROR: Unknown command."));
          }
          
        } else {
          if(index >= 201) {
            // Error, too long
            index = 0;
            client.println(F("ERROR: Command too long."));
            DBG(F("Error: Command too long.\n\r"));
            client.flush();
          } else {
            command[index] = c;
            index++;
          }
        }
      }
    }
  }
}


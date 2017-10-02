// defines pins numbers
int buzzPin = 8;
int trigPin = 9;
int echoPin = 10;

// defines variables
// the variables for ultrasonic sensor
long duration;
int distance;

// count the number of push-up
int count;

// the variables for push up detection
int flag1 = 1;
int flag2 = 1;

void setup() {
  // put your setup code here, to run once:

  // Ultrasonic sensor connection configuration
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // buzz connection configuration
  pinMode(buzzPin, OUTPUT);


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
      tone(buzzPin, 1000); // Send 1KHz sound signal
      delay(50);        
      noTone(buzzPin);     // Stop sound
      delay(50);
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


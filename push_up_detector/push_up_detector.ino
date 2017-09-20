// defines pins numbers
int trigPin = 9;
int echoPin = 10;

// defines variables
long duration;
int distance;

void setup() {
  // put your setup code here, to run once:

  // Ultrasonic sensor connection configuration
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);


  Serial.begin(9600); // Starts the serial communication


}

void loop() {
  // put your main code here, to run repeatedly:

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
}


#include <WiFi.h>
#include <ThingSpeak.h>
#include <BluetoothSerial.h>
#include <LiquidCrystal_I2C.h>

// WiFi credentials
const char* ssid = "netLabs!UG";
const char* password = "netLabs!UGPA55";

// ThingSpeak credentials
WiFiClient client;
unsigned long myChannelNumber = 2619528; 
const char* myWriteAPIKey = "LRYM2G8MTW2YNQ17";

// Pin Definitions
const int pirPin = 12;
const int trigPin = 13;
const int echoPin = 14;
const int buzzerPin = 26;
const int TRIG_PIN = 27;
const int ECHO_PIN = 25;
const int TRIG_RPIN = 33;
const int ECHO_RPIN = 32;
const int RELAY_PIN = 35;
const int BUZZER_PIN = 34;
const int redLED = 19;

const int DISTANCE_THRESHOLD = 10;
const int DISTANCE1_THRESHOLD = 20;
const int DISTANCE2_THRESHOLD = 10;
const int DISTANCE0_THRESHOLD = 50;

// Variables
float duration_us, distance;
float duration1_us, distance1;
bool motionDetected = false;
long duration;
float Distance_cm;

// LCD Initialization
LiquidCrystal_I2C lcd(0x27, 20, 4);

// Bluetooth Initialization
BluetoothSerial SerialBT;

void setup() {
  Serial.begin(115200);
  
  // LCD Setup
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Initializing...");

  // Pin Setup
  pinMode(pirPin, INPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(TRIG_RPIN, OUTPUT);
  pinMode(ECHO_RPIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(redLED, OUTPUT);

  // Bluetooth Setup
  SerialBT.begin("Tank-Remote-control");

  // WiFi Setup
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  
  // ThingSpeak Setup
  ThingSpeak.begin(client);

  lcd.setCursor(0, 1);
  lcd.print("System Ready!");
  delay(1000);
}

void loop() {
  relaydisp();
  ultrasound();
  ReserveUltrasound();

  // Send Data to ThingSpeak
  ThingSpeak.setField(1, distance);
  ThingSpeak.setField(2, distance1);
  
  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if (x == 200) {
    Serial.println("Data sent to ThingSpeak successfully!");
  } else {
    Serial.println("Problem sending data. HTTP error code: " + String(x));
  }

  // Bluetooth Commands
  if (SerialBT.available()) {
    char command = SerialBT.read();
    if (command == 'P') {
      digitalWrite(RELAY_PIN, HIGH);
    }
  }

  ultrasound1();

  // PIR Sensor Logic
  motionDetected = digitalRead(pirPin);
  if (motionDetected == HIGH) {
    Serial.println("Motion detected! Checking distance...");
    lcd.setCursor(0, 2);
    lcd.print("Motion Detected!   ");
    ultrasound1();
  } else {
    Serial.println("No Motion Detected");
    lcd.setCursor(0, 2);
    lcd.print("No Motion Detected");
  }

  delay(1000);
}

void ultrasound1() {
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  Distance_cm = (duration * 0.0343) / 2;

  lcd.setCursor(0, 3);
  lcd.print("Distance: ");
  lcd.print(Distance_cm);
  lcd.print(" cm    ");

  if (Distance_cm < DISTANCE0_THRESHOLD) {
    digitalWrite(buzzerPin, HIGH);
    Serial.println("Intruder detected within critical distance!");
    lcd.setCursor(0, 1);
    lcd.print("Intruder Detected!  ");
  } else {
    digitalWrite(buzzerPin, LOW);
    Serial.println("Safe Distance");
    lcd.setCursor(0, 1);
    lcd.print("No Intruder          ");
  }

  ThingSpeak.setField(1, buzzerPin);
  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if (x == 200) {
    Serial.println("Channel update successful");
  } else {
    Serial.println("Error updating channel. HTTP error code: " + String(x));
  }
}

void ReserveUltrasound() {
  digitalWrite(TRIG_RPIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_RPIN, LOW);

  duration1_us = pulseIn(ECHO_RPIN, HIGH);
  distance1 = 0.017 * duration1_us;

  if (distance1 < DISTANCE1_THRESHOLD) {
    digitalWrite(redLED, HIGH);
    lcd.setCursor(1, 2);
    lcd.print("Reserved Tank: Empty ");
    Alarm();
  } else {
    digitalWrite(redLED, LOW);
    lcd.setCursor(1, 2);
    lcd.print("Reserved Tank: Full  ");
  }
}

void Alarm() {
  if (distance1 < DISTANCE2_THRESHOLD) {
    digitalWrite(BUZZER_PIN, HIGH);
  } else {
    digitalWrite(BUZZER_PIN, LOW);
  }
}

void ultrasound() {
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  duration_us = pulseIn(ECHO_PIN, HIGH);
  distance = 0.017 * duration_us;

  if (distance < DISTANCE_THRESHOLD) {
    digitalWrite(RELAY_PIN, HIGH);
    lcd.print("Distance: ");
    lcd.println(distance);
    lcd.println("Relay: ON  ");
  } else {
    digitalWrite(RELAY_PIN, LOW);
    lcd.print("Distance: ");
    lcd.println(distance);
    lcd.println("Relay: OFF ");
  }
}

void relaydisp() {
  if (digitalRead(RELAY_PIN) == HIGH) {
    Serial.println("Relay On: Filling Tank");
    lcd.println("Relay On: Filling Tank");
    digitalWrite(BUZZER_PIN, HIGH);
  } else {
    Serial.println("Relay Off");
    lcd.println("Relay Off");
    digitalWrite(BUZZER_PIN, LOW);
  }
}
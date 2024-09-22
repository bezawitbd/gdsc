#include <MFRC522.h>
#include <Servo.h>
#include <LiquidCrystal.h>

// RFID setup
#define SS_PIN 10
#define RST_PIN 9
MFRC522 rfid(SS_PIN, RST_PIN);

// LCD setup
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);

// Servo setup
Servo doorServo;
int servoPin = 8;

// Buzzer setup
int buzzerPin = A4;

// Ultrasonic setup for two tools
int trigPin1 = A0;
int echoPin1 = A1;
int trigPin2 = A2;
int echoPin2 = A3;

// Tool borrowed state
bool toolBorrowed = false;
unsigned long borrowStartTime;
String lastBorrower = "";

// RFID UID to Name mapping
String uid1 = "0A 23 B4 1F";
String uid2 = "1B 4C D3 7A";

void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();

  lcd.begin(16, 2);
  doorServo.attach(servoPin);
  pinMode(buzzerPin, OUTPUT);
  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);
  
  lcd.print("System Ready");
  delay(2000);
}

void loop() {
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    String rfidID = getRFIDContent();
    String name = getPersonName(rfidID);

    if (toolBorrowed) {
      if (isToolReturned()) {
        toolBorrowed = false;
        lastBorrower = "";
        displayMessage("Tool Returned", name);
        closeDoor();
      } else {
        if (millis() - borrowStartTime > 10000) {
          activateBuzzer();
          displayMessage("Tool Not Returned", lastBorrower);
        }
      }
    } else {
      openDoor();
      borrowStartTime = millis();
      toolBorrowed = true;
      lastBorrower = name;
      displayMessage("Tool Borrowed", name);
    }
  }
}

String getRFIDContent() {
  String content = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    content += String(rfid.uid.uidByte[i] < 0x10 ? "0" : "");
    content += String(rfid.uid.uidByte[i], HEX);
  }
  content.toUpperCase();
  return content;
}

String getPersonName(String rfidID) {
  if (rfidID == uid1) {
    return "person 1";// owner of UID 1
  } else if (rfidID == uid2) {
    return "person 2";// owner of UID 2
  } else {
    return "Unknown";//if RFID doesn't match
  }
}

bool isToolReturned() {
  long duration1, distance1, duration2, distance2;

  digitalWrite(trigPin1, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin1, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin1, LOW);
  duration1 = pulseIn(echoPin1, HIGH);
  distance1 = duration1 * 0.034 / 2;

  digitalWrite(trigPin2, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin2, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin2, LOW);
  duration2 = pulseIn(echoPin2, HIGH);
  distance2 = duration2 * 0.034 / 2;

  return (distance1 < 10 && distance2 < 10);
}

void activateBuzzer() {
  digitalWrite(buzzerPin, HIGH);
  delay(1000);
  digitalWrite(buzzerPin, LOW);
}

void displayMessage(String status, String person) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(status);
  lcd.setCursor(0, 1);
  lcd.print(person);
}

void openDoor() {
  doorServo.write(90);
  delay(1000);
}

void closeDoor() {
  doorServo.write(0);
  delay(1000);
}

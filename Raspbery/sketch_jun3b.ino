#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10
#define RST_PIN 9

MFRC522 rfid(SS_PIN, RST_PIN);

const int redLed = 2;
const int greenLed = 3;
const int buttonPin = 4;
const int buzzerPin = 5;
const int alarmLed = 7;

bool armed = false;
bool alarmActive = false;

void setup() {
  Serial.begin(9600);

  pinMode(redLed, OUTPUT);
  pinMode(greenLed, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(buzzerPin, OUTPUT);
  pinMode(alarmLed, OUTPUT);

  SPI.begin();
  rfid.PCD_Init();

  digitalWrite(redLed, HIGH);   // DISARMED
  digitalWrite(greenLed, LOW);
  digitalWrite(alarmLed, LOW);

  Serial.println("READY");
}

void loop() {

  // ================= RFID =================
  if (rfid.PICC_IsNewCardPresent() &&
      rfid.PICC_ReadCardSerial()) {

    armed = !armed;
    alarmActive = false;

    noTone(buzzerPin);
    digitalWrite(alarmLed, LOW);

    if (armed) {
      digitalWrite(redLed, LOW);
      digitalWrite(greenLed, HIGH);
      Serial.println("ARMED");
    } else {
      digitalWrite(redLed, HIGH);
      digitalWrite(greenLed, LOW);
      Serial.println("DISARMED");
    }

    rfid.PICC_HaltA();
    delay(500);
  }

  // ================= BUTTON =================
  if (armed && digitalRead(buttonPin) == LOW) {
    alarmActive = true;
    Serial.println("ALARM_BUTTON");
    delay(200);
  }

  // ================= Raspberry Pi =================
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd == "ALARM") {
      if (armed) {
        alarmActive = true;
        Serial.println("ALARM_PI");
      }
    }

    if (cmd == "DISARM") {
      alarmActive = false;
      noTone(buzzerPin);
      digitalWrite(alarmLed, LOW);
      Serial.println("DISARM_PI");
    }
  }

  // ================= ALARM =================
  if (alarmActive) {

    digitalWrite(alarmLed, HIGH);

    for (int f = 500; f <= 2000; f += 50) {
      tone(buzzerPin, f);
      delay(10);
    }

    for (int f = 2000; f >= 500; f -= 50) {
      tone(buzzerPin, f);
      delay(10);
    }

    digitalWrite(alarmLed, LOW);
    delay(100);

  } else {

    noTone(buzzerPin);
    digitalWrite(alarmLed, LOW);
  }
}
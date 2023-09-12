#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 53
#define RST_PIN 9

#define RPI_REQ 2 //dodatkowy pin dla Rpi do konfiguracji komunikacji
#define TIME_REQ 1000 //co ile wysyłka do rpi

//sygnały z czujników
#define TANK_SENSOR_1 11
#define TANK_SENSOR_2 12
#define TANK_SENSOR_3 13
#define TANK_SENSOR_4 14
#define TANK_SENSOR_5 15
#define TANK_SENSOR_6 16
#define TANK_SENSOR_7 17
#define TANK_SENSOR_8 18

MFRC522 mfrc522(SS_PIN, RST_PIN); // Utwórz instancję modułu MFRC522

bool authorized_detected = false;
bool unauthorized_detected = false;

bool cardReadingInProgress = false;

bool tank_detected = false;
int detected_tank = -1; // Numer wykrytego baniaka

uint32_t authorizedCardUID = 0x530BF699; //przykładowa karta autoryzacyjna
uint32_t detectedCardUID = 0; //globalna do przechowywania odczytanego UID

void proximitySensor()
{
  if (digitalRead(TANK_SENSOR_1) == LOW) detected_tank = 1;
  else if (digitalRead(TANK_SENSOR_2) == LOW) detected_tank = 2;
  else if (digitalRead(TANK_SENSOR_3) == LOW) detected_tank = 3;
  else if (digitalRead(TANK_SENSOR_4) == LOW) detected_tank = 4;
  else if (digitalRead(TANK_SENSOR_5) == LOW) detected_tank = 5;
  else if (digitalRead(TANK_SENSOR_6) == LOW) detected_tank = 6;
  else if (digitalRead(TANK_SENSOR_7) == LOW) detected_tank = 7;
  else if (digitalRead(TANK_SENSOR_8) == LOW) detected_tank = 8;

  if (detected_tank != -1) {
    Serial.print("Wykryto baniak ");
    Serial.println(detected_tank);
    digitalWrite(TANK_SENSOR_1 + detected_tank - 1, HIGH); // Ustaw stan baniaka na HIGH
    tank_detected = true;
    delay(100);
  }
}

void printUID(MFRC522::Uid uid) {
  for (byte i = 0; i < uid.size; i++) {
    Serial.print(uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(uid.uidByte[i], HEX);
  }
  Serial.println();
}

bool compareUID(byte detectedUID[], uint32_t storedUID) {

  for (byte i = 0; i < 4; i++) {
    detectedCardUID = (detectedCardUID << 8) | detectedUID[i];
  }

  return detectedCardUID == storedUID;
}

void cardRead()
{
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    if (compareUID(mfrcrc522.uid.uidByte, authorizedCardUID)) {
      authorized_detected = true;
      Serial.println("Authorized card detected with UID:");
      printUID(mfrc522.uid);
    } else {
      unauthorized_detected = true;
      Serial.println("Unauthorized card detected with UID:");
      printUID(mfrc522.uid);
    }
    mfrc522.PICC_HaltA();
  }
}

void setup() {
  Serial.begin(115200);
  SPI.begin();
  mfrc522.PCD_Init(); // Initialize MFRC522 reader
  Serial.println("Place an RFID card near the reader...");
  //konfiguracja pinów czujników zbiornika
  for (int i = TANK_SENSOR_1; i <= TANK_SENSOR_8; i++) {
    pinMode(i, INPUT);
    digitalWrite(i, HIGH);
  }
  //konfiguracja pinu dodatkowego rpi
  pinMode(RPI_REQ, OUTPUT);
  digitalWrite(RPI_REQ, LOW);
}

void loop()
{
  if (!cardReadingInProgress) {
    cardRead();
    proximitySensor();
  }

  if (authorized_detected == true || unauthorized_detected == true)
  {
    authorized_detected == true ? Serial.println("Authorized card info") : Serial.println("Unauthorized card info");
    Serial.println(detectedCardUID, HEX);
    authorized_detected = false;
    unauthorized_detected = false;
    cardReadingInProgress = false; // Zakończ odczyt karty
  }

  if (tank_detected == true)
  {
    tank_detected = false;
    detected_tank = -1; // Zresetuj numer wykrytego baniaka
  }
}

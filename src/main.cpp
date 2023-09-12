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

uint32_t authorizedCardUID = 0x530BF699; //przykładowa karta autoryzacyjna
uint32_t detectedCardUID = 0; //globalna do przechowywania odczytanego UID

void proximitySensor()
{
  if(digitalRead(TANK_SENSOR_1) == LOW ||
     digitalRead(TANK_SENSOR_2) == LOW ||
     digitalRead(TANK_SENSOR_3) == LOW ||
     digitalRead(TANK_SENSOR_4) == LOW ||
     digitalRead(TANK_SENSOR_5) == LOW ||
     digitalRead(TANK_SENSOR_6) == LOW ||
     digitalRead(TANK_SENSOR_7) == LOW ||
     digitalRead(TANK_SENSOR_8) == LOW)
  {
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
    if (compareUID(mfrc522.uid.uidByte, authorizedCardUID)) {
      authorized_detected = true;
    } else {
      unauthorized_detected = true;
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
  pinMode(TANK_SENSOR_1, INPUT);
  pinMode(TANK_SENSOR_2, INPUT);
  pinMode(TANK_SENSOR_3, INPUT);
  pinMode(TANK_SENSOR_4, INPUT);
  pinMode(TANK_SENSOR_5, INPUT);
  pinMode(TANK_SENSOR_6, INPUT);
  pinMode(TANK_SENSOR_7, INPUT);
  pinMode(TANK_SENSOR_8, INPUT);
  digitalWrite(TANK_SENSOR_1, HIGH);
  digitalWrite(TANK_SENSOR_2, HIGH);
  digitalWrite(TANK_SENSOR_3, HIGH);
  digitalWrite(TANK_SENSOR_4, HIGH);
  digitalWrite(TANK_SENSOR_5, HIGH);
  digitalWrite(TANK_SENSOR_6, HIGH);
  digitalWrite(TANK_SENSOR_7, HIGH);
  digitalWrite(TANK_SENSOR_8, HIGH);
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
    Serial.println("Wykryto baniak 1");
    digitalWrite(TANK_SENSOR_1, HIGH);
    tank_detected = false;
    delay(100);
    
    // Odblokuj odczyt karty RFID
    cardReadingInProgress = false;
  }
}
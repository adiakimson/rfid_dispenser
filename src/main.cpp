#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 53
#define RST_PIN 49

MFRC522 mfrc522(SS_PIN, RST_PIN);

uint32_t authorizedCardUID = 0x530BF699; // Authorized card UID in hexadecimal

void printUID(MFRC522::Uid uid) {
  for (byte i = 0; i < uid.size; i++) {
    Serial.print(uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(uid.uidByte[i], HEX);
  }
  Serial.println();
}

bool compareUID(byte detectedUID[], uint32_t storedUID) {
  uint32_t detectedCardUID = 0;

  for (byte i = 0; i < 4; i++) {
    detectedCardUID = (detectedCardUID << 8) | detectedUID[i];
  }

  return detectedCardUID == storedUID;
}

void setup() {
  Serial.begin(115200);
  SPI.begin();
  mfrc522.PCD_Init(); // Initialize MFRC522 reader.
  Serial.println("Place an RFID card near the reader...");
}

void loop() {
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    if (compareUID(mfrc522.uid.uidByte, authorizedCardUID)) {
      Serial.println("Authorized card detected.");
    } else {
      Serial.println("Unauthorized card detected with UID:");
      printUID(mfrc522.uid);
    }
    mfrc522.PICC_HaltA();
  }
}


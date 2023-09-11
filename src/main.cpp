#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 53
#define RST_PIN 49

MFRC522 mfrc522(SS_PIN, RST_PIN);

byte authorizedCardUID[] = {0x53, 0x0B, 0xF6, 0x99}; // Authorized card UID in little-endian hexadecimal

void printUID(MFRC522::Uid uid) {
  for (byte i = 0; i < uid.size; i++) {
    Serial.print(uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(uid.uidByte[i], HEX);
  }
  Serial.println();
}

bool compareUID(MFRC522::Uid detectedUID, byte storedUID[], byte length) {
  if (detectedUID.size != length) {
    return false; // The UID sizes don't match, not the same card.
  }

  for (byte i = 0; i < detectedUID.size; i++) {
    if (detectedUID.uidByte[i] != storedUID[i]) {
      return false; // At least one byte is different, not the same card.
    }
  }

  return true; // All bytes match, it's the same card.
}

void setup() {
  Serial.begin(115200);
  SPI.begin();
  mfrc522.PCD_Init(); // Initialize MFRC522 reader.
  Serial.println("Place an RFID card near the reader...");
}

void loop() {
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    if (compareUID(mfrc522.uid, authorizedCardUID, sizeof(authorizedCardUID))) {
      Serial.println("Authorized card detected.");
      printUID(mfrc522.uid);
    } else {
      Serial.println("Unauthorized card detected with UID:");
      printUID(mfrc522.uid);
    }
    mfrc522.PICC_HaltA();
  }
}


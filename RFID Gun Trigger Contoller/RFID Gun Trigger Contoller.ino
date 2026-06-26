#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>

#define SS_PIN 10
#define RST_PIN 9
#define SERVO_PIN 6

MFRC522 mfrc522(SS_PIN, RST_PIN);
Servo servo;

bool servoActive = false;
unsigned long lastSeen = 0;
const unsigned long timeout = 700;

// 🔐 STORED HASH (NOT UID)
// Replace this after generating from your card
unsigned long authorizedHash = 0x7CAAF891;

void setup() {
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();

  servo.attach(SERVO_PIN);
  servo.write(0);

  Serial.println("Secure RFID System Ready");
}

// 🔐 Simple lightweight hash function
unsigned long hashUID(byte *uid, byte size) {
  unsigned long hash = 5381;

  for (byte i = 0; i < size; i++) {
    hash = ((hash << 5) + hash) + uid[i]; // hash * 33 + byte
  }

  return hash;
}

void loop() {

  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {

    unsigned long currentHash = hashUID(mfrc522.uid.uidByte, mfrc522.uid.size);

    Serial.print("Card Detected (HASH): ");
    Serial.println(currentHash, HEX);

    // ✅ AUTH CHECK (NO UID USED)
    if (currentHash == authorizedHash) {

      lastSeen = millis();

      if (!servoActive) {
        servo.write(90);
        Serial.println("Authorized → Servo 90°");
        servoActive = true;
      }
    }

    // ⚠ DO NOT use HALT (for continuous scanning)
  }

  // Timeout logic
  if (servoActive && (millis() - lastSeen > timeout)) {
    servo.write(0);
    Serial.println("Servo → 0° (Removed)");
    servoActive = false;
  }

  delay(100);
}
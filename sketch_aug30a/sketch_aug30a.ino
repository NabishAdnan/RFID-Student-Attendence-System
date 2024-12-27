#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10
#define RST_PIN 9
#define GREEN_LED 7
#define RED_LED 6

MFRC522 rfid(SS_PIN, RST_PIN);

// Define the known RFID card UIDs and their associated names
struct AuthorizedCard {
  byte uid[4];
  String name;
};

AuthorizedCard authorizedCards[] = {
  {{0xB8, 0x4C, 0x46, 0x12}, "N A B I S H"},
  {{0x62, 0xEE, 0x7A, 0x51}, "S A G A R"},
  {{0x83, 0x12, 0x5B, 0x11}, "A D M I N"}  // Add more users as needed
};

void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, LOW);

  Serial.println("Place your RFID card near the reader...");
}

void loop() {
  if (!rfid.PICC_IsNewCardPresent()) {
    return;
  }

  if (!rfid.PICC_ReadCardSerial()) {
    return;
  }

  // Display UID in the Serial Monitor
  Serial.print("UID tag: ");
  String content = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(rfid.uid.uidByte[i], HEX);
    content.concat(String(rfid.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(rfid.uid.uidByte[i], HEX));
  }
  Serial.println();

  // Compare the scanned UID with the known UIDs and print associated name
  bool isAuthorized = false;
  String userName = "";
  for (byte i = 0; i < sizeof(authorizedCards) / sizeof(authorizedCards[0]); i++) {
    bool match = true;
    for (byte j = 0; j < rfid.uid.size; j++) {
      if (rfid.uid.uidByte[j] != authorizedCards[i].uid[j]) {
        match = false;
        break;
      }
    }
    if (match) {
      isAuthorized = true;
      userName = authorizedCards[i].name;
      break;
    }
  }

  // Prepare data to send to the Python script
  String logEntry = content + ", "; // UID
  logEntry += isAuthorized ? userName : "Unknown"; // User name or "Unknown"

  if (isAuthorized) {
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(RED_LED, LOW);
    Serial.print("Access granted! Welcome ");
    Serial.println(userName);
    logEntry += ", Entry"; // Log as an entry action
  } else {
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(RED_LED, HIGH);
    Serial.println("Access denied! Card details not found.");
    logEntry += ", Access Denied"; // Log as a failed access
  }

  // Send the log entry to the Serial port
  Serial.println(logEntry);

  delay(1000); // Keep the LED on for 1 second
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, LOW);

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

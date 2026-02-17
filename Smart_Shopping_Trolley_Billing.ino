#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN 9
#define SS_PIN 10

MFRC522 rfid(SS_PIN, RST_PIN);
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Define RFID card UIDs
String cardUIDs[4] = {
  "EF 69 35 1E",  // Milk
  "F3 AB DD E2",  // Bread
  "63 97 60 E4",  // Juice
  "97 8B 7A 00"   // Checkout
};

String productNames[3] = {"Milk", "Bread", "Juice"};
int productPrices[3] = {30, 20, 25};
bool productScanned[3] = {false, false, false};
int totalAmount = 0;
unsigned long lastScanTime = 0;
bool checkoutDone = false;

void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Scan your card");
}

void loop() {
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) return;

  // Read UID
  String scannedUID = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) scannedUID += "0";
    scannedUID += String(rfid.uid.uidByte[i], HEX);
    if (i < rfid.uid.size - 1) scannedUID += " ";
  }
  scannedUID.toUpperCase();
  Serial.println("Scanned UID: " + scannedUID);

  // Check if it's the checkout card
  if (scannedUID == cardUIDs[3] && !checkoutDone) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Checkout Done");
    lcd.setCursor(0, 1);
    lcd.print("Total: Rs." + String(totalAmount));
    checkoutDone = true;
    lastScanTime = millis();
  }
  // Check if it's a product card
  else if (!checkoutDone) {
    for (int i = 0; i < 3; i++) {
      if (scannedUID == cardUIDs[i]) {
        if (!productScanned[i]) {
          productScanned[i] = true;
          totalAmount += productPrices[i];
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print(productNames[i]);
          lcd.setCursor(0, 1);
          lcd.print("Rs." + String(productPrices[i]));
        } else {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Already added:");
          lcd.setCursor(0, 1);
          lcd.print(productNames[i]);
        }
        lastScanTime = millis();
        break;
      }
    }
  }

  delay(1000); // 1 second delay after each scan
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();

  // Reset system after 2 seconds of checkout
  if (checkoutDone && millis() - lastScanTime >= 2000) {
    resetSystem();
  }
}

void resetSystem() {
  totalAmount = 0;
  for (int i = 0; i < 3; i++) productScanned[i] = false;
  checkoutDone = false;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Restarted...");
  delay(1000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Scan your card");
}
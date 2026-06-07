#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
// Pin Definitions
#define SS_PIN 10
#define RST_PIN 9
#define BUZZER 8
MFRC522 mfrc522(SS_PIN, RST_PIN);
LiquidCrystal_I2C lcd(0x27, 16, 2);
SoftwareSerial gsm(6, 7); // RX, TX
// Product Database (Tag IDs ? Name and Price)
struct Product {
String tag;
String name;
int price;
};
Product products[] = {
{"88004AB3", "Milk", 30},
{"3FC56D92", "Bread", 25},
{"8B443F11", "Eggs", 45}
};
int numProducts = sizeof(products) / sizeof(products[0]);
int total = 0;
String scannedTags[10];
int tagCount = 0;
void setup() {
Serial.begin(9600);
gsm.begin(9600);
SPI.begin();
mfrc522.PCD_Init();
lcd.init();
lcd.backlight();
pinMode(BUZZER, OUTPUT);
29
lcd.setCursor(0, 0);
lcd.print("Smart Trolley");
delay(2000);
lcd.clear();
lcd.print("Scan item...");
}
void loop() {
if (!mfrc522.PICC_IsNewCardPresent()) return;
if (!mfrc522.PICC_ReadCardSerial()) return;
String tagID = "";
for (byte i = 0; i < mfrc522.uid.size; i++) {
tagID += String(mfrc522.uid.uidByte[i], HEX);
}
tagID.toUpperCase();
if (isDuplicate(tagID)) {
lcd.clear();
lcd.print("Item Already");
lcd.setCursor(0, 1);
lcd.print("Scanned");
tone(BUZZER, 1000, 300);
delay(2000);
lcd.clear();
lcd.print("Scan item...");
return;
}
bool found = false;
for (int i = 0; i < numProducts; i++) {
if (tagID == products[i].tag) {
total += products[i].price;
scannedTags[tagCount++] = tagID;
lcd.clear();
lcd.print(products[i].name);
lcd.setCursor(0, 1);
30
lcd.print("Price: ");
lcd.print(products[i].price);
tone(BUZZER, 2000, 300);
delay(2000);
lcd.clear();
lcd.print("Total: ");
lcd.print(total);
found = true;
break;
}
}
if (!found) {
lcd.clear();
lcd.print("Item Unknown");
tone(BUZZER, 500, 400);
delay(2000);
}
lcd.clear();
lcd.print("Scan item...");
mfrc522.PICC_HaltA();
}
// Helper to check duplicate scans
bool isDuplicate(String tag) {
for (int i = 0; i < tagCount; i++) {
if (scannedTags[i] == tag) return true;
}
return false;
}
// Call this function to send SMS (trigger manually or after shopping ends)
void sendSMS() {
gsm.println("AT+CMGF=1"); // Set SMS mode to text
delay(1000);
31
gsm.println("AT+CMGS=\"+91xxxxxxxxxx\""); // Replace with user phone number
delay(1000);
gsm.print("Smart Trolley Bill\nTotal: Rs ");
gsm.print(total);
gsm.write(26); // ASCII code for Ctrl+Z to send
delay(2000);
}
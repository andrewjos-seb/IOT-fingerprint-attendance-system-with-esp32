#include <WiFi.h>
#include <WiFiClientSecure.h>  // Secure WiFi Client for HTTPS
#include <Arduino.h>
#include <HTTPClient.h>
#include <Adafruit_Fingerprint.h>
#include <TFT_eSPI.h>

// WiFi Credentials
#define WIFI_SSID "datalogger"
#define WIFI_PASSWORD "datalogger"

// Google Sheets API URL
const char* sheet_url = "yoursheedid?name=";
//https://script.google.com/macros/s/AKfycbzWCCos0pdQE921ADdz6aA59n1sMGveYok0SUw62ao9ROc0x65H_2tvTQAxJZuqbR3A/exec?name=
// Hardware Pins
#define BUZZER 15
#define FINGERPRINT_RX 16  // RX of Fingerprint Sensor (Connect to TX on ESP32)
#define FINGERPRINT_TX 17  // TX of Fingerprint Sensor (Connect to RX on ESP32)

// Initialize Components
TFT_eSPI tft = TFT_eSPI();
HardwareSerial mySerial(2);  // UART2 for fingerprint sensor
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

// Function Prototypes
void placeFingerMessage();
void verifying();
void notfound();
void verifiedMessage(String data);
int getFingerprintID();
void sendToGoogleSheets(int userID);
void connectWiFi();

// Setup Function
void setup() {
  Serial.begin(115200);
  mySerial.begin(57600, SERIAL_8N1, FINGERPRINT_RX, FINGERPRINT_TX);
  pinMode(BUZZER, OUTPUT);

  tft.begin();
  tft.fillRect(0, 0, 128, 160, 0x0);
  tft.setRotation(4);

  connectWiFi();

  if (finger.verifyPassword()) {
    Serial.println("Fingerprint sensor found!");
  } else {
    Serial.println("Fingerprint sensor NOT found!");
    while (1);
  }

  // Wait for user input
  Serial.println("Type 'enroll' to register a new fingerprint or wait to continue...");
  unsigned long startTime = millis();
  while (millis() - startTime < 5000) {  // 5-second window
    if (Serial.available()) {
      String command = Serial.readStringUntil('\n');
      command.trim();
      if (command.equalsIgnoreCase("enroll")) {
        Serial.println("Entering enrollment mode...");
        enrollFingerprint();
      }
    }
  }
}

// Function to enroll a new fingerprint
void enrollFingerprint() {
  int id;
  Serial.println("Enter ID for new fingerprint (1-127):");
  while (true) {
    if (Serial.available()) {
      id = Serial.parseInt();
      if (id > 0 && id < 128) break;
      Serial.println("Invalid ID. Enter a number between 1-127:");
    }
  }

  Serial.print("Place finger on the scanner...");

  while (finger.getImage() != FINGERPRINT_OK);

  if (finger.image2Tz(1) != FINGERPRINT_OK) {
    Serial.println("Failed to process fingerprint. Try again.");
    return;
  }

  Serial.println("Remove finger...");
  delay(2000);
  while (finger.getImage() != FINGERPRINT_NOFINGER);

  Serial.println("Place the same finger again...");
  while (finger.getImage() != FINGERPRINT_OK);

  if (finger.image2Tz(2) != FINGERPRINT_OK) {
    Serial.println("Failed to process second scan. Try again.");
    return;
  }

  if (finger.createModel() != FINGERPRINT_OK) {
    Serial.println("Failed to create fingerprint model.");
    return;
  }

  if (finger.storeModel(id) == FINGERPRINT_OK) {
    Serial.println("Fingerprint enrolled successfully!");
  } else {
    Serial.println("Error storing fingerprint.");
  }
}

int flag = 0;
// Loop Function
void loop() {
  placeFingerMessage();

  int userID = getFingerprintID();
  if (userID > 0) {
    flag = 0;
    verifying();
    Serial.print("User ID: ");
    Serial.println(userID);

    // Send to Google Sheets
    sendToGoogleSheets(userID);

    // Buzzer Indication
    digitalWrite(BUZZER, HIGH);
    delay(200);
    digitalWrite(BUZZER, LOW);
    delay(200);
    digitalWrite(BUZZER, HIGH);
    delay(200);
    digitalWrite(BUZZER, LOW);

    delay(3000);  // Extra delay to prevent rapid resets
  } else if (userID == -2) {
    Serial.println("Wrong Fingerprint");
    no_Finger();
    digitalWrite(BUZZER, HIGH);
    delay(1000);
    digitalWrite(BUZZER, LOW);
    flag = 0;
  }

  delay(2000);  // Wait before next scan
}

// Function to Get Fingerprint ID
int getFingerprintID() {
  int result = finger.getImage();
  if (result != FINGERPRINT_OK) return -1;

  result = finger.image2Tz();
  if (result != FINGERPRINT_OK) return -2;

  result = finger.fingerFastSearch();
  if (result != FINGERPRINT_OK) return -2;

  return finger.fingerID;
}

// Function to Ensure WiFi is Connected
void connectWiFi() {
  Serial.print("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000) {
    Serial.print("."); 
    nowifi();   
    delay(500);
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected");
    Serial.println("IP address: " + WiFi.localIP().toString());
  } else {
    nowifi();
    Serial.println("\nFailed to connect to WiFi. Restarting...");
    ESP.restart();  // Restart ESP32 if WiFi fails
  }
}
void nowifi(){
  static const unsigned char PROGMEM image_download_bits[] = { 0x00, 0x03, 0x00, 0x00, 0x00, 0x07, 0x80, 0x00, 0x00, 0x0f, 0xc0, 0x00, 0x00, 0x0f, 0xc0, 0x00, 0x00, 0x1f, 0xe0, 0x00, 0x00, 0x3c, 0xf0, 0x00, 0x00, 0x3c, 0xf0, 0x00, 0x00, 0x7c, 0xf8, 0x00, 0x00, 0xfc, 0xfc, 0x00, 0x00, 0xfc, 0xfc, 0x00, 0x01, 0xfc, 0xfe, 0x00, 0x03, 0xfc, 0xff, 0x00, 0x03, 0xfc, 0xff, 0x00, 0x07, 0xfc, 0xff, 0x80, 0x0f, 0xfc, 0xff, 0xc0, 0x0f, 0xfc, 0xff, 0xc0, 0x1f, 0xfc, 0xff, 0xe0, 0x3f, 0xff, 0xff, 0xf0, 0x3f, 0xff, 0xff, 0xf0, 0x7f, 0xfc, 0xff, 0xf8, 0xff, 0xfc, 0xff, 0xfc, 0xff, 0xff, 0xff, 0xfc, 0x7f, 0xff, 0xff, 0xf8 };

  tft.fillRect(0, 0, 128, 160, 0x0);

  tft.drawEllipse(65, 78, 31, 29, 0xFAAA);

  tft.setTextColor(0xFAAA);
  tft.setTextSize(1);
  tft.setFreeFont();
  tft.drawString("NO INTERNET CONNECTION", 3, 137);

  tft.drawBitmap(51, 64, image_download_bits, 30, 23, 0xFFEA);
}
// Function to Send Data to Google Sheets
void sendToGoogleSheets(int userID) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected! Reconnecting...");
    connectWiFi();
  }

  if (WiFi.status() == WL_CONNECTED) {
    WiFiClientSecure client;
    client.setInsecure();  // Ignore SSL verification (For Google Sheets)

    String url = sheet_url + String(userID);
    Serial.println("Sending request: " + url);

    HTTPClient http;
    http.setTimeout(10000);                                  // Increase timeout to 10 seconds
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);  // Enable redirection handling

    if (http.begin(client, url)) {  // Use Secure Client
      int httpCode = http.GET();
      if (httpCode > 0) {
        String response = http.getString();
        Serial.println("Full Response:");
        Serial.println(response);
        verifiedMessage(response);
        Serial.printf("Server Response Code: %d\n", httpCode);
      } else {
        Serial.printf("HTTP Request failed: %s\n", http.errorToString(httpCode).c_str());
      }
      http.end();
    } else {
      Serial.println("Failed to connect to server");
      notfound();
    }
  } else {
    Serial.println("WiFi not connected, cannot send data.");
  }
}

// TFT Display Functions
void placeFingerMessage() {
  if (flag == 0) {
    tft.fillRect(0, 0, 128, 160, 0x0);
    flag = 1;
  }
  tft.setTextColor(0xA800);
  tft.setTextSize(1);
  tft.drawString("ATTENDANCE LOGGER", 13, 8);
  tft.drawRect(0, 0, 128, 25, 0xAAA0);
  tft.drawRect(0, 55, 128, 55, 0x57FF);
  tft.setTextColor(0x57EA);
  tft.drawString("PLACE FINGER AT", 14, 65);
  tft.setTextColor(0xAAA0);
  //tft.drawString("IEDC@Saintgits", 22, 150);
  tft.setTextColor(0x57EA);
  tft.drawString("THE READER", 31, 83);
}
void no_Finger() {
  static const unsigned char PROGMEM image_download_bits[] = { 0x00, 0x03, 0x00, 0x00, 0x00, 0x07, 0x80, 0x00, 0x00, 0x0f, 0xc0, 0x00, 0x00, 0x0f, 0xc0, 0x00, 0x00, 0x1f, 0xe0, 0x00, 0x00, 0x3c, 0xf0, 0x00, 0x00, 0x3c, 0xf0, 0x00, 0x00, 0x7c, 0xf8, 0x00, 0x00, 0xfc, 0xfc, 0x00, 0x00, 0xfc, 0xfc, 0x00, 0x01, 0xfc, 0xfe, 0x00, 0x03, 0xfc, 0xff, 0x00, 0x03, 0xfc, 0xff, 0x00, 0x07, 0xfc, 0xff, 0x80, 0x0f, 0xfc, 0xff, 0xc0, 0x0f, 0xfc, 0xff, 0xc0, 0x1f, 0xfc, 0xff, 0xe0, 0x3f, 0xff, 0xff, 0xf0, 0x3f, 0xff, 0xff, 0xf0, 0x7f, 0xfc, 0xff, 0xf8, 0xff, 0xfc, 0xff, 0xfc, 0xff, 0xff, 0xff, 0xfc, 0x7f, 0xff, 0xff, 0xf8 };

  tft.fillRect(0, 0, 128, 160, 0x0);

  tft.drawEllipse(65, 78, 31, 29, 0xFAAA);

  tft.setTextColor(0xFAAA);
  tft.setTextSize(1);
  tft.setFreeFont();
  tft.drawString("FINGERRINT NOT FOUND", 3, 137);

  tft.drawBitmap(51, 64, image_download_bits, 30, 23, 0xFFEA);
}

void verifying() {
  static const unsigned char PROGMEM image_download_bits[] = { 0x00, 0x40, 0x00, 0x00, 0xe0, 0x00, 0x01, 0x40, 0x00, 0x02, 0xa0, 0x00, 0x05, 0x10, 0x00, 0x0a, 0x08, 0x00, 0x14, 0x08, 0x00, 0x28, 0x08, 0x00, 0x50, 0x08, 0x00, 0xe0, 0x08, 0x00, 0x50, 0x08, 0x00, 0x08, 0x07, 0xe0, 0x07, 0xe0, 0x10, 0x00, 0x14, 0x0a, 0x00, 0x17, 0xe7, 0x00, 0x17, 0xca, 0x00, 0x17, 0x94, 0x00, 0x17, 0x28, 0x00, 0x12, 0x50, 0x00, 0x08, 0xa0, 0x00, 0x05, 0x40, 0x00, 0x02, 0x80, 0x00, 0x07, 0x00, 0x00, 0x02, 0x00 };
  tft.fillRect(0, 0, 128, 160, 0x0);

  tft.setTextColor(0xA800);
  tft.setTextSize(1);
  tft.setFreeFont();
  tft.drawString("DATA LOGGER", 32, 10);

  tft.drawRect(0, 0, 128, 25, 0xAAA0);

  tft.drawEllipse(64, 86, 38, 35, 0xAAA0);

  tft.drawBitmap(52, 73, image_download_bits, 24, 24, 0xFFFF);

  tft.setTextColor(0x57FF);
  tft.drawString("Verifying ..", 44, 131);
}

void verifiedMessage(String data) {
  static const unsigned char PROGMEM image_download_bits[] = { 0x00, 0x00, 0xf0, 0x00, 0x00, 0xf0, 0x00, 0x03, 0xf0, 0x00, 0x03, 0xf0, 0x00, 0x03, 0xc0, 0x00, 0x0f, 0xc0, 0x00, 0x0f, 0x00, 0xf0, 0x3f, 0x00, 0xf0, 0x3f, 0x00, 0xfc, 0xfc, 0x00, 0xfc, 0xfc, 0x00, 0x3f, 0xf0, 0x00, 0x3f, 0xf0, 0x00, 0x0f, 0xc0, 0x00, 0x0f, 0xc0, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x00 };
  tft.fillRect(0, 0, 128, 160, 0x0);
  tft.setTextColor(0xA800);
  tft.setTextSize(1);
  tft.setFreeFont();
  tft.drawString("LOG", 48, 12);

  tft.drawRect(0, 0, 128, 25, 0xAAA0);

  tft.drawEllipse(64, 86, 38, 35, 0x57EA);

  tft.setTextColor(0x57EA);
  tft.drawString("Verified " + data, 2, 139);

  tft.drawBitmap(54, 77, image_download_bits, 20, 17, 0x57EA);
}

void notfound() {
  tft.fillRect(0, 0, 128, 160, 0x0);
  tft.setTextColor(0xFAAA);
  tft.setTextSize(1);
  tft.drawString("ERROR REGISTERING", 3, 137);
}

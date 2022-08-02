#include "Arduino.h"

#include <HTTPClient.h>
#include <WiFi.h>

#include <NTPClient.h>
#include <WiFiUdp.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <Firebase_ESP_Client.h>
#include "time.h"
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define API_KEY "paste API key here"
#define USER_EMAIL "paste Email here"
#define USER_PASSWORD "paste Password here"
#define DATABASE_URL "paste Database URL here"

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Wifi setup
const char* ssid = "paste Network name here";
const char* password =  "paste Password here";
HTTPClient http;
bool online = false;

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// Firebase Setup
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
String uid;
String databasePath;
String globalTimestamp;
FirebaseJson json;

// Declare pin numbers
int inputPinAnalog =  32;
int inputPin1 = 4;
int inputPin2 = 0;

unsigned long localTime = millis();
int input1 = HIGH;
int input2 = HIGH;
int analogValue = 4095;

int patience = 10;
int interval = 1000;

int analogValues[60];
unsigned long timeValues[60];
int count = 0;
int recorded = 0;

// Function Declerations
void outputDisplay(String message, int x, int y);
String costumDate(String formattedDate);

void setup() {
  Serial.begin(115200); // For communication with laptop
  
  // Setup display
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  delay(2000);
  outputDisplay("", 0, 0);

  // Set up pins
  pinMode(inputPin1, INPUT_PULLUP);
  pinMode(inputPin2, INPUT_PULLUP);

  // Set up wifi
  WiFi.begin(ssid, password);
  unsigned long startedWaiting = millis();
  unsigned long howLongToWait = 20 * 1000; // wait 20 seconds for connection
  while (WiFi.status() != WL_CONNECTED && millis() - startedWaiting <= howLongToWait) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  if (WiFi.status() == WL_CONNECTED){
    Serial.println("Connected to the WiFi network");
    online = true;

    // Initialize a NTPClient to get time
    timeClient.begin();
    timeClient.setTimeOffset(7200); // GMT +1 = 3600

    // Lets get Firebase runing
    config.api_key = API_KEY;
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;
    config.database_url = DATABASE_URL;
    config.token_status_callback = tokenStatusCallback;
    config.max_token_generation_retry = 5;
    Firebase.begin(&config, &auth);

    // Getting the user UID might take a few seconds
    Serial.println("Getting User UID");
    while ((auth.token.uid) == "") {
      Serial.print('.');
      delay(1000);
    }

    uid = auth.token.uid.c_str();
    Serial.print("User UID: ");
    Serial.print(uid);

    while(!timeClient.update()) {
      timeClient.forceUpdate();
    }
    globalTimestamp = costumDate(timeClient.getFormattedDate());
    databasePath = uid + "/" + globalTimestamp;
    Serial.println(databasePath);

    localTime = millis();
    Serial.println(localTime);
  } else{
    Serial.println("Did not connect to the WiFi network");
    WiFi.disconnect();
    online = false;
  }
}

void loop() {

  // Check if online
  if (WiFi.status() != WL_CONNECTED){
    online = false;
  } else {
    online = true;
  }

  // Read pins
  input1 = digitalRead(inputPin1);
  input2 = digitalRead(inputPin2);
  analogValue = analogRead(inputPinAnalog);

  if ((millis() - localTime)/interval - count > 1){
    count++;

    analogValues[recorded] = analogValue;
    timeValues[recorded] = millis() - localTime;
    recorded++;

    Serial.println(recorded);

    if(online && Firebase.ready() && recorded > patience){
      
      for(int j=0; j < recorded; j++){
        String presPath = "/" + String(timeValues[j]);
        json.set(presPath.c_str(), String(analogValues[j]));
      }
     
      Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&fbdo, databasePath.c_str(), &json) ? "ok" : fbdo.errorReason().c_str());
      recorded = 0;
    }
  }

  // reset Display
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  
  outputDisplay("Analog: " + String(analogValue), 0, 10);
  outputDisplay("Input1: " + String(input1), 0, 20);
  outputDisplay("Input2: " + String(input2), 0, 30);
  outputDisplay("Online: " + String(online), 0, 40);

  display.display();
  
  delay(100);
}

void outputDisplay(String message, int x, int y){
  display.setCursor(x, y);
  display.println(message); 
}

String costumDate(String formattedDate){
  // The formattedDate comes with the following format:
  // 2018-05-28T16:00:13Z
  // We want 08-07-2022 16:55:03
  int splitT = formattedDate.indexOf("T");
  String dateStamp = formattedDate.substring(0, splitT);
  String yearStamp = dateStamp.substring(0, 4);
  String monthStamp = dateStamp.substring(5, 7);
  String dayStamp = dateStamp.substring(8, 10);
  String timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);
  
  return dayStamp + "-" + monthStamp + "-" + yearStamp + " " + timeStamp;
}

#include <Wire.h>
#include "FirebaseESP8266.h"
#include <ESP8266WiFi.h>

// Firebase and Wi-Fi Configuration
#define FIREBASE_HOST "esp8266-nawaf-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "kbi6IBpDiUAGiHFmosDBbtvTwBSSx5CRbrEoHHUG"
#define WIFI_SSID "iPhone Nawaf"
#define WIFI_PASSWORD "88888887"

// Firebase Data Object
FirebaseData firebaseData;

// Game Variables
int slaveAddress1 = 8;
int stage = -1;
int points = -1;
int totalPoints = -1;
int timeTakenForStage = -1;
int totalTimeTaken = 0;
String stgTimeMsg;
String ttlTimeMsg;

void setup() {
  Serial.begin(115200);
  Wire.begin(D1, D2);

  // Connect to Wi-Fi
  Serial.print("Connecting to Wi-Fi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println("\nConnected to Wi-Fi!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Initialize Firebase
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

  // Log Firebase initialization status
  if (Firebase.ready()) {
    Serial.println("Firebase initialized successfully!");
  } else {
    Serial.println("Failed to initialize Firebase.");
  }
  resetScores();
}

void loop() {
  // Read data from the Arduino slave
  readDataFromSlave();

  // Only update Firebase if valid data is received
  if (stage != -1 && points != -1 && totalPoints != -1 && timeTakenForStage != -1) {
    stgTimeMsg = String(timeTakenForStage) + " Seconds";
    ttlTimeMsg = String(totalTimeTaken) + " Seconds";
    // Construct the Firebase path
    String path = "/Stages/Stage" + String(stage);
    if (stage >= 1 && stage <= 5) {
      // Create a JSON object to send
      FirebaseJson json;
      json.set("Stage", stage);
      json.set("Points", points);
      json.set("Time taken", stgTimeMsg);
      json.set("Total Points", totalPoints);
      if (stage == 5) json.set("Total time taken", ttlTimeMsg);

      // Log the JSON data
      String jsonString;
      json.toString(jsonString, true);
      Serial.print("Sending JSON to Firebase: ");
      Serial.println(jsonString);

      // Attempt to update Firebase
      if (Firebase.updateNode(firebaseData, path, json)) {
        Serial.println("Firebase update successful!");
      } else {
        Serial.print("Firebase update failed: ");
        Serial.println(firebaseData.errorReason());
      }
    }
  }

  delay(500); // Adjust the delay as needed
}

void readDataFromSlave() {
  // Request data from the Arduino slave
  Wire.requestFrom(slaveAddress1, sizeof(int) * 3);
  if (Wire.available() >= sizeof(int) * 3) {
    // Read stage, points, totalPoints and time taken
    stage = Wire.read() | (Wire.read() << 8);
    points = Wire.read() | (Wire.read() << 8);
    totalPoints = Wire.read() | (Wire.read() << 8);
    timeTakenForStage = Wire.read() | (Wire.read() << 8);
    if (stage == 5) {
      totalTimeTaken = Wire.read() | (Wire.read() << 8);
    }

    // Log received data
    Serial.print("Stage: ");
    Serial.println(stage);
    Serial.print("Points: ");
    Serial.println(points);
    Serial.print("Time Taken: ");
    Serial.println(timeTakenForStage);
  } else {
    Serial.println("No data available from Arduino.");
  }
}

void resetScores() {
  for (int i = 1; i <= 5; i++) {
    String path = "/Stages/Stage" + String(i);
    FirebaseJson json;
    json.set("Stage", i);
    json.set("Points", 0);
    json.set("Total Points", 0);
    json.set("Time taken", "N/A");
    json.set("Total time taken", "N/A");

    if (Firebase.updateNode(firebaseData, path, json)) {
      Serial.println("Reset successful for " + path);
    } else {
      Serial.print("Reset failed for " + path + ": ");
      Serial.println(firebaseData.errorReason());
    }
  }
}

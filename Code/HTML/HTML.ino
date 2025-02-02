#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FirebaseESP8266.h>

#define FIREBASE_HOST "esp8266-nawaf-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "kbi6IBpDiUAGiHFmosDBbtvTwBSSx5CRbrEoHHUG"

// Wi-Fi credentials
const char* ssid = "iPhone Nawaf";
const char* password = "88888887";

// Initialize Firebase and Web Server
FirebaseData firebaseData;
ESP8266WebServer server(80);

// Variables to hold data from Firebase
String currentStage = "1"; // Start with stage 1
String points = "0";
String totalPoints = "0";
String timeTaken = "0";
String totalTime = "N/A"; // Default to N/A if not available
bool gameWon = false; // Flag to check if the game is won
unsigned long winTime = 0; // Timer for win screen

// Timer variables
unsigned long lastUpdate = 0;
const unsigned long updateInterval = 5000; // Fetch data every 5 seconds

// Function to fetch the stage data dynamically from Firebase
void fetchFirebaseData() {
  String stagePath = "/Stages/Stage" + currentStage;
  bool errorOccurred = false;

  // Fetch points and time taken for the current stage
  if (Firebase.getString(firebaseData, stagePath + "/Points")) {
    points = firebaseData.stringData();
  } else {
    Serial.println("Error fetching points: " + firebaseData.errorReason());
    points = "N/A";
    errorOccurred = true;
  }

  if (Firebase.getString(firebaseData, stagePath + "/Time taken")) {
    timeTaken = firebaseData.stringData();
  } else {
    Serial.println("Error fetching time taken: " + firebaseData.errorReason());
    timeTaken = "N/A";
    errorOccurred = true;
  }

  // Fetch total points
  if (Firebase.getString(firebaseData, "/TotalPoints")) {
    totalPoints = firebaseData.stringData();
  } else {
    Serial.println("Error fetching total points: " + firebaseData.errorReason());
    totalPoints = "N/A";
  }

  // Debug output
  Serial.println("Firebase Data Updated:");
  Serial.println("  Current Stage: " + currentStage);
  Serial.println("  Points: " + points);
  Serial.println("  Time Taken: " + timeTaken);
  Serial.println("  Total Points: " + totalPoints);

  // Do not update the stage in this function
}

// Function to update the current stage based on conditions
void updateStage() {
  if (timeTaken != "N/A" && currentStage != "5") {
    int stageNumber = currentStage.toInt();
    if (stageNumber < 5) { // Assuming you have stages 1-5
      currentStage = String(stageNumber + 1);
      Serial.println("Moving to Stage " + currentStage);
    }
  }

  // Check if the game is won
  if (currentStage == "5" && !gameWon) {
    gameWon = true;
    winTime = millis();
    Serial.println("Game Won! Showing Win Screen in 20 seconds...");
  }
}

void setup() {
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected. IP address: " + WiFi.localIP().toString());

  // Initialize Firebase
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
  Serial.println("Firebase connected.");

  // Define route to serve dynamic HTML
  server.on("/", []() {
    // Generate dynamic HTML
    String html = R"rawliteral(
      <!DOCTYPE html>
      <html>
      <head>
          <title>Pandora's Box - Dynamic Stage</title>
          <meta http-equiv="refresh" content="5"> <!-- Auto-refresh every 5 seconds -->
          <style>
              body {
                  font-family: 'Courier New', Courier, monospace;
                  background: black;
                  background-size: cover;
                  color: #ecf0f1;
                  text-align: center;
                  margin: 0;
                  padding: 0;
              }
              h1 {
                  font-size: 3em;
                  color: #e74c3c;
                  text-shadow: 0 0 15px #e74c3c, 0 0 30px #e74c3c;
                  margin-top: 20px;
              }
              .status, .stats {
                  margin: 20px 0;
                  font-size: 1.5em;
                  color: #ecf0f1;
              }
              .footer {
                  position: fixed;
                  bottom: 10px;
                  width: 100%;
                  text-align: center;
                  color: #bdc3c7;
              }
              .win-screen {
                  background-color: green;
                  color: white;
                  font-size: 5em;
                  position: absolute;
                  top: 0;
                  bottom: 0;
                  left: 0;
                  right: 0;
                  display: flex;
                  justify-content: center;
                  align-items: center;
                  opacity: 0;
                  transition: opacity 1s;
              }
              .show-win {
                  opacity: 1;
              }
          </style>
      </head>
      <body>
          <h1>Pandora's Box - Stage )rawliteral" + currentStage + R"rawliteral(</h1>

          <!-- Status -->
          <div class="status">
              Current Stage: <b>)rawliteral" + currentStage + R"rawliteral(</b>
          </div>

          <!-- Stats -->
          <div class="stats">
              <p>Points for this stage: <b>)rawliteral" + points + R"rawliteral(</b></p>
              <p>Total Points: <b>)rawliteral" + totalPoints + R"rawliteral(</b></p>
              <p>Time Taken for this stage: <b>)rawliteral" + timeTaken + R"rawliteral(</b> seconds</p>
              <p>Total Time Taken: <b>)rawliteral" + totalTime + R"rawliteral(</b> seconds</p>
          </div>

          <!-- Footer -->
          <div class="footer">
              <p>&copy; 2024 Pandora's Box Game</p>
          </div>

          <!-- Win Screen -->
          <div id="win-screen" class="win-screen">
              You Win
          </div>

          <script>
            // Show the win screen if the game is won and 20 seconds have passed
            if (")rawliteral" + (gameWon ? "true" : "false") + R"rawliteral(" === "true" && Date.now() - ")rawliteral" + winTime + R"rawliteral(" >= 20000) {
              document.getElementById('win-screen').classList.add('show-win');
            }
          </script>
      </body>
      </html>
    )rawliteral";

    server.send(200, "text/html", html);
  });

  // Start the server
  server.begin();
  Serial.println("HTTP server started.");

  // Fetch initial data
  fetchFirebaseData();
}

void loop() {
  // Handle client requests
  server.handleClient();

  // Periodically fetch Firebase data and update stage
  if (millis() - lastUpdate >= updateInterval) {
    fetchFirebaseData();
    updateStage();
    lastUpdate = millis();
  }
}

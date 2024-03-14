#include <WiFi.h>
#include <Wire.h>
#include <time.h>
#include <Arduino_BuiltIn.h>
#include <PubSubClient.h>
#include "utils.h"

#define echoPin 2               // CHANGE PIN NUMBER HERE IF YOU WANT TO USE A DIFFERENT PIN
#define trigPin 4               // CHANGE PIN NUMBER HERE IF YOU WANT TO USE A DIFFERENT PIN

long duration, distance;
unsigned long previousTime = 0; // Variable to store the last time a reading was taken
const long interval = 10000;    // Interval between readings (in milliseconds)

const char* ssid = "Utkarsh";
const char* password = "12345678";

void setup() {
  Serial.begin(9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  // Connect to Wi-Fi
  connectToWiFi();

  // Initialize and synchronize time with NTP server
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");

  // Wait for time to synchronize
  while (!time(nullptr)) {
    delay(1000);
    Serial.println("Waiting for time synchronization...");
  }

  connectAWS(); // Connect to AWS IoT
}

void loop() {
  unsigned long currentTime = millis(); // Get the current time
  
  if (currentTime - previousTime >= interval) { // Check if it's time to take a new reading
    previousTime = currentTime; // Update the previous time
    
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    
    duration = pulseIn(echoPin, HIGH);
    distance = duration / 58.2;
    
    // Get current time
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
      Serial.println("Failed to obtain time");
      return;
    }
    
    // Print timestamp
    Serial.print("Timestamp: ");
    printTime(timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    
    // Print distance
    Serial.print(" - Distance: ");
    Serial.print(distance);
    Serial.println(" cm");

    // Publish timestamp and distance to AWS
    publishMessage(timeinfo, distance);
  }

  client.loop();
  delay(1000);
}

void printTime(int hours, int minutes, int seconds) {
  // Print the time in HH:MM:SS format
  if (hours < 10) Serial.print("0");
  Serial.print(hours);
  Serial.print(":");
  if (minutes < 10) Serial.print("0");
  Serial.print(minutes);
  Serial.print(":");
  if (seconds < 10) Serial.print("0");
  Serial.print(seconds);
}

void connectToWiFi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to Wi-Fi");
}

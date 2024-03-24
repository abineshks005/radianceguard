#include <WiFi.h>
#include <ThingSpeak.h>
#include <Arduino.h>
#include <HTTPClient.h> // Include the HTTPClient library

// Replace with your WiFi credentials and ThingSpeak API keys
const char* ssid = "vivo 1920";
const char* password = "ksvp1234";
const char* apiWriteKey = "88H8OQPY55B78RH8";
const int channelID = 2446165;

const int sensorPin = 35;
const int speakerPin = 5;
const int thresholdHigh = 3500;

int sensorValue;
bool isAlarmOn = false;

WiFiClient client;

// Function declaration
String base64_encode(String data);

void setup() {
  Serial.begin(9600);
  pinMode(speakerPin, OUTPUT);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to WiFi");
  ThingSpeak.begin(client);
}

void loop() {
  sensorValue = analogRead(sensorPin);
  Serial.println(sensorValue);

  if (sensorValue > thresholdHigh && !isAlarmOn) {
    int writeStatus = ThingSpeak.writeField(channelID, 8, sensorValue, apiWriteKey);
    Serial.print("Write Status: ");
    Serial.println(writeStatus);

    // Send SMS notification using Twilio
    sendSMSNotification();

    tone(speakerPin, 1000);
    delay(500);

    isAlarmOn = true;
  } else if (sensorValue <= thresholdHigh && isAlarmOn) {
    noTone(speakerPin);
    isAlarmOn = false;
  }

  delay(1000);
}

void sendSMSNotification() {
  HTTPClient http; // Declare an object of class HTTPClient

  // Construct the Twilio API URL with your account SID and authentication token
  String url = "https://api.twilio.com/2010-04-01/Accounts/ACbbdb1a9c3942f2973120416566173421/Messages.json";
  
  // Replace YOUR_ACCOUNT_SID, YOUR_AUTH_TOKEN, YOUR_TWILIO_NUMBER, and YOUR_DESTINATION_NUMBER with your Twilio credentials
  String message = "Your sensor value exceeded the threshold!";
  String postData = "From=+17816795326&To=+916374313452&Body=" + message;
  
  // HTTP Basic Authentication
  String auth = "ACbbdb1a9c3942f2973120416566173421:73d7ebae2593a558113cb3a99e8ecfbd";
  String authHeader = "Basic " + base64_encode(auth);
  
  // Set HTTP headers
  http.begin(url);
  http.addHeader("Authorization", authHeader);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  
  // Make an HTTP POST request
  int httpResponseCode = http.POST(postData);
  
  // Print the HTTP response code
  Serial.print("HTTP Response code: ");
  Serial.println(httpResponseCode);
  
  // Print the response from Twilio
  String response = http.getString();
  Serial.println(response);
  
  // End HTTP connection
  http.end();
}

// Function definition for base64_encode
String base64_encode(String data) {
  static const char* b64chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  String encoded = "";
  int i = 0;

  while (i < data.length()) {
    // Extract 3 bytes (24 bits)
    uint32_t octet_a = i < data.length() ? (unsigned char)data[i++] : 0;
    uint32_t octet_b = i < data.length() ? (unsigned char)data[i++] : 0;
    uint32_t octet_c = i < data.length() ? (unsigned char)data[i++] : 0;

    // Combine the 3 bytes into a 24-bit number
    uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

    // Split the 24-bit number into four 6-bit numbers and encode them
    encoded += b64chars[(triple >> 3 * 6) & 0x3F];
    encoded += b64chars[(triple >> 2 * 6) & 0x3F];
    encoded += b64chars[(triple >> 1 * 6) & 0x3F];
    encoded += b64chars[(triple >> 0 * 6) & 0x3F];
  }

  // Pad the base64 string with '=' characters as needed
  while (encoded.length() % 4 != 0) {
    encoded += '=';
  }

  return encoded;
}
#include <WiFi.h>
#include <HttpClient.h>
#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include <Arduino.h>

char ssid[] = "Sigma Apple Pi chapter";         // your network SSID (name) 
char pass[] = "dontpeeinthepool69";     // your network password

// Name of the server we want to connect to
const char serverAddress[] = "54.177.222.57";
int port = 5000;

// Timing
unsigned long lastRead = 0;
unsigned long readInterval = 5000;

// Number of milliseconds to wait without receiving any data before we give up
const int kNetworkTimeout = 30*1000;
// Number of milliseconds to wait if no data is available before trying again
const int kNetworkDelay = 1000;

//Pin definitions
#define SDA_PIN 21
#define SCL_PIN 22

// Adafruit connection
Adafruit_AHTX0 aht;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Wire.begin(SDA_PIN, SCL_PIN);

  // Connect to Wi-Fi
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);

  if (!aht.begin()) {  // ADDED
    Serial.println("Could not find AHT20 sensor.");
    while (1) {
      delay(10);
    }
  }

  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  if (millis() - lastRead >= readInterval) {
    lastRead = millis();

    sensors_event_t humEvent, tempEvent;
    aht.getEvent(&humEvent, &tempEvent);

    float humidity = humEvent.relative_humidity;
    float temp = tempEvent.temperature;

    Serial.println("~~New read~~");
    Serial.print("Humidity: ");
    Serial.println(humidity, 4);
    Serial.print("Temp: ");
    Serial.println(temp, 4);

    String path = "/?var=";
    path += temp;
    path += "_Celsius";
    path += humidity;
    path += "Percent_Humidity";

    WiFiClient client;
    HttpClient http(client);

    Serial.println("Started request...");
    int err = http.get(serverAddress, port, path.c_str());
    if (err == 0) {

      err = http.responseStatusCode();
      if (err >= 0) {
        Serial.print("Got status code: ");
        Serial.println(err);

        err = http.skipResponseHeaders();
        if (err >= 0) {
          int bodyLen = http.contentLength();
          Serial.print("Content length is: ");
          Serial.println(bodyLen);
          Serial.println();
          Serial.println("Body returned follows:");

          unsigned long timeoutStart = millis();
          char c;
          while ((http.connected() || http.available()) &&
                 ((millis() - timeoutStart) < kNetworkTimeout)) {
            if (http.available()) {
              c = http.read();
              Serial.print(c);

              bodyLen--;

              timeoutStart = millis();
            } else {
              delay(kNetworkDelay);
            }
          }
          Serial.println();
        } else {
          Serial.print("Failed to skip response headers: ");
          Serial.println(err);
        }

      } else {
        Serial.print("Getting response failed: ");
        Serial.println(err);
      }
    } else {
      Serial.print("Connect failed: ");
      Serial.println(err);
    }
    http.stop();
  }
} 
 
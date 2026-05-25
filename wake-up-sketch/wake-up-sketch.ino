#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <SD.h>
#include <SPI.h>
#include <WiFi.h>
#include <Wire.h>

#include "RTClib.h"
#include "time.h"

#include "secrets.h"

/* Definitions */

#define RXD2  16
#define TXD2  17
#define SDA   21
#define SCL   22
#define SD_CS 13

#define LINK_BAUD_RATE 19200

#define DEBUG_TIME 0

HardwareSerial Link(2);
HTTPClient http;
JsonDocument doc;
RTC_DS3231 rtc;

struct tm timeinfo;

/* Function definitions */
uint8_t checksum(String msg);
void sendCommand(String cmd);

void setupWiFi();
void setupRTC();
void setupSD();

void updateTime();

/* Constants definitions */
const char* ntpServer = "pool.ntp.org";

const char* apiServer = "192.168.1.140:5500";
const char* weatherPath = "/api/weather";

/* Core functions */
void setup() {
  // Serial connection
  Serial.begin(115200);
  // Link with the screen
  Link.begin(LINK_BAUD_RATE, SERIAL_8N1, RXD2, TXD2);
  // Timing chip function
  Wire.begin(SDA, SCL);

  // Setup WiFi
  setupWiFi();
  // Setup D3231 RTC module in time
  setupRTC();
  // Initialize SD module
  setupSD();

  // Recovery delay
  delay(1000);

  // get weather
  getCurrentWeather();
}

void loop() {
  updateTime();
  delay(1000);
}

/* Functions */
// serial functions
uint8_t checksum(String msg) {
  uint8_t sum = 0;

  for (int i = 0; i < msg.length(); i++)
    sum += msg[i];

  return sum;
}

void sendCommand(String cmd) {
  uint8_t cs = checksum(cmd);
  String packet = cmd + "|" + String(cs);

  Link.println(packet);
  Serial.print("Envoyé: ");
  Serial.println(packet);
}

// setup functions
void setupWiFi() {
  // WiFi connection
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting WiFi ");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");
}

void setupRTC() {
  // RTC initialization
  if (!rtc.begin()) {
    Serial.println("DS3231 not found");
    return;
  }

  // NTP server sync
  configTzTime("CET-1CEST,M3.5.0,M10.5.0/3", ntpServer);

  if (!getLocalTime(&timeinfo)) {
    Serial.println("NTP error");
    return;
  }

  Serial.println("Internet time received");

  // Update the DS3231 timing module
  rtc.adjust(DateTime(
    timeinfo.tm_year + 1900,
    timeinfo.tm_mon + 1,
    timeinfo.tm_mday,
    timeinfo.tm_hour,
    timeinfo.tm_min,
    timeinfo.tm_sec
  ));

  Serial.println("DS3231 in time");
}

void setupSD() {
  SPI.begin(18, 19, 23, SD_CS);

  if (!SD.begin(SD_CS, SPI)) {
    Serial.println("Erreur SD");
    return;
  }

  Serial.println("SD OK");

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);

  Serial.print("Taille SD: ");
  Serial.print(cardSize);
  Serial.println(" MB");
}

// Update functions
void updateTime() {
  DateTime now = rtc.now();

  Link.printf(
    "SET_TIME:%02d/%02d/%04d,%02d:%02d:%02d\n",
    now.day(),
    now.month(),
    now.year(),
    now.hour(),
    now.minute(),
    now.second()
  );

  if (DEBUG_TIME) Serial.printf(
    "SET_TIME:%02d/%02d/%04d,%02d:%02d:%02d\n",
    now.day(),
    now.month(),
    now.year(),
    now.hour(),
    now.minute(),
    now.second()
  );
}

void getCurrentWeather() {
  String url = String("http://") + apiServer + weatherPath;
  http.begin(url);

  // send request
  int httpResponseCode = http.GET();

  if (httpResponseCode > 0) {
    String payload = http.getString();

    // Parse JSON
    DeserializationError error = deserializeJson(doc, payload);

    if (error) {
      Serial.print("JSON parse failed: ");
      Serial.println(error.c_str());
      return;
    }

    // Extract values
    float temperature =       doc["current"]["temperature_2m"];
    float windSpeed =         doc["current"]["wind_speed_10m"];
    const char* weatherCode = doc["current"]["weather_string_code"];
    const char* sunrise =     doc["daily"]["sunrise"][0];

    // Normalize weather code
    char weatherCodeNormalized[64];
    size_t i = 0;

    for (; weatherCode[i] != '\0' && i < sizeof(weatherCodeNormalized) - 1; ++i)
      weatherCodeNormalized[i] = weatherCode[i] == '-' ? '_' : weatherCode[i];

    weatherCodeNormalized[i] = '\0';

    // Send weather to the screen
    Serial.printf(
      "SET_WEATHER:%d,%d,%s,%s\n",
      (int)(temperature),
      (int)(windSpeed),
      sunrise,
      weatherCodeNormalized
    );
    Link.printf(
      "SET_WEATHER:%d,%d,%s,%s\n",
      (int)(temperature),
      (int)(windSpeed),
      sunrise,
      weatherCodeNormalized
    );
  } else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }

  // Free resources
  http.end();
}

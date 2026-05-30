#include <ArduinoJson.h>
#include <EEPROM.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <WiFi.h>
#include <Wire.h>

#include "RTClib.h"
#include "time.h"

/* Definitions */

#define RXD2 16
#define TXD2 17
#define SDA 21
#define SCL 22

#define REMOTE_PIN   14
#define WIFI_LED_PIN 12

#define EEPROM_SIZE 128
#define WIFI_SSID_EEPROM_ADDR 0
#define WIFI_SSID_MAX_LEN 32
#define WIFI_PASSWORD_EEPROM_ADDR (WIFI_SSID_EEPROM_ADDR + WIFI_SSID_MAX_LEN + 1)
#define WIFI_PASSWORD_MAX_LEN 64

// #define LINK_BAUD_RATE 115200
#define LINK_BAUD_RATE 19200

#define DEBUG_TIME    0
#define DEBUG_WEATHER 1
#define DEBUG_WIFI    1

#define WIFI_INITIAL_CONNECT_DELAY 15000
#define WIFI_RECONNECT_DELAY 10000

HardwareSerial Link(2);
HTTPClient http;
JsonDocument doc;
RTC_DS3231 rtc;

struct tm timeinfo;

unsigned long lastWeatherPing = 9e5;
unsigned long lastWiFiReconnectAttempt = 0;
unsigned long lastTimePing = 0;

// WiFi variables
String selectedSSID = "";
String wifiPassword = "";
bool wifiPasswordLoaded = false;
bool wifiSSIDLoaded = false;

bool ssidSelected = false;

/* Function definitions */
uint8_t checksum(String msg);
void sendCommand(String cmd);

void setupWiFi();
void maintainWiFiConnection();
void setupRTC();
bool loadWiFiSSIDFromEEPROM();
bool loadWiFiPasswordFromEEPROM();
bool saveWiFiSSIDToEEPROM(const String& ssid);
bool saveWiFiPasswordToEEPROM(const String& password);

void scanNetworks();
void connectWiFi();
void handleCommand(String cmd);

void pollRemote();

void pollSerial();
void updateTime();

/* Constants definitions */
const char* ntpServer = "pool.ntp.org";

// const char* apiServer = "192.168.1.140:5500";
const char* apiServer = "172.24.125.204:5500";
const char* weatherPath = "/api/weather";

/* Core functions */
void setup() {
  // Serial connection
  Serial.begin(115200);
  // Link with the screen
  Link.begin(LINK_BAUD_RATE, SERIAL_8N1, RXD2, TXD2);
  // Timing chip function
  Wire.begin(SDA, SCL);

  // Start using the EEPROM and try to fetch the WiFi password
  EEPROM.begin(EEPROM_SIZE);
  wifiSSIDLoaded = loadWiFiSSIDFromEEPROM();
  wifiPasswordLoaded = loadWiFiPasswordFromEEPROM();

  // Setup WiFi
  pinMode(WIFI_LED_PIN, OUTPUT);
  setupWiFi();
  // Setup D3231 RTC module in time
  setupRTC();

  if (DEBUG_WIFI) {
    if (wifiSSIDLoaded) {
      Serial.print("SSID chargé depuis l'EEPROM: ");
      Serial.println(selectedSSID);
    } else {
      Serial.println("SSID introuvable dans l'EEPROM");
    }

    if (wifiPasswordLoaded) {
      Serial.println("Mot de passe WiFi chargé depuis l'EEPROM");
    } else {
      Serial.println("Mot de passe WiFi introuvable dans l'EEPROM");
    }
  }

  // Listen for any remote control events
  pinMode(REMOTE_PIN, INPUT);

  // Recovery delay
  delay(1000);

  // Init weather
  getCurrentWeather(0);
}

void loop() {
  unsigned long now = millis();

  maintainWiFiConnection();
  updateTime(now);
  getCurrentWeather(now);

  // Poll link
  if (Link.available() > 0) {
    Serial.printf("%c", Link.read());
  }

  // Poll serial
  pollSerial();

  // Poll remote controlled events
  pollRemote(now);
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

void handleCommand(String cmd) {
  cmd.trim();

  if (cmd.startsWith("USE_WIFI:")) {
    selectedSSID = cmd.substring(9);
    ssidSelected = true;

    if (saveWiFiSSIDToEEPROM(selectedSSID)) {
      wifiSSIDLoaded = true;
      Serial.println("SSID enregistré dans l'EEPROM");
    } else {
      Serial.println("NACK:WIFI_SSID_SAVE_FAILED");
    }

    Serial.print("SSID sélectionné : ");
    Serial.println(selectedSSID);

    connectWiFi();
  } else if (cmd.startsWith("WIFI_PASSWORD:")) {
    String newPassword = cmd.substring(14);

    if (saveWiFiPasswordToEEPROM(newPassword)) {
      wifiPassword = newPassword;
      wifiPasswordLoaded = true;

      Serial.println("Mot de passe enregistré dans l'EEPROM");

      if (ssidSelected) {
        connectWiFi();
      }
    } else {
      Serial.println("NACK:WIFI_PASSWORD_SAVE_FAILED");
    }
  } else if (cmd == "SCAN") {
    scanNetworks();
  } else if (cmd.startsWith("RING:1")) {
    Serial.println("Ringing the alarm with a sweet music");
    Link.printf("RING:1\n");
  } else {
    Serial.println("Commande inconnue");
  }
}

void pollSerial() {
  if (Serial.available() > 0) {  // TODO: replace Serial with Link !
    String msg = Serial.readStringUntil('\n');
    handleCommand(msg);

    // Serial.printf("\nGOT: %s\n", msg);
  }
}

// setup functions
void setupWiFi() {
  bool led_on = true;
  unsigned long start = millis();

  if (selectedSSID.isEmpty() || !wifiPasswordLoaded || wifiPassword.isEmpty()) {
    Serial.println("WiFi non configuré: SSID ou mot de passe manquant");
    return;
  }

  // WiFi connection
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
  WiFi.begin(selectedSSID.c_str(), wifiPassword.c_str());
  digitalWrite(WIFI_LED_PIN, LOW);

  Serial.print("Connecting WiFi ");

  do {
    if (DEBUG_WIFI) {
      digitalWrite(WIFI_LED_PIN, led_on);
      led_on = !led_on;
    }

    delay(500);
    Serial.print(".");
  } while (WiFi.status() != WL_CONNECTED && millis() - start < WIFI_INITIAL_CONNECT_DELAY);

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nWiFi not connected yet, will keep retrying in loop");
    lastWiFiReconnectAttempt = millis();
    return;
  }

  Serial.println("\nWiFi connected");
  digitalWrite(WIFI_LED_PIN, DEBUG_WIFI);

  lastWiFiReconnectAttempt = millis();
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
    timeinfo.tm_sec));

  Serial.println("DS3231 in time");
}

// WiFi

void maintainWiFiConnection() {
  if (WiFi.status() == WL_CONNECTED) {
    digitalWrite(WIFI_LED_PIN, DEBUG_WIFI);
    return;
  }

  digitalWrite(WIFI_LED_PIN, LOW);

  unsigned long now = millis();
  if (now - lastWiFiReconnectAttempt < WIFI_RECONNECT_DELAY) return;

  lastWiFiReconnectAttempt = now;

  Serial.println("WiFi disconnected, reconnecting...");
  if (selectedSSID.isEmpty() || !wifiPasswordLoaded || wifiPassword.isEmpty()) {
    Serial.println("WiFi reconnect ignored: SSID or password missing");
    return;
  }

  WiFi.begin(selectedSSID.c_str(), wifiPassword.c_str());
}

bool loadWiFiPasswordFromEEPROM() {
  char buffer[WIFI_PASSWORD_MAX_LEN + 1];
  size_t length = 0;

  for (; length < WIFI_PASSWORD_MAX_LEN; ++length) {
    uint8_t value = EEPROM.read(WIFI_PASSWORD_EEPROM_ADDR + length);
    if (value == 0x00 || value == 0xFF) {
      break;
    }

    buffer[length] = static_cast<char>(value);
  }

  buffer[length] = '\0';
  wifiPassword = String(buffer);

  return !wifiPassword.isEmpty();
}

bool loadWiFiSSIDFromEEPROM() {
  char buffer[WIFI_SSID_MAX_LEN + 1];
  size_t length = 0;

  for (; length < WIFI_SSID_MAX_LEN; ++length) {
    uint8_t value = EEPROM.read(WIFI_SSID_EEPROM_ADDR + length);
    if (value == 0x00 || value == 0xFF) {
      break;
    }

    buffer[length] = static_cast<char>(value);
  }

  buffer[length] = '\0';
  selectedSSID = String(buffer);
  ssidSelected = !selectedSSID.isEmpty();

  return ssidSelected;
}

bool saveWiFiSSIDToEEPROM(const String& ssid) {
  if (ssid.length() > WIFI_SSID_MAX_LEN) {
    Serial.println("SSID trop long pour l'EEPROM");
    return false;
  }

  for (size_t i = 0; i < WIFI_SSID_MAX_LEN; ++i) {
    uint8_t value = i < ssid.length() ? static_cast<uint8_t>(ssid[i]) : 0x00;
    EEPROM.write(WIFI_SSID_EEPROM_ADDR + i, value);
  }

  return EEPROM.commit();
}

bool saveWiFiPasswordToEEPROM(const String& password) {
  if (password.length() > WIFI_PASSWORD_MAX_LEN) {
    Serial.println("Mot de passe trop long pour l'EEPROM");
    return false;
  }

  for (size_t i = 0; i < WIFI_PASSWORD_MAX_LEN; ++i) {
    uint8_t value = i < password.length() ? static_cast<uint8_t>(password[i]) : 0x00;
    EEPROM.write(WIFI_PASSWORD_EEPROM_ADDR + i, value);
  }

  return EEPROM.commit();
}


void scanNetworks() {
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Scan WiFi ignoré: ESP déjà connectée");
    return;
  }

  Serial.println("\nScan des réseaux WiFi...");
  int n = WiFi.scanNetworks();

  if (n == 0) {
    Serial.println("Aucun réseau trouvé");
  } else {
    Serial.println("Réseaux disponibles :");
    for (int i = 0; i < n; ++i) {
      Serial.printf("%d: %s (RSSI: %d)\n",
                    i + 1,
                    WiFi.SSID(i).c_str(),
                    WiFi.RSSI(i));
      delay(10);
    }
  }

  WiFi.scanDelete();
}

void connectWiFi() {
  if (!ssidSelected) return;

  if (!wifiPasswordLoaded || wifiPassword.isEmpty()) {
    Serial.println("NACK:WIFI_PASSWORD_NOT_FOUND_IN_EEPROM");
    return;
  }

  Serial.println("\nConnecting WiFi...");
  Serial.print("SSID: ");
  Serial.println(selectedSSID);

  WiFi.begin(selectedSSID.c_str(), wifiPassword.c_str());

  int timeout = 20;  // 10 secondes environ
  while (WiFi.status() != WL_CONNECTED && timeout > 0) {
    delay(500);
    Serial.print(".");
    timeout--;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nACK:WIFI_CONNECTED");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nNACK:WIFI_CONNECTION_FAILED");
  }
}

// Update functions
unsigned long lastRemotePing = 0;

void pollRemote(unsigned long now) {
  if (now - lastRemotePing < 1000) return;
  lastRemotePing = now;

  // Serial.printf("Remote event: %d\n", digitalRead(REMOTE_PIN));
  if (digitalRead(REMOTE_PIN)) {
    Serial.println("Ringing the alarm with a sweet music");
    Link.printf("RING:1\n");
  }
}

void updateTime(unsigned long currentTime) {
  if (currentTime - lastTimePing < 1000) return;
  lastTimePing = currentTime;

  DateTime now = rtc.now();

  Link.printf(
    "SET_TIME:%02d/%02d/%04d,%02d:%02d:%02d\n",
    now.day(),
    now.month(),
    now.year(),
    now.hour(),
    now.minute(),
    now.second());

  if (DEBUG_TIME) Serial.printf(
    "SET_TIME:%02d/%02d/%04d,%02d:%02d:%02d\n",
    now.day(),
    now.month(),
    now.year(),
    now.hour(),
    now.minute(),
    now.second());
}

void getCurrentWeather(unsigned long now) {
  if (now - lastWeatherPing < 9e5) return; // 15 mins
  lastWeatherPing = now;

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
    float temperature = doc["current"]["temperature_2m"];
    float windSpeed = doc["current"]["wind_speed_10m"];
    const char* weatherCode = doc["current"]["weather_string_code"];
    const char* sunrise = doc["daily"]["sunrise"][0];

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
      weatherCodeNormalized);
    Link.printf(
      "SET_WEATHER:%d,%d,%s,%s\n",
      (int)(temperature),
      (int)(windSpeed),
      sunrise,
      weatherCodeNormalized);
  } else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }

  // Free resources
  http.end();
}

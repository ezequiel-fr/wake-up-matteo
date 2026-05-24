#define RXD2 16
#define TXD2 17

// definitions
HardwareSerial Link(2);

uint8_t checksum(String msg);
void sendCommand(String cmd);

// core functions
void setup() {
  Serial.begin(115200);
  Link.begin(9600, SERIAL_8N1, 16, 17);

  delay(1000);
}

void loop() {
  sendCommand("LED_ON");
  delay(5000);

  sendCommand("LED_OFF");
  delay(5000);

  // if (Link.available()) {
  //   String msg = Link.readStringUntil('\n');
  //   msg.trim();
  //   Serial.println("From screen: " + msg);
  // }
}

// functions
uint8_t checksum(String msg) {
  uint8_t sum = 0;
  for (int i = 0; i < msg.length(); i++) {
    sum += msg[i];
  }
  return sum;
}

void sendCommand(String cmd) {
  uint8_t cs = checksum(cmd);

  String packet = cmd;
  // String packet = cmd + "|" + String(cs);

  Link.println(packet);
  Serial.print("Envoyé: ");
  Serial.println(packet);
}

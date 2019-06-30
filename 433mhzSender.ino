#include <RCSwitch.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

#define OUT_PIN 4
#define PULSE_LENGTH 340 // µs

const int _codeCount = 9; // increase when adding items!
char* _codes[][2] = {
  {"switch-off",            "1001101001101101101001001001001001101101101001101001001001001101001001101"},
  {"switch-on",             "1001101001101101101001001001001001101101101001101001001001001001001001101"},
  {"dim-down",              "1001101001101101101001001001001001101101101001101001001001001001101001001"},
  {"dim-up",                "1001101001101101101001001001001001101101101001101001001001001101001001001"},
  {"toggle-lights",         "1011001011001011001011001011001001001011001011011011001"},
  {"power-up",              "1011001011001011001011001011001011001"},
  {"power-down",            "1011001011001011001011001011001001011"},
  {"toggle-power",          "1011001011001011001011001011001001001011001011011001001"},
  {"toggle-ambient-lights", "1011001011001011001011001011001011011"}
};

RCSwitch _433Sender = RCSwitch();

ESP8266WebServer server(80);

void setupIO() {
  pinMode(OUT_PIN, OUTPUT);
  _433Sender.enableTransmit(OUT_PIN);
  _433Sender.setPulseLength(PULSE_LENGTH);

  Serial.begin(115200);
}

void setupWifi(void) {
  WiFiManager wifiManager;
  wifiManager.autoConnect();
  Serial.print("Connected to ");
  Serial.println(WiFi.SSID());
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

char* findCode(String key) {
  for (int i = 0; i < _codeCount; i++) {
    if (String(_codes[i][0]).compareTo(key) == 0)
      return _codes[i][1];
  }
  return NULL;
}

void httpHandleRoot() {
  String message = "esp8266 online\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\nSSID:";
  message += WiFi.SSID();
  
  if(server.args() > 0) {
    String code = server.arg("code");
    if (code != NULL) {
      char codeArray[code.length()];
      code.toCharArray(codeArray, code.length());
      _433Sender.send(codeArray);
      
      message += "\nCode: ";
      message += code;
    }
    else if (server.arg("key") != NULL) {
      message += "\nKey: ";
      message += server.arg("key");
      
      char* code = findCode(server.arg("key"));
      if (code != NULL) {        
        _433Sender.send(code);
        
        message += "\nCode: ";
        message += String(code);
      }
      else {
        message += "\nUnknown key!";    
        server.send(504, "text/plain", message);
        return;
      }
    }
  }
  server.send(200, "text/plain", message);
}

void httpHandleTest() {
  _433Sender.send("10011010011011011010010010010010011011011010011010010010010010010010011010");
  server.send(200, "text/plain", "ok");
}

void setupHttpServer(void) {
  server.on("/", httpHandleRoot);
  server.on("/test", httpHandleTest);
  //server.onNotFound(httpHandleNotFound);
  server.begin();
  Serial.println("HTTP server started");
}

void setup() {
  setupIO();
  setupWifi();
  setupHttpServer();
}

void loop() {
  server.handleClient();
}

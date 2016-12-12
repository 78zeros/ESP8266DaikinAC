#include <IRDaikinESP.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "Credentials.h" // moved wifi credentials out into header file

#define TRIGGER_PIN 0

int hpTemp = 19;

IRDaikinESP daikinir(D1);

ESP8266WebServer server(80);

void setup() {

  Serial.begin(115200);
  daikinir.begin();
  WiFi.begin(ssid, pass);
  Serial.println("\r\nConnecting...");

  while (WiFi.status() != WL_CONNECTED) { // Wait for connection
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(WiFi.SSID());
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Webserver definitions

  server.on("/", handleRoot);

  server.on("/HPheatstatus", []() {

    if (daikinir.getPower() == 1 && daikinir.getMode() == DAIKIN_HEAT) {

      server.send(200, "text/plain", "1");
    } else {
      server.send(200, "text/plain", "0");
    }
  }
           );

  server.on("/HPcoolstatus", []() {

    if (daikinir.getPower() == 1 && daikinir.getMode() == DAIKIN_COOL) {

      server.send(200, "text/plain", "1");
    } else {
      server.send(200, "text/plain", "0");
    }
  }
           );

  server.on("/HPautostatus", []() {

    if (daikinir.getPower() == 1 && daikinir.getMode() == DAIKIN_AUTO) {

      server.send(200, "text/plain", "1");
    } else {
      server.send(200, "text/plain", "0");
    }
  }
           );

  server.on("/dummystatus", []() {
    server.send(200, "text/plain", "0");
  });

  server.on("/weather", []() {
    String message = "";
    message += "{ \"temperature\": ";
    message += hpTemp;
    message += "}";

    server.send(200, "text/plain", message);
  });

  server.on("/HPtempup", []() {
    hpTemp++;
    daikinir.setTemp(hpTemp);
    daikinir.send();
    Serial.println(hpTemp);
    //server.send(200, "text/plain", "OK\r\n");
  });

  server.on("/HPtempdown", []() {
    hpTemp--;
    daikinir.setTemp(hpTemp);
    daikinir.send();
    Serial.println(hpTemp);
    //server.send(200, "text/plain", "0");
  });

  server.on("/HPoff", []() {
    if (daikinir.getPower() == 1) {
      daikinir.off();
      daikinir.send();
      Serial.println("Switched it OFF");
    } else {
      Serial.println("Already OFF");
    }
    server.send(200, "text/plain", "OK\r\n");
  });


  server.on("/HPheat", []() {

    daikinir.on();
    //daikinir.setFan(0);
    daikinir.setMode(DAIKIN_HEAT);
    daikinir.setTemp(hpTemp);
    //daikinir.setSwingVertical(0);
    daikinir.send();
    Serial.println("Switched it on to HEAT");
    server.send(200, "text/plain", "OK\r\n");
  });

  server.on("/HPcool", []() {

    daikinir.on();
    //daikinir.setFan(0);
    daikinir.setMode(DAIKIN_COOL);
    daikinir.setTemp(hpTemp);
    //daikinir.setSwingVertical(0);
    daikinir.send();
    Serial.println("Switched it on to COOL");
    server.send(200, "text/plain", "OK\r\n");
  });

  server.on("/HPauto", []() {

    daikinir.on();
    //daikinir.setFan(0);
    daikinir.setMode(DAIKIN_AUTO);
    daikinir.setTemp(hpTemp);
    //daikinir.setSwingVertical(0);
    daikinir.send();
    Serial.println("Switched it on to AUTO");
    server.send(200, "text/plain", "OK\r\n");
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");

  //Daikin IR stuff
  //setting some sane defaults so we know what's what

  daikinir.off();
  daikinir.setFan(0);  //fan speed = auto
  daikinir.setMode(DAIKIN_AUTO); //mode = auto
  daikinir.setTemp(hpTemp); // temp = default temp defined earlier, i.e. 19 deg C
  daikinir.setSwingVertical(1); // swing on
  daikinir.send(); // send the command

  // Set up the NTP sync schedule - do this last because of the delays

}

void loop() {

  if ( digitalRead(TRIGGER_PIN) == LOW ) {
    hpTemp = 19;
    daikinir.on();

    daikinir.setFan(0);  //fan speed = auto
    daikinir.setMode(DAIKIN_AUTO); //mode = auto
    daikinir.setTemp(hpTemp); // temp = default temp defined earlier, i.e. 19 deg C
    daikinir.setSwingVertical(1); // swing on
    daikinir.send(); // send the command
    Serial.println("Default Button Triggered");
    delay(500);
  }

  server.handleClient();

}

void handleRoot() {

  String message = "Hello. ";

  message += " --- IP Address: ";
  String ipAddress = WiFi.localIP().toString();

  server.send(200, "text/html", message);
  Serial.println(message);

}

void handleNotFound() {

  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    if (server.argName(i) == "test" && server.arg(i) == "test") {
      Serial.println("success");
    }

  }
  server.send(404, "text/plain", message);

}


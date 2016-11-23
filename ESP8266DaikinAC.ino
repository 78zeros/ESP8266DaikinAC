
#include <IRDaikinESP.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <TimeLib.h>

char ssid[] = "xxxxx";  //  your network SSID (name)
char pass[] = "xxxxx";       // your network password

int hpTemp = 19;

IRDaikinESP daikinir(D1);

//web server stuffs

ESP8266WebServer server(80);

const int led = 13;

unsigned int localPort = 2390;      // local port to listen for UDP packets

IPAddress timeServerIP;
const char* ntpServerName = "time.iinet.net.au";

const int timeZone = 11;

// A UDP instance to let us send and receive packets over UDP
WiFiUDP udp;

time_t  getNtpTime();




void setup() {
  Serial.begin(115200);
  daikinir.begin();


  //web server stuff

  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  //  Serial.begin(115200);
  WiFi.begin(ssid, pass);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);

  server.on("/gettime", []() {

    handleRoot();
    time_t  getNtpTime();
  });

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

  server.on("/HPoff", []() {

    daikinir.off();
    daikinir.setFan(0);
    daikinir.setMode(DAIKIN_AUTO);
    daikinir.setTemp(hpTemp);
    daikinir.setSwingVertical(0);
    daikinir.send();
    server.send(200, "text/plain", "OFF, as requested");
    Serial.println("Switched it OFF");
  });


  server.on("/HPheat", []() {

    daikinir.on();
    daikinir.setFan(0);
    daikinir.setMode(DAIKIN_HEAT);
    daikinir.setTemp(hpTemp);
    daikinir.setSwingVertical(0);
    daikinir.send();
    server.send(200, "text/html", "<p>Heat and 19 degrees, as requested</p>");


    Serial.println("Switched it on to HEAT");
  });

  server.on("/HPcool", []() {

    daikinir.on();
    daikinir.setFan(0);
    daikinir.setMode(DAIKIN_COOL);
    daikinir.setTemp(hpTemp);
    daikinir.setSwingVertical(0);
    daikinir.send();
    server.send(200, "text/plain", "Cool and 19 degrees, as requested");
    Serial.println("Switched it on to COOL");
  });

  server.on("/HPauto", []() {

    daikinir.on();
    daikinir.setFan(0);
    daikinir.setMode(DAIKIN_AUTO);
    daikinir.setTemp(hpTemp);
    daikinir.setSwingVertical(0);
    daikinir.send();
    server.send(200, "text/plain", "Auto and 19 degrees, as requested");
    Serial.println("Switched it on to AUTO");
  });


  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");

  //NTP stuff

  // We start by connecting to a WiFi network
  Serial.print("Connecting to ");
  Serial.println(ssid);
  //  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println("Starting UDP");
  udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(udp.localPort());

  setSyncProvider(getNtpTime);
  setSyncInterval(300);
  //Daikin IR stuff

  //setting some sane defaults so we know what's what

  daikinir.off();
  daikinir.setFan(0);  //fan speed = auto
  daikinir.setMode(DAIKIN_AUTO); //mode = auto
  daikinir.setTemp(hpTemp); // temp = default temp defined earlier, i.e. 19 deg C
  daikinir.setSwingVertical(0); // swing off
  daikinir.send(); // send the command

}


void loop() {

  server.handleClient();
  timerOn(15, 0, 19, 0, DAIKIN_AUTO);

}

void timerOn(int timerHour, int timerMinute, int timerHpTemp, int timerHpFan, uint8_t timerHpMode) {
  time_t t = now(); // store the current time in time variable t
  if (hour(t) == timerHour && minute(t) == timerMinute) {

    daikinir.on();
    daikinir.setFan(timerHpFan);  //fan speed = auto = 0, otherwise 1-5
    daikinir.setMode(timerHpMode); //mode = auto = DAIKIN_AUTO
    daikinir.setTemp(timerHpTemp); // temp = default temp defined earlier, i.e. 19 deg C
    daikinir.setSwingVertical(0); // swing off
    daikinir.send(); // send the command

  }

}

void handleRoot() {
  digitalWrite(led, 1);

  String htmlContent = "<HTML><HEAD><TITLE>Besthaus Climate Control MK2</TITLE></HEAD>";
  htmlContent += "<BODY><H1>Besthaus Climate Control MK2</H1>";
  htmlContent += "<h4>Heat Pump Controls</h4><h5>";

  String message = (String)hour();
  if (minute() < 10) message += ":0"; else message += ":";
  message += (String)minute();
  if (second() < 10) message += ":0"; else message += ":";
  message += (String)second();
  if (day() < 10) message += " 0"; else message += " ";
  message += (String)day();
  if (month() < 10) message += "-0"; else message += "-";
  message += (String)month();
  message += "-";
  message += (String)year();

  htmlContent += message;

  htmlContent += "</h5><a href=\"/HPheat\">Heat to 19 degrees C</a><br>";
  htmlContent += "<a href=\"/HPcool\">Cool to 19 degrees C</a><br>";
  htmlContent += "<a href=\"/HPauto\">Auto to 19 degrees C</a><br>";
  htmlContent += "<a href=\"/HPoff\">OFF</a><br>";
  htmlContent += "<a href=\"/gettime\">Update Time</a><br>";

  server.send(200, "text/html", htmlContent);
  digitalWrite(led, 0);

}

void handleNotFound() {
  digitalWrite(led, 1);
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
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}

const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

time_t getNtpTime()
{
  IPAddress ntpServerIP; // NTP server's ip address

  while (udp.parsePacket() > 0) ; // discard any previously received packets
  Serial.println("Transmit NTP Request");
  // get a random server from the pool
  WiFi.hostByName(ntpServerName, ntpServerIP);
  Serial.print(ntpServerName);
  Serial.print(": ");
  Serial.println(ntpServerIP);
  sendNTPpacket(ntpServerIP);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Receive NTP Response");
      udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }
  Serial.println("No NTP Response :-(");
  return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(address, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}

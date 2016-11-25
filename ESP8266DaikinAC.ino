
#include <IRDaikinESP.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
//#include <WiFiClient.h> // don't actually need this right now
#include <ESP8266WebServer.h>
#include <TimeLib.h>
#include "Credentials.h" // moved wifi credentials out into header file
//#include <ArduinoOTA.h> // will look to add in OTA updating when I get the rest working!

int hpTemp = 19;

IRDaikinESP daikinir(D1);

//web server stuffs

ESP8266WebServer server(80);

const int led = 13;

unsigned int localPort = 2390;      // local port to listen for UDP packets

IPAddress timeServerIP;
const char* ntpServerName = "au.pool.ntp.org";

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
  //Serial.begin(115200);
  delay(1000); //give everything else a moment to init
  WiFi.begin(ssid, pass);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(WiFi.status());
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

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

  server.on("/HPoff", []() {

    daikinir.off();
    //daikinir.setFan(0);
    //daikinir.setMode(DAIKIN_AUTO);
    //daikinir.setTemp(hpTemp);
    //daikinir.setSwingVertical(0);
    daikinir.send();
    Serial.println("Switched it OFF");
/*
    String htmlContent = "<head><meta http-equiv=\"Refresh\" content=\"0; url=http://";
    String ipAddress = WiFi.localIP().toString();
    htmlContent += ipAddress;

    htmlContent += "\" /></head><p>OFF, as requested</p>";

    server.send(200, "text/html", htmlContent);
*/
  });


  server.on("/HPheat", []() {

    daikinir.on();
    //daikinir.setFan(0);
    daikinir.setMode(DAIKIN_HEAT);
    //daikinir.setTemp(hpTemp);
    //daikinir.setSwingVertical(0);
    daikinir.send();


    Serial.println("Switched it on to HEAT");
    /*
    String htmlContent = "<head><meta http-equiv=\"Refresh\" content=\"0; url=http://";
    String ipAddress = WiFi.localIP().toString();
    htmlContent += ipAddress;

    htmlContent += "\" /></head><p>HEAT, as requested</p>";

    server.send(200, "text/html", htmlContent);
 */
  });

  server.on("/HPcool", []() {

    daikinir.on();
    //daikinir.setFan(0);
    daikinir.setMode(DAIKIN_COOL);
    //daikinir.setTemp(hpTemp);
    //daikinir.setSwingVertical(0);
    daikinir.send();
    Serial.println("Switched it on to COOL");
/*
    String htmlContent = "<head><meta http-equiv=\"Refresh\" content=\"0; url=http://";
    String ipAddress = WiFi.localIP().toString();
    htmlContent += ipAddress;

    htmlContent += "\" /></head><p>COOL, as requested</p>";

    server.send(200, "text/html", htmlContent);
*/
  });

  server.on("/HPauto", []() {

    daikinir.on();
    daikinir.setFan(0);
    daikinir.setMode(DAIKIN_AUTO);
    daikinir.setTemp(hpTemp);
    daikinir.setSwingVertical(0);
    daikinir.send();
    Serial.println("Switched it on to AUTO");
   /* String htmlContent = "<head><meta http-equiv=\"Refresh\" content=\"0; url=http://";
    String ipAddress = WiFi.localIP().toString();
    htmlContent += ipAddress;

    htmlContent += "\" /></head><p>AUTO, as requested</p>";

    server.send(200, "text/html", htmlContent);

    */
  });


  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");

  //NTP stuff
  Serial.println("Starting UDP");
  udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(udp.localPort());


  //Daikin IR stuff

  //setting some sane defaults so we know what's what

  daikinir.off();
  daikinir.setFan(0);  //fan speed = auto
  daikinir.setMode(DAIKIN_AUTO); //mode = auto
  daikinir.setTemp(hpTemp); // temp = default temp defined earlier, i.e. 19 deg C
  daikinir.setSwingVertical(1); // swing on
  daikinir.send(); // send the command

  // Set up the NTP sync schedule - do this last because of the delays

  setSyncProvider(getNtpTime);
  setSyncInterval(300);

}


void loop() {

  server.handleClient();
  timerOn(15, 0, 19, 0, DAIKIN_AUTO, 1); // 3pm, 19 deg, auto fan, auto mode, swing on

}

void timerOn(int timerHour, int timerMinute, int timerHpTemp, int timerHpFan, uint8_t timerHpMode, int timerHpSwing) {
  time_t t = now(); // store the current time in time variable t

  if (hour(t) == timerHour && minute(t) == timerMinute && second(t) == 1) {

    daikinir.on();
    daikinir.setFan(timerHpFan);  //fan speed = auto = 0, otherwise 1-5
    daikinir.setMode(timerHpMode); //mode = auto = DAIKIN_AUTO, _HEAT, _COOL, _DRY
    daikinir.setTemp(timerHpTemp); // temp = default temp defined earlier, i.e. 19 deg C
    daikinir.setSwingVertical(timerHpSwing); // swing on/off 1/0
    daikinir.send(); // send the command
    Serial.println("Timer Triggered");
    Serial.println("");
    delay(1000);
  }

}



void handleRoot() {
  digitalWrite(led, 1);

  /* String htmlContent = "<HTML><HEAD><TITLE>Besthaus Climate Control MK2</TITLE></HEAD>";
  htmlContent += "<BODY><H1>Besthaus Climate Control MK2</H1>";
  htmlContent += "<h4>Heat Pump Controls</h4>";
*/
  String message = "Hello. ";
  if (hour() < 10) message += "Time: 0"; else message += "Time: ";
  message += (String)hour();
  if (minute() < 10) message += ":0"; else message += ":";
  message += (String)minute();
  if (second() < 10) message += ":0"; else message += ":";
  message += (String)second();
  if (day() < 10) message += " | Date: 0"; else message += "  Date: ";
  message += (String)day();
  if (month() < 10) message += "-0"; else message += "-";
  message += (String)month();
  message += "-";
  message += (String)year();
  message += "  IP Address: ";
  String ipAddress = WiFi.localIP().toString();
  message += ipAddress;

  /* htmlContent += message;

  htmlContent += "</h5><a href=\"/HPheat\">Heat to 19 degrees C</a><br>";
  htmlContent += "<a href=\"/HPcool\">Cool to 19 degrees C</a><br>";
  htmlContent += "<a href=\"/HPauto\">Auto to 19 degrees C</a><br>";
  htmlContent += "<a href=\"/HPoff\">OFF</a><br><br>";
  */
  server.send(200, "text/plain", message);
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
      Serial.println(secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR);
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

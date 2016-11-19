
#include <IRDaikinESP.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

char ssid[] = "xxxxx";  //  your network SSID (name)
char pass[] = "xxxxx";       // your network password

int daylightSaving = 0;

int timeHour = 0;
int timeMin = 0;
int timeSec = 0;


int hpState = 0;
int hpTemp = 19;

IRDaikinESP dakinir(D1);

//web server stuffs

ESP8266WebServer server(80);

const int led = 13;

void handleRoot() {
  digitalWrite(led, 1);

  char htmlContent[] = "<HTML><HEAD><TITLE>Besthaus Climate Control MK2</TITLE></HEAD>"
                       "<BODY><H1>Besthaus Climate Control MK2</H1><h4>Heat Pump Controls</h2>"
                       "<a href=\"/HPheat\">Heat to 19 degrees C</a><br>"
                       "<a href=\"/HPcool\">Cool to 19 degrees C</a><br>"
                       "<a href=\"/HPauto\">Auto to 19 degrees C</a><br>"
                       "<a href=\"/HPoff\">OFF</a><br>";

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



unsigned int localPort = 2390;      // local port to listen for UDP packets

/* Don't hardwire the IP address or we won't get the benefits of the pool.
    Lookup the IP address for the host name instead */
//IPAddress timeServer(129, 6, 15, 28); // time.nist.gov NTP server
IPAddress timeServerIP; // time.nist.gov NTP server address
const char* ntpServerName = "time.iinet.net.au";

const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message

byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

// A UDP instance to let us send and receive packets over UDP
WiFiUDP udp;


void setup() {
  dakinir.begin();
  //Serial.begin(115200);

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

server.on("/checkstatus", []() {
  
  server.send(200, "text/plain", String(hpState));
});

  server.on("/HPheatstatus", []() {

    switch (hpState) {
      case 1:
        server.send(200, "text/plain", "1");
        break;

      case 2:
      case 3:
      default:
        server.send(200, "text/plain", "0");
        break;
    }
  }
           );

  server.on("/HPcoolstatus", []() {

    switch (hpState) {
      case 2:
        server.send(200, "text/plain", "1");
        break;

      case 1:
      case 3:
      default:
        server.send(200, "text/plain", "0");
        break;
    }
  }
           );
  server.on("/HPautostatus", []() {

    switch (hpState) {
      case 3:
        server.send(200, "text/plain", "1");
        break;

      case 1:
      case 2:
      default:
        server.send(200, "text/plain", "0");
        break;
    }
  }
           );

  server.on("/HPoff", []() {
    hpState = 0;

    dakinir.off();
    dakinir.setFan(0);
    dakinir.setMode(DAIKIN_AUTO);
    dakinir.setTemp(hpTemp);
    dakinir.setSwingVertical(0);
    dakinir.send();
    server.send(200, "text/plain", "OFF, as requested");
    Serial.println("Switched it OFF");
  });


  server.on("/HPheat", []() {

    hpState = 1;

    dakinir.on();
    dakinir.setFan(0);
    dakinir.setMode(DAIKIN_HEAT);
    dakinir.setTemp(hpTemp);
    dakinir.setSwingVertical(0);
    dakinir.send();
    server.send(200, "text/html", "<p>Heat and 19 degrees, as requested</p>");


    Serial.println("Switched it on to HEAT");
  });

  server.on("/HPcool", []() {
    hpState = 2;

    dakinir.on();
    dakinir.setFan(0);
    dakinir.setMode(DAIKIN_COOL);
    dakinir.setTemp(hpTemp);
    dakinir.setSwingVertical(0);
    dakinir.send();
    server.send(200, "text/plain", "Cool and 19 degrees, as requested");
    Serial.println("Switched it on to COOL");
  });

  server.on("/HPauto", []() {
    hpState = 3;

    dakinir.on();
    dakinir.setFan(0);
    dakinir.setMode(DAIKIN_AUTO);
    dakinir.setTemp(hpTemp);
    dakinir.setSwingVertical(0);
    dakinir.send();
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

  //Daikin IR stuff

  //setting some sane defaults so we know what's what

  dakinir.off();
  dakinir.setFan(0);
  dakinir.setMode(DAIKIN_AUTO);
  dakinir.setTemp(hpTemp);
  dakinir.setSwingVertical(0);
  dakinir.send();

  hpState = 0;
  getTime();
}


void loop() {

  server.handleClient();

  if (timeHour == 1) {
    getTime();
  }
}


void getTime() {
  //NTP function
  //get a random server from the pool
  WiFi.hostByName(ntpServerName, timeServerIP);

  sendNTPpacket(timeServerIP); // send an NTP packet to a time server
  // wait to see if a reply is available
  delay(1000);

  int cb = udp.parsePacket();
  if (!cb) {
    Serial.println("no packet yet");
  }
  else {
    Serial.print("packet received, length=");
    Serial.println(cb);
    // We've received a packet, read the data from it
    udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, esxtract the two words:

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    Serial.print("Seconds since Jan 1 1900 = " );
    Serial.println(secsSince1900);

    // now convert NTP time into everyday time:
    Serial.print("Unix time = ");
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;
    // subtract seventy years:
    unsigned long epoch = secsSince1900 - seventyYears;
    // print Unix time:
    Serial.println(epoch);

    // print the hour, minute and second:
    Serial.print("The current time is ");       // UTC is the time at Greenwich Meridian (GMT)

    switch (daylightSaving) {

      case 1:
        Serial.print(((epoch  % 86400L) / 3600) + 11); // print the hour (86400 equals secs per day)
        timeHour = (((epoch  % 86400L) / 3600) + 11);
        break;

      default:
      case 0:
        Serial.print(((epoch  % 86400L) / 3600) + 10); // print the hour (86400 equals secs per day)
        timeHour = (((epoch  % 86400L) / 3600) + 10);
        break;
    }

    //Serial.print(((epoch  % 86400L) / 3600)+ 11); // print the hour (86400 equals secs per day)
    Serial.print(':');
    if ( ((epoch % 3600) / 60) < 10 ) {
      // In the first 10 minutes of each hour, we'll want a leading '0'
      Serial.print('0');
    }
    Serial.print((epoch  % 3600) / 60); // print the minute (3600 equals secs per minute)
    timeMin = ((epoch  % 3600) / 60);
    Serial.print(':');
    if ( (epoch % 60) < 10 ) {
      // In the first 10 seconds of each minute, we'll want a leading '0'
      Serial.print('0');
    }
    Serial.println(epoch % 60); // print the second
    timeSec = (epoch % 60);


  }
}

unsigned long sendNTPpacket(IPAddress& address)
{
  Serial.println("sending NTP packet...");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(address, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}

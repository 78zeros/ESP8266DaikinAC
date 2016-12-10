
#include <IRDaikinESP.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ESP8266WebServer.h>
#include <TimeLib.h>
#include "Credentials.h" // moved wifi credentials out into header file
#include <EEPROM.h>

#define TRIGGER_PIN 0

int hpTemp = 19;

IRDaikinESP daikinir(D1);

ESP8266WebServer server(80);

unsigned int localPort = 2390;      // local port to listen for UDP packets

IPAddress timeServerIP;
const char* ntpServerName = "au.pool.ntp.org";

const int timeZone = 11;

String dataToRead;

WiFiUDP udp;

time_t  getNtpTime();


void setup() {

  Serial.begin(115200);
  daikinir.begin();
  EEPROM.begin(512);
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

  server.on("/HPoff", []() {

    daikinir.off();
    daikinir.send();
    Serial.println("Switched it OFF");
    server.send(200, "text/plain", "OK\r\n");
  });


  server.on("/HPheat", []() {

    daikinir.on();
    //daikinir.setFan(0);
    daikinir.setMode(DAIKIN_HEAT);
    //daikinir.setTemp(hpTemp);
    //daikinir.setSwingVertical(0);
    daikinir.send();
    Serial.println("Switched it on to HEAT");
    server.send(200, "text/plain", "OK\r\n");
  });

  server.on("/HPcool", []() {

    daikinir.on();
    //daikinir.setFan(0);
    daikinir.setMode(DAIKIN_COOL);
    //daikinir.setTemp(hpTemp);
    //daikinir.setSwingVertical(0);
    daikinir.send();
    Serial.println("Switched it on to COOL");
    server.send(200, "text/plain", "OK\r\n");
  });

  server.on("/HPauto", []() {

    daikinir.on();
    //daikinir.setFan(0);
    daikinir.setMode(DAIKIN_AUTO);
    //daikinir.setTemp(hpTemp);
    //daikinir.setSwingVertical(0);
    daikinir.send();
    Serial.println("Switched it on to AUTO");
    server.send(200, "text/plain", "OK\r\n");
  });

  // completely experimental

  server.on("/HPtimerread", []() {

    for (int i = 0; i < 27; i++) {

      dataToRead += char(EEPROM.read(i));
      Serial.print("Read: ");
      Serial.println(dataToRead[i]);
    }
    //Serial.println(dataToRead);
    Serial.println("");
//if ( char(EEPROM.read(7)) == 0) {
//  Serial.println("Off");
//} else {
//  Serial.println("On");
//}
    //
    //    String hpTestVar;
    //    hpTestVar += char(EEPROM.read(0));
    //    hpTestVar += char(EEPROM.read(1));
    //    int hpTestVar2 = hpTestVar.toInt();
    //
    //    for (int i = 0; i < 19; i++) {
    //      Serial.print(dataToRead[i]);
    //      switch (i) {
    //        case 1:
    //        case 3:
    //        case 5:
    //        case 6:
    //        case 7:
    //          Serial.print(",");
    //          break;
    //        default:
    //          break;
    //      }
    //    }
    //    Serial.println(dataToRead);
    //    Serial.println(hpTestVar2);
String message = "";
message += char(EEPROM.read(7));

switch (message.toInt()) {
  case 0:
  message = "Off\r\n";
  break;
  case 1:
  default:
  message = "On\r\n";
  break;
}

    server.send(200, "text/plain", message);
  });


  server.on("/HPtimerset", []() {

    String dataToWrite;

    for (uint8_t i = 0; i < server.args(); i++) {

      if (i == 8 || i == 10) {
        dataToWrite += addZero(server.arg(i));
      }
      else
      {
        dataToWrite += server.arg(i);
      }
    }

    //    Serial.println("This is what I will write");
    //    Serial.println(dataToWrite);
    //
    //uint8_t dummyDays = "1111111";



    for (int i = 0; i < 27; i++) {

      EEPROM.write(i, dataToWrite[i]);
      Serial.print("Wrote: ");
      Serial.println(dataToWrite[i]);
    }
    EEPROM.commit();
    Serial.println("");

    //
    //    // timerOn(15, 0, 19, 0, 1, DAIKIN_AUTO); // 3pm, 19 deg, auto fan, swing on, auto mode
    //    Serial.println("You requested:");
    //    Serial.print("Turning on at ");
    //
    //
    //
    //    int onHour = server.arg(0).toInt();
    //
    //    if (onHour < 10 && !server.arg(0).startsWith("0")) {
    //      String onHourStr = "0";
    //      onHourStr += server.arg(0);
    //      Serial.print(onHourStr);
    //    }
    //    else {
    //      String onHourStr = "";
    //      onHourStr += server.arg(0);
    //      Serial.print(onHourStr);
    //    }
    //
    //    int onMin = server.arg(1).toInt();
    //
    //    if (onMin < 10 && !server.arg(1).startsWith("0")) {
    //      String onMinStr = "0";
    //      onMinStr += server.arg(1);
    //      Serial.print(onMinStr);
    //    }
    //    else {
    //      String onMinStr = "";
    //      onMinStr += server.arg(1);
    //      Serial.print(onMinStr);
    //    }
    //
    //    //Serial.print(server.arg(1));
    //    Serial.println("");
    //    Serial.print(server.arg(2));
    //    Serial.print(" degrees\r\n");
    //
    //    int fanSpeed = server.arg(3).toInt();
    //
    //    switch (fanSpeed) {
    //      case 0:
    //        Serial.println("Auto Fan");
    //        break;
    //      default:
    //        Serial.print("Fan Speed: ");
    //        Serial.print(server.arg(3));
    //        break;
    //    }
    //
    //    int swingMode = server.arg(4).toInt();
    //
    //    switch (swingMode) {
    //      case 0:
    //        Serial.println("Swing Off");
    //        break;
    //      default:
    //        Serial.println("Swing On");
    //        break;
    //    }
    //    Serial.print("Selected mode: ");
    //    Serial.print(server.arg(5));
    //    Serial.println("");


    server.send(200, "text/plain", "OK\r\n");
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

  if ( digitalRead(TRIGGER_PIN) == LOW ) {
    daikinir.off();
    daikinir.send(); // send the command
    delay(500);
    Serial.println("Off Button Triggered");
  }

  server.handleClient();

  //if (timeStatus() == timeNotSet || timeStatus() == timeNeedsSync) setSyncInterval(30);

  //  timerOn(15, 0, 19, 0, 1, DAIKIN_AUTO); // 3pm, 19 deg, auto fan, swing on, auto mode
    timerOff(22, 5); //10:05pm
String timerEnabled = "";
timerEnabled += char(EEPROM.read(7));
if (timerEnabled.toInt() == 1) timerOnEEPROM();

}

void timerOn(uint8_t timerHour, uint8_t timerMinute, uint8_t timerHpTemp, uint8_t timerHpFan, uint8_t timerHpSwing, uint8_t timerHpMode) {
  time_t t = now(); // store the current time in time variable t

  if (hour(t) == timerHour && minute(t) == timerMinute && second(t) == 1) {

    daikinir.on();
    daikinir.setFan(timerHpFan);  //fan speed = auto = 0, otherwise 1-5
    daikinir.setMode(timerHpMode); //mode = auto = DAIKIN_AUTO, _HEAT, _COOL, _DRY
    daikinir.setTemp(timerHpTemp); // temp = default temp defined earlier, i.e. 19 deg C
    daikinir.setSwingVertical(timerHpSwing); // swing on/off 1/0
    daikinir.send(); // send the command
    Serial.println("On Timer Triggered");
    Serial.println("");
    delay(1000);
  }

}

void timerOnEEPROM() {

  time_t t = now();

  String timerHour;
  timerHour += char(EEPROM.read(8));
  timerHour += char(EEPROM.read(9));

  String timerMinute;
  timerMinute += char(EEPROM.read(10));
  timerMinute += char(EEPROM.read(11));

  char timerHpTemp;
  timerHpTemp += char(EEPROM.read(12));
  timerHpTemp += char(EEPROM.read(13));


  char timerHpFan = char(EEPROM.read(14));
  char timerHpSwing = char(EEPROM.read(15));

  char timerHpMode;

  for (int i = 16; i < 27; i++) {
    timerHpMode += char(EEPROM.read(i));
  }


  if (hour(t) == timerHour.toInt() && minute(t) == timerMinute.toInt() && second(t) == 1) {

    daikinir.on();
    daikinir.setFan(timerHpFan);  //fan speed = auto = 0, otherwise 1-5
    daikinir.setMode(timerHpMode); //mode = auto = DAIKIN_AUTO, _HEAT, _COOL, _DRY
    daikinir.setTemp(timerHpTemp); // temp = default temp defined earlier, i.e. 19 deg C
    daikinir.setSwingVertical(timerHpSwing); // swing on/off 1/0
    daikinir.send(); // send the command
    Serial.println("EEPROM On Timer Triggered");
    Serial.println("");
    delay(1000);

  }

}

void timerOff(int timerHour, int timerMinute) {
  time_t t = now(); // store the current time in time variable t

  if (hour(t) == timerHour && minute(t) == timerMinute && second(t) == 1 && daikinir.getPower() == 1) {
    daikinir.off();
    daikinir.send(); // send the command
    Serial.println("Off Timer Triggered");
    Serial.println("");
    delay(1000);
  }
}


String addZero(String val) {

  int tempVal = val.toInt();
  if (tempVal < 10 && !val.startsWith("0")) {
    String zeroVal = "0";
    zeroVal += val;
    return zeroVal;
  } else {
    return val;
  }

}

void handleRoot() {

String timerEnabled = "";
timerEnabled += char(EEPROM.read(7));

switch (timerEnabled.toInt()) {
  case 0:
  timerEnabled = "<input type='radio' name='enabled' value='1'>On <input type='radio' name='enabled' value='0' checked>Off<br>";
  break;
  case 1:
  default:
  timerEnabled = "<input type='radio' name='enabled' value='1' checked>On <input type='radio' name='enabled' value='0'>Off<br>";
  break;
}

String timerHour = "";
timerHour += char(EEPROM.read(8));
timerHour += char(EEPROM.read(9));

String timerMin = "";
timerMin += char(EEPROM.read(10));
timerMin += char(EEPROM.read(11));

String timerTemp = "";
timerTemp += char(EEPROM.read(12));
timerTemp += char(EEPROM.read(13));

String timerFan = "";
timerFan += char(EEPROM.read(14));

String timerSwing = "";
timerSwing +=char(EEPROM.read(15));

switch (timerSwing.toInt()) {
  case 0:
  timerSwing = "<input type='radio' name='swing' value='1'>On <input type='radio' name='swing' value='0' checked>Off<br>";
  break;
  case 1:
  default:
  timerSwing = "<input type='radio' name='swing' value='1' checked>On <input type='radio' name='swing' value='0'>Off<br>";
  break;
}


String timerMode;

  for (int i = 23; i < 27; i++) {
    timerMode += char(EEPROM.read(i));
  }


  String message = "Hello. ";
  if (hour() < 10) message += "Time: 0"; else message += "Time: ";
  message += (String)hour();
  if (minute() < 10) message += ":0"; else message += ":";
  message += (String)minute();
  if (second() < 10) message += ":0"; else message += ":";
  message += (String)second();
  if (day() < 10) message += " --- Date: 0"; else message += "  Date: ";
  message += (String)day();
  if (month() < 10) message += "-0"; else message += "-";
  message += (String)month();
  message += "-";
  message += (String)year();
  message += " --- IP Address: ";
  String ipAddress = WiFi.localIP().toString();
  message += ipAddress;
  message += "\r\n";
  // for ( int i =0; i < 26; i++ ) message += EEPROM.read(i);
  message += "<html><br><hr><form method='post' action='HPtimerset'>";
  message += "<label>Repeat each: </label><input type='hidden' name='mon' value='1' checked>Monday ";
  message += "<input type='hidden' name='tue' value='1' checked>Tuesday ";
  message += "<input type='hidden' name='wed' value='1' checked>Wednesday ";
  message += "<input type='hidden' name='thu' value='1' checked>Thursday ";
  message += "<input type='hidden' name='fri' value='1' checked>Friday ";
  message += "<input type='hidden' name='sat' value='1' checked>Saturday ";
  message += "<input type='hidden' name='sun' value='1' checked>Sunday<br>";
  message += "<label>Timer Enabled? <br></label>";
  message += timerEnabled;
  //message += "<input type='radio' name='enabled' value='1' checked>On ";
  //message += "<input type='radio' name='enabled' value='0'>Off<br>";
  message += "<label>Hour 0-23: </label><input type='number' name='hour' min='0' max='23' size='2' value='";
  message += timerHour;
  message += "' >";
  message += "<label>Minute 0-59: </label><input type='number' name='min' min='0' max='59' size='2' value='";
  message += timerMin;
  message += "' ><br>";
  message += "<label>Temperature: </label><input type ='number' name='temp' min='14' max='35' value='";
  message += timerTemp;
  message += "' ><br>";
  message += "<label>Fan (0=auto, 1-5): </label><input name='fan' size='1' value='";
  message += timerFan;
  message += "'><br>";
  message += "<label>Swing<br></label>";
  message += timerSwing;
//  message += "<input type='radio' name='swing' value='1' checked>On ";
//  message += "<input type='radio' name='swing' value='0'>Off<br>";
  message += "<label>Mode (Currently set to ";
  message += timerMode;
  message += ")<br></label>";
  message += "<input type='radio' name='mode' value='DAIKIN_AUTO' checked>Auto ";
  message += "<input type='radio' name='mode' value='DAIKIN_HEAT'>Heat ";
  message += "<input type='radio' name='mode' value='DAIKIN_COOL'>Cool ";
  message += "<input type='radio' name='mode' value='DAIKIN_DRY'>Dry <br>";
  message += "<input type='submit'></form>";
  message += "</html>";
  // timerOn(15, 0, 19, 0, 1, DAIKIN_AUTO); // 3pm, 19 deg, auto fan, swing on, auto mode
  // mode = auto = DAIKIN_AUTO, _HEAT, _COOL, _DRY
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
      Serial.println("Received NTP Response");
      udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      //Serial.println(secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR);
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

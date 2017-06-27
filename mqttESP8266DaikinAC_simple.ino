#include <IRsend.h>
#include <ir_Daikin.h>

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "Credentials.h" // moved wifi credentials out into header file
#include <OneWire.h>

#define TRIGGER_PIN 0

int hpTemp = 19;
float currTemp;

IRDaikinESP daikinir(D5);

OneWire ds(D4);
const char* mqtt_server = "10.1.1.4";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;


void setup() {
WiFi.mode(WIFI_STA);
  Serial.begin(115200);
  daikinir.begin();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  
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

  //Daikin IR stuff
  //setting some sane defaults so we know what's what

  daikinir.off();
  daikinir.setFan(0);  //fan speed = auto
  daikinir.setMode(DAIKIN_AUTO); //mode = auto
  daikinir.setTemp(hpTemp); // temp = default temp defined earlier, i.e. 19 deg C
  daikinir.setSwingVertical(1); // swing on
  daikinir.send(); // send the command

getTemp();
}


void callback(char* topic, byte* payload, unsigned int length) {
  String recdPayload;
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
 //   char recdPayload = (char)payload[i];
  }
  Serial.println();
 // Serial.print((char)recdPayload);

 
if (strcmp(topic, "heatpump/set/temperature") == 0) {
   for (int i = 0; i < length; i++) {
   // Serial.print((char)payload[i]);
  recdPayload += char(payload[i]);
   
  }
  hpTemp = recdPayload.toInt();
  Serial.print("Temp sent to heat pump: ");
    Serial.println(hpTemp);
    daikinir.setTemp(hpTemp);
    daikinir.send();
}

if (strcmp(topic, "heatpump/set/mode") == 0) {
   for (int i = 0; i < length; i++) {
   // Serial.print((char)payload[i]);
  recdPayload += char(payload[i]);
   
  }
  int hpMode = recdPayload.toInt();
  Serial.print("mode sending to heat pump: ");
    Serial.println(hpMode);

    switch(hpMode){
      case 0:
      daikinir.off();
      break;

      case 1:
      daikinir.on();
      daikinir.setMode(DAIKIN_HEAT);
      break;

      case 2:
      daikinir.on();
      daikinir.setMode(DAIKIN_COOL);
      break;

      case 3:
      default:
      daikinir.on();
      daikinir.setMode(DAIKIN_AUTO);
      break;
    }
    Serial.println(daikinir.getMode());
    
    daikinir.send();
}




}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("heatpump/get/status", "system ready");
      // ... and resubscribe
      client.subscribe("heatpump/set/#");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
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

//  server.handleClient();

if (!client.connected()) {
    reconnect();
  }
  client.loop();


//every minute, publish the temperature

  long now = millis();
  if (now - lastMsg > 60000) {
    lastMsg = now;
    ++value;
    snprintf (msg, 75, "%d" , int(currTemp));
    Serial.print(" ");
    Serial.println(msg);
    client.publish("heatpump/get/temperature", msg);
  }


getTemp();

}

void getTemp() {

byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];
  float celsius;
  
  if ( !ds.search(addr)) {
    Serial.print("~");
    //Serial.println();
    ds.reset_search();
    delay(1000);
    return;
  }
  

  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);        // start conversion, with parasite power on at the end
  
  delay(1000);     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.
  
  present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);         // Read Scratchpad

  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
//    Serial.print(data[i], HEX);
//    Serial.print(" ");
  }
  // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
  }
  celsius = (float)raw / 16.0;
 // Serial.print("  Temperature = ");
  Serial.print(celsius);
  //Serial.print(" Celsius, ");
 currTemp = celsius;
}

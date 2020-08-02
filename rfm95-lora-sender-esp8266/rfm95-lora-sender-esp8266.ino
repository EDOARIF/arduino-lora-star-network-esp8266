//Transfer
#include <SPI.h>
#include <LoRa.h>
#include <ArduinoJson.h>

//define the pins used by the transceiver module

#define ss 16   //D0
#define rst 2   //D4
#define dio0 15 //D8

uint8_t node_call_id = 1; //max 50, 1-25 are sensor node, 26-50 are control node
bool node_call_ready = true;

bool node_status[50];
float node_data[50];


int counter = 0;
long timer = 0;
void setup() {
  //initialize Serial Monitor
  Serial.begin(115200);

  //setup LoRa transceiver module
  LoRa.setPins(ss, rst, dio0);
  
  //replace the LoRa.begin(---E-) argument with your location's frequency 
  //433E6 for Asia
  //866E6 for Europe
  //915E6 for North America
  while (!LoRa.begin(866E6)) {
    Serial.println(".");
    delay(500);
  }
   // Change sync word (0xF3) to match the receiver
  // The sync word assures you don't get LoRa messages from other LoRa transceivers
  // ranges from 0-0xFF
  LoRa.setSyncWord(0xF3);
  Serial.println("LoRa Initializing OK!");
  timer = millis();
}

void loop() {
  //Send LoRa packet to receiver
  if(millis()-timer >= 20000){
    Serial.println(F("request node: ")); Serial.println(node_call_id);
    call_node_sensor(node_call_ready, node_call_id, node_status[node_call_id], node_data[node_call_id]);
    if(node_call_ready){
      node_call_id++;
      if(node_call_id>2)node_call_id=1;
      Serial.print("Call ready for node: ");Serial.println(node_call_id);
    }
    timer = millis();
  }
  
  
}

void call_node_sensor(bool &_call_ready, int _id, bool &_status, float &_data){
  static unsigned long previousMillis = 0;
  unsigned long currentMillis = millis();
  if(_call_ready){
    _call_ready = false;
    StaticJsonDocument<200> doc;
    doc["s"] = "m1";
    doc["i"] = _id;
    doc["d"] = "r";
  
    LoRa.beginPacket();
    serializeJson(doc, LoRa);
    LoRa.endPacket();
    previousMillis = currentMillis;
    delay(1000);

    while(true){
      int packetSize = LoRa.parsePacket();
      if (packetSize){
        while (LoRa.available()) {
          String LoRaData = LoRa.readString();
          Serial.println(LoRaData);
        }
        _call_ready = true;
        break;
      }
      if(currentMillis - previousMillis >= 20000){
        previousMillis = currentMillis;
        Serial.println(F("Timout"));
        _call_ready = true;
        break;
      }
    }
  }
}

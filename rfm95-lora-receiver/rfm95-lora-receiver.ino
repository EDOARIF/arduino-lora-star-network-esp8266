//node
#include <SPI.h>
#include <LoRa.h>
#include <ArduinoJson.h>

//define the pins used by the transceiver module
const int ss = 10;
const int rst = 5;
const int dio0 = 2;

#define receiving_led A1
#define waiting_led A0

#define node_req "r"

//(RSSI)Received Signal Strength Indication(dBm)
const int rssi_minimum = -120;  //weakest signal strength
const int rssi_offset = -30;    //fail status of rssi

bool IsTimout = false;
long timer = 0, timout = 30000;

uint8_t node_id = 2;
bool node_status = true;
float node_data = 0.45;

void setup() {
  //initialize Serial Monitor
  Serial.begin(115200);

  pinMode(receiving_led, OUTPUT);
  pinMode(waiting_led, OUTPUT);
  
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
}

void loop() {
  // try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    //Reset timout event
    timer = millis();
    IsTimout = false;

    StaticJsonDocument<200> doc;
    DeserializationError ERROR = deserializeJson(doc, LoRa);

    if (!ERROR){
      String get_station = doc["s"];
      int get_id = doc["i"];
      String get_data = doc[F("d")];
      if((get_station="m1") && (get_id=node_id) && (get_data=node_req)){
         Serial.print("id = "); Serial.println(get_id);
         Serial.print("data = "); Serial.println(get_data);
         node_status = (rssi_minimum-LoRa.packetRssi()<rssi_offset);
         node_response(node_id, node_status, node_data);
      }
    }else{
      Serial.println("Fail");
    }
  }

  if(millis()-timer >= timout){
    IsTimout = true;
  }

  digitalWrite(receiving_led, !IsTimout);
  digitalWrite(waiting_led, IsTimout);
}

void node_response(uint8_t _id, bool _status, float _data){
  StaticJsonDocument<200> node2main;
  node2main["s"] = "n1";
  node2main["i"] = _id;
  node2main["st"] = _status;
  node2main["d"] = _data;
 
  LoRa.beginPacket();
  serializeJson(node2main, LoRa);
  LoRa.endPacket();
}

//Receiver
#include <SPI.h>
#include <LoRa.h>

//define the pins used by the transceiver module
#define ss 10
#define rst 5
#define dio0 2

#define receiving_led A1
#define waiting_led A0

bool IsTimout = false;

long timer = 0, timout = 10000;

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
    
    // received a packet
    Serial.print("Received packet '");
    
    // read packet
    while (LoRa.available()) {
      String LoRaData = LoRa.readString();
      Serial.print(LoRaData);
      digitalWrite(waiting_led, 1);
    }
    
    // print RSSI of packet
    Serial.print("' with RSSI ");
    Serial.println(LoRa.packetRssi());

    delay(1000);
    LoRa.beginPacket();
    LoRa.print("Response!");
    LoRa.endPacket();
    
  }

  if(millis()-timer >= timout){
    IsTimout = true;
  }

  digitalWrite(receiving_led, !IsTimout);
  digitalWrite(waiting_led, IsTimout);
}

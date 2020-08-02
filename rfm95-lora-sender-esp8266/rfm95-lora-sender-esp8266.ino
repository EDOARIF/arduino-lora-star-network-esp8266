//Transfer
#include <SPI.h>
#include <LoRa.h>

//define the pins used by the transceiver module

#define ss 16   //D0
#define rst 2   //D4
#define dio0 15 //D8

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
  LoRa.beginPacket();
  LoRa.print("AdsfgdsfgsdfgsdfgsdfgsdfgsdfgsdfgsdA");
  LoRa.print(counter);
  LoRa.endPacket();
  counter++;

  delay(1000);

  Serial.print(F("waiting: "));
  timer = millis();
  Serial.println(timer);
  while(true){
    int packetSize = LoRa.parsePacket();
    if (packetSize){
      while (LoRa.available()) {
        String LoRaData = LoRa.readString();
        Serial.println(LoRaData);
      }
      break;
    }
    if(millis()-timer >= 1000){
      Serial.println(F("Timout"));
      break;
    }
  }
  
  delay(5000);
}

//Transfer
#include <SPI.h>
#include <LoRa.h>

//define the pins used by the transceiver module

#define sending_led A0
#define waiting_led A1

#define ss 10
#define rst 5
#define dio0 2

int counter = 0;

void setup() {
  //initialize Serial Monitor
  Serial.begin(115200);

  pinMode(sending_led, OUTPUT);
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
  
  Serial.print("Sending packet: ");
  Serial.println(counter);
  digitalWrite(sending_led, 1);
  digitalWrite(waiting_led, 0);
  delay(200);
  
  
  //Send LoRa packet to receiver
  LoRa.beginPacket();
  LoRa.print("hello ");
  LoRa.print(counter);
  LoRa.endPacket();

  counter++;

  digitalWrite(sending_led, 0);
  digitalWrite(waiting_led, 1);
  delay(5000);
}

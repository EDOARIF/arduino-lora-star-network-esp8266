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

String msg;
byte msg_len;

const byte sync_word = 0xF3;
const byte station_id = 3;
const byte master_id = 0;

long timer;
volatile bool request_status = false;

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
  LoRa.setSyncWord(sync_word);
  LoRa.onReceive(onReceive);
  LoRa.receive();
  Serial.println("LoRa Initializing OK!");
  timer = millis();
}

void loop() {
  if(request_status){
    String ssm = "I am Node #3.";
    digitalWrite(receiving_led, 1);
    digitalWrite(waiting_led, 0);
    sendMessage(master_id, station_id, ssm);
    LoRa.receive();   // go back into receive mode
    request_status = false;
    delay(1000);
  }
  digitalWrite(receiving_led, 0);
  digitalWrite(waiting_led, 1);
}

void sendMessage(byte _node_addr, byte _station_addr, String _msg) {
  LoRa.beginPacket();                   // start packet
  LoRa.write(_node_addr);               // add destination address
  LoRa.write(_station_addr);            // add sender address
  LoRa.write(_msg.length());            // add payload length
  LoRa.print(_msg);                     // add payload
  LoRa.endPacket();                     // finish packet and send it
}

void onReceive(int packetSize) {
  if (packetSize == 0) return;          // if there's no packet, return

  // read packet header bytes:
  int recipient = LoRa.read();          // recipient address
  byte sender = LoRa.read();            // sender address
  
  // same station address -> read msg or skip msg
  if(recipient==station_id && sender==master_id){
    request_status = true;
                
    byte incomingLength = LoRa.read();    // incoming msg length
    String incoming = "";                 // payload of packet
    while (LoRa.available()) {            // can't use readString() in callback, so
      incoming += (char)LoRa.read();      // add bytes one by one
    }

    if (incomingLength != incoming.length()) {   // check length for error
      Serial.println("error: message length does not match length");
      //return;                             // skip rest of function
    }

    // if message is for this device, or broadcast, print details:
    Serial.println("Received from: 0x" + String(sender, HEX));
    Serial.println("Sent to: 0x" + String(recipient, HEX));
    Serial.println("Message length: " + String(incomingLength));
    Serial.println("Message: " + incoming);
    Serial.println("RSSI: " + String(LoRa.packetRssi()));
    Serial.println("Snr: " + String(LoRa.packetSnr()));
    Serial.println();
  }else{
    Serial.println("Message is not for me!");
  }
}

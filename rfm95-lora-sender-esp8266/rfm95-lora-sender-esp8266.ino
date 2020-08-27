//Transfer
#include <SPI.h>
#include <LoRa.h>
#include <ArduinoJson.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

//define the pins used by the transceiver module
#define ss 16   //D0
#define rst 2   //D4
#define dio0 15 //D8

long timer, period;
byte node_id2req = 1;

String msg;
byte msg_len;

const byte sync_word = 0xF3;
const byte station_id = 0;
const byte node_num  = 3;

//Create OLED obj
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

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
  LoRa.setSyncWord(sync_word);
  LoRa.onReceive(onReceive);
  LoRa.receive();
  Serial.println("LoRa Initializing OK!");

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  delay(2000);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  
  display.setCursor(30, 30);
  display.println("Lora Gateway");
  display.display();
  delay(3000);
  oled_update(100, 0, 5000, -120);
  timer = millis();
}

void loop() {
  if(millis()-timer> 5000){
    Serial.println("Request node:");
    String ssm = "hi";
    sendMessage(node_id2req, station_id, ssm);
    timer = millis();
    LoRa.receive();                     // go back into receive mode
    node_id2req++;
    if(node_id2req > node_num) node_id2req = 1;
  }
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
  Serial.println("To Node: " + (String)recipient);
  // same station address -> read msg or skip msg
  if(recipient == station_id){
    byte sender = LoRa.read();            // sender address
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

void oled_update(int _id, float _data, long _period, int _rssi){
  display.clearDisplay();
  display.setCursor(30, 16);
  display.println("Nhuii Gateway");
  
  display.setCursor(0, 30);
  display.print("id: ");display.print(_id);
  display.setCursor(70, 30);
  display.print("d: ");display.println(_data);
  
  display.setCursor(0, 45);
  display.print("rssi: ");display.print(_rssi);
  display.setCursor(70, 45);
  display.print("t: ");display.println(_period);
  display.display();
}

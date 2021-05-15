// Licensed under the MIT license.
// Based on the following work:
// https://github.com/jonans/jump_bms
// https://github.com/alex3dbros/LGH1ScooterPackPower
// https://github.com/sandeepmistry/arduino-CAN


#include <CAN.h>

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);
  while (!Serial);
  delay(1000);

  Serial.println("CAN Sender");

  // start the CAN bus at 100 kbps
  if (!CAN.begin(100E3)) {
    yield();
    Serial.println("Starting CAN failed!");
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
    while (1);
  }
  Serial.println("CAN Ready");
  digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
  // send packet: id is 11 bits, packet can contain up to 8 bytes of data
  Serial.print("Sending packet ON ... ");
  switchON();
  Serial.println("done");
  readCANSerial(3000);

  Serial.print("Sending packet BATT ... ");
  requestBattCharge();
  Serial.println("done");
  readCANSerial(3000);
  
  Serial.print("Sending packet OFF ... ");
  switchOFF();
  Serial.println("done");
  readCANSerial(3000);

}

/***************************************************************************************************************/

void switchON() {
  CAN.beginPacket(0x630); //SDO-Receive to 0X30 (Battery ID)
  CAN.write(0x2F); //Download expedited transfer with payload of 1 byte
  CAN.write(0x00); //Index 0x2200 (little endian)
  CAN.write(0x22); //Index 0x2200 (little endian)
  CAN.write(0x01); //SubIndex 0x01
  CAN.write(0x01); //Set to 1
  CAN.write(0x00);
  CAN.write(0x00);
  CAN.write(0x00);
  CAN.endPacket();
}

void switchOFF() {
  CAN.beginPacket(0x630); //
  CAN.write(0x2F);
  CAN.write(0x00);
  CAN.write(0x22);
  CAN.write(0x01);
  CAN.write(0x00);
  CAN.write(0x00);
  CAN.write(0x00);
  CAN.write(0x00);
  CAN.endPacket();
}

void requestBattCharge() {
  CAN.beginPacket(0x630); //
  CAN.write(0x40);
  CAN.write(0x10);
  CAN.write(0x20);
  CAN.write(0x01);
  CAN.write(0x00);
  CAN.write(0x00);
  CAN.write(0x00);
  CAN.write(0x00);
  CAN.endPacket();
}

void readCANSerial(unsigned int millisec) { //millisec: time to spend waiting for CAN messages

    unsigned long previousMillis = millis();
    while(millis()- previousMillis < millisec) {
      int packetSize = CAN.parsePacket();
      if (packetSize) {
        // received a packet
        digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on
        Serial.print("Received ");
     
        Serial.print("packet with id 0x");
        Serial.print(CAN.packetId(), HEX);
    
        if (CAN.packetRtr()) {
          Serial.print(" and requested length ");
          Serial.println(CAN.packetDlc());
        } else {
          Serial.print(" and length ");
          Serial.println(packetSize);
    
          // only print packet data for non-RTR packets
          while (CAN.available()) {
            Serial.print(CAN.read(), HEX);
            Serial.print(" ");
          }
          Serial.println();
        }
    
        Serial.println();
        digitalWrite(LED_BUILTIN, LOW); 
      }
  }
}

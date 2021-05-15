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
    while (1);

  Serial.println("CAN Ready");
  //yield();
  }
}

void loop() {
  // send packet: id is 11 bits, packet can contain up to 8 bytes of data
  Serial.print("Sending packet ON ... ");

  CAN.beginPacket(0x630); //
  CAN.write(0x2F);
  CAN.write(0x00);
  CAN.write(0x22);
  CAN.write(0x01);
  CAN.write(0x01);
  CAN.write(0x00);
  CAN.write(0x00);
  CAN.write(0x00);
  CAN.endPacket();

  Serial.println("done");
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)

  readCAN(3000);

  Serial.print("Sending packet BATT ... ");

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

  Serial.println("done");
  
  readCAN(3000);
  
  Serial.print("Sending packet OFF ... ");

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

  Serial.println("done");
  //digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  //delay(1000);                       // wait
  digitalWrite(LED_BUILTIN, LOW); 
  
  readCAN(3000);

}

void readCAN(unsigned int millisec) {

    unsigned long previousMillis = millis();
    while(millis()- previousMillis < millisec) {
      int packetSize = CAN.parsePacket();
      if (packetSize) {
        // received a packet
        Serial.print("Received ");
    
        if (CAN.packetExtended()) {
          Serial.print("extended ");
        }
    
        if (CAN.packetRtr()) {
          // Remote transmission request, packet contains no data
          Serial.print("RTR ");
        }
    
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
      }
  }
}

// Licensed under the MIT license.
// Based on the following work:
// https://github.com/jonans/jump_bms
// https://github.com/alex3dbros/LGH1ScooterPackPower
// https://github.com/sandeepmistry/arduino-CAN


#include <CAN.h>

char* can_func[]={\
  "NMT  ",\
  "SY/EY",\
  "TIME ",\
  "PDO1T",\
  "PDO1R",\
  "PDO2T",\
  "PDO2R",\
  "PDO3T",\
  "PDO3R",\
  "PDO4T",\
  "PDO4R",\
  "SDO-T",\
  "SDO-R",\
  "UKNWN",\
  "HRTBT"\
  };

typedef struct
{
  uint32_t id;
  uint8_t  data[8];
  uint8_t  len;
} CAN_msg_t;

typedef struct
{
  uint16_t voltage=0; //in mV -- 0x2000 0x01
  uint8_t  charge=0; //in % -- 0x2010 0x01
  uint16_t maxcapacity; //in mAh -- 0x2010 0x02
  int16_t  current=0; // in mA -- 0x2000 0x03 
  bool updated=false;
} Battery_val;

Battery_val batt; //Battery values

/******************************************************************************************/


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

  //testReadCANSerialRaw(); // Switch ON, OFF and ask battery charge. Dump raw data to Serial
  //testReadCANSerialCANopen(); // Switch ON, OFF and ask battery charge. Parse CANopen to Serial
  testReadCANSerialBatt(); // Switch ON and fetch battery info, print to Serial
}

/***************************************************************************************************************/

void testReadCANSerialRaw(){ // Switch ON, OFF and ask battery charge. Dump raw data to Serial
  Serial.print("Sending packet ON ... ");
  switchON();
  Serial.println("done");
  readCANSerialBatt(3000);

  Serial.print("Sending packet BATT ... ");
  requestBattCharge();
  Serial.println("done");
  readCANSerialBatt(3000);
  
  Serial.print("Sending packet OFF ... ");
  switchOFF();
  Serial.println("done");
  readCANSerialBatt(3000);
}

void testReadCANSerialCANopen(){ // Switch ON, OFF and ask battery charge. Parse CANopen to Serial
  Serial.print("Sending packet ON ... ");
  switchON();
  Serial.println("done");
  readCANSerialCANopen(3000);

  Serial.print("Sending packet BATT ... ");
  requestBattCharge();
  Serial.println("done");
  readCANSerialCANopen(3000);
  
  Serial.print("Sending packet OFF ... ");
  switchOFF();
  Serial.println("done");
  readCANSerialCANopen(3000);
}

void testReadCANSerialBatt() { // Switch ON and fetch battery info, print to Serial
  switchON();
  readCANSerialBatt(3000);
  getIndex(0x200001); //voltage in mV
  getIndex(0x201001); //charge in % 
  getIndex(0x201002); // maxcapacity in mAh 
  getIndex(0x200003); // current in mA
  
  readCANSerialBatt(3000);
}

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

void getIndex(uint32_t index) {
  CAN.beginPacket(0x630); //
  CAN.write(0x40); //Upload
  CAN.write((index & 0xFF00)>>8);
  CAN.write((index & 0xFF0000)>>16);
  CAN.write(index & 0xFF);
  CAN.write(0x00);
  CAN.write(0x00);
  CAN.write(0x00);
  CAN.write(0x00);
  CAN.endPacket();
}


void requestBattCharge() {
  getIndex(0x201001);
/*  CAN.beginPacket(0x630); //
  CAN.write(0x40);
  CAN.write(0x10);
  CAN.write(0x20);
  CAN.write(0x01);
  CAN.write(0x00);
  CAN.write(0x00);
  CAN.write(0x00);
  CAN.write(0x00);
  CAN.endPacket();*/
}

void readCANSerialRaw(unsigned int millisec) { //millisec: time to spend waiting for CAN messages

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

void readCANSerialCANopen(unsigned int millisec) { //millisec: time to spend waiting for CAN messages

    unsigned long previousMillis = millis();
    while(millis()- previousMillis < millisec) {
      int packetSize = CAN.parsePacket();
      if (packetSize) {
        // received a packet
        digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on
        CAN_msg_t CAN_RX_msg;
        CAN_RX_msg.id = CAN.packetId();
        CAN_RX_msg.len = packetSize;
        Serial.print("COB-ID: 0x");
        if (CAN_RX_msg.id < 0x100) Serial.print("0");
        if (CAN_RX_msg.id < 0x10) Serial.print("00");
        Serial.print(CAN_RX_msg.id, HEX);
        Serial.print(" ");
      
        int f = CAN_RX_msg.id & 0x780;
        f = f >> 7;
        Serial.print(can_func[f]);
        Serial.print(" ");
        int n = CAN_RX_msg.id & 0x7F;
        Serial.print(n, HEX);
        Serial.print(" ");
      
        Serial.print(" DLC: ");
        Serial.print(CAN_RX_msg.len);
        Serial.print(" Data: ");

        int j = 0;
        while (CAN.available()) {
            CAN_RX_msg.data[j++] = CAN.read();
          }
        
       for(int i=0; i<CAN_RX_msg.len; i++) {
        Serial.print(CAN_RX_msg.data[i], HEX);
        Serial.print(" ");
       }
      Serial.println();
   }
   digitalWrite(LED_BUILTIN, LOW); 
   }
}

void readCANSerialBatt(unsigned int millisec) { //millisec: time to spend waiting for CAN messages
  readCANBatt(millisec, &batt);
  serial_Batt(&batt);
}


void serial_Batt(Battery_val* b)
{
    Serial.print("Charge: ");
    Serial.print(b->charge);
    Serial.println(" %");
    Serial.print("Voltage: ");
    Serial.print(b->voltage);
    Serial.println(" mV");
    Serial.print("Current: ");
    Serial.print((int16_t)b->current);
    Serial.println(" mA");
    Serial.print("Max Capacity: ");
    Serial.print((int16_t)b->maxcapacity);
    Serial.println(" mAh");
    Serial.println();
}

void readCANBatt(unsigned int millisec, Battery_val* b) { //millisec: time to spend waiting for CAN messages

  unsigned long previousMillis = millis();
  while(millis()- previousMillis < millisec) {
    int packetSize = CAN.parsePacket();
    if (packetSize) {
      // received a packet
      digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on
      CAN_msg_t CAN_RX_msg;
      CAN_RX_msg.id = CAN.packetId();
      CAN_RX_msg.len = packetSize;
      int j = 0;
      while (CAN.available()) {
        CAN_RX_msg.data[j++] = CAN.read();
        }

      if (CAN_RX_msg.id == 0x580 + 0x30){ //SDO-T from 0x30
        int index = CAN_RX_msg.data[2]*0x10000 + CAN_RX_msg.data[1]*0x100 + CAN_RX_msg.data[3]; // Build index+subindex
        switch (index) {
          case 0x201001: //Percent level of charge  State of charge in percent. Calculated as 2000x04 / 2010x02 
            b->charge = CAN_RX_msg.data[4];
            break;
  
          case 0x200001: //Pack voltage in mV 
            b->voltage = CAN_RX_msg.data[5]*0x100 + CAN_RX_msg.data[4];
            break;
  
          case 0x200003: //Current +in -out 
            b->current = CAN_RX_msg.data[5]*0x100 + CAN_RX_msg.data[4];
            break;

          case 0x201002: //Max Capacity in mAh 
            b->maxcapacity = CAN_RX_msg.data[5]*0x100 + CAN_RX_msg.data[4];
            break;
  
          case 0x220001: //Power output enabled. Cool!
            break;
          
          default:
            Serial.print("INDEX: ");
            Serial.print(index, HEX);
            Serial.print("  DATA: ");
            for(int i=0; i<CAN_RX_msg.len; i++) {
              Serial.print(CAN_RX_msg.data[i], HEX);
              Serial.print(" ");
            }
            Serial.println();
        }
       }
     digitalWrite(LED_BUILTIN, LOW); 
     }
  }
}

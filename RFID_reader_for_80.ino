#include "RFID.h"
#include <SoftwareSerial.h>
#define test A0
int i = 0;
const byte rxPin = 10;
const byte txPin = 11;
// Set up a new SoftwareSerial object
SoftwareSerial mySerial (rxPin, txPin);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  mySerial.begin(9600);
  RFID_init();
  pinMode(13,OUTPUT);
  pinMode(test,OUTPUT);
  digitalWrite(test,HIGH);
  
}

void loop() {
    digitalWrite(test,!digitalRead(test));
    delay(200);
    digitalWrite(13,LOW);
    delay(200);
    //mySerial.println("kgjdfgçajk");
    uint64_t rfid_on=0;
    request_RFID();
    
    switch(read_RFID(&rfid_on)){
      case STATUS_ERR_TIMEOUT:
        mySerial.println("TIME_OUT\n");
        break;
      case STATUS_ERR_BAD_DATA:
       //Serial.print("ERRO CRC\n");
       //mySerial.println((uint32_t)(rfid_on-982000000000000));
       // Serial.write(0x8E);
        break;
      case STATUS_OK:
        mySerial.println((uint32_t)(rfid_on-982000000000000));
        digitalWrite(13,HIGH);
        for (int i = 0; i<=100; i++){
          Serial.write(0x7e);
          Serial.write((uint8_t*)&rfid_on,8);
        }
        break;
    }
         
}

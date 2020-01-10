#include <C:\Users\User\Documents\Arduino\libraries\espsoftwareserial-master\src\SoftwareSerial.h>

//#include <SoftwareSerial.h> //https://github.com/plerup/espsoftwareserial
#include <ModbusRtu.h>
#include <IRac.h>
#include <IRsend.h>
#include <IRtimer.h>
#include <IRremoteESP8266.h>

#include "ir_Mitsubishi.h"        //case 12

#define MAX485_RE_NEG 4 //D2 RS485 has a enable/disable pin to transmit or receive data. Arduino Digital Pin 2 = Rx/Tx 'Enable'; High to Transmit, Low to Receive
#define TTX_PIN 5 // GPIO5 for IR transmitter (5 for wroom 2 for 8266)

uint16_t au16data[6] = {11, 22, 33, 44, 55, 66};
uint16_t au16datatemp[6] = {0, 0, 0, 0, 0, 0};

uint8_t acAuto;
uint8_t acCool;
uint8_t acDry;
uint8_t acHeat;
uint8_t acFan;

Modbus slave(1, 0, MAX485_RE_NEG);
IRMitsubishiAC ac(TTX_PIN);

void setup() {
  //pinMode(LED_BUILTIN, OUTPUT);
  pinMode(MAX485_RE_NEG, OUTPUT);
  digitalWrite(MAX485_RE_NEG, HIGH); //Switch to transmit data
  digitalWrite(MAX485_RE_NEG, LOW); //Switch to receive data
  Serial.begin(9600);
  slave.begin();

  acAuto = kMitsubishiAcAuto;
  acDry = kMitsubishiAcDry;
  acHeat = kMitsubishiAcHeat;
  acCool = kMitsubishiAcCool;

  ac.begin();
}

void loop() {
 
  slave.poll( au16data, 6 );
  
  if (onUpdate())
  {
  //digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on (Note that LOW is the voltage level

        if (au16data[0] == 0) { // 0 - on/off
          ac.off();
        } else {
          ac.on();
        }

       ac.setTemp((int) au16data[1]); // 1 - temperature

        switch (au16data[2]) { // 2 - mode
        case 1:
          ac.setMode(acAuto);
          break;
        case 2:
          ac.setMode(acCool);
          break;
        case 3:
          ac.setMode(acDry);
          break;
        case 4:
          ac.setMode(acHeat);
          break;
        default:
          ac.setMode(acAuto);
          break;
        }

        ac.setFan((int) au16data[3]); // 3 - fan speed 0 is auto, 1-5 is the speed, 6 is silen

        ac.setVane((int) au16data[4]); //4 - vane 7 - auto, 1-6 -mod

        ac.send();

    //digitalWrite(LED_BUILTIN, HIGH);  
    }
}

bool onUpdate() {
  if (changes(au16data, 6)) {
    for (int j = 0; j < 19; j++) {
      au16datatemp[j] = au16data[j];
    }
    return true;
  } else
    return false;
}

bool changes(uint16_t * input, int inputSize) {
  for (int i = 0; i < inputSize; i++) {
    if (au16datatemp[i] != input[i])
      return true;
  }
  return false;
}

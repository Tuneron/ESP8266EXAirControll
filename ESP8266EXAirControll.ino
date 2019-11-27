#include <SoftwareSerial.h> //https://github.com/plerup/espsoftwareserial
#include <ModbusRtu.h>
#include <IRac.h>
#include <IRsend.h>
#include <IRtimer.h>
#include <IRremoteESP8266.h>
#define TX_PIN      2  //D4 for RS485
#define MAX485_RE_NEG  4 //D2 RS485 has a enable/disable pin to transmit or receive data. Arduino Digital Pin 2 = Rx/Tx 'Enable'; High to Transmit, Low to Receive
#define RX_PIN      0 //D3 for RS485
#define TRX_PIN  5 // D1 for IR transmitter

const uint16_t kIrLed = 5; 
uint16_t au16data[6] = {11, 22, 33, 44, 55, 66};
uint16_t au16datatemp[6] = {0, 0, 0, 0, 0, 0};
Modbus slave(1,0,0);

void setup() {

  Serial.begin(9600, SERIAL_8N1);
  slave.begin(9600);
}

void loop() {

  slave.poll( au16data, 6 );
}

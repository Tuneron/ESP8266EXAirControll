#include <ESP8266WiFi.h>    // Include the Wi-Fi library
#include <SoftwareSerial.h> //https://github.com/plerup/espsoftwareserial
#include <IRac.h>
#include <IRsend.h>
#include <IRtimer.h>
#include <IRremoteESP8266.h>
#include "ir_Mitsubishi.h"        //case 12

#define MAX485_RE_NEG 0  // GPIO0 for RS485 has a enable/disable pin to transmit or receive data. Arduino Digital Pin 2 = Rx/Tx 'Enable'; High to Transmit, Low to Receive
#define TTX_PIN  2 // GPIO2 for IR transmitter

const char* ssid     = "SmartWifi";         // The SSID (name) of the Wi-Fi network you want to connect to
const char* password = "ReeveOffice1!";     // The password of the Wi-Fi network
const char* host = "192.168.1.23";          // Server IP
const uint16_t port = 21;                   // Server Port
const int REGISTERS = 5;

uint16_t au16data[REGISTERS] = {11, 22, 33, 44, 55};
uint16_t au16datatemp[19] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
uint16_t inputPack[19] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

uint8_t pack[(REGISTERS * 2) + 5];
uint8_t answ16f[8] = {0x01, 0x10, 0x00, 0x00, 0x00, 0x05, 0x00, 0x0A};
uint8_t acAuto; 
uint8_t acCool;
uint8_t acDry;
uint8_t acHeat;
uint8_t acFan;

int count = 0;
long lastLogTime = millis();
String logBuffer = "";

WiFiClient client;
IRMitsubishiAC ac (TTX_PIN);

void setup() {
  
  pinMode(MAX485_RE_NEG, OUTPUT);
  digitalWrite(MAX485_RE_NEG, HIGH); //Switch to transmit data
  digitalWrite(MAX485_RE_NEG, LOW); //Switch to receive data 
  pinMode(TTX_PIN, OUTPUT);

  WiFi.begin(ssid, password);             // Connect to the network
  while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
    delay(1000);
  }  
  client.connect(host, port);
    log("ESP8266 answer");

  acAuto = kMitsubishiAcAuto;
  acDry = kMitsubishiAcDry;
  acHeat = kMitsubishiAcHeat;
  acCool = kMitsubishiAcCool;
   
  ac.begin();    

  Serial.begin(9600, SERIAL_8N1);
}

void loop() {
  
  digitalWrite(MAX485_RE_NEG, LOW); //Switch to receive data 
  if(Serial.available() > 0) {
    inputPack[count] = Serial.read();
    count++;
  }

   if (count == 19){
    
    if(onUpdate()){
      ac.on();
      ac.setTemp(inputPack[8]);
      digitalWrite(TTX_PIN, HIGH); // ON
      ac.send(); 
      digitalWrite(TTX_PIN, LOW); // OFF
      //log("Command was send");
    }
    
    //log(printRegisters(inputPack, 19));
    digitalWrite(MAX485_RE_NEG, HIGH); //Switch to transmit data
    Serial.write(answ16f, 8);
    count = 0;
    delay (1500);
   }
   
  

}

String printRegisters(uint16_t* registers, uint16_t regSize) {
  String line = "[ ";
  for (int i = 0; i < regSize; i++){
    line += registers[i];
    line += ", ";
  }
  line += "]";
  return line;
}

String printRegisters8t(uint8_t* registers, uint8_t regSize) {
  String line = "[ ";
  for (int i = 0; i < regSize; i++){
    line += registers[i];
    line += ", ";
  }
  line += "]";
  return line;
}

bool onUpdate() {
  if (changes(inputPack, 19)) {
    log(printRegisters(inputPack, 19));
    for (int j = 0; j < 19; j++) {
      au16datatemp[j] = inputPack[j];
    }
    return true;
  }
  else
  return false;
}

bool changes(uint16_t *input, int inputSize){
  for (int i = 0; i < inputSize; i++){ 
    if (au16datatemp[i] != input[i])
    return true;
  }
  return false;
}

void logWithBuffer(String line) {
  if ((millis() - lastLogTime > 5000 && logBuffer != "") || logBuffer.length() > 200) {
    if (!client.connected()){
      if (client.connect(host, port)) {
        client.println("\n"+logBuffer);
        logBuffer = ""; 
        lastLogTime = millis();          
      }
    } else {
      client.println("\n"+logBuffer);
      logBuffer = "";    
      lastLogTime = millis();                 
    }
  } else {
    logBuffer += line;
    logBuffer += " | ";
  }
}

void log(String line) {
  if (!client.connected()){
    if (client.connect(host, port)) {
      client.println(line);          
    }
  } else {
    client.println(line);               
  }
}

//контрольная сумма по подсчёту
uint16_t MRTUCRC(uint8_t* data, uint8_t len)
{
        short res_CRC = 0xFFFF;
        short count = 0;
        uint8_t count_crc;
        uint8_t dt;
        while(count < len)
        {
                //
                count_crc = 0;
                dt = (uint8_t)(data[count]);
                res_CRC ^= (ushort)(dt);
                //
                while(count_crc < 8)
                {
                        if((res_CRC & 0x0001) < 1)
                        {
                                res_CRC = (res_CRC >> 1) & 0x7FFF;
                        }
                        else
                        {
                                res_CRC = (res_CRC >> 1) & 0x7FFF;
                                res_CRC ^= 0xA001;
                        };
                        count_crc++;
                };
                count++;
        }

        // swap for Modbus
        res_CRC = ((res_CRC & 0x00FF) << 8) | ((res_CRC & 0xFF00) >> 8);
        
        return (res_CRC);
}

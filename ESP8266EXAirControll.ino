#include <ESP8266WiFi.h>    // Include the Wi-Fi library
#include <SoftwareSerial.h> //https://github.com/plerup/espsoftwareserial
#include <ModbusRtu.h>
#include <IRac.h>
#include <IRsend.h>
#include <IRtimer.h>
#include <IRremoteESP8266.h>

//#define TX_PIN      3     //D4 for RS485
#define MAX485_RE_NEG 2  //D2 RS485 has a enable/disable pin to transmit or receive data. Arduino Digital Pin 2 = Rx/Tx 'Enable'; High to Transmit, Low to Receive
//#define RX_PIN     1      //D3 for RS485
//#define TTX_PIN    0      //D1 for IR transmitter

const char* ssid     = "SmartWifi";         // The SSID (name) of the Wi-Fi network you want to connect to
const char* password = "ReeveOffice1!";     // The password of the Wi-Fi network
const char* host = "192.168.1.23";          // Server IP
const uint16_t port = 21;                   // Server Port
const int REGISTERS = 5;
uint16_t au16data[REGISTERS] = {11, 22, 33, 44, 55};
uint16_t au16datatemp[REGISTERS] = {0, 0, 0, 0, 0};

WiFiClient client;
Modbus slave(1,0, MAX485_RE_NEG);
long lastLogTime = millis();
String logBuffer = "";

int inc = 0;

void setup() {
  pinMode(MAX485_RE_NEG, OUTPUT);
  digitalWrite(MAX485_RE_NEG, HIGH); //Switch to transmit data
  digitalWrite(MAX485_RE_NEG, LOW); //Switch to receive data 

  WiFi.begin(ssid, password);             // Connect to the network
  while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
    delay(1000);
  }  
  client.connect(host, port);
    log("123");

  Serial.begin(9600, SERIAL_8N1);
  slave.begin(9600);
}

void loop() {
//  slave.poll(au16data, REGISTERS); 
//  onUpdate();
  if(Serial.available() > 0) {
    log("123");
    digitalWrite(MAX485_RE_NEG, HIGH); //Switch to transmit data
    Serial.write(Serial.read());
    digitalWrite(MAX485_RE_NEG, LOW); //Switch to receive data 
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

void onUpdate() {
  if (changes(au16data, REGISTERS)) {
    log(printRegisters(au16data, REGISTERS));
    for (int j = 0; j < REGISTERS; j++) {
      au16datatemp[j] = au16data[j];
    }
  }
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

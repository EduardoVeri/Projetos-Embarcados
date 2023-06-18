#include <Arduino.h>
#include <Wire.h>

const int slaveAdress = 8;

int flagDisplay = 0;

void setup() {
  Serial.begin(9600);
  Wire.begin(slaveAdress);
  Wire.onReceive(receiveEvent);
}

void loop() {
  if(flagDisplay == 0){
    if(Serial.available()){
      String received = Serial.readString();
      received.trim();

      if(received == "LigarLed"){
        Wire.beginTransmission(slaveAdress);
        Wire.write(1);
        Wire.endTransmission();
        Serial.println("Led ligado");
      }
      else if(received == "DesligarLed"){
        Wire.beginTransmission(slaveAdress);
        Wire.write(0);
        Wire.endTransmission();
        Serial.println("Led desligado");
      }
      else if(received == "LigarBuzzer"){
        Wire.beginTransmission(slaveAdress);
        Wire.write(2);
        Wire.endTransmission();
        Serial.println("Buzzer ligado");
      }
      else if(received == "DesligarBuzzer"){
        Wire.beginTransmission(slaveAdress);
        Wire.write(3);
        Wire.endTransmission();
        Serial.println("Buzzer desligado");
      }
      else if(received == "DisplayLCD"){
        Wire.beginTransmission(slaveAdress);
        Wire.write(4);
        Wire.endTransmission();
        Serial.println("Display LCD");
        flagDisplay = 1;
      }
      else{
        Serial.println("Comando invalido");
      }
    }
  }
  else{
    if(Serial.available()){
      String received = Serial.readString();
      received.trim();
      
      char buffer[16];
      received.toCharArray(buffer, 16);

      Wire.beginTransmission(slaveAdress);
      Wire.write(buffer);
      Wire.endTransmission();
      Serial.println("Mensagem Enviada!");
      
      flagDisplay = 0;
    }
  }
}

void receiveEvent(int howMany){
  if (Wire.available()){
    char received = Wire.read();
    if (received == 'A'){
      Serial.println("Botao A: HIGH");
    }
    else if(received == 'a'){
      Serial.println("Botao A: LOW");
    }
    else if(received == 'B'){
      Serial.println("Botao B: HIGH");
    }
    else if(received == 'b'){
      Serial.println("Botao B: LOW");
    }
  }
}
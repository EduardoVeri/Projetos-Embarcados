#include <Arduino.h>
#include <Wire.h>

const int buttonPin = 4;
const int slaveAdress = 8;
boolean buttonState;
boolean lastButtonState = LOW;
int ledState = HIGH;

unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 100;

const int ledPin = 7;

void setup() {
  Wire.begin(slaveAdress);
  Wire.onReceive(receiveEvent);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
}

void loop() {
  int reading = digitalRead(buttonPin);

  if((millis() - lastDebounceTime) > debounceDelay){

    if(reading != buttonState){
      buttonState = reading;

      if(buttonState == HIGH){
        ledState = !ledState;
        Wire.beginTransmission(slaveAdress);
        Wire.write(ledState);
        Wire.endTransmission();
      }
    }
  }

  lastButtonState = reading;
}

void receiveEvent(int howMany){
  if (Wire.available()){
    char received = Wire.read();
    if (received == 0){
      digitalWrite(ledPin, LOW);
    }
    else if(received == 1){
      digitalWrite(ledPin, HIGH);
    }
  }
}

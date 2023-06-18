#include <Wire.h>

const int ledPin = 7;
const int MyAdress = 8;

const int buttonPin = 4;
boolean buttonState;
boolean lastButtonState = LOW;
int ledState = HIGH;

unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 100;


void setup() {
  Wire.begin(MyAdress);
  Wire.onReceive(receiveEvent);
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
}

void loop() {
  int reading = digitalRead(buttonPin);
   
  if((millis() - lastDebounceTime) > debounceDelay){

    if(reading != buttonState){
      buttonState = reading;
  
      if(buttonState == HIGH){
        ledState = !ledState;
        Wire.beginTransmission(MyAdress);
        Wire.write(ledState);
        Wire.endTransmission();
      }
    }
  }
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

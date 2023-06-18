#include <Arduino.h>
#include <Wire.h>
//#include <LiquidCrystal_I2C.h> // Nao esquecer de baixar esse modulo

unsigned long debounceDelay = 50;
unsigned long lastDebounceTime = 0;

LiquidCrystal_I2C lcd(0x27,16,2);

int last_One = LOW;
int One_buttonState;
int One_pin = 7;
bool flagOne = LOW;


int last_Two = LOW;
int Two_buttonState;
int Two_pin = 6;
bool flagTwo = LOW;


int ledPin = 10;
int masterAdress = 8;


bool LCDflag = false;


void sendButton(int button, int* flag){
  char c;
  switch(button){
        case 0:
          c = (*flag) ? ('A') : ('a');
          break;
        case 1:
          c = (*flag) ? ('B') : ('b');
          break;
     
      Wire.beginTransmission(masterAdress);
      Wire.write(c);
      Wire.endTransmission();
  }
}


void debouncer(int* flag, int* buttonState, int* last, int leitura, int button){
  if(leitura != *last){
    lastDebounceTime = millis();
  }
   
  if ((millis() - (lastDebounceTime)) > debounceDelay) {
    if (leitura != (*buttonState)) {
      *buttonState = leitura;
      *flag = !(*flag);
      sendButton(button, flag);
    }
  }


  *last = leitura;
}


void switchLed(int state){
  //Troca estado do led
}


void switchBuzzer(int state){
  //Troca estado do Buzzer
}


void printDisplay(String message){
  char buffer[32];
  message.toCharArray(buffer, 16);

  lcd.setCursor(0,0);

  int i = 0, x = 0;
  while(buffer[i] != '\n' && i < 32){
    lcd.print(buffer[i]);
    if(i == 15){
      lcd.setCursor(0,1);
    }
    i++;
  }
}


void receiveCommand(int howMany){
  if(Wire.available()){ 
    if(!LCDflag){
      int reading = Wire.read();
      switch(reading){
        case 0:
        case 1:
          switchLed(reading);
          break;
      
        case 2:
        case 3:
          switchBuzzer(reading - 2);
          break;
      
        case 4:
          printDisplay(String("Digite a sua Mensagem:"));
          LCDflag = true;
          break;
      }
    }
    else{
      printDisplay(String("Mensagem Recebida"));
      LCDflag = false;
    }
  }
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(One_pin, INPUT);
  pinMode(Two_pin, INPUT);
  pinMode(ledPin, OUTPUT);

  lcd.init();
  lcd.setBacklight(HIGH);

  Wire.begin(masterAdress);
  Wire.onReceive(receiveCommand);
}

int flag_1 = LOW;
int flag_2 = LOW;

void loop() {
  debouncer(&flag_1, &One_buttonState, &last_One, digitalRead(One_pin), 0);
  debouncer(&flag_2, &Two_buttonState, &last_Two, digitalRead(Two_pin), 1);
}

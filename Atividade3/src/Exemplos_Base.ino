// Constantes para os valores dos botões e LEDS
const int button1 = 2; // número do pino do botão (pino 2)
const int ledVerde = 11; // número do pino do LED (pino 13)
const int ledAmarelo = 7;
const int ledVermelho = 9;

int last1 = LOW; // Guarda o último estado do botão;
int estado1 = LOW; // Guarda o estado atual do botão

unsigned long lastDebounceTime1 = 0;
unsigned long debounceDelay = 50;  // Valor para o intervalo do debounce

unsigned long print_timer = 0; // Timer para a impressão na porta

// Valores dos estados dos LEDS
int estVermelho;
int estAmarelo;
int estVerde;


void setup() {
  Serial.begin(9600);
  pinMode(button1, INPUT); 
  pinMode(ledAmarelo, OUTPUT); 
  pinMode(ledVermelho, OUTPUT); 
  pinMode(ledVerde, OUTPUT); 
  estVermelho = LOW;
  estAmarelo = LOW;
  estVerde = LOW;
  last1 = digitalRead(button1);
}

int flag = 0;

void debouncer(unsigned long* lastDebounceTime, int* buttonState, int* last, int leitura){
  if(leitura != *last){
    *lastDebounceTime = millis();
  }
    
  if ((millis() - (*lastDebounceTime)) > debounceDelay) {
    if (leitura != (*buttonState)) {
      *buttonState = leitura;
      flag = 1;
    }
  }

  *last = leitura;
}

int piscada = 0;

void acenderLed(){
  if(estado1 == HIGH){
    estVermelho = HIGH;
    estAmarelo = HIGH;
    estVerde = HIGH;
  }
  else{
    estVermelho = LOW;
    estAmarelo = LOW;
    estVerde = LOW;
  }
  /*
  estVermelho = HIGH;
  estAmarelo = HIGH;
  estVerde = HIGH;
  */
  
  digitalWrite(ledVermelho, estVermelho);
  digitalWrite(ledAmarelo, estAmarelo);
  digitalWrite(ledVerde, estVerde);
} 

void mostrarSerial(){
  if (millis() - print_timer > 1000) {   
    Serial.println();    
    print_timer = millis();
  }
}

void loop() {
  int read1 = digitalRead(button1);
  
  debouncer(&lastDebounceTime1, &estado1, &last1, read1);
  acenderLed();
  //mostrarSerial();
}

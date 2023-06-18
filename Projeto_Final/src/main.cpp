/* O projeto a seguir eh um sistema de auxilio residencial para pessoas com
deficiencia auditiva

Ele precisara ter as seguintes funcionalidades:
1) Campanhia -  Caso alguem pressione um botao, o arduino mandara um sinal para o PIC
que ira interpretar e reenviar um outro sinal para o arduino, fazendo com que os leds
pisquem e avisem ao morador que alguem tocou a campanhia

2) Alerta de telefonema - Caso um telefonema toque (representado por uma mensagem via monitor serial) no arduino,
ele mandara para o PIC e o mesmo fara com que o arduino pisque luzes para alertar 

3) Monitor de temperatura - Caso esteja muito quente ou frio, a ventoinha devera ser ligada no PIC

4) Despertador - Caso o despertador toque, as luzes devem piscar de fomra intensa ate o morador desligar via
bluetooth (HC05) ou via um botao*/

#include <Arduino.h>
#include <SoftwareSerial.h>

SoftwareSerial bt(2,3);

const int led_verm1 = 13;
const int led_verd1 = 12;
const int led_amr1 = 11;

const int botao_camp = 10;
const int botao_desp = 9;

unsigned long lastDebounceTime1 = 0;
unsigned long lastDebounceTime2 = 0;
unsigned long debounceDelay = 50;  // Valor para o intervalo do debounce

int last_camp = LOW; // Guarda o último estado do botão
int estado_camp = LOW; // Guarda o estado atual do botão

int last_desp = LOW; // Guarda o último estado do botão
int estado_desp = LOW; // Guarda o estado atual do botão

int flag_debouncer = 0;
int flag_despertador = 0;

int camp_PIC = LOW; // Sinaliza se o PIC retornou algo sobre a campainha
int alarme_PIC = LOW; // Sinaliza se o PIC retornou algo sobre o alarme
int ligac_PIC = LOW; // Sinaliza se o PIC retornou algo sobre a ligacao

unsigned long despertador = 0;

unsigned long hora_atual = 0;
unsigned long ultimo_minuto = millis();

void enviar_PIC(char);
void debouncer(unsigned long*, int*, int*, int);
void receber_sinal_PIC();
void realizar_tarefas();
void que_horas_sao();
void tempo_real();
void leitor_bluetooth();

void setup() {
  bt.begin(9600);
  Serial.begin(9600);
  pinMode(led_verm1, OUTPUT);
  pinMode(led_verd1, OUTPUT);
  pinMode(led_amr1, OUTPUT);
  pinMode(botao_camp, INPUT);
  pinMode(botao_desp, INPUT);
  
  digitalWrite(led_verm1, LOW);
  digitalWrite(led_verd1, LOW);
  digitalWrite(led_amr1, LOW); 
}

void loop() {
  debouncer(&lastDebounceTime1, &estado_camp, &last_camp, botao_camp);
  debouncer(&lastDebounceTime2, &estado_desp, &last_desp, botao_desp);

  leitor_bluetooth();

  if(estado_camp == HIGH && flag_debouncer == 1){
    enviar_PIC('C');
    flag_debouncer = 0;
    que_horas_sao();
  }
  if(estado_desp == HIGH && flag_debouncer == 1){
    enviar_PIC('d');
    flag_debouncer = 0;
  }
  if(despertador == hora_atual && alarme_PIC == LOW && flag_despertador == 0){
    enviar_PIC('D');
    flag_despertador = 1;
  }
  if(Serial.available()){
    String mensagem = Serial.readStringUntil('\n');
    mensagem.trim();
    mensagem.toUpperCase();

    if(mensagem == "LIGACAO"){
      enviar_PIC('L');
    }
  }
  receber_sinal_PIC(); // Verifica se o PIC retornou algo

  realizar_tarefas(); // Realiza as tarefas de acordo com o que o PIC retornou
  
  tempo_real(); // Atualiza o tempo real

}

void leitor_bluetooth(){
  if(bt.available())	
  { 
    char c = bt.read();	
    if(c == '?'){
      que_horas_sao();
      return;
    }
    
    if(c == 'S'){
      enviar_PIC('d');
      return;      
    }
    if(c == 'H'){
      char buffer[10]; 
      int i = 0;
      int hora = 0;
      int minuto = 0;
      
      delay(1);      
      while(bt.available()){
        delay(1);    
        c = bt.read();	
        buffer[i] = c;

        if(c == 'h'){
          buffer[i] = '\0';
          hora = atoi(buffer);
          i = -1;
        }
        if(c == 'm'){
          buffer[i] = '\0';
          minuto = atoi(buffer);
          hora_atual = hora*60 + minuto;
          bt.println("Hora configurada!");
          
          return;
        }
        i++;
      } 
      bt.println("Erro: Hora atual nao configurada!");     
    }
    else if(c == 'A'){
     
      char buffer[10]; 
      int i = 0;
      int hora = 0;
      int minuto = 0;
      flag_despertador = 0;  
      
      delay(1); 
      while(bt.available()){   
        delay(1); 
        c = bt.read();	
        buffer[i] = c;

        if(c == 'h'){
          buffer[i] = '\0';
          hora = atoi(buffer);
          i = -1;
        }
        if(c == 'm'){
          buffer[i] = '\0';
          minuto = atoi(buffer);
          despertador = hora*60 + minuto;
          bt.println("Despertador configurado!");
          return;
        }
        i++;
      }
      bt.println("Erro: Despertador nao configurado!"); 
    }   
  }
}

void que_horas_sao(){
  bt.print("Hora Atual: ");
  bt.print(hora_atual/60);
  bt.print("h ");
  bt.print(hora_atual%60);
  bt.println("m");

  bt.print("Despertador: ");
  bt.print(despertador/60);
  bt.print("h ");
  bt.print(despertador%60);
  bt.println("m");
}

void enviar_PIC(char msg){
  switch(msg){
    case 'C':
      camp_PIC = HIGH; // Trocar por um sinal de envio para o PIC
      Serial.println("Enviado: 'A'");
      break;
    case 'D':
      alarme_PIC = HIGH; // Trocar por um sinal de envio para o PIC
      Serial.println("Enviado: 'D'");
      break;
    case 'd':
      alarme_PIC = LOW; // Trocar por um sinal de envio para o PIC
      Serial.println("Enviado: 'd'");
      break;
    case 'L':
      ligac_PIC = HIGH; // Trocar por um sinal de envio para o PIC
      Serial.println("Enviado: 'L'");
      break;
  }
}

void receber_sinal_PIC(){

}

int estados_campainha = 0;
unsigned long ultimo_tempo = 0;
int tempo_espera_campanhia = 500; // Tempo de espera para a campainha
int em_uso = 0; // Sinaliza se o sistema esta em uso ou nao

int estados_despertador = 0;
int tempo_espera_despertador = 1000; // Tempo de espera para o despertador
int tempo_espera_ligacao = 100; // Tempo de espera para a ligacao

void realizar_tarefas(){
  static int contador = 0;
  
  if((em_uso == 0 || em_uso == 1) && camp_PIC == HIGH){
    em_uso = 1;
    if((millis() - ultimo_tempo) >= tempo_espera_campanhia){
      estados_campainha++;
      int leds_campainha = 0;
      switch (estados_campainha){
        case 1:
          leds_campainha = 0b100;
          break;
        case 2:
          leds_campainha = 0b010;
          break;
        case 3:
          leds_campainha = 0b001;
          break;
        case 4:
        case 6:
        case 8:
          leds_campainha = 0b111;
          tempo_espera_campanhia = 1000;
          break;
        case 5:
        case 7:
          leds_campainha = 0b000;
          tempo_espera_campanhia = 500;  
          break;    
        default:
          leds_campainha = 0;
          estados_campainha = 0;
          tempo_espera_campanhia = 500;
          camp_PIC = LOW;
          em_uso = 0;
      }

      digitalWrite(led_amr1, leds_campainha & 0b100);
      digitalWrite(led_verm1, leds_campainha & 0b010);
      digitalWrite(led_verd1, leds_campainha & 0b001);
      ultimo_tempo = millis();
    }
  }
  
  if((em_uso == 0 || em_uso == 2) && (alarme_PIC == HIGH)){
    em_uso = 2;
    if((millis() - ultimo_tempo) >= tempo_espera_despertador){
      int leds_despertador = 0;
      switch(contador%2){
        case 1:
          leds_despertador = 0b000;
          tempo_espera_despertador = 200;
          break;
        case 0:
          leds_despertador = 0b111;
          tempo_espera_despertador = 1500;
          break;
      }
      digitalWrite(led_amr1, leds_despertador & 0b100);
      digitalWrite(led_verm1, leds_despertador & 0b010);
      digitalWrite(led_verd1, leds_despertador & 0b001);
      ultimo_tempo = millis();
      contador++;
    }
  }
  else if(em_uso == 2){
    digitalWrite(led_amr1, LOW);
    digitalWrite(led_verm1, LOW);
    digitalWrite(led_verd1, LOW);
    em_uso = 0;
    contador = 0;
  }

  if((em_uso == 0 || em_uso == 3) && (ligac_PIC == HIGH)){
    em_uso = 3;
    if((millis() - ultimo_tempo) >= tempo_espera_ligacao){
      int leds_ligacao = 0;
      if(contador%21 == 0){
        leds_ligacao = 0b111;
        tempo_espera_ligacao = 1000;
      }
      else{
        switch(contador%3){
          case 0:
            tempo_espera_ligacao = 100;
            leds_ligacao = 0b001;
            break;
          case 1:
            leds_ligacao = 0b010;
            break;
          case 2:
            leds_ligacao = 0b100;
            break;
        }
      }
      digitalWrite(led_amr1, leds_ligacao & 0b100);
      digitalWrite(led_verm1, leds_ligacao & 0b010);
      digitalWrite(led_verd1, leds_ligacao & 0b001);
      ultimo_tempo = millis();
      contador++;
    }
  }
}

/* Essa funcao deve receber uma valor em XXhMMmin e transformar apenas para minutos.
Utilizar a funcao millis para incrementar esse valor sempre que passar 1 min */
void tempo_real(){
  if((millis() - ultimo_minuto) >= 60000){
    hora_atual++;
    ultimo_minuto = millis();
  }
  if(hora_atual >= 1440){
    flag_despertador = 0;
    hora_atual = hora_atual%1440;
  }
}

void debouncer(unsigned long* lastDebounceTime, int* buttonState, int* last, int button){
  int leitura = digitalRead(button);

  if(leitura != *last){
    *lastDebounceTime = millis();
  }
    
  if ((millis() - (*lastDebounceTime)) > debounceDelay) {
    if (leitura != (*buttonState)) {
      *buttonState = leitura;
      flag_debouncer = 1;
    }
  }

  *last = leitura;
}

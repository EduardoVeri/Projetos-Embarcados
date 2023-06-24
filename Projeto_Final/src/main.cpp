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
#include <ctype.h>

SoftwareSerial bt(2,3);
 //2 1 5 4 3
const int led_verm1 = 13;
const int led_verm2 = 8;
const int led_verm3 = 7;
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
int emerg_PIC = LOW; // Sinaliza se o PIC retornou algo sobre a emergencia

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
void leitor_serial();

void setup() {
  bt.begin(9600);
  Serial.begin(9600);
  pinMode(led_verm1, OUTPUT);
  pinMode(led_verm2, OUTPUT);
  pinMode(led_verm3, OUTPUT);
  pinMode(led_verd1, OUTPUT);
  pinMode(led_amr1, OUTPUT);
  pinMode(botao_camp, INPUT);
  pinMode(botao_desp, INPUT);
  
  digitalWrite(led_verm1, LOW);
  digitalWrite(led_verm2, LOW);
  digitalWrite(led_verm3, LOW);
  digitalWrite(led_verd1, LOW);
  digitalWrite(led_amr1, LOW); 
}

void loop() {
  static int cont = 0;

  debouncer(&lastDebounceTime1, &estado_camp, &last_camp, botao_camp);
  debouncer(&lastDebounceTime2, &estado_desp, &last_desp, botao_desp);

  /* Realizar a leitura do módulo HC-05 para receber os caracteres
  vindos do celular conectado */
  leitor_bluetooth(); 

  /* Reliza a leitura do monitor serial, para verificar se alguma 
  informacao relevante foi passada pelo canal de transmissao*/
  leitor_serial();

  // Botao para a campainha 
  if(estado_camp == HIGH && flag_debouncer == 1){
    enviar_PIC('C');
    flag_debouncer = 0;
    que_horas_sao();
  }
  // Botao para o despertador e emergencia
  if(estado_desp == HIGH && flag_debouncer == 1){
    flag_debouncer = 0;

    // Caso uma emergencia esteja ativa, 2 toques no botao faz com que ele pare
    if(emerg_PIC == HIGH){
      cont++;
      if(cont >= 2){
        enviar_PIC('e');
        cont = 0;
      }
    }
    else{
      // Desativa o alarme caso ele esteja ativo
      if(alarme_PIC == HIGH){
        enviar_PIC('d');
      }
    }
  }
  // Verifica as condicoes para o toque do despertador
  if(despertador == hora_atual && alarme_PIC == LOW && flag_despertador == 0){
    enviar_PIC('D');
    flag_despertador = 1;
  }


  receber_sinal_PIC(); // Verifica se o PIC retornou algo

  realizar_tarefas(); // Realiza as tarefas de acordo com o que o PIC retornou
  
  tempo_real(); // Atualiza o tempo real do arduino

}

void leitor_bluetooth(){
  if(bt.available())	
  { 
    char c = bt.read();	
    c = toupper(c);

    if(c == '?'){
      que_horas_sao();
      return;
    }
    if(c == 'A'){
      enviar_PIC('A');
      return;
    }
    if(c == 'S'){
      enviar_PIC('d');
      return;      
    }
    if(c == 'T'){
      char buffer[10]; 
      int i = 0;
      int cont = 0;
      int hora = 0;
      int minuto = 0;
      
      delay(1);      
      while(bt.available() && cont < 9){
        delay(1);    
        c = bt.read();
        if(isalpha(c)){
          c = toupper(c);
        }	
        buffer[i] = c;

        if(c == 'H'){
          buffer[i] = '\0';
          hora = atoi(buffer);
          i = -1;
        }
        if(c == 'M'){
          buffer[i] = '\0';
          minuto = atoi(buffer);
          hora_atual = hora*60 + minuto;
          bt.println("Hora configurada!");
          
          return;
        }
        i++;
        cont++;
      } 
      bt.println("Erro: Hora atual nao configurada!");     
    }
    else if(c == 'D'){
     
      char buffer[10]; 
      int i = 0;
      int cont = 0;
      int hora = 0;
      int minuto = 0;
      flag_despertador = 0;  
      
     
      delay(1); 
      while(bt.available() && cont < 9){   
        
        delay(1); 
        c = bt.read();	
        if(isalpha(c)){
          c = toupper(c);
        }
        buffer[i] = c;

        if(c == 'H'){
          buffer[i] = '\0';
          hora = atoi(buffer);
          i = -1;
        }
        if(c == 'M'){
          buffer[i] = '\0';
          minuto = atoi(buffer);
          despertador = hora*60 + minuto;
          bt.println("Despertador configurado!");
          return;
        }
        i++;
        cont++;
      }
      bt.println("Erro: Despertador nao configurado!"); 
    }   
  }
}

void leitor_serial(){
  if(Serial.available()){
    String mensagem = Serial.readStringUntil('\n');
    mensagem.trim();
    mensagem.toUpperCase();

    if(mensagem == "LIGACAO"){
      enviar_PIC('L');
    }
    else if(mensagem == "ATENDER"){
      enviar_PIC('A');
    } 
    else if(mensagem == "EMERGENCIA"){
      enviar_PIC('E');
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
    case 'A':
      ligac_PIC = LOW;
      Serial.println("Enviado: 'A'");
      break;
    case 'E':
      emerg_PIC = HIGH; // Trocar por um sinal de envio para o PIC
      Serial.println("Enviado: 'E'");
      break;
    case 'e':
      emerg_PIC = LOW; // Trocar por um sinal de envio para o PIC
      Serial.println("Enviado: 'e'");
      break;
  }
}

void receber_sinal_PIC(){

}


unsigned long ultimo_tempo = 0;
int tempo_espera = 500; // Tempo de espera para a campainha
int em_uso = 0; // Sinaliza se o sistema esta em uso ou nao

int tempo_espera_despertador = 1000; // Tempo de espera para o despertador
int tempo_espera_ligacao = 100; // Tempo de espera para a ligacao
int tempo_espera_emergencia = 100; // Tempo de espera para a ligacao

void realizar_tarefas(){
  static int contador = 0;
  
  if((em_uso == 0 || em_uso == 1) && camp_PIC == HIGH){
    em_uso = 1;
    if((millis() - ultimo_tempo) >= tempo_espera){
      contador++;
      int leds_campainha = 0;
      switch (contador){
        case 1:
          leds_campainha = 0b10000;
          tempo_espera = 500;
          break;
        case 2:
          leds_campainha = 0b01110;
          tempo_espera = 500;
          break;
        case 3:
          leds_campainha = 0b00001;
          tempo_espera = 500;
          break;
        case 4:
        case 6:
        case 8:
          leds_campainha = 0b11111;
          tempo_espera = 1000;
          break;
        case 5:
        case 7:
          leds_campainha = 0b00000;
          tempo_espera = 500;  
          break;    
        default:
          leds_campainha = 0;
          tempo_espera = 500;
          camp_PIC = LOW;
          em_uso = 0;
          contador = 0;
      }

      digitalWrite(led_amr1, leds_campainha & 0b10000);
      digitalWrite(led_verm1, leds_campainha & 0b01000);
      digitalWrite(led_verm2, leds_campainha & 0b00100);
      digitalWrite(led_verm3, leds_campainha & 0b00010);
      digitalWrite(led_verd1, leds_campainha & 0b00001);
      ultimo_tempo = millis();
    }
  }
  
  if((em_uso == 0 || em_uso == 2) && (alarme_PIC == HIGH)){
    em_uso = 2;
    if((millis() - ultimo_tempo) >= tempo_espera){
      int leds_despertador = 0;
      switch(contador%2){
        case 1:
          leds_despertador = 0b00000;
          tempo_espera = 200;
          break;
        case 0:
          leds_despertador = 0b11111;
          tempo_espera = 1500;
          break;
      }
      digitalWrite(led_amr1, leds_despertador & 0b10000);
      digitalWrite(led_verm1, leds_despertador & 0b01000);
      digitalWrite(led_verm2, leds_despertador & 0b00100);
      digitalWrite(led_verm3, leds_despertador & 0b00010);
      digitalWrite(led_verd1, leds_despertador & 0b00001);
      ultimo_tempo = millis();
      contador++;
    }
  }
  else if(em_uso == 2){
    digitalWrite(led_amr1, LOW);
    digitalWrite(led_verm1, LOW);
    digitalWrite(led_verm2, LOW);
    digitalWrite(led_verm3, LOW);
    digitalWrite(led_verd1, LOW);
    em_uso = 0;
    contador = 0;
  }

  if((em_uso == 0 || em_uso == 3) && (ligac_PIC == HIGH)){
    em_uso = 3;
    if((millis() - ultimo_tempo) >= tempo_espera){
      int leds_ligacao = 0;
      if(contador%30 == 0){
        leds_ligacao = 0b11111;
        tempo_espera = 1000;
      }
      else{
        switch(contador%5){
          case 0:
            leds_ligacao = 0b00001;
            tempo_espera = 100;
            break;
          case 1:
            leds_ligacao = 0b00010;
            tempo_espera = 100;
            break;
          case 2:
            leds_ligacao = 0b00100;
            tempo_espera = 100;
            break;
          case 3:
            leds_ligacao = 0b01000;
            tempo_espera = 100;
            break;
          case 4:
            leds_ligacao = 0b10000;
            tempo_espera = 100;
            break;
        }
      }
      digitalWrite(led_amr1, leds_ligacao & 0b10000);
      digitalWrite(led_verm1, leds_ligacao & 0b01000);
      digitalWrite(led_verm2, leds_ligacao & 0b00100);
      digitalWrite(led_verm3, leds_ligacao & 0b00010);
      digitalWrite(led_verd1, leds_ligacao & 0b00001);
      ultimo_tempo = millis();
      contador++;
    }
  }
  else if(em_uso == 3){
    digitalWrite(led_amr1, LOW);
    digitalWrite(led_verm1, LOW);
    digitalWrite(led_verd1, LOW);
    em_uso = 0;
    contador = 0;
  }

  if((em_uso == 0 || em_uso == 4) && (emerg_PIC == HIGH)){
    em_uso = 4;
    if((millis() - ultimo_tempo) >= tempo_espera){
      contador = (contador + 1)%1000;
      int leds_emergencia = 0;
      switch (contador%2){
        case 0:
          leds_emergencia = 0b01110;
          tempo_espera = 200;
          break;
        case 1:
          leds_emergencia = 0b00000;
          tempo_espera = 200;
          break;
      }

      digitalWrite(led_amr1, leds_emergencia & 0b10000);
      digitalWrite(led_verm1, leds_emergencia & 0b01000);
      digitalWrite(led_verm2, leds_emergencia & 0b00100);
      digitalWrite(led_verm3, leds_emergencia & 0b00010);
      digitalWrite(led_verd1, leds_emergencia & 0b00001);
      ultimo_tempo = millis();
    }
  }
  else if(em_uso == 4){
    digitalWrite(led_amr1, LOW);
    digitalWrite(led_verm1, LOW);
    digitalWrite(led_verm2, LOW);
    digitalWrite(led_verm3, LOW);
    digitalWrite(led_verd1, LOW);
    em_uso = 0;
    contador = 0;
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

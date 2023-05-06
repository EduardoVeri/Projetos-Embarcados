/*Os leds deverão ser usados para indicar a quantidade de vezes que os botões
(b1, b2 e b3) foram pressionados.
✔ Cada led deve ser acionado de forma suave usando o PWM.
✔ O desligamento dos leds deve ser feito de forma direta, ou seja, aplicando nível
lógico 0 para desligá-los.
✔ A soma da quantidade de pressionamento dos botões (b1, b2 e b3) deve ser
mostrado nos leds, usando a representação binária do valor da somatória. 
✔ Quando a contagem (b1+b2+b3) ultrapassar o valor decimal igual a 15, os leds
devem ser ligados por 1 segundo e desligar por 1 segundos, três vezes
consecutivas.
✔ Após os leds piscarem por três vezes, o contador deve ser zerado e os leds
devem retornar ao valor inicial (zero).
✔ O LM35 deve realizar a medição da temperatura a cada 1 segundo.
✔ Quando a temperatura medida for superior ao valor limite estabelecido pelo
usuário, o buzzer deve ser ligado por 0,5 segundos e desligado por 2 segundos,
3 vezes consecutivas.
✔ Sempre que a temperatura estiver abaixo do limite estabelecido, o buzzer deve
permanecer desligado.
✔ Quando o botão 1 for pressionado, deve ser mostrado no monitor serial o menor
valor de temperatura mensurado durante o monitoramento
✔ Quando o botão 2 for pressionado, deve ser mostrado no monitor serial o valor
médio de temperatura calculada a partir das últimas 30 medições realizadas
(lembrando que as medições devem ser feitas a cada 1 segundo).
✔ Quando a chave 3 for pressionada, deve ser mostrado no monitor serial a
temperatura máxima mensurada durante o monitoramento. 

Atenção: Proibido o uso da função delay no código.
*/

#include <Arduino.h>

#define TEMP_LIMITE 23 // Temperatura para acionar o buzzer

// =============== Constantes para os dispositivos ================== //
const int button1 = 13; 
const int button2 = 12; 
const int button3 = 11; 
const int ledVermelho1 = 5; 
const int ledVermelho2 = 6;
const int ledVermelho3 = 9;
const int ledVermelho4 = 10;
const int buzzer = 7; 
const int lm35 = A0;

// ================ Variáveis para os botões ================ //
int last1 = LOW; // Guarda o último estado do botão;
int last2 = LOW; // Guarda o último estado do botão;
int last3 = LOW; // Guarda o último estado do botão;

int estado1 = LOW; // Guarda o estado atual do botão
int estado2 = LOW; // Guarda o estado atual do botão
int estado3 = LOW; // Guarda o estado atual do botão

unsigned long lastDebounceTime1 = 0;
unsigned long lastDebounceTime2 = 0;
unsigned long lastDebounceTime3 = 0;

unsigned long debounceDelay = 50;  // Valor para o intervalo do debounce

// ============= Valores dos estados dos LEDS ================ //
int estVermelho1 = LOW;
int estVermelho2 = LOW;
int estVermelho3 = LOW;
int estVermelho4 = LOW;
int estBuzzer = LOW;

// ============= Protótipos das funções ================ //
void debouncer(unsigned long* lastDebounceTime, int* buttonState, int* last, int botao);
void acenderLed(int estado1, int estado2, int estado3, int estado4);
void mostrarSerial(double temp, int chave);

// ============= Variáveis globais ================ //
int flag = 0; // Flag para o debounce

int contador = 0; // Contador para os botões
unsigned long tempo_leds = 0; // Timer para os leds piscarem
int flag_piscada = 0; // Sinaliza se estao acesos ou apagados

unsigned long tempo_temp = 0; // Timer para a leitura da temperatura
double temp_maxima; // Temperatura máxima
double temp_minima; // Temperatura minima
double temp_media = 0; // Media das ultimas 30 temperaturas
double vet_temp[30]; // Vetor para armazenar as ultimas 30 temperaturas
int indice = 0; // Indice para o vetor de temperaturas
int temp_lidas = 0; // Contador para as temperaturas lidas

unsigned long tempo_buzzer = 0; // Timer para o buzzer
int flag_buzzer = 0; // Sinaliza se o buzzer esta ligado ou desligado


// ============= Configurações iniciais ================ //
void setup() {
  Serial.begin(9600);
  pinMode(button1, INPUT); 
  pinMode(button2, INPUT);
  pinMode(button3, INPUT);
  pinMode(ledVermelho1, OUTPUT); 
  pinMode(ledVermelho2, OUTPUT); 
  pinMode(ledVermelho3, OUTPUT); 
  pinMode(ledVermelho4, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(lm35, INPUT);

  last1 = digitalRead(button1);
  last2 = digitalRead(button2);
  last3 = digitalRead(button3);

  analogReference(INTERNAL1V1); // Referencia interna de 1.1V
}

// ============= Loop principal ================ //
void loop() {  
  // ================== Debouncer ======================= //
  debouncer(&lastDebounceTime1, &estado1, &last1, button1);
  debouncer(&lastDebounceTime2, &estado2, &last2, button2);
  debouncer(&lastDebounceTime3, &estado3, &last3, button3);

  // =================== Botoes ======================== //
  int contador_aux = contador;
  if(estado1 == HIGH && flag == 1){
    contador_aux++;
    mostrarSerial(temp_minima, 1);
    flag = 0;
  }
  if(estado2 == HIGH && flag == 1){
    contador_aux++;
    mostrarSerial(temp_media, 2);
    flag = 0;
  }
  if(estado3 == HIGH && flag == 1){
    contador_aux++;
    mostrarSerial(temp_maxima, 3);
    flag = 0;
  }

  // ================== Leitura do LM35 ================== //
  double temp = 0;
  double soma = 0;

  if((millis() - tempo_temp) > 1000){
    tempo_temp = millis();

    temp = analogRead(lm35)*0.1075268817;
    vet_temp[indice] = temp;
    indice = (indice+1)%30; // Reinicia o indice quando chega no 30

    if(temp_lidas < 30){
      temp_lidas++;
    }

    for (int i = 0; i < temp_lidas; i++){
      soma += vet_temp[i];
    }

    temp_media = soma/temp_lidas;

    if(temp > temp_maxima){
      temp_maxima = temp;
    }
    if(temp < temp_minima){
      temp_minima = temp;
    }
  }


  // ================== Buzzer ================== //
  if(temp > TEMP_LIMITE){
    if((millis() - tempo_buzzer) > 2000 && flag_buzzer == 0){
      tempo_buzzer = millis();
      digitalWrite(buzzer, HIGH);
      flag_buzzer = 1;
    }
    else if((millis() - tempo_buzzer) > 500 && flag_buzzer == 1){
      tempo_buzzer = millis();
      flag_buzzer = 0;
      digitalWrite(buzzer, LOW);
    }
  }
  else{
    if((millis() - tempo_buzzer) > 500 && flag_buzzer == 1){
      tempo_buzzer = millis();
      flag_buzzer = 0;
      digitalWrite(buzzer, LOW);
    }
    else if(flag_buzzer == 1){
      digitalWrite(buzzer, HIGH);
    }
    else{
      flag_buzzer = 0;
      digitalWrite(buzzer, LOW);
    }
  }

  // ================== Contador ================== //
  if(contador_aux != contador){
    contador = contador_aux;
    acenderLed(LOW, LOW, LOW, LOW); // Valores dos estados dos LEDS nao importa quando contador < 15
  }

  if(contador > 15){
    if(((millis() - tempo_leds) > 1000) && (flag_piscada%2 == 0)){
      acenderLed(LOW, LOW, LOW, LOW);
      tempo_leds = millis();
      flag_piscada++;
    }
    else if (((millis() - tempo_leds) > 1000) && (flag_piscada%2 == 1)){
      acenderLed(HIGH, HIGH, HIGH, HIGH);
      tempo_leds = millis();
      flag_piscada++;
    }
    else if(flag_piscada >= 7){ // TODO: Verificar se o contador esta zerando corretamente
      flag_piscada = 0;
      contador = 0;
    }
  }

}


// ============= Funções ================ //

void debouncer(unsigned long* lastDebounceTime, int* buttonState, int* last, int botao){
  int leitura = digitalRead(botao);
  
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

void mostrarSerial(double temp, int chave){ 
  switch (chave){
    case 1:
      Serial.print("Temperatura minima: ");
      Serial.println(temp);
      break;
    case 2:
      Serial.print("Temperatura media: ");
      Serial.println(temp);
      break;
    case 3:
      Serial.print("Temperatura maxima: ");
      Serial.println(temp);
      break;
  }
}

void acenderLed(int estado1, int estado2, int estado3, int estado4){
  int pwm = 0;

  if(contador > 15){
    digitalWrite(ledVermelho1, estado1);
    digitalWrite(ledVermelho2, estado2);
    digitalWrite(ledVermelho3, estado3);
    digitalWrite(ledVermelho4, estado4);

    return;
  }
  

  switch (contador){
    case 0:
      digitalWrite(ledVermelho1, LOW);
      digitalWrite(ledVermelho2, LOW);
      digitalWrite(ledVermelho3, LOW);
      digitalWrite(ledVermelho4, LOW);
      break;
    case 1:
      digitalWrite(ledVermelho2, LOW);
      digitalWrite(ledVermelho3, LOW);
      digitalWrite(ledVermelho4, LOW);

      for (pwm = 0; pwm <= 255; pwm++){
        analogWrite(ledVermelho1, pwm);
        delay(1);
      }

      break;
    case 2:
      digitalWrite(ledVermelho1, LOW);
      digitalWrite(ledVermelho3, LOW);
      digitalWrite(ledVermelho4, LOW);

      for (pwm = 0; pwm <= 255; pwm++){
        analogWrite(ledVermelho2, pwm);
        delay(1);
      }

      break;
    case 3: 
      digitalWrite(ledVermelho3, LOW);
      digitalWrite(ledVermelho4, LOW);

      for (pwm = 0; pwm <= 255; pwm++){
        analogWrite(ledVermelho1, pwm);
        analogWrite(ledVermelho2, pwm);
        delay(1);
      }

      break;
    case 4:
      digitalWrite(ledVermelho1, LOW);
      digitalWrite(ledVermelho2, LOW);
      digitalWrite(ledVermelho4, LOW);

      for (pwm = 0; pwm <= 255; pwm++){
        analogWrite(ledVermelho3, pwm);
        delay(1);
      }

      break;
    case 5:
      digitalWrite(ledVermelho2, LOW);
      digitalWrite(ledVermelho4, LOW);

      for (pwm = 0; pwm <= 255; pwm++){
        analogWrite(ledVermelho1, pwm);
        analogWrite(ledVermelho3, pwm);
        delay(1);
      }

      break;
    case 6:
      digitalWrite(ledVermelho1, LOW);
      digitalWrite(ledVermelho4, LOW);

      for (pwm = 0; pwm <= 255; pwm++){
        analogWrite(ledVermelho2, pwm);
        analogWrite(ledVermelho3, pwm);
        delay(1);
      }

      break;
    case 7:
      digitalWrite(ledVermelho4, LOW);

      for (pwm = 0; pwm <= 255; pwm++){
        analogWrite(ledVermelho1, pwm);
        analogWrite(ledVermelho2, pwm);
        analogWrite(ledVermelho3, pwm);
        delay(1);
      }

      break;
    case 8:
      digitalWrite(ledVermelho1, LOW);
      digitalWrite(ledVermelho2, LOW);
      digitalWrite(ledVermelho3, LOW);

      for (pwm = 0; pwm <= 255; pwm++){
        analogWrite(ledVermelho4, pwm);
        delay(1);
      }

      break;
    case 9:
      digitalWrite(ledVermelho2, LOW);
      digitalWrite(ledVermelho3, LOW);

      for (pwm = 0; pwm <= 255; pwm++){
        analogWrite(ledVermelho1, pwm);
        analogWrite(ledVermelho4, pwm);
        delay(1);
      }

      break;
    case 10:
      digitalWrite(ledVermelho1, LOW);
      digitalWrite(ledVermelho3, LOW);

      for (pwm = 0; pwm <= 255; pwm++){
        analogWrite(ledVermelho2, pwm);
        analogWrite(ledVermelho4, pwm);
        delay(1);
      }

      break;
    case 11:
      digitalWrite(ledVermelho3, LOW);

      for (pwm = 0; pwm <= 255; pwm++){
        analogWrite(ledVermelho1, pwm);
        analogWrite(ledVermelho2, pwm);
        analogWrite(ledVermelho4, pwm);
        delay(1);
      }
      break;

    case 12:
      digitalWrite(ledVermelho1, LOW);
      digitalWrite(ledVermelho2, LOW);

      for (pwm = 0; pwm <= 255; pwm++){
        analogWrite(ledVermelho3, pwm);
        analogWrite(ledVermelho4, pwm);
        delay(1);
      }
      break;
    case 13:
      digitalWrite(ledVermelho2, LOW);

      for (pwm = 0; pwm <= 255; pwm++){
        analogWrite(ledVermelho1, pwm);
        analogWrite(ledVermelho3, pwm);
        analogWrite(ledVermelho4, pwm);
        delay(1);
      }

      break;
    case 14:
      digitalWrite(ledVermelho1, LOW);

      for (pwm = 0; pwm <= 255; pwm++){
        analogWrite(ledVermelho2, pwm);
        analogWrite(ledVermelho3, pwm);
        analogWrite(ledVermelho4, pwm);
        delay(1);
      }

      break;
    case 15:
      for (pwm = 0; pwm <= 255; pwm = pwm++){
        analogWrite(ledVermelho1, pwm);
        analogWrite(ledVermelho2, pwm);
        analogWrite(ledVermelho3, pwm);
        analogWrite(ledVermelho4, pwm);
        delay(1);
      }

      break;
  }
} 


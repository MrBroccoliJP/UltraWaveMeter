/**
 * UltraWaveMeter - Ultrasonic measurement system designed for precise distance and wave detection
 * Copyright (c) 2022-2025 João Fernandes
 * 
 * This work is licensed under the Creative Commons Attribution-NonCommercial 
 * 4.0 International License. To view a copy of this license, visit:
 * http://creativecommons.org/licenses/by-nc/4.0/
 * 
 * 
 * @author João Fernandes
 * @version 5.1
 * @date 2022
 * 
 * Este programa utiliza um sensor ultrassônico para medir distâncias ao longo do tempo,
 * que pode ser usado para aplicações de medição de ondas. Fornece diferentes
 * modos de medição e envia dados através de comunicação serial.
 */

#include <NewPing.h>

// Configuração do hardware
#define TRIGGER_PIN 13 // Pino do Arduino conectado ao pino de trigger do sensor ultrassônico
#define ECHO_PIN 15    // Pino do Arduino conectado ao pino de echo do sensor ultrassônico
#define MAX_DISTANCE 170 // Distância máxima de medição em centímetros

// Configuração da biblioteca NewPing para o sensor ultrassônico
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

// Variáveis de ambiente e calibração
float temp = 25;                                  // Temperatura em Celsius (valor padrão)
float factor = sqrt(1 + temp / 273.15) / 60.368;  // Cálculo do fator de velocidade do som baseado na temperatura
short int Mode = 0;                               // Modo de operação (0: espera, 1: medição padrão, 2: intervalo personalizado)
bool btnState;                                    // Estado atual dos botões
unsigned long lastButtonPress = 0;                // Timestamp do último estado do botão para debounce
short int btnDelay = 250;                         // Delay para debounce do botão em milissegundos
short int readingN = 0;                           // Contador para sessões de medição
unsigned long ReadingStart = 0;                   // Timestamp de quando a sessão de medição começou
String READING;                                   // Armazena dados formatados de medição para saída
String mydata;                                    // Armazena entrada da porta serial
float avg = 0;                                    // Média das medições base

// Instruções para o usuário e informações do programa
String inst = "\nJoão Fernandes 2022\n Para começar leituras clique no botão 1, botão 2 ou insira 'S' no comando de série.\n Para parar clique no botão 1, botão 2 ou insira 'S' no comando de série.\n Para fazer uma leitura com um numero diferente de tempo entre amostras (default = 40ms) envie 'N'\n Para RESET envie 'R'";
String Code_version = "V5.1";
short int Rate = 40; // Intervalo padrão entre amostras em milissegundos

/**
 * Função de configuração inicial - executa uma vez no início do programa
 */
void setup() {
  // Inicializa comunicação serial
  Serial.begin(9600);
  
  // Configura modos dos pinos
  pinMode(16, OUTPUT); // LED
  pinMode(0, INPUT_PULLUP); // Botão 1
  pinMode(10, INPUT_PULLUP); // Botão 2

  // Sequência de inicialização do LED para confirmação visual
  digitalWrite(16, HIGH);
  delay(100);
  digitalWrite(16, LOW);
  delay(500);
  digitalWrite(16, HIGH);
  delay(100);
  digitalWrite(16, LOW);
  delay(1000);
  
  // Imprime mensagem de inicio
  Serial.print("\nPROGRAMA A INICIAR \n");
  Serial.print("\nInsira a temperatura ambiente: ");

  // Aguarda entrada da temperatura enquanto pisca o LED
  while (!Serial || !Serial.available()) {
    // Pisca LED enquanto aguarda
    if (millis() % 250 == 0) {
      digitalWrite(16, HIGH);
    }
    else if (millis() % 100 == 0) {
      digitalWrite(16, LOW);
    }
    
    // Repete o prompt periodicamente
    if (millis() % 3000 == 0) {
      Serial.print("\nInsira a temperatura ambiente: ");
      Serial.flush();
    }
    yield();
  }

  // Processa entrada da temperatura
  while (mydata.toInt() == 0) {
    mydata = Serial.readStringUntil('\n');
    Serial.println(mydata);
    Serial.println("Temperatura guardada\n===== Programa Iniciado =====\n");
    Serial.flush();
    yield();
  }
  
  // Define temperatura e calcula fator de velocidade do som
  temp = mydata.toInt();
  factor = sqrt(1 + temp / 273.15) / 60.368; // Cálculo do fator baseado na temperatura

  // Realiza medições iniciais da distância base
  for (int i = 2; i > 0; i--) {
    float readingB = ((float)sonar.ping_median(5) * factor);
    avg = avg + readingB;
    READING = "Distancia BASE: " + (String)readingB + " cm ";
    READING.replace(".", ","); // Formata para compatibilidade com CSV
    Serial.println(READING);
    delay(300);
  }
  
  // Calcula e mostra a média da leitura base
  avg = avg / 2;
  READING = "\nMedia: " + (String)avg + " cm ";
  READING.replace(".", ",");
  Serial.println(READING);
  
  // Exibe informações do programa e instruções
  Serial.print("\n Versão: ");
  Serial.println(Code_version);
  Serial.println(inst);
  Serial.println("****************************\n");
  Serial.println("A aguardar inicio -> Botão 1 ou Botão 2 ou escreva 'S' OU 'N' ");
  
  // Define o modo inicial para espera
  Mode = 0;
}

/**
 * Loop principal do programa
 */
void loop() {
  // MODO 0: Espera - aguarda entrada do utilizador para iniciar medição
  if (Mode == 0) {
    mydata = "";
    while (Mode == 0) {
      // Padrão de piscar LED para modo de espera
      if (millis() % 500 == 0) {
        digitalWrite(16, HIGH);
      }
      else if (millis() % 250 == 0) {
        digitalWrite(16, LOW);
      }

      // Verifica input 
      if (Serial.available() > 0) {
        mydata = Serial.readStringUntil('\n');
        Serial.println(mydata);

        // Processa comandos
        if (mydata == "S") {
          Mode = 1; // Inicia medição padrão
          Serial.flush();
        }
        else if (mydata == "R") {
          ESP.reset(); // Reinicia o sistema
        }
        else if (mydata == "N") {
          Mode = 2; // Inicia medição com intervalo personalizado
          Serial.flush();
        }
        else {
          Serial.println("Modo inválido -> 'S' OU 'N'");
          mydata = "";
          Serial.flush();
        }
      }

      // Verifica estado do Botão 1
      btnState = digitalRead(0);
      if (btnState == LOW) {
        if (millis() - lastButtonPress > btnDelay) {
          Mode = 1; // Inicia medição padrão
          btnState = HIGH;
        }
        lastButtonPress = millis();
      }
      
      // Verifica estado do Botão 2
      btnState = digitalRead(4);
      if (btnState == LOW) {
        if (millis() - lastButtonPress > btnDelay) {
          Mode = 1; // Inicia medição padrão
          btnState = HIGH; 
        }
        lastButtonPress = millis();
      }
      
      yield();
    }
  }
  
  // MODO 2: Medição com intervalo personalizado
  else if (Mode == 2) {
    Rate = 0;
    avg = 0;

    Serial.print("\nMODO 2\n");
    Serial.print("\nInsira o numero de ms entre amostras pretendido (minimo 40ms): ");

    // Obtém entrada de intervalo válida
    while (Rate < 40) {
      if (Serial.available() > 0) {
        mydata = Serial.readStringUntil('\n');
        
        // Processa entrada
        if (mydata.toInt() > 40 && mydata.toInt() < 5000) {
          Rate = mydata.toInt();
          Serial.flush();
        }
        else if (mydata == "R") {
          ESP.reset();
        }
        else if (mydata == "S") {
          Mode = 0;
          Serial.flush();
          break;
        }
        else {
          Serial.println("\nValor Inválido ('R' PARA RESET & 'S' Para voltar ao menu inicial)\nInsira um valor >40ms");
          Serial.flush();
        }
        // Código comentado para debug
        // Serial.println(mydata);
        // Serial.println(mydata.toInt());
      }
      yield();
    }

    // Inicia nova sessão de medição
    readingN++;
    if (Mode != 0) {
      Serial.print("\n********** LEITURA NUMERO: ");
      Serial.print(readingN);
      Serial.println(" **********");

      // Realiza medições de linha de base
      for (int i = 5; i > 0; i--) {
        float readingB = ((float)sonar.ping_median(5) * factor);
        avg = avg + readingB;
        READING = "Distancia BASE: " + (String)readingB + " cm ";
        READING.replace(".", ",");
        Serial.println(READING);
        delay(300);
      }
      
      // Calcula e exibe média da distância base
      avg = avg / 5;
      READING = "\nDistancia BASE: Média = " + (String)avg + " cm ";
      READING.replace(".", ",");
      Serial.println(READING);
      Serial.print("\nDISTÂNCIA (cm) || TEMPO (ms)\n\n");

      // Regista tempo de início para medição de tempo relativo
      ReadingStart = millis();
    }

    // Loop contínuo de medição
    while (Mode != 0) {
      digitalWrite(16, HIGH); // LED aceso durante medição

      // Verifica comandos seriais
      if (Serial.available() > 0) {
        mydata = Serial.readStringUntil('\n');
        if (mydata == "S") {
          Mode = 0;
          Serial.flush();
        }
        if (mydata == "R") {
          ESP.reset();
        }
      }

      // Verifica estado do Botão 1
      btnState = digitalRead(0);
      if (btnState == LOW) {
        if (millis() - lastButtonPress > btnDelay) {
          Mode = 0;
          btnState = HIGH; 
        }
        lastButtonPress = millis();
      }
      
      // Verifica estado do Botão 2
      btnState = digitalRead(4);
      if (btnState == LOW) {
        if (millis() - lastButtonPress > btnDelay) {
          Mode = 0;
          btnState = HIGH; 
        }
        lastButtonPress = millis();
      }

      // Realiza medição no intervalo especificado
      if (millis() % Rate == 0) {
        READING = String(((float)sonar.ping_median(1) * factor), 2) + " " + 
                  (String)(millis() - ReadingStart) + "\n";
        READING.replace(".", ","); // Formata para compatibilidade com CSV
        Serial.print(READING);
        Serial.flush();
      }

      yield(); 
    }
  }
  
  // MODO 1: Medição padrão com intervalo de 40ms
  else {
    avg = 0;
    readingN++;
    
    Serial.print("\n********** LEITURA NUMERO: ");
    Serial.print(readingN);
    Serial.println(" **********");
    // Código comentado para debug
    // Serial.print("BASE: ");
    // Serial.print(avg);

    // Realiza medições da distância base
    for (int i = 5; i > 0; i--) {
      float readingB = ((float)sonar.ping_median(5) * factor);
      avg = avg + readingB;
      READING = "Distancia BASE: " + (String)readingB + " cm ";
      READING.replace(".", ",");
      Serial.println(READING);
      delay(300);
    }
    
    // Calcula e exibe média da distância base
    avg = avg / 5;
    READING = "\nDistancia BASE: Média = " + (String)avg + " cm ";
    READING.replace(".", ",");
    Serial.println(READING);
    Serial.print("\n Intervalo de tempo entre amostras: 40ms \n\n DISTÂNCIA (cm) || TEMPO (ms)\n\n");

    // Regista tempo de início para medição de tempo relativo
    ReadingStart = millis();

    // Loop contínuo de medição
    while (Mode != 0) {
      digitalWrite(16, HIGH); // LED aceso durante medição

      // Verifica comandos seriais
      if (Serial.available() > 0) {
        mydata = Serial.readStringUntil('\n');

        if (mydata == "S") {
          Mode = 0;
          Serial.flush();
        }
        else if (mydata == "R") {
          ESP.reset();
        }
        else {
          Serial.flush();
        }
      }

      // Verifica estado do Botão 1
      btnState = digitalRead(0);
      if (btnState == LOW) {
        if (millis() - lastButtonPress > btnDelay) {
          Mode = 0;
          btnState = HIGH;
        }
        lastButtonPress = millis();
      }
      
      // Verifica estado do Botão 2
      btnState = digitalRead(4);
      if (btnState == LOW) {
        if (millis() - lastButtonPress > btnDelay) {
          Mode = 0;
          btnState = HIGH;
        }
        lastButtonPress = millis();
      }

      // Realiza medição com o intervalo padrão (40ms)
      if (millis() % 40 == 0) {
        READING = String(((float)sonar.ping_median(1) * factor), 2) + " " + 
                  (String)(millis() - ReadingStart) + "\n";
        READING.replace(".", ","); // Formata para compatibilidade com CSV
        Serial.print(READING);
        Serial.flush();
      }

      yield(); 
    }
  }
}

//JOAO FERNANDES 2022
#include <stdarg.h>

// Definição de tipo para nossa callback
using CallbackFunc = void(*)(); 

// Variáveis globais
volatile CallbackFunc threadFunc[5]; 
volatile int numThread, threadInterval[5], threadTime[5];

// Função de configuração das threads
void threadSetup(){
  numThread = 0;
}

// Função de configuração das threads com parâmetros variáveis
void threadSetup(CallbackFunc callback, int threadIntervalFunc, ...) {
  numThread = 1;
  va_list args;
  va_start(args, threadIntervalFunc);
  
  // Contagem do número de threads e inicialização dos parâmetros
  while(va_arg(args, CallbackFunc)) { 
    va_arg(args, int);
    numThread++;
  }

  // Inicialização dos parâmetros da primeira thread
  threadTime[0] = 0;
  threadFunc[0] = callback;
  threadInterval[0] = threadIntervalFunc;

  // Inicialização dos parâmetros das demais threads
  va_start(args, threadIntervalFunc);
  for(int i = 1; i < numThread; i++) {
    threadTime[i] = 0;
    threadFunc[i] = va_arg(args, CallbackFunc);
    threadInterval[i] = va_arg(args, int);
  }
  va_end(args);
  
  // Desliga as interrupções para evitar erros durante a configuração 
  cli(); 
  // Configuração do Timer2 para geração de interrupções em intervalos regulares
  TCCR2A = 0b00000010; // Modo comparação
  TCCR2B = 0b00000100; // Configuração do prescaler em 64
  TIMSK2 = 0b00000010; // Habilita a interrupção por overflow do timer 
  OCR2A  = 249; // Compare match register = [ 16,000,000Hz/ (prescaler * desired interrupt frequency) ] - 1
  sei(); // Habilita interrupções globais
}

// Rotina de Serviço de Interrupção (ISR) do Timer2
ISR(TIMER2_COMPA_vect) {
  // Itera sobre todas as threads agendadas
  for(int i = 0; i < numThread; i++) {
    // Verifica se o tempo decorrido desde a última chamada é maior ou igual ao intervalo definido
    if(threadTime[i] >= threadInterval[i]) {
      // Chama a função correspondente e reseta o tempo
      threadFunc[i]();
      threadTime[i] = 0;
    }
    else {
      // Incrementa o tempo decorrido
      threadTime[i]++;
    }
  } 
}

// Função para criar uma espera de tempo semelhante ao delay()
void threadDelay(unsigned long ms) {
  unsigned long start = millis();
  while (millis() - start <= ms);
}
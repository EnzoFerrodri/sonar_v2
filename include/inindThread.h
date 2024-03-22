#include <stdarg.h>

// Defini��o de tipo para nossa callback
using CallbackFunc = void(*)(); 

// Vari�veis globais
volatile CallbackFunc threadFunc[5]; 
volatile int numThread, threadInterval[5], threadTime[5];

// Fun��o de configura��o das threads
void threadSetup(){
  numThread = 0;
}

// Fun��o de configura��o das threads com par�metros vari�veis
void threadSetup(CallbackFunc callback, int threadIntervalFunc, ...) {
  numThread = 1;
  va_list args;
  va_start(args, threadIntervalFunc);
  
  // Contagem do n�mero de threads e inicializa��o dos par�metros
  while(va_arg(args, CallbackFunc)) { 
    va_arg(args, int);
    numThread++;
  }

  // Inicializa��o dos par�metros da primeira thread
  threadTime[0] = 0;
  threadFunc[0] = callback;
  threadInterval[0] = threadIntervalFunc;

  // Inicializa��o dos par�metros das demais threads
  va_start(args, threadIntervalFunc);
  for(int i = 1; i < numThread; i++) {
    threadTime[i] = 0;
    threadFunc[i] = va_arg(args, CallbackFunc);
    threadInterval[i] = va_arg(args, int);
  }
  va_end(args);
  
  // Desliga as interrup��es para evitar erros durante a configura��o 
  cli(); 
  // Configura��o do Timer2 para gera��o de interrup��es em intervalos regulares
  TCCR2A = 0b00000010; // Modo compara��o
  TCCR2B = 0b00000100; // Configura��o do prescaler em 64
  TIMSK2 = 0b00000010; // Habilita a interrup��o por overflow do timer 
  OCR2A  = 249; // Compare match register = [ 16,000,000Hz/ (prescaler * desired interrupt frequency) ] - 1
  sei(); // Habilita interrup��es globais
}

// Rotina de Servi�o de Interrup��o (ISR) do Timer2
ISR(TIMER2_COMPA_vect) {
  // Itera sobre todas as threads agendadas
  for(int i = 0; i < numThread; i++) {
    // Verifica se o tempo decorrido desde a �ltima chamada � maior ou igual ao intervalo definido
    if(threadTime[i] >= threadInterval[i]) {
      // Chama a fun��o correspondente e reseta o tempo
      threadFunc[i]();
      threadTime[i] = 0;
    }
    else {
      // Incrementa o tempo decorrido
      threadTime[i]++;
    }
  } 
}

// Fun��o para criar uma espera de tempo semelhante ao delay()
void threadDelay(unsigned long ms) {
  unsigned long start = millis();
  while (millis() - start <= ms);
}
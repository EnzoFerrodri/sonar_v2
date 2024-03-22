#include <NewPing.h>
#include <inindThread.h>
#include <Arduino.h>

#define FILTER_ORDER 10 // Ordem do filtro digital
#define TRIGGER_PIN 2
#define ECHO_PIN 3
#define MAX_DISTANCE 200
#define LED_PIN 9 // Pino do LED PWM

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // Cria um objeto NewPing para o sensor ultrassônico

// Definição da estrutura do filtro
typedef struct {
  double *buffer;
  unsigned int index;
  unsigned int filterOrder;
} WeightedMovingAverageFilter;

WeightedMovingAverageFilter filter;

void WeightedMovingAverageFilter_init(WeightedMovingAverageFilter* f, unsigned int filterOrder) {
  f->buffer = (double *)malloc(filterOrder * sizeof(double));
  for (unsigned int i = 0; i < filterOrder; ++i) f->buffer[i] = 0;
  f->filterOrder = filterOrder;
  f->index = 0;
}

double WeightedMovingAverageFilter_put(WeightedMovingAverageFilter* f, double input) {
  f->buffer[f->index++] = input;
  if (f->index >= f->filterOrder) f->index = 0;

  // pwm: Calcular a média ponderada dos valores no buffer
  double weightedSum = 0;
  double weightSum = 0;
  for (unsigned int i = 0; i < f->filterOrder; ++i) {
    double weight = 1.0 / (i + 1); // Peso decrescente para as leituras mais recentes
    weightedSum += f->buffer[i] * weight;
    weightSum += weight;
  }
  return weightedSum / weightSum;
}

void setup() {
  Serial.begin(9600);

  // Inicialização do filtro
  WeightedMovingAverageFilter_init(&filter, FILTER_ORDER);

  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT); // Configura o pino do LED como saída

  // Leitura inicial do sensor
  unsigned int duration = sonar.ping_median(5);
  double distance_cm = (duration / 2.0) * 0.0343;
  for (unsigned int i = 0; i < FILTER_ORDER; ++i) {
    WeightedMovingAverageFilter_put(&filter, distance_cm);
  }

  // Espera inicial para estabilização
  threadDelay(500);
}

void loop() {
  unsigned int duration = sonar.ping_median(5); // Realiza a medição da duração do eco em microssegundos
  double distance_cm = (duration / 2.0) * 0.0343; // Converte a duração do eco em centímetros
  double filtered_distance = WeightedMovingAverageFilter_put(&filter, distance_cm); // Aplica o filtro de média móvel ponderada

  // pwm: Imprimir a distância filtrada para fins de debugging
  Serial.print("Distância filtrada: ");
  Serial.print(filtered_distance);
  Serial.println(" cm");

  // pwm: Calcula o valor PWM baseado na distância
  int pwmValue = map(filtered_distance, 0, MAX_DISTANCE, 0, 255);
  analogWrite(LED_PIN, pwmValue); // Define a intensidade do LED com PWM

  // Aguarde um curto período antes da próxima leitura
  threadDelay(100);
}
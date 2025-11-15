#include "HX711.h"
#include <LiquidCrystal.h>

// =====================
// Pinos de ligação
// =====================
#define DT_PIN A1
#define SCK_PIN A0

#define LED_SUP 9     // LED para limite superior
#define LED_INF 8     // LED para limite inferior

#define POT_PIN A2    // potenciômetro para ajustar limites

#define BUZZER 10     // BUZZER ATIVO

// LCD (RS, E, D4, D5, D6, D7)
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

HX711 balanca;

// =====================
// Variáveis globais
// =====================
float fatorCalibracao = -7050.0;
float peso = 0.0;
bool calibrado = false;

float limiteSuperior = 0.5;
float limiteInferior = 0.2;

float pesoReferencia = 1.0;  // 1 kg para calibrar

// =====================
// LER POTENCIÔMETRO (0–5 kg)
// =====================
float lerLimitePot() {
  int leitura = analogRead(POT_PIN);
  float kg = (leitura / 1023.0) * 5.0;
  return kg;
}

// =====================
// AJUSTAR LIMITES
// =====================
void ajustarLimites() {

  // ---- Ajuste do limite inferior ----
  lcd.clear();
  lcd.print("Ajuste LIMITE");
  lcd.setCursor(0, 1);
  lcd.print("INFERIOR...");
  delay(1800);

  unsigned long t0 = millis();
  while (millis() - t0 < 8000) {  
    limiteInferior = lerLimitePot();

    lcd.clear();
    lcd.print("Lim Inf: ");
    lcd.print(limiteInferior, 2);
    lcd.print(" kg");

    delay(300);
  }

  // ---- Ajuste do limite superior ----
  lcd.clear();
  lcd.print("Ajuste LIMITE");
  lcd.setCursor(0, 1);
  lcd.print("SUPERIOR...");
  delay(1800);

  t0 = millis();
  while (millis() - t0 < 8000) {  
    limiteSuperior = lerLimitePot();

    lcd.clear();
    lcd.print("Lim Sup: ");
    lcd.print(limiteSuperior, 2);
    lcd.print(" kg");

    delay(300);
  }

  // Limites confirmados
  lcd.clear();
  lcd.print("Limites salvos!");
  delay(1500);
}

// =====================
// CALIBRAÇÃO
// =====================
void calibrar() {

  long leituraComPeso = balanca.read_average(10);
  long leituraSemPeso;

  lcd.clear();
  lcd.print("Remova o peso");
  delay(4000);

  leituraSemPeso = balanca.read_average(10);

  long diferenca = leituraComPeso - leituraSemPeso;

  fatorCalibracao = diferenca / pesoReferencia;
  if (fatorCalibracao < 0) fatorCalibracao *= -1;

  balanca.set_scale(fatorCalibracao);
  calibrado = true;

  lcd.clear();
  lcd.print("Calibrado!");
  lcd.setCursor(0, 1);
  lcd.print("Fator:");
  lcd.print(fatorCalibracao, 1);

  delay(2500);
  lcd.clear();
}

// =====================
// SETUP
// =====================
void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);

  pinMode(LED_SUP, OUTPUT);
  pinMode(LED_INF, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  digitalWrite(LED_SUP, LOW);
  digitalWrite(LED_INF, LOW);
  digitalWrite(BUZZER, LOW); // buzzer ativo começa desligado

  // Inicia HX711
  balanca.begin(DT_PIN, SCK_PIN);
  balanca.set_scale();
  balanca.tare();

  // --- AJUSTE DOS LIMITES ---
  ajustarLimites();

  // --- CALIBRAR COM 1 kg ---
  lcd.clear();
  lcd.print("Coloque 1 kg");
  lcd.setCursor(0, 1);
  lcd.print("para calibrar...");
  delay(6000);

  calibrar();
}

// =====================
// LOOP PRINCIPAL
// =====================
void loop() {

  if (calibrado) {

    peso = balanca.get_units(10);

    // Exibição do peso
    lcd.setCursor(0, 0);
    lcd.print("Peso: ");
    lcd.print(peso, 3);
    lcd.print(" kg ");

    // =========================
    // ALERTAS COM LEDs + BUZZER (ATIVO)
    // =========================

    // ---- LIMITE SUPERIOR ----
    if (peso > limiteSuperior) {
      digitalWrite(LED_SUP, HIGH);
      digitalWrite(BUZZER, HIGH);   // buzzer ligado
    }
    else {
      digitalWrite(LED_SUP, LOW);
      digitalWrite(BUZZER, LOW);    // buzzer desligado
    }

    // ---- LIMITE INFERIOR ----
    if (peso < limiteInferior) {
      digitalWrite(LED_INF, HIGH);
      digitalWrite(BUZZER, HIGH);   // buzzer ligado
    }
    else {
      digitalWrite(LED_INF, LOW);

      // se dentro da faixa, buzzer desliga
      if (peso <= limiteSuperior) {
        digitalWrite(BUZZER, LOW);
      }
    }

    // Mensagem da linha 2

    lcd.setCursor(0, 1);
    if (peso > limiteSuperior) {
      lcd.print("ALERTA > LIM SUP ");
    }
    else if (peso < limiteInferior) {
      lcd.print("ALERTA < LIM INF ");
    }
    else {
      lcd.print("Dentro da faixa  ");
    }

    delay(400);
  }
}

#include "HX711.h"
#include <LiquidCrystal.h>

// =====================
// Pinos de ligação
// =====================
#define DT_PIN A1
#define SCK_PIN A0

#define LED_SUP 9
#define LED_INF 8

#define POT_PIN A2

#define BUZZER 10

#define BOTAO 7   // Botão de 3 estágios

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

float pesoReferencia = 1.0;

// ----- Botão -----
int estadoBotao;
int ultimoEstadoBotao = HIGH;
unsigned long debounce = 0;
int passo = 0;  // 1 = lim sup | 2 = lim inf | 3 = leitura


// =====================
// Ler potenciômetro (0–5 kg)
// =====================
float lerLimitePot() {
  int leitura = analogRead(POT_PIN);
  float kg = (leitura / 1023.0) * 5.0;
  return kg;
}


// =====================
// Calibração
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

  pinMode(BOTAO, INPUT_PULLUP);

  digitalWrite(LED_SUP, LOW);
  digitalWrite(LED_INF, LOW);
  digitalWrite(BUZZER, LOW);

  balanca.begin(DT_PIN, SCK_PIN);
  balanca.set_scale();
  balanca.tare();

  lcd.clear();
  lcd.print("Aguardando botoes");
  lcd.setCursor(0, 1);
  lcd.print("3 etapas...");
}


// =====================
// LOOP PRINCIPAL
// =====================
void loop() {

  // ------------------------------------------------------------------
  // LEITURA DO BOTÃO (FUNCIONA EM TODOS OS PASSOS)
  // ------------------------------------------------------------------

  estadoBotao = digitalRead(BOTAO);

  if (estadoBotao != ultimoEstadoBotao) {
    if (millis() - debounce > 200) {
      debounce = millis();

      if (estadoBotao == LOW) {  // botão pressionado

        passo++;

        if (passo == 1) {
          lcd.clear();
          lcd.print("Ajuste Lim Sup");
          delay(200);
        }

        else if (passo == 2) {
          lcd.clear();
          lcd.print("Ajuste Lim Inf");
          delay(200);
        }

        else if (passo == 3) {
          lcd.clear();
          lcd.print("Coloque 1 kg...");
          lcd.setCursor(0, 1);
          lcd.print("para calibrar");
          delay(6000);

          calibrar();

          lcd.clear();
          lcd.print("Iniciando...");
          delay(1000);
        }
      }
    }
  }

  ultimoEstadoBotao = estadoBotao;


  // ------------------------------------------------------------------
  // AJUSTE DO LIMITE SUPERIOR  (PASSO 1)
  // ------------------------------------------------------------------
  if (passo == 1) {

    limiteSuperior = lerLimitePot();

    lcd.setCursor(0, 1);
    lcd.print(limiteSuperior, 2);
    lcd.print(" kg    ");

    delay(100);
    return;
  }


  // ------------------------------------------------------------------
  // AJUSTE DO LIMITE INFERIOR  (PASSO 2)
  // ------------------------------------------------------------------
  if (passo == 2) {

    limiteInferior = lerLimitePot();

    lcd.setCursor(0, 1);
    lcd.print(limiteInferior, 2);
    lcd.print(" kg    ");

    delay(100);
    return;
  }


  // ------------------------------------------------------------------
  // SÓ COMEÇA A LER PESO APÓS O PASSO 3
  // ------------------------------------------------------------------
  if (passo < 3) return;


  // ========================
  // LEITURA DA BALANÇA
  // ========================
  if (calibrado) {

    peso = balanca.get_units(10);

    lcd.setCursor(0, 0);
    lcd.print("Peso: ");
    lcd.print(peso, 3);
    lcd.print(" kg  ");

    // ALERTAS
    if (peso > limiteSuperior) {
        digitalWrite(LED_SUP, HIGH);
        digitalWrite(BUZZER, HIGH);
    } else {
        digitalWrite(LED_SUP, LOW);
    }

    if (peso < limiteInferior) {
        digitalWrite(LED_INF, HIGH);
        digitalWrite(BUZZER, HIGH);
    } else {
        digitalWrite(LED_INF, LOW);

        if (peso <= limiteSuperior) {
            digitalWrite(BUZZER, LOW);
        }
    }

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

    delay(300);
  }
}

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
SoftwareSerial BT(6, 5);

const int pinLaser   = 4;
const int pinBuzzer  = 3;
const int pinBoton   = 2;
const int pinLDR1    = A0;
const int pinLDR2    = A1;
const int pinRojo    = 9;
const int pinVerde   = 10;
const int pinAzul    = 11;

int puntaje          = 0;
int tiempoRestante   = 30;
bool juegoActivo     = false;
unsigned long timerAnterior  = 0;
unsigned long antirebote     = 0;
unsigned long cooldownLDR1   = 0;  // ← nuevo
unsigned long cooldownLDR2   = 0;  // ← nuevo
const int COOLDOWN_MS        = 2000; // 2 segundos entre impactos
int umbralLDR        = 500;

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  BT.begin(9600);

  pinMode(pinLaser,  OUTPUT);
  pinMode(pinBuzzer, OUTPUT);
  pinMode(pinBoton,  INPUT_PULLUP);
  pinMode(pinRojo,   OUTPUT);
  pinMode(pinVerde,  OUTPUT);
  pinMode(pinAzul,   OUTPUT);

  digitalWrite(pinLaser, LOW);
  mostrarEspera();
}

void loop() {
  if (BT.available()) {
    char comando = BT.read();
    if (comando == 'I' && !juegoActivo) {
      iniciarJuego();
    }
  }

  if (juegoActivo) {
    if (millis() - timerAnterior >= 1000) {
      timerAnterior = millis();
      tiempoRestante--;
      actualizarLCD();
      BT.print("T:" + String(tiempoRestante) + "\n");

      if (tiempoRestante <= 0) {
        terminarJuego();
        return;
      }
    }

    // Botón disparo
    if (digitalRead(pinBoton) == LOW && millis() - antirebote > 300) {
      antirebote = millis();
      digitalWrite(pinLaser, HIGH);
      delay(200);
      digitalWrite(pinLaser, LOW);
    }

    // Blanco 1 con cooldown
    if (analogRead(pinLDR1) > umbralLDR && millis() - cooldownLDR1 > COOLDOWN_MS) {
      cooldownLDR1 = millis();
      impacto(10, 1);
    }

    // Blanco 2 con cooldown
    if (analogRead(pinLDR2) > umbralLDR && millis() - cooldownLDR2 > COOLDOWN_MS) {
      cooldownLDR2 = millis();
      impacto(20, 2);
    }
  }
}

void iniciarJuego() {
  puntaje        = 0;
  tiempoRestante = 30;
  juegoActivo    = true;
  timerAnterior  = millis();
  cooldownLDR1   = 0;
  cooldownLDR2   = 0;

  analogWrite(pinRojo,  0);
  analogWrite(pinVerde, 0);
  analogWrite(pinAzul,  255);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("  JUEGO INICIADO");
  lcd.setCursor(0, 1);
  lcd.print("  A DISPARAR!   ");
  tone(pinBuzzer, 1000, 500);
  delay(1000);
  actualizarLCD();
}

void impacto(int puntos, int blanco) {
  puntaje += puntos;

  analogWrite(pinRojo,  0);
  analogWrite(pinVerde, 255);
  analogWrite(pinAzul,  0);
  tone(pinBuzzer, 1500, 150);
  delay(300);
  analogWrite(pinVerde, 0);
  analogWrite(pinAzul,  255);

  actualizarLCD();
  BT.print("P:" + String(puntaje) + "\n");
  BT.print("B:" + String(blanco) + "\n");
}

void terminarJuego() {
  juegoActivo = false;
  digitalWrite(pinLaser, LOW);

  analogWrite(pinRojo,  255);
  analogWrite(pinVerde, 0);
  analogWrite(pinAzul,  0);

  tone(pinBuzzer, 800, 200); delay(250);
  tone(pinBuzzer, 600, 200); delay(250);
  tone(pinBuzzer, 400, 400); delay(500);
  noTone(pinBuzzer);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("JUEGO TERMINADO!");
  lcd.setCursor(0, 1);
  lcd.print("Puntos: " + String(puntaje));

  BT.print("F:" + String(puntaje) + "\n");
  delay(3000);
  mostrarEspera();
}

void actualizarLCD() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Pts: " + String(puntaje));
  lcd.setCursor(0, 1);
  lcd.print("Tiempo: " + String(tiempoRestante) + "s");
}

void mostrarEspera() {
  analogWrite(pinRojo,  0);
  analogWrite(pinVerde, 0);
  analogWrite(pinAzul,  0);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("  LASER  GAME  ");
  lcd.setCursor(0, 1);
  lcd.print(" Presiona INIT ");
}
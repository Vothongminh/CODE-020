// #VTM https: //www.you.tube.com/c/VTMVlogVoThongMinh
#include <Wire.h>
#include <LiquidCrystal_I2C.h>          //https://drive.google.com/file/d/1lpJyuODmZXfi7wwBM2B79BqBIBmBfanS/view?usp=sharing
#include "MAX30105.h"                   //https://github.com/oxullo/Arduino-MAX30100.git
LiquidCrystal_I2C lcd(0x27, 20, 4);
#define start 3
#define stopp 4
#define stepPin 5
#define dirPin 2
#define led 6
#define sw 7
#include "heartRate.h"
MAX30105 particleSensor;
const byte RATE_SIZE = 4;
byte rates[RATE_SIZE];
byte rateSpot = 0;
long lastBeat = 0;
float beatsPerMinute;
int beatAvg;
long aa, bb, a;
double b;
boolean demStart = 0;
boolean manual = 0;
long irValue;
unsigned long time1;
unsigned long time2;
//....................................................
void setup()
{
  Serial.begin(9600);
  Serial.println("Initializing...");
  lcd.init();
  lcd.backlight();
  pinMode(start, INPUT_PULLUP);
  pinMode(stopp, INPUT_PULLUP);
  pinMode(sw, INPUT_PULLUP);
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(led, OUTPUT);
  lcd.setCursor(0, 0);
  lcd.print("Speed   :");
  lcd.setCursor(15, 0);
  lcd.print(" ");
  lcd.setCursor(0, 1);
  lcd.print("Distance:");
  lcd.setCursor(0, 2);
  lcd.print("BPM     :");
  lcd.setCursor(0, 3);
  lcd.print("IR:");
  lcd.setCursor(6, 3);
  lcd.print("%");


  if (!particleSensor.begin(Wire, I2C_SPEED_FAST))
  {
    Serial.println("MAX30105 was not found. Please check wiring/power. ");
    while (1);
  }
  Serial.println("Place your index finger on the sensor with steady pressure.");
  particleSensor.setup();
  particleSensor.setPulseAmplitudeRed(0x0A);
  particleSensor.setPulseAmplitudeGreen(0);
}
//....................................................
void loop() {
  button();
  Var();
  if (demStart == 1) {
    Sensor();
    button();
    if (((irValue / 1300 > 90) && (irValue / 1300 < 100) && (beatsPerMinute > 1) && (beatsPerMinute < 100)) || (manual == 1)) {
      Serial.println("motor");
      digitalWrite(dirPin, HIGH);
      for (int x = 0; x < a; x++) {
        digitalWrite(stepPin, HIGH);
        delayMicroseconds(100 * b);
        digitalWrite(stepPin, LOW);
        delayMicroseconds(100 * b);
        button();
      }
      time1 = millis();
      while ((millis() - time1) < 1000) {
        Sensor();
        button();
        //Var();
      }
      digitalWrite(dirPin, LOW);
      for (int x = 0; x < a; x++) {
        digitalWrite(stepPin, LOW);
        delayMicroseconds(100 * b);
        digitalWrite(stepPin, HIGH);
        delayMicroseconds(100 * b);
        button();
      }
      time1 = millis();
      while ((millis() - time1) < 1000) {
        Sensor();
        button();
        Var();
      }
    }
  }
}
//....................................................
void Var() {
  aa =  analogRead(A0);
  bb = analogRead(A1);
  a = map(aa, 0, 1023, 0, 800);//số xung cho động cơ quay (từ 0 đến 1600)
  b = map(bb, 0, 1023, 5, 30);//tốc độ động cơ từ 0.1 - 1.0 vòng/giây
  if (a < 1000) {
    lcd.setCursor(13, 1);
    lcd.print(" ");
  }
  if (a < 100) {
    lcd.setCursor(12, 1);
    lcd.print(" ");
  }
  if (a < 10) {
    lcd.setCursor(11, 1);
    lcd.print(" ");
  }
  if ( demStart == 1) {
    lcd.setCursor(15, 3);
    lcd.print("START");
  }
  else {
    lcd.setCursor(15, 3);
    lcd.print("STOP ");
  }

  lcd.setCursor(10, 0);
  lcd.print(b / 10);
  lcd.setCursor(10, 1);
  lcd.print(a);
  if (digitalRead(sw) == 0) {
    manual = 1;
    lcd.setCursor(8, 3);
    lcd.print("Manual");
  }
  if  (digitalRead(sw) == 1) {
    manual = 0;
    lcd.setCursor(8, 3);
    lcd.print("      ");
  }
}
//....................................................
void Sensor() {
  irValue = particleSensor.getIR();
  if (checkForBeat(irValue) == true)
  {
    long delta = millis() - lastBeat;
    lastBeat = millis();
    beatsPerMinute = 60 / (delta / 1000.0);
    if (beatsPerMinute < 255 && beatsPerMinute > 20)
    {
      rates[rateSpot++] = (byte)beatsPerMinute;
      rateSpot %= RATE_SIZE;
      beatAvg = 0;
      for (byte x = 0; x < RATE_SIZE; x++)
        beatAvg += rates[x];
      beatAvg /= RATE_SIZE;
    }
  }
  Serial.print("IR=");
  Serial.print(irValue);
  Serial.print(", BPM=");
  Serial.print(beatsPerMinute);
  Serial.print(", Avg BPM=");
  Serial.print(beatAvg);
  lcd.setCursor(4, 3);
  lcd.print(irValue / 1300);  //in ra chỉ số IR ở dạng %
  if (irValue / 1300 < 10) {
    lcd.setCursor(5, 3);
    lcd.print(" ");
  }

  if (irValue < 50000) {
    Serial.print(" No finger?");
    lcd.setCursor(10, 2);
    lcd.print("No finger");
  }
  else {
    if (beatsPerMinute != 0) {
      lcd.setCursor(10, 2);
      lcd.print(beatsPerMinute);
      if (beatsPerMinute < 100) {
        lcd.setCursor(16, 2);
        lcd.print("   ");
      }
      if (beatsPerMinute < 100) {
        lcd.setCursor(15, 2);
        lcd.print("    ");
      }
      if (beatsPerMinute < 10) {
        lcd.setCursor(14, 2);
        lcd.print("     ");
      }
    }
    if (beatsPerMinute == 0) {
      lcd.setCursor(10, 2);
      lcd.print("Loading..");
    }
  }
  Serial.println();
}
//....................................................
void button() {
  if (digitalRead(start) == 0) {
    demStart = 1;
    digitalWrite(led, HIGH);
    lcd.setCursor(15, 3);
    lcd.print("START");
  }
  if (digitalRead(stopp) == 0) {
    demStart = 0;
    digitalWrite(led, LOW);
    lcd.setCursor(15, 3);
    lcd.print("STOP ");
    lcd.setCursor(10, 2);
    lcd.print("No finger");
    lcd.setCursor(4, 3);
    lcd.print("  ");
  }
  if (digitalRead(sw) == 0) {
    manual = 1;
    //    lcd.setCursor(8, 3);
    //    lcd.print("Manual");
  }
  if  (digitalRead(sw) == 1) {
    manual = 0;
    //    lcd.setCursor(8, 3);
    //    lcd.print("      ");
  }
}

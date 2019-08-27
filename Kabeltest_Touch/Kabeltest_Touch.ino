/*
   Kabeltester für bis zu 25 Leitungen
   @author: Joel Klein
   last change: 26.08.2019
*/

#define VERSION 1.20

// Librarys hinzufügen
#include <Adafruit_GFX.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_ILI9341.h>

//Capacitive Display
//#include <Adafruit_FT6206.h>

//Resistive Display Controller
#include "Adafruit_STMPE610.h"

// Display Konfiguration
//Adafruit_FT6206 ts = Adafruit_FT6206();
Adafruit_STMPE610 ts = Adafruit_STMPE610();

#define TFT_CS A0
#define TFT_DC A1
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

#define TS_MINX 150
#define TS_MINY 130
#define TS_MAXX 3800
#define TS_MAXY 4000

// Prüfen ob Kabeltest aufgerufen wird
boolean TestStart = false;

// Farben für Text
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define LIME    0x07FF


// Aufzählung Fehlertypen
enum ergebnis {FEHLERFREI, KURZSCHLUSS, KEINEVERBINDUNG, FALSCHEVERBINDUNG };

// Pins, die mit den Kabelenden am Anfang und am Ende des Kabels verbunden sind
//                         0  1  2  3   4   5   6   7   8   9   10  11  12  13  14  15  16  17  18  19  20  21  22  23  24  25   //  26   27 RESERVE
byte pinsKabelAnfang[] = { 3, 5, 7, 9,  11, 15, 17, 19, 23, 25, 27, 29, 31, 33, 35, 37, 39, 41, 43, 45, 47, 49, A3, A5, A7, A9}; //, A11, A13 };
byte pinsKabelEnde[]   = { 4, 6, 8, 10, 12, 14, 16, 18, 22, 24, 26, 28, 30, 32, 34, 36, 38, 40, 42, 44, 46, 48, A2, A4, A6, A8}; //, A10, A12 };
byte safeFEHLERFREI[]  = { 0, 0, 0, 0,   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0};

void setOutputPin(byte pinNumber)
{ // Alle Pins auf INPUT_PULLUP setzen
  for (int i = 0; i < sizeof(pinsKabelAnfang); i++)
  {
    pinMode(pinsKabelAnfang[i], INPUT_PULLUP);
    pinMode(pinsKabelEnde[i], INPUT_PULLUP);
  }
  // Einen Pin auf Output setzen
  pinMode(pinNumber, OUTPUT);
}

// Testroutine
void Kabeltest()
{
  ergebnis result;
  byte errorOtherPin;
  tft.setCursor(0, 10); // Abstand zum oberen Displayrand

  for (int i = 0; i < sizeof(pinsKabelAnfang); i++) // Alle Leitungen testen
  {
    result = FEHLERFREI; // Fehlerfreiheit annehmen
    // Funktionsaufruf, um zu testenden Pin als einzigen auf OUTPUT zu setzen
    setOutputPin(pinsKabelAnfang[i]);
    digitalWrite(pinsKabelAnfang[i], LOW); // den zu testenden Pin LOW setzen
    // Testen, ob der Gegenpol verbunden ist
    if (digitalRead(pinsKabelEnde[i]) != LOW)
    {
      result = KEINEVERBINDUNG;
      errorOtherPin = i;
    }
    // In Schleife auf falsche Verbindungen testen
    for (int j = 0; j < sizeof(pinsKabelEnde); j++)
    {
      if (j != i && digitalRead(pinsKabelEnde[j]) == LOW)
      {
        result = FALSCHEVERBINDUNG;
        errorOtherPin = j;
      }
    }
    // In Schleife auf Kurzschluss testen
    for (int j = 1; j < sizeof(pinsKabelAnfang); j++)
    {
      if (j != i && digitalRead(pinsKabelAnfang[j]) == LOW)
      {
        result = KURZSCHLUSS;
        errorOtherPin = j;
      }
    }

    // Ausgabe auf dem Display
    tft.setTextSize(1);
    tft.setTextColor(ILI9341_WHITE);
    tft.print(" Leitung ");

    // führende 0 für einstellige Zahlen
    if (i <= 8) {
      tft.print("0");
    }

    tft.print(i + 1);
    switch (result)
    {
      case FEHLERFREI:
        tft.setTextColor(GREEN);
        tft.println(" OK");
        safeFEHLERFREI[i] = 1;
        break;

      case KURZSCHLUSS:
        tft.setTextColor(RED);
        tft.print(" KURZSCHLUSS Leitung ");
        if (errorOtherPin <= 8) {
          tft.print("0");
        }
        tft.println(errorOtherPin + 1);
        break;

      case KEINEVERBINDUNG:
        tft.setTextColor(LIME);
        tft.print(" KEINE VERBINDUNG Leitung ");
        if (errorOtherPin <= 8) {
          tft.print("0");
        }
        tft.println(errorOtherPin + 1);
        break;

      case FALSCHEVERBINDUNG:
        tft.setTextColor(RED);
        tft.print(" FALSCHE VERBINDUNG Leitung ");
        if (errorOtherPin <= 8) {
          tft.print("0");
        }
        tft.println(errorOtherPin + 1);
        break;;
    }
  }
  KabelsatzTest(); // Funktionsaufruf für Ausgabe Kabelsatz OK
}

// Text auf dem Starbildschirm
void startText()
{
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(80, 30);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(3);
  tft.println("KABELTEST");
  tft.setCursor(50 , 120);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.println("ZUM STARTEN TIPPEN");

  // Ausgabe Versionsnummer
  tft.setCursor(235 , 225);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(1);
  tft.print("Version ");
  tft.println(VERSION);

  TestStart = false;
}

//Text auf dem Kabeltestbildschirm
void resetText()
{
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(230 , 225);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(1);
  tft.println("Reset = tippen");
  TestStart = true;

}

void KabelsatzTest()
{
  tft.setCursor(0 , 225);
  tft.setTextColor(GREEN);
  tft.setTextSize(1);
  // Teste ob Kabelsatz für Servomotor richtig ist
  if (
    safeFEHLERFREI[6] == 1 &&
    safeFEHLERFREI[7] == 1 &&
    safeFEHLERFREI[8] == 1 &&
    safeFEHLERFREI[9] == 1 &&
    safeFEHLERFREI[10] == 1 &&
    safeFEHLERFREI[11] == 1 &&
    safeFEHLERFREI[25] == 1 ) {
    tft.println(" Resolverleitung OK");
  }
  tft.setCursor(125 , 225);
  if (
    safeFEHLERFREI[12] == 1 &&
    safeFEHLERFREI[13] == 1 &&
    safeFEHLERFREI[14] == 1) {
    tft.println("Motorleitung OK");
  }

  // Reset Array
  for (byte i = 0; i < sizeof(safeFEHLERFREI); i++) {
    safeFEHLERFREI[i] = 0;
  }
}


void setup(void)
{
  // Serielle ausgebe für den PC
  Serial.begin(9600);
  tft.begin();
  if (!ts.begin()) {
    Serial.println("Unable to start touchscreen.");
  }
  else {
    Serial.println("Touchscreen started.");
  }

  tft.fillScreen(ILI9341_BLACK); // Bildschirm schwarz machen
  tft.setRotation(3); // legt rotation des Bildschirms fest
  startText(); // Aufruf des Startseitentext

  // Fehler bei Fehlkonfiguration
  if (sizeof(pinsKabelAnfang) != sizeof(pinsKabelEnde))
  {
    tft.setCursor(50, 80);
    tft.setTextColor(RED);
    tft.setTextSize(2);
    tft.println("Fehlkonfiguration!");

    // Überschreibe ZUM STARTEN TIPPEN
    tft.setCursor(50 , 120);
    tft.setTextColor(ILI9341_BLACK);
    tft.setTextSize(2);
    tft.println("ZUM STARTEN TIPPEN");

    // Ausgabe im Seriellen Monitor
    Serial.println("Konfiguration fehlerhaft!");
    Serial.println("Deklaration korrigieren: pinsKabelAnfang[] und pinsKabelEnde[]");
    Serial.println("Anzahl der Leitungen muss gleich sein!");
    while (true == true); // Stopp mit Endlosschleife
  }
}

void loop()
{
  uint16_t x, y;
  uint8_t z;

  // Koordinaten empfangen
  // read x & y & z;
  if (! ts.bufferEmpty())
  {
    TS_Point p = ts.getPoint();

    p.x = map(p.x, TS_MINX, TS_MAXX, 0, tft.width());
    p.y = map(p.y, TS_MINY, TS_MAXY, 0, tft.height());
  }
  // Testen auf Toucheingabe
  if (ts.touched())
  {
    //Serielle Ausgabe für Touch Punkte
    while (! ts.bufferEmpty()) {
      Serial.print(ts.bufferSize());
      ts.readData(&x, &y, &z);
      Serial.print("->(");
      Serial.print(x); Serial.print(", ");
      Serial.print(y); Serial.print(", ");
      Serial.print(z);
      Serial.println(")");
    }

    if (TestStart)
    {
      Serial.println("Reset Button");
      startText(); // zurück zum Starbildschirm
    }
    else //Record is off (TestStart == false)
    {
      Serial.println("Start Button");
      resetText(); // Informationstext ausgeben
      Kabeltest(); // Kabeltestroutine aufrufen
    }

    Serial.println(TestStart);
  }
}

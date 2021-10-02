
#include "Arduino.h"
#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>
#include "PumpenZeit.h"


#define STR_BLANK " "
#define STR_HASH  ">"

#define  I2C_ADDR 0x27
//Welches Display ist angeschlossen
// !! bitte nicht lcd.begin() vergessen !!
#define FORMAT_20x4
//#define FORMAT_16x2

#define LABEL_STATUS "Betrieb:"
#define LABEL_MODE "Modus: "
#define LABEL_MENGE "Menge: "
#define LABEL_LITER "Liter"

// die Anzeige-Tafel
LiquidCrystal_PCF8574 lcd(I2C_ADDR);

// Die verschiedenen Anzeigetexte
const char* strMode[2]    = {"Filtrieren", "Umpumpen  "};
const char* strStatus[3]  = {"AUS  ", "EIN  ", "PAUSE"};

// welche(r) Mode/Status/Füllmenge ist aktuell
int  iSelMode       = 1;
int  iStatus        = 1; // der wirkliche Status
int  iSelStatus     = 2; // der selektierte Status
int  iSelFuellmenge = 1;

// ist ein Mode Selektiert ?
bool bSelMode       = false;
bool bSelFuellmenge = false;
bool bSelStatus     = false;

// Die Pin-Belegung der Knöpfe
#ifdef DEBUG
#define MIN_FUELLMENGE 20UL
#endif
#ifndef DEBUG
#define MIN_FUELLMENGE 200UL
#endif

unsigned long fuellmenge = 0UL;

const char* strKOLB = "HZT,.-:";
// Die Zeitverwaltung
PumpenZeit pz;

//========================================
// *********** Init das Display **********
//========================================
void init_Monitor() {
  lcd.begin(20, 4);
  lcd.setBacklight(true);
  fuellmenge = MIN_FUELLMENGE;
  
  // die Start-Selektion
  iSelMode = 1;
  iSelStatus = 0;
  iSelFuellmenge = 0;  
  
  // init die Auswahl
  bSelMode = true;
  bSelStatus = false;
  bSelFuellmenge = false;  
}
//========================================
// *********** showZeile 1 20x4 **********
//========================================
void showZeile_1_20x4 () {

  // Die Selektion
  lcd.setCursor(0, 0);
  if (bSelMode) {
    lcd.print(STR_HASH);
  } else lcd.print(STR_BLANK);
  
  // die Beschreibung
  lcd.setCursor(1, 0);
  lcd.print(LABEL_MODE);

  // total bekloppt, aber geht
  if (iSelMode < 1) iSelMode = 1;
  
  // Der Modus
  lcd.setCursor(10, 0);
  lcd.print(strMode[iSelMode-1]);
}

//========================================
// ***********  showZeile 2 20x4 *********
//========================================
void showZeile_2_20x4 () {

  // Die Selektion
  lcd.setCursor(0, 1);
  if (bSelFuellmenge) {
    lcd.print(STR_HASH);
  } else lcd.print(STR_BLANK);

  // die Beschreibung
  lcd.setCursor(1, 1);
  lcd.print(LABEL_MENGE);

  lcd.setCursor(15, 1);
  lcd.print("Liter");

  // setze Wert
  if (fuellmenge > 99UL) {
    lcd.setCursor(10, 1);
    lcd.print(fuellmenge);
  } else if (fuellmenge > 9UL) {
    lcd.setCursor(10, 1);
    lcd.print(" ");
    lcd.setCursor(11, 1);
    lcd.print(fuellmenge);
  } else { // < 10
    lcd.setCursor(10, 1);
    lcd.print("  ");
    lcd.setCursor(12, 1);
    lcd.print(fuellmenge);
  }
}

//========================================
// *********** showZeile 3 20x4 **********
//========================================
void showZeile_3_20x4 () {
  
  lcd.setCursor(0, 2);
  if (bSelStatus) {
    lcd.print(STR_HASH);
  } else lcd.print(STR_BLANK);
  
  // total bekloppt, aber geht
  if (iSelStatus < 1) iSelStatus = 1;
  
  // die Beschreibung
  lcd.setCursor(1, 2);
  lcd.print(LABEL_STATUS);

  // der Status
  lcd.setCursor(10, 2);
  lcd.print(strStatus[iSelStatus-1]);
}

int pos = 0;
//========================================
// *********** showZeile 4 20x4 **********
//========================================
void showZeile_4_20x4 () {

  lcd.setCursor(0, 3);
  lcd.print(strStatus[iStatus-1]);

  lcd.setCursor(15, 3);
  float zz = pz.GetRestHour();
  if (zz < 0.0) {
    Serial.print("Fehler: RestZeit ist abgelaufen! zz = ");
    Serial.println(zz,1);
    zz = 0.0;
  }
  lcd.print(zz,1);

  // ein kleines Gimmick
  lcd.setCursor(10, 3);
  if (pos == 1) lcd.print(strKOLB[0]);
  if (pos == 2) lcd.print(strKOLB[1]);
  if (pos == 3) lcd.print(strKOLB[2]);
  if (pos == 4) lcd.print(strKOLB[3]);
  if (pos == 5) lcd.print(strKOLB[4]);
  if (pos == 6) lcd.print(strKOLB[5]);
  if (pos == 7) { lcd.print(strKOLB[6]); pos = 0; }
  pos++;
  
  // setze Einheit für die Stunden
  lcd.setCursor(19, 3);
  lcd.print("h");
}

//========================================
// *********** showZeile 1 ***************
//========================================
void showZeile_1_16x2 () {
  
  lcd.setCursor(0, 0);
  
  if (bSelMode) {
    lcd.print(STR_HASH);
  } else lcd.print(STR_BLANK);

  lcd.setCursor(1, 0);
  lcd.print(strMode[iSelMode-1]);

  lcd.setCursor(9, 0);
  lcd.print(strStatus[iStatus-1]);
  
  lcd.setCursor(10, 0);
  
  if (bSelStatus) {
    lcd.print(STR_HASH);
  } else lcd.print(STR_BLANK);

  lcd.setCursor(11, 0);
  lcd.print(strStatus[iSelStatus-1]);
}

//========================================
// ***********  showZeile 2 **************
//========================================
void showZeile_2_16x2 () {
  
  lcd.setCursor(0, 1);
  
  if (bSelFuellmenge) {
    lcd.print(STR_HASH);
  }
  else lcd.print(STR_BLANK);

  lcd.setCursor(5, 1);
  lcd.print("l");

  lcd.setCursor(7, 1);
  //lcd.print("        ");
  lcd.setCursor(11, 1);
  float zz = pz.GetRestHour();
  if (zz < 0.0) {
    Serial.print("Fehler: RestZeit ist abgelaufen! zz = ");
    Serial.println(zz,1);
    zz = 0.0;
  }
  lcd.print(zz,1);
  
  // setze Einheit für die Stunden
  lcd.setCursor(15, 1);
  lcd.print("h");
  
}

//========================================
// *********** Zeige nur Zeile 1 *********
//========================================
void showZeile1 () {
#ifdef FORMAT_16x2
  showZeile_1_16x2();
#endif
#ifdef FORMAT_20x4  
  showZeile_1_20x4();
#endif
}

//========================================
// *********** Zeige nur Zeile 2 *********
//========================================
void showZeile2 () {
#ifdef FORMAT_16x2
  showZeile_2_16x2();
#endif
#ifdef FORMAT_20x4  
  showZeile_2_20x4();
#endif
}

//========================================
// *********** Zeige nur Zeile 2 *********
//========================================
void showZeileAll () {
#ifdef FORMAT_16x2
  showZeile_1_16x2();
  showZeile_2_16x2();
#endif
#ifdef FORMAT_20x4  
  showZeile_1_20x4();
  showZeile_2_20x4();
  showZeile_3_20x4();
  showZeile_4_20x4();
#endif
}

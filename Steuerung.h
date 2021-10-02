
#include "Arduino.h"
#include <SimpleRotary.h>
#include "Monitor.h"

/**
  Hardware:
  - Arduino Nano
  - .lcd. mit I2C mit PCF8574
  - Drehgebervon IDUINO
*/

// Die Pin-Belegung des Encoders
#define  PIN_SW  2
#define  PIN_DT  5
#define  PIN_CLK 6

// der Drehknopf
SimpleRotary  enc(PIN_DT, PIN_CLK, PIN_SW);

// Pin-Belegung des DruckSchalters
#define PIN_DRUCKSCHALTER  7

// Pin-Belegung der DruckWarnLampe
#define PIN_WARNLAMPE  9

// Das "Relais" von der Pumpe
#define PIN_PUMPE  8

void EinstellMode();
void EinstellStatus(bool bpause);
void EinstellFuellmenge();
bool AntiPrell (int pin);

bool iDruckSchalter = false;

//========================================
// *********** Konstruktor ***************
//========================================
void init_bedingung ()
{
  Serial.println("init_bedingung ... ");

  enc.setTrigger(HIGH);
  enc.setDebounceDelay (15);
  enc.setErrorDelay(50);

  pinMode(PIN_PUMPE, OUTPUT);
  digitalWrite(PIN_PUMPE, LOW);

  pinMode(PIN_DRUCKSCHALTER, INPUT_PULLUP);

  pinMode(PIN_WARNLAMPE, OUTPUT);
  digitalWrite(PIN_WARNLAMPE, LOW);

  iDruckSchalter = false;

  init_Monitor();
  
}

//========================================
// *********  EinstellenArt **************f
//========================================
void EinstellenArt() {

  iSelStatus = 2; // EIN
  iStatus = 1; // AUS

  // erstma alles initialisieren
  showZeileAll();

  // wir fangen mit dem Modus an
  int stellung = 1;
  
  do {
    // ermittle die Knopfdrehung
    int rot = enc.rotate();
    
    // der knopf wurde nicht gedreht
    if (rot == 0) continue;

    // welche Stellung ist gewünscht
    if (rot == 2) stellung++;
    if (rot == 1) stellung--;

    // wir lassen die Selektion rotieren
    if (stellung > 3) stellung = 1;
    if (stellung < 1) stellung = 3;
    
    if (stellung == 1) { // der Mode wird gewählt
        bSelMode = true;
        bSelFuellmenge = false;
        bSelStatus = false;
    } else if (stellung == 2) { // die Fuellmenge wird gewählt
        bSelMode = false;
        bSelFuellmenge = true;
        bSelStatus = false;
    } else if (stellung == 3) { // der Status wird gewählt
        bSelMode = false;
        bSelFuellmenge = false;
        bSelStatus = true;
    }
    showZeileAll();
  } while (!enc.push());

  // Der Knopf wurde gedrückt
  // es wird in die Sub-Menues verzweigt
  if (bSelMode) EinstellMode();
  if (bSelStatus) EinstellStatus(false);
  if (bSelFuellmenge) EinstellFuellmenge();
}

//==========================================
// ** EinstellMode (Umpumpen/Filtern)  *****f
//==========================================
void EinstellMode () {
  int s = 1;
  // solange der Knopf nicht gedrückt wird,
  // kann an dem Knopf gedreht werden
  while (!enc.push()) {
    int rot = enc.rotate();
    
    // welche Option ist gewünscht
    if (rot == 2) s++;
    if (rot == 1) s--;

    // wir lassen die Selektion rotieren
    if (s > 2) s = 1;
    if (s < 1) s = 2;

    iSelMode = s;
    showZeileAll();
  }
  if (iSelMode == 2) pz.SetUmpumpen();
  if (iSelMode == 1) pz.SetFiltieren();
  // zurück zur Hauptauswahl
  EinstellenArt();
}

//========================================
// ******** EinstellStatusLaufend ********
//========================================
void EinstellStatusLaufend () {
  
  // Pumpe läuft normal --> Zeige Option "PAUSE"
  iSelStatus = 3; // PAUSE
  iStatus = 2; // EIN
  showZeileAll();

  // eine reine Poll-Routine für die Pumpzeit
  // wenn der Knopf gedrückt wird, dann wird eine Pause verlangt
  while (!enc.push()) {

    // die Pumpe soll nicht laufen
    if (!pz.SollLaufen()) {
      iSelStatus = 2; // EIN
      iStatus = 1; // AUS
      showZeileAll();
      digitalWrite(PIN_PUMPE, LOW);
      // zurück zur Menü-Auswahl
      EinstellenArt();
    }
    showZeileAll();
    // Der Druckschalter hat sich gerührt
    if (AntiPrell(PIN_DRUCKSCHALTER)) {
      digitalWrite(PIN_WARNLAMPE, HIGH);
      iDruckSchalter = true;
      break;
    }
  }

  // Setze auf "PAUSE"
  iSelStatus = 2; // EIN
  iStatus = 3; // PAUSE
  showZeileAll();
  
  pz.SetPauseBeginn();
  
  digitalWrite(PIN_PUMPE, LOW);
  
  EinstellStatus (true);
}

//========================================
// ******** EinstellStatus ***************
//========================================
void EinstellStatus (bool bpause) {

  // es soll eine Pause gemacht werden
  if (bpause) {
    iSelStatus = 2; // EIN
    iStatus = 3; // PAUSE
    showZeileAll();
    
    // warten bis die Pause zuende ist
    while (!enc.push()) { showZeileAll(); delay (30); }
    
    pz.SetPauseEnde();
    iDruckSchalter = false;
    iSelStatus = 3; // PAUSE
    iStatus = 2; // EIN
    iDruckSchalter = false;
    digitalWrite(PIN_WARNLAMPE, LOW);
  } else {
    iSelStatus = 3; // PAUSE
    iStatus = 2; // EIN
    pz.SetPumpZeit(fuellmenge); 
  }
  showZeileAll();
  
  // mach die Pumpe EIN
  digitalWrite(PIN_PUMPE, HIGH);

  // warte auf die PAUSE / das AUS
  EinstellStatusLaufend();
}

//========================================
// ****** EinstellFuellmenge *************f
//========================================
void EinstellFuellmenge () {
  // solange der Knopf nicht gedrückt wird,
  // kann an dem Knopf gedreht werden
  while (!enc.push()) {
    int rot = enc.rotate();
    if (rot == 0) continue;
    if (rot == 1 && fuellmenge > MIN_FUELLMENGE) fuellmenge -= 20UL;
    if (rot == 2 && fuellmenge < 650UL) fuellmenge += 20UL;
    showZeileAll();
  }
  EinstellenArt();
}

//========================================
// ***********  AntiPrell ****************
// entprellt das Signal
//========================================
bool AntiPrell (int pin) {
  bool isHIGH = false;
  /**
    Serial.print("gedrückt  ");
    Serial.print(pin);
    Serial.print("  ");
    Serial.println(digitalRead(pin));
  **/
  if (!digitalRead(pin)) isHIGH = true;
  else isHIGH = false;
  delay (15);
  if (!digitalRead(pin) && isHIGH) isHIGH = true;
  else isHIGH = false;
  delay(30);
  if (!digitalRead(pin) && isHIGH) isHIGH = true;
  else isHIGH = false;
  if (isHIGH) Serial.println("Knopf gedrückt");
  return isHIGH;
}

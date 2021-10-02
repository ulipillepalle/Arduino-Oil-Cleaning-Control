
#include "Arduino.h"
#include <stdlib.h>

//#define DEBUG

// gibt die Förderleistung der Pumpe an in Liter/Stunde
#define FOERDERLEISTUNG 20UL

//========================================
//========================================
// *******  Beginn class PumpenZeit ******
//========================================
//========================================
class PumpenZeit {

public:


//========================================
// Konstruktor
//========================================
  PumpenZeit() {
    FaktorPumpZeit  = 1UL;
    MilliLaufzeit   = 0UL;
    PauseZeitBeginn = 0UL;
  }

  
//========================================
// setze auf Umpumpen
//========================================
  inline void SetUmpumpen() {
    FaktorPumpZeit = 1UL;
  }

//========================================
// setze auf Filtrieren
//========================================
  inline void SetFiltieren() {
    FaktorPumpZeit = 4UL;
  }

  
//========================================
// Ermittle den Status der Filtration
// 1 -> Umpumpen
// 4 -> Filtrieren
//========================================
  inline int GetStatus () {
    return (int) FaktorPumpZeit;
  }
//========================================
// Berechne die PumpZeit
//========================================
  inline void SetPumpZeit (unsigned long Literzahl) {
    
    // die Laufzeit des Jobs in Sekunden
    MilliLaufzeit = (unsigned long) (Literzahl / FOERDERLEISTUNG) * 3600UL * FaktorPumpZeit * 1000UL;
    StartZeit = millis();
    Serial.print(StartZeit);
    Serial.print(" , ");
    Serial.print(MilliLaufzeit);
    Serial.println(" Pumpzeit gesetzt!");
  }

  
//========================================
// prüfe, ob Pumpe laufen soll
//========================================
  inline bool SollLaufen () {
    
    // es soll gerade pausiert werden
    if (PauseZeitBeginn > 0L) return false;

    // die Pumpe hat fertig
    if ((millis() - StartZeit) > MilliLaufzeit) {
      MilliLaufzeit = 0UL;
      StartZeit     = 0UL;
      return false;
    }
    return true;
  }

//========================================
// merke den Beginn der Pausen-Zeit
//========================================
inline bool IsPause() {
  // Wenn die Pumpe nicht läuft, kann es auch keine Pause geben
  if (MilliLaufzeit == 0UL) return false;
  // Es wurde eine Pause eingestellt
  if (PauseZeitBeginn > 0L) return true;
  return false;
}
//========================================
// merke den Beginn der Pausen-Zeit
//========================================
  inline void SetPauseBeginn () {

    if (!SollLaufen()) {
      PauseZeitBeginn = 0UL;
      return;
    }
    PauseZeitBeginn = millis();
  }

//========================================
// Es soll abgebrochen werden
//========================================
  inline void SetAbbruch () {

    // Es gibt keine Startzeit mehr
    StartZeit = 0UL;
    
    // keine Laufzeit mehr
    MilliLaufzeit = 0UL;
    
    // keine Pause
    PauseZeitBeginn = 0UL;
  }
  
//========================================
// setze Ende der Pausenzeit und ermittle neue Lauf-Zeit
//========================================
  inline void SetPauseEnde () {
    
    // hänge die pausierte Zeit noch dran
    MilliLaufzeit = MilliLaufzeit + (millis() - PauseZeitBeginn);
    
    // setze die Pause zurück
    PauseZeitBeginn = 0UL;
  }

//========================================
// ermittle die RestLaufzeit des Programms
//========================================
  float GetRestHour() {
    if (StartZeit == 0UL) return -1.0;
    if (MilliLaufzeit == 0UL) return -2.0;
    unsigned long Endzeit = StartZeit + MilliLaufzeit;
    if (Endzeit < millis()) return -3.0;
    float Rest = ((float) (Endzeit - millis())) / 3600000.0;
    return (float) Rest;
  }

//========================================
// zeige alle Paramter
//========================================
  void show() {
    Serial.print("FaktorPumpZeit: ");
    Serial.println(FaktorPumpZeit);
    Serial.print("MilliLaufzeit: ");
    Serial.println(MilliLaufzeit);
    Serial.print("StartZeit: ");
    Serial.println(StartZeit);
    Serial.print("PauseZeitBeginn: ");
    Serial.println(PauseZeitBeginn);
  }
  
private:

  // Variablen
  unsigned long FaktorPumpZeit;
  unsigned long MilliLaufzeit;
  unsigned long StartZeit;
  unsigned long PauseZeitBeginn;
};
//========================================
//========================================
// *******  Ende class PumpenZeit ********
//========================================
//========================================

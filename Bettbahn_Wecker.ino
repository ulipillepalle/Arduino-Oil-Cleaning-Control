/**
 * TODOS:
 * - Zahlendarstellung bei Wechsel von 10h auf 9h läßt eine 0 zuviel hinter dem Komma erscheinen
 * -
 */


#include "Steuerung.h"

//========================================
// ***********  SETUP ********************
//========================================
void setup() {
  Serial.begin(9600);
  Serial.println("Aber jetzt...");
  init_bedingung();
} // ende setup


//========================================
// ***********  LOOP *********************
//========================================
void loop() {
  EinstellenArt ();
  Serial.println("Jetzt  aber ...");
} // ende loop

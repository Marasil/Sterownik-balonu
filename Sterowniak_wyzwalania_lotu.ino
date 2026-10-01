//#include <DFRobotDFPlayerMini.h>

// A i B zamienione miejscami
const byte PRZYCISK_A = 17;
const byte PRZYCISK_B = 2;
const byte PRZYCISK_BAZOWANIE = 16;

const byte LED_RA = 12;
const byte LED_GA = 11;

const byte LED_RB = 3;
const byte LED_GB = 4;

// STYCZNIK
const byte BAZOWANIE = 10;
const byte START = 13;

bool POWROT = false;

int CZAS_LOTU_1 = 1000;
int CZAS_LOTU_2 = 1000;

void zwarcie_chwilowe(byte styk){
  digitalWrite(styk, LOW);
  delay(500);
  digitalWrite(styk, HIGH);
  delay(50);
}

void setup(){

  pinMode(PRZYCISK_A, INPUT_PULLUP);
  pinMode(PRZYCISK_B, INPUT_PULLUP);
  pinMode(PRZYCISK_BAZOWANIE, INPUT_PULLUP);

  pinMode(LED_RA, OUTPUT);
  pinMode(LED_GA, OUTPUT);
  pinMode(LED_RB, OUTPUT);
  pinMode(LED_GB, OUTPUT);

  pinMode(BAZOWANIE, OUTPUT);
  pinMode(START, OUTPUT);

  digitalWrite(BAZOWANIE, HIGH);
  digitalWrite(START, HIGH);

  // Punkt początkowy A
  digitalWrite(LED_RA, LOW);
  digitalWrite(LED_GA, HIGH);

  digitalWrite(LED_RB, HIGH);
  digitalWrite(LED_GB, LOW);
}

void loop(){

  // KIERUNEK A
  if(POWROT == false){

    digitalWrite(LED_RA, LOW);
    digitalWrite(LED_GA, HIGH);

    digitalWrite(LED_RB, HIGH);
    digitalWrite(LED_GB, LOW);

    if(digitalRead(PRZYCISK_A) == LOW){
      delay(50);

      if(digitalRead(PRZYCISK_A) == LOW){

        digitalWrite(LED_GA, LOW);
        digitalWrite(LED_RA, HIGH);

        zwarcie_chwilowe(START);

        delay(CZAS_LOTU_1);

        while(digitalRead(PRZYCISK_A) == LOW){
          delay(10);
        }

        POWROT = true;
      }
    }
  }

  // KIERUNEK B
  else{

    digitalWrite(LED_RA, HIGH);
    digitalWrite(LED_GA, LOW);

    digitalWrite(LED_RB, LOW);
    digitalWrite(LED_GB, HIGH);

    if(digitalRead(PRZYCISK_B) == LOW){
      delay(50);

      if(digitalRead(PRZYCISK_B) == LOW){

        digitalWrite(LED_GB, LOW);
        digitalWrite(LED_RB, HIGH);

        zwarcie_chwilowe(START);

        delay(CZAS_LOTU_2);

        while(digitalRead(PRZYCISK_B) == LOW){
          delay(10);
        }

        POWROT = false;
      }
    }
  }

  // BAZOWANIE
  if(digitalRead(PRZYCISK_BAZOWANIE) == LOW){
    delay(50);

    if(digitalRead(PRZYCISK_BAZOWANIE) == LOW){

      digitalWrite(LED_GA, LOW);
      digitalWrite(LED_GB, LOW);

      zwarcie_chwilowe(BAZOWANIE);

      // Migają oba przyciski
      for(int i = 0; i < 10; i++){

        digitalWrite(LED_RA, HIGH);
        digitalWrite(LED_RB, HIGH);
        delay(500);

        digitalWrite(LED_RA, LOW);
        digitalWrite(LED_RB, LOW);
        delay(500);
      }

      // Po bazowaniu zawsze punkt A
      POWROT = false;

      digitalWrite(LED_RA, LOW);
      digitalWrite(LED_GA, HIGH);

      digitalWrite(LED_RB, HIGH);
      digitalWrite(LED_GB, LOW);
    }
  }
}//#include <DFRobotDFPlayerMini.h>

const byte PRZYCISK_A = 2;
const byte PRZYCISK_B = 17;
const byte PRZYCISK_BAZOWANIE = 16;

const byte LED_RA = 3;
const byte LED_GA = 4;

const byte LED_RB = 12;
const byte LED_GB = 11;

// STYCZNIK
const byte BAZOWANIE = 10;
const byte START = 13;

bool POWROT = false;
int CZAS_LOTU_1 = 1000;
int CZAS_LOTU_2 = 1000;

void zwarcie_chwilowe(byte styk){
  digitalWrite(styk, LOW);
  delay(500);
  digitalWrite(styk, HIGH);
  delay(50);
}

void setup(){

  pinMode(PRZYCISK_A, INPUT_PULLUP);
  pinMode(PRZYCISK_B, INPUT_PULLUP);
  pinMode(PRZYCISK_BAZOWANIE, INPUT_PULLUP);

  pinMode(LED_RA, OUTPUT);
  pinMode(LED_GA, OUTPUT);
  pinMode(LED_RB, OUTPUT);
  pinMode(LED_GB, OUTPUT);

  pinMode(BAZOWANIE, OUTPUT);
  pinMode(START, OUTPUT);

  // Stan gotowości - kierunek A
  digitalWrite(BAZOWANIE, HIGH);
  digitalWrite(START, HIGH);

  digitalWrite(LED_RA, LOW);
  digitalWrite(LED_GA, HIGH);

  digitalWrite(LED_RB, HIGH);
  digitalWrite(LED_GB, LOW);
}

void loop(){

  // KIERUNEK A
  if(POWROT == false){

    digitalWrite(LED_RA, LOW);
    digitalWrite(LED_GA, HIGH);

    digitalWrite(LED_RB, HIGH);
    digitalWrite(LED_GB, LOW);

    if(digitalRead(PRZYCISK_A) == LOW){
      delay(50);

      if(digitalRead(PRZYCISK_A) == LOW){

        digitalWrite(LED_GA, LOW);
        digitalWrite(LED_RA, HIGH);

        zwarcie_chwilowe(START);

        delay(CZAS_LOTU_1);

        while(digitalRead(PRZYCISK_A) == LOW){
          delay(10);
        }

        POWROT = true;
      }
    }
  }

  // POWRÓT - KIERUNEK B
  else{

    digitalWrite(LED_RA, HIGH);
    digitalWrite(LED_GA, LOW);

    digitalWrite(LED_RB, LOW);
    digitalWrite(LED_GB, HIGH);

    if(digitalRead(PRZYCISK_B) == LOW){
      delay(50);

      if(digitalRead(PRZYCISK_B) == LOW){

        digitalWrite(LED_GB, LOW);
        digitalWrite(LED_RB, HIGH);

        zwarcie_chwilowe(START);

        delay(CZAS_LOTU_2);

        while(digitalRead(PRZYCISK_B) == LOW){
          delay(10);
        }

        POWROT = false;
      }
    }
  }

  // BAZOWANIE
  if(digitalRead(PRZYCISK_BAZOWANIE) == LOW){
    delay(50);

    if(digitalRead(PRZYCISK_BAZOWANIE) == LOW){

      if(POWROT == false){
        digitalWrite(LED_GA, LOW);

        zwarcie_chwilowe(BAZOWANIE);

        for(int i = 0; i < 10; i++){
          digitalWrite(LED_RA, HIGH);
          delay(500);
          digitalWrite(LED_RA, LOW);
          delay(500);
        }

        digitalWrite(LED_GA, HIGH);
      }
      else{
        digitalWrite(LED_GB, LOW);

        zwarcie_chwilowe(BAZOWANIE);

        for(int i = 0; i < 10; i++){
          digitalWrite(LED_RB, HIGH);
          delay(500);
          digitalWrite(LED_RB, LOW);
          delay(500);
        }

        digitalWrite(LED_GB, HIGH);
      }
    }
  }
}

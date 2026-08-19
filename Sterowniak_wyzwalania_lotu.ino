//#include <DFRobotDFPlayerMini.h>

const byte PRZYCISK = 2;
const byte PRZYCISK_BAZOWANIE = 17;

const byte LED_R = 3;
const byte LED_G = 4;
// STYCZNIK
const byte BAZOWANIE = 10;
const byte START = 13;

bool POWROT = false;
int CZAS_LOTU_1 = 1000;
int CZAS_LOTU_2 = 1000;

void zwarcie_chwilowe(byte styk){
  digitalWrite(styk,LOW);
  delay(500);
  digitalWrite(styk,HIGH);
  delay(50);
}

void setup(){
  
  pinMode(PRZYCISK, INPUT_PULLUP);
  pinMode(PRZYCISK_BAZOWANIE, INPUT_PULLUP);

  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(BAZOWANIE,OUTPUT);
  pinMode(START,OUTPUT);

  // Stan gotowości
  digitalWrite(BAZOWANIE, HIGH);
  digitalWrite(START, HIGH);
  digitalWrite(LED_R, LOW);
  digitalWrite(LED_G, HIGH);
}
 void loop(){
  if(digitalRead(PRZYCISK)==LOW){
    delay(50);
    if(digitalRead(PRZYCISK)==LOW){
      digitalWrite(LED_G,LOW);
      digitalWrite(LED_R,HIGH);
      zwarcie_chwilowe(START);
      if(POWROT==true){
        delay(CZAS_LOTU_2);
      }      
      else{
        delay(CZAS_LOTU_1);
      }
      
      digitalWrite(LED_R,LOW);
      digitalWrite(LED_G,HIGH);
      while (digitalRead(PRZYCISK)==LOW){
        delay(10);
      }
      POWROT = !POWROT;
    }
  }
  if(digitalRead(PRZYCISK_BAZOWANIE)==LOW){
    delay(50);
    if(digitalRead(PRZYCISK_BAZOWANIE)==LOW){
      digitalWrite(LED_G,LOW);
      zwarcie_chwilowe(BAZOWANIE);
      for(int i = 0; i < 10; i++){
      digitalWrite(LED_R, HIGH);
      delay(500);
      digitalWrite(LED_R, LOW);
      delay(500);
      }
      digitalWrite(LED_G,HIGH);
    }
  }
 }


#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <FastLED.h>
#include <DFRobotDFPlayerMini.h>
#include <esp_sleep.h>
#include <driver/gpio.h>

#include "web_ui.h"

// ============================================================
// 1. PINY
// ============================================================

constexpr uint8_t PIN_LADOWARKI  = 0;   // LOW = ladowarka, HIGH = poza ladowarka
constexpr uint8_t PIN_LED        = 1;   // WS2812B / ARGB
constexpr uint8_t PIN_DF_RX      = 20;  // ESP RX  <- TX DFPlayera
constexpr uint8_t PIN_DF_TX      = 21;  // ESP TX  -> RX DFPlayera przez 1 kOhm

// ============================================================
// 2. USTAWIENIA STALE
// ============================================================

constexpr char NAZWA_WIFI[] = "BALON";
constexpr char HASLO_WIFI[] = "balon1234";

constexpr uint8_t  GLOSNOSC_DFPLAYERA = 25;  // 0..30
constexpr uint8_t  NUMER_UTWORU       = 1;   // 001.mp3
constexpr uint8_t  JASNOSC_LED         = 150;

constexpr uint8_t  MAKS_EFEKTOW        = 15;
constexpr uint32_t MAKS_CZAS_S         = 3600;

constexpr uint32_t FILTR_CZUJNIKA_MS   = 100;
constexpr uint32_t PLOMIEN_CO_MS       = 45;

// 5 minut bez wykrycia LOW na GPIO0 -> Deep Sleep.
constexpr uint32_t CZAS_DO_USPIENIA_MS = 5UL * 60UL * 1000UL;

// ============================================================
// 3. KONFIGURACJA EFEKTOW
// ============================================================

struct Efekt {
  uint32_t startS;
  uint32_t koniecS;
};

struct Trasa {
  uint8_t liczbaEfektow;
  Efekt efekty[MAKS_EFEKTOW];
};

enum Kierunek : uint8_t {
  A_DO_B = 0,
  B_DO_A = 1
};

struct Konfiguracja {
  uint32_t znacznik;
  uint16_t wersja;
  Trasa ab;
  Trasa ba;
  uint8_t nastepnyKierunek;
};

constexpr uint32_t ZNACZNIK = 0x42414C4F; // "BALO"
constexpr uint16_t WERSJA   = 1;

Konfiguracja cfg;

// ============================================================
// 4. STAN SYSTEMU
// ============================================================

enum Stan : uint8_t {
  CZEKA_NA_LADOWARKE,
  GOTOWY,
  LOT,
  CZEKA_NA_LADOWANIE
};

Stan stan = CZEKA_NA_LADOWARKE;

Kierunek kierunekLotu = A_DO_B;

bool efektAktywny = false;
bool dfPlayerOK    = false;

uint32_t startLotuMs        = 0;
uint32_t ostatniPlomienMs   = 0;
uint32_t ostatnieLadowanieMs = 0;
uint32_t liczbaTras         = 0;

// Czasy konca programu sa liczone tylko po wczytaniu/zapisie konfiguracji.
uint32_t czasProgramuAB = 0;
uint32_t czasProgramuBA = 0;

// ============================================================
// 5. CZUJNIK LADOWARKI
// ============================================================

bool surowyStan    = HIGH;
bool stabilnyStan  = HIGH;
uint32_t zmianaStanuMs = 0;

// ============================================================
// 6. SPRZET
// ============================================================

CRGB led[1];
DFRobotDFPlayerMini dfPlayer;
WebServer serwer(80);
Preferences pamiec;

// ============================================================
// 7. LOG WWW + SERIAL
// ============================================================
//
// Wszystkie komunikaty programu ida jednoczesnie:
//  - do zwyklego Serial Monitora
//  - do bufora widocznego na stronie WWW
//
// Bufor ma staly rozmiar, wiec nie rosnie w nieskonczonosc.
//

class Logger : public Print {
public:
  void begin(uint32_t baud) {
    Serial.begin(baud);
  }

  void flush() {
    Serial.flush();
  }

  size_t write(uint8_t c) override {
    Serial.write(c);
    dopisz(c);
    return 1;
  }

  const char* tekst() const {
    return bufor;
  }

private:
  static constexpr size_t ROZMIAR = 5000;
  char bufor[ROZMIAR] = {0};
  size_t dlugosc = 0;

  void dopisz(char c) {
    // Gdy bufor sie zapelni, usuwamy najstarsze 500 znakow.
    if (dlugosc >= ROZMIAR - 1) {
      constexpr size_t USUN = 500;
      memmove(bufor, bufor + USUN, ROZMIAR - USUN);
      dlugosc -= USUN;
    }

    bufor[dlugosc++] = c;
    bufor[dlugosc] = '\0';
  }
};

Logger Log;

// ============================================================
// 8. FUNKCJE POMOCNICZE
// ============================================================

const char* nazwaKierunku(Kierunek k) {
  return (k == A_DO_B) ? "A -> B" : "B -> A";
}

const char* nazwaStanu() {
  switch (stan) {
    case CZEKA_NA_LADOWARKE: return "Oczekiwanie na ladowarke";
    case GOTOWY:             return "Gotowy do startu";
    case LOT:                return "Lot trwa";
    case CZEKA_NA_LADOWANIE: return "Program zakonczony - oczekiwanie na ladowanie";
  }
  return "Nieznany";
}

Kierunek nastepnyKierunek() {
  return (cfg.nastepnyKierunek == B_DO_A) ? B_DO_A : A_DO_B;
}

Trasa& trasa(Kierunek k) {
  return (k == A_DO_B) ? cfg.ab : cfg.ba;
}

uint32_t& czasProgramu(Kierunek k) {
  return (k == A_DO_B) ? czasProgramuAB : czasProgramuBA;
}

uint32_t policzCzasProgramu(const Trasa& t) {
  uint32_t koniec = 0;

  for (uint8_t i = 0; i < t.liczbaEfektow; ++i) {
    if (t.efekty[i].koniecS > koniec) {
      koniec = t.efekty[i].koniecS;
    }
  }

  return koniec;
}

void przeliczCzasyProgramow() {
  czasProgramuAB = policzCzasProgramu(cfg.ab);
  czasProgramuBA = policzCzasProgramu(cfg.ba);
}

uint32_t pozostaloDoUspieniaMs(uint32_t teraz) {
  if (stabilnyStan == LOW) {
    return CZAS_DO_USPIENIA_MS;
  }

  const uint32_t minelo = teraz - ostatnieLadowanieMs;

  if (minelo >= CZAS_DO_USPIENIA_MS) {
    return 0;
  }

  return CZAS_DO_USPIENIA_MS - minelo;
}

// ============================================================
// 9. PAMIEC
// ============================================================

void ustawDomyslnaTrase(Trasa& t) {
  memset(&t, 0, sizeof(t));

  t.liczbaEfektow = 4;
  t.efekty[0] = {2,  8};
  t.efekty[1] = {15, 23};
  t.efekty[2] = {30, 42};
  t.efekty[3] = {50, 55};
}

void ustawDomyslnaKonfiguracje() {
  memset(&cfg, 0, sizeof(cfg));

  cfg.znacznik = ZNACZNIK;
  cfg.wersja   = WERSJA;
  cfg.nastepnyKierunek = A_DO_B;

  ustawDomyslnaTrase(cfg.ab);
  ustawDomyslnaTrase(cfg.ba);
}

bool konfiguracjaPoprawna() {
  return
    cfg.znacznik == ZNACZNIK &&
    cfg.wersja == WERSJA &&
    cfg.ab.liczbaEfektow <= MAKS_EFEKTOW &&
    cfg.ba.liczbaEfektow <= MAKS_EFEKTOW &&
    cfg.nastepnyKierunek <= B_DO_A;
}

void zapiszKonfiguracje() {
  pamiec.putBytes("ustawienia", &cfg, sizeof(cfg));
  Log.println("Ustawienia zapisane");
}

void wczytajKonfiguracje() {
  pamiec.begin("balon", false);

  if (pamiec.getBytesLength("ustawienia") == sizeof(cfg)) {
    pamiec.getBytes("ustawienia", &cfg, sizeof(cfg));
  }

  if (!konfiguracjaPoprawna()) {
    Log.println("Tworzenie ustawien domyslnych");
    ustawDomyslnaKonfiguracje();
    zapiszKonfiguracje();
  }

  przeliczCzasyProgramow();
}

// ============================================================
// 10. LED / PLOMIEN
// ============================================================
// ============================================================
// 10. LED / PLOMIEN
// ============================================================
//
// DWA TRYBY PLOMIENIA:
//
// 1. SPOKOJNY PLOMIEN
//    - gdy glowny efekt jest wylaczony
//    - czerwono-pomaranczowy
//    - delikatny
//    - bez dzwieku
//
// 2. MOCNY PLOMIEN
//    - gdy glowny efekt jest aktywny
//    - jasniejszy, bardziej zolto-bialy
//    - mniej czerwony
//    - razem z dzwiekiem
//

void pokazSpokojnyPlomien() {
  // Czerwien -> pomarancz.
  const uint8_t hue = random8(0, 24);

  // Wysokie nasycenie = malo bieli.
  const uint8_t saturation = random8(235, 255);

  // Niska jasnosc.
  const uint8_t brightness = random8(25, 75);

  led[0] = CHSV(hue, saturation, brightness);
  FastLED.show();
}

void pokazMocnyPlomien() {
  // Pomarancz -> zolty.
  const uint8_t hue = random8(18, 46);

  // Mniejsze nasycenie = wiecej bieli.
  const uint8_t saturation = random8(70, 175);

  // Wysoka jasnosc.
  const uint8_t brightness = random8(175, 255);

  led[0] = CHSV(hue, saturation, brightness);
  FastLED.show();
}

void zgasLED() {
  led[0] = CRGB::Black;
  FastLED.show();
}

void aktualizujPlomien(uint32_t teraz) {
  if (teraz - ostatniPlomienMs < PLOMIEN_CO_MS) {
    return;
  }

  ostatniPlomienMs = teraz;

  if (efektAktywny) {
    pokazMocnyPlomien();
  } else {
    pokazSpokojnyPlomien();
  }
}

// ============================================================
// 11. EFEKT: DZWIEK + PLOMIEN
// ============================================================

void uruchomEfekt() {
  if (efektAktywny) {
    return;
  }

  efektAktywny = true;
  ostatniPlomienMs = 0;

  Log.println("EFEKT: START - mocny plomien + dzwiek");

  if (dfPlayerOK) {
    dfPlayer.loop(NUMER_UTWORU);
  }

  pokazMocnyPlomien();
}

void zatrzymajEfekt() {
  if (efektAktywny) {
    if (dfPlayerOK) {
      dfPlayer.stop();
    }

    Log.println("EFEKT: STOP - spokojny plomien");
  }

  efektAktywny = false;
  ostatniPlomienMs = 0;

  // Poza glownym efektem LED nadal pokazuje spokojny plomien.
  pokazSpokojnyPlomien();
}

bool efektPowinienDzialac(const Trasa& t, uint32_t sekundaLotu) {
  for (uint8_t i = 0; i < t.liczbaEfektow; ++i) {
    const Efekt& e = t.efekty[i];

    if (sekundaLotu >= e.startS && sekundaLotu < e.koniecS) {
      return true;
    }
  }

  return false;
}

// ============================================================
// 12. LOT
// ============================================================

void rozpocznijLot(uint32_t teraz) {
  if (stan != GOTOWY) {
    return;
  }

  zatrzymajEfekt();

  kierunekLotu = nastepnyKierunek();
  startLotuMs  = teraz;
  stan         = LOT;

  Log.println();
  Log.println("==============================");
  Log.println("BALON PODNIESIONY");
  Log.print("KIERUNEK: ");
  Log.println(nazwaKierunku(kierunekLotu));
  Log.println("START PROGRAMU");
  Log.println("==============================");
}

void przerwijLot() {
  zatrzymajEfekt();
  stan = GOTOWY;

  Log.println();
  Log.println("==============================");
  Log.println("BALON WYLADOWAL ZA WCZESNIE");
  Log.println("TRASA NIEZALICZONA");
  Log.print("NASTEPNY KIERUNEK NADAL: ");
  Log.println(nazwaKierunku(nastepnyKierunek()));
  Log.println("==============================");
}

void zakonczTrase() {
  zatrzymajEfekt();

  ++liczbaTras;

  cfg.nastepnyKierunek =
    (kierunekLotu == A_DO_B) ? B_DO_A : A_DO_B;

  // Kierunek musi przetrwac restart / Deep Sleep.
  pamiec.putUChar("kierunek", cfg.nastepnyKierunek);

  stan = GOTOWY;

  Log.println();
  Log.println("==============================");
  Log.println("TRASA ZALICZONA");
  Log.print("LICZBA TRAS: ");
  Log.println(liczbaTras);
  Log.print("NASTEPNY KIERUNEK: ");
  Log.println(nazwaKierunku(nastepnyKierunek()));
  Log.println("==============================");
}

void obsluzLadowanie(uint32_t teraz) {
  ostatnieLadowanieMs = teraz;

  if (stan == LOT) {
    const uint32_t sekundaLotu = (teraz - startLotuMs) / 1000UL;

    if (sekundaLotu >= czasProgramu(kierunekLotu)) {
      zakonczTrase();
    } else {
      przerwijLot();
    }
    return;
  }

  if (stan == CZEKA_NA_LADOWANIE) {
    zakonczTrase();
    return;
  }

  if (stan == CZEKA_NA_LADOWARKE) {
    zatrzymajEfekt();
    stan = GOTOWY;

    Log.println();
    Log.println("LADOWARKA WYKRYTA");
    Log.println("UKLAD GOTOWY DO STARTU");
  }
}

void aktualizujLot(uint32_t teraz) {
  if (stan != LOT) {
    return;
  }

  const Trasa& t = trasa(kierunekLotu);
  const uint32_t sekundaLotu = (teraz - startLotuMs) / 1000UL;

  if (sekundaLotu >= czasProgramu(kierunekLotu)) {
    zatrzymajEfekt();
    stan = CZEKA_NA_LADOWANIE;

    Log.println();
    Log.println("PROGRAM EFEKTOW ZAKONCZONY");
    Log.println("OCZEKIWANIE NA LADOWANIE");
    return;
  }

  const bool maDzialac = efektPowinienDzialac(t, sekundaLotu);

  if (maDzialac && !efektAktywny) {
    uruchomEfekt();
  } else if (!maDzialac && efektAktywny) {
    zatrzymajEfekt();
  }

  if (efektAktywny) {
    aktualizujPlomien(teraz);
  }
}

// ============================================================
// 13. CZUJNIK GPIO0
// ============================================================

void aktualizujCzujnik(uint32_t teraz) {
  const bool odczyt = digitalRead(PIN_LADOWARKI);

  if (odczyt != surowyStan) {
    surowyStan = odczyt;
    zmianaStanuMs = teraz;
  }

  if (
    odczyt != stabilnyStan &&
    teraz - zmianaStanuMs >= FILTR_CZUJNIKA_MS
  ) {
    const bool poprzedni = stabilnyStan;
    stabilnyStan = odczyt;

    Log.print("GPIO0 = ");
    Log.println(stabilnyStan == LOW ? "LOW - LADOWARKA" : "HIGH - BRAK LADOWANIA");

    if (poprzedni == LOW && stabilnyStan == HIGH) {
      rozpocznijLot(teraz);
    }

    if (poprzedni == HIGH && stabilnyStan == LOW) {
      obsluzLadowanie(teraz);
    }
  }

  // LOW zeruje licznik Deep Sleep przez caly czas.
  if (stabilnyStan == LOW) {
    ostatnieLadowanieMs = teraz;
  }
}

// ============================================================
// 14. DEEP SLEEP
// ============================================================

void wejdzWDeepSleep() {
  // Ostatnia kontrola, zeby nie zasnac dokladnie w chwili ladowania.
  if (digitalRead(PIN_LADOWARKI) == LOW) {
    ostatnieLadowanieMs = millis();
    return;
  }

  // Przed Deep Sleep zatrzymujemy dzwiek i calkowicie gasimy LED.
  if (dfPlayerOK) {
    dfPlayer.stop();
  }

  efektAktywny = false;
  zgasLED();

  Log.println();
  Log.println("==============================");
  Log.println("BRAK LADOWANIA PRZEZ 5 MINUT");
  Log.println("PRZEJSCIE W DEEP SLEEP");
  Log.println("WYBUDZENIE: GPIO0 = LOW");
  Log.println("==============================");

  serwer.stop();
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_OFF);

  Log.flush();
  delay(50);

  // Utrzymujemy pull-up na GPIO0 i wybudzamy stanem LOW.
  gpio_pullup_en((gpio_num_t)PIN_LADOWARKI);
  gpio_pulldown_dis((gpio_num_t)PIN_LADOWARKI);

  const esp_err_t err = esp_deep_sleep_enable_gpio_wakeup(
    1ULL << PIN_LADOWARKI,
    ESP_GPIO_WAKEUP_GPIO_LOW
  );

  if (err != ESP_OK) {
    Log.print("BLAD konfiguracji Deep Sleep: ");
    Log.println((int)err);
    ostatnieLadowanieMs = millis();
    return;
  }

  esp_deep_sleep_start();
}

void aktualizujUspienie(uint32_t teraz) {
  if (stabilnyStan == LOW) {
    return;
  }

  if (teraz - ostatnieLadowanieMs >= CZAS_DO_USPIENIA_MS) {
    wejdzWDeepSleep();
  }
}

// ============================================================
// 15. API WWW
// ============================================================

String jsonStatus(uint32_t teraz) {
  String s;
  s.reserve(320);

  const uint32_t pozostaloMs = pozostaloDoUspieniaMs(teraz);

  s += "{";
  s += "\"stan\":\"" + String(nazwaStanu()) + "\",";
  s += "\"gpio0\":\"" + String(stabilnyStan == LOW ? "LOW" : "HIGH") + "\",";
  s += "\"ladowanie\":";
  s += (stabilnyStan == LOW ? "true" : "false");
  s += ",";
  s += "\"kierunek\":\"" + String(nazwaKierunku(nastepnyKierunek())) + "\",";
  s += "\"efekt\":";
  s += (efektAktywny ? "true" : "false");
  s += ",";
  s += "\"trasy\":" + String(liczbaTras) + ",";
  s += "\"sleepRemainingMs\":" + String(pozostaloMs) + ",";
  s += "\"sleepTotalMs\":" + String(CZAS_DO_USPIENIA_MS);
  s += "}";

  return s;
}

String jsonKonfiguracja() {
  String s;
  s.reserve(1000);

  auto dodajTrase = [&](const Trasa& t) {
    s += "{\"count\":";
    s += String(t.liczbaEfektow);
    s += ",\"effects\":[";

    for (uint8_t i = 0; i < MAKS_EFEKTOW; ++i) {
      if (i) s += ",";
      s += "[";
      s += String(t.efekty[i].startS);
      s += ",";
      s += String(t.efekty[i].koniecS);
      s += "]";
    }

    s += "]}";
  };

  s += "{\"ab\":";
  dodajTrase(cfg.ab);
  s += ",\"ba\":";
  dodajTrase(cfg.ba);
  s += "}";

  return s;
}

bool pobierzLiczbe(const String& nazwa, uint32_t& wynik) {
  if (!serwer.hasArg(nazwa)) {
    return false;
  }

  const long v = serwer.arg(nazwa).toInt();

  if (v < 0 || v > (long)MAKS_CZAS_S) {
    return false;
  }

  wynik = (uint32_t)v;
  return true;
}

bool odczytajTrase(const char* prefix, Trasa& t, String& blad) {
  const String countName = String(prefix) + "_count";

  if (!serwer.hasArg(countName)) {
    blad = "Brak liczby efektow";
    return false;
  }

  const int count = serwer.arg(countName).toInt();

  if (count < 0 || count > MAKS_EFEKTOW) {
    blad = "Nieprawidlowa liczba efektow";
    return false;
  }

  t.liczbaEfektow = (uint8_t)count;

  for (uint8_t i = 0; i < MAKS_EFEKTOW; ++i) {
    uint32_t start = 0;
    uint32_t koniec = 0;

    const String startName = String(prefix) + "_s" + String(i);
    const String endName   = String(prefix) + "_e" + String(i);

    if (!pobierzLiczbe(startName, start) || !pobierzLiczbe(endName, koniec)) {
      blad = "Nieprawidlowa wartosc efektu " + String(i + 1);
      return false;
    }

    if (i < t.liczbaEfektow && koniec <= start) {
      blad = "Efekt " + String(i + 1) + ": koniec musi byc pozniej niz start";
      return false;
    }

    t.efekty[i] = {start, koniec};
  }

  return true;
}

void apiZapisz() {
  if (stan == LOT || stan == CZEKA_NA_LADOWANIE) {
    serwer.send(409, "text/plain; charset=utf-8", "Nie mozna zapisywac podczas trasy.");
    return;
  }

  Konfiguracja nowa = cfg;
  String blad;

  if (!odczytajTrase("ab", nowa.ab, blad)) {
    serwer.send(400, "text/plain; charset=utf-8", "A -> B: " + blad);
    return;
  }

  if (!odczytajTrase("ba", nowa.ba, blad)) {
    serwer.send(400, "text/plain; charset=utf-8", "B -> A: " + blad);
    return;
  }

  cfg = nowa;
  przeliczCzasyProgramow();
  zapiszKonfiguracje();

  serwer.send(200, "text/plain; charset=utf-8", "OK");
}

void uruchomWWW() {
  serwer.on("/", HTTP_GET, []() {
    serwer.sendHeader("Cache-Control", "no-store, no-cache, must-revalidate, max-age=0");
    serwer.sendHeader("Pragma", "no-cache");
    serwer.send_P(200, "text/html; charset=utf-8", STRONA_HTML);
  });

  serwer.on("/api/status", HTTP_GET, []() {
    serwer.send(200, "application/json", jsonStatus(millis()));
  });

  serwer.on("/api/config", HTTP_GET, []() {
    serwer.send(200, "application/json", jsonKonfiguracja());
  });

  serwer.on("/api/log", HTTP_GET, []() {
    serwer.send(200, "text/plain; charset=utf-8", Log.tekst());
  });

  serwer.on("/api/save", HTTP_POST, apiZapisz);

  serwer.begin();

  Log.println();
  Log.println("WWW uruchomione");
  Log.print("Siec: ");
  Log.println(NAZWA_WIFI);
  Log.print("Adres: http://");
  Log.println(WiFi.softAPIP());
}

// ============================================================
// 16. SETUP
// ============================================================

void setup() {
  Log.begin(115200);
  delay(800);

  Log.println();
  Log.println("==============================");
  Log.println("STEROWNIK BALONU");
  Log.println("==============================");

  if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_GPIO) {
    Log.println("WYBUDZENIE Z DEEP SLEEP: GPIO0 = LOW");
  }

  wczytajKonfiguracje();

  // Jeżeli nowy, lekki zapis kierunku istnieje, ma pierwszenstwo.
  cfg.nastepnyKierunek = pamiec.getUChar(
    "kierunek",
    cfg.nastepnyKierunek
  );

  pinMode(PIN_LADOWARKI, INPUT_PULLUP);

  surowyStan   = digitalRead(PIN_LADOWARKI);
  stabilnyStan = surowyStan;
  zmianaStanuMs = millis();
  ostatnieLadowanieMs = millis();

  FastLED.addLeds<WS2812B, PIN_LED, GRB>(led, 1);
  FastLED.setBrightness(JASNOSC_LED);
  zgasLED();
  randomSeed(micros());

  Log.println("Uruchamianie DFPlayera");

  Serial0.begin(
    9600,
    SERIAL_8N1,
    PIN_DF_RX,
    PIN_DF_TX
  );

  delay(1200);

  if (dfPlayer.begin(Serial0)) {
    dfPlayerOK = true;
    dfPlayer.volume(GLOSNOSC_DFPLAYERA);
    dfPlayer.EQ(DFPLAYER_EQ_NORMAL);
    dfPlayer.outputDevice(DFPLAYER_DEVICE_SD);
    dfPlayer.stop();
    Log.println("DFPlayer uruchomiony");
  } else {
    dfPlayerOK = false;
    Log.println("DFPlayer nie odpowiada - praca bez dzwieku");
  }

  WiFi.mode(WIFI_AP);
  WiFi.setSleep(true);

  if (WiFi.softAP(NAZWA_WIFI, HASLO_WIFI)) {
    uruchomWWW();
  } else {
    Log.println("BLAD uruchomienia Wi-Fi");
  }

  zatrzymajEfekt();

  Log.print("Nastepny kierunek: ");
  Log.println(nazwaKierunku(nastepnyKierunek()));

  if (stabilnyStan == LOW) {
    stan = GOTOWY;
    Log.println("Balon na ladowarce - system gotowy");
  } else {
    stan = CZEKA_NA_LADOWARKE;
    Log.println("Brak ladowania - start odliczania 5 minut do uspienia");
  }
}

// ============================================================
// 17. LOOP
// ============================================================

void loop() {
  const uint32_t teraz = millis();

  aktualizujCzujnik(teraz);
  aktualizujLot(teraz);
  aktualizujUspienie(teraz);

  // Spokojny plomien poza efektem, mocny podczas efektu.
  aktualizujPlomien(teraz);

  serwer.handleClient();

  // Oddajemy czas systemowi/Wi-Fi zamiast krecic pusta petla na 100% CPU.
  delay(2);
}

# Sterownik balonu

Sterownik modułu balonu do makiety oparty na **ESP32-C3 Super Mini**.

Układ automatycznie wykrywa start i lądowanie balonu, wykonuje zaprogramowane efekty świetlne i dźwiękowe oraz obsługuje dwa kierunki jazdy:

* **A → B**
* **B → A**

Konfiguracja odbywa się przez lokalny panel WWW dostępny przez Wi-Fi.

## Funkcje

* automatyczne wykrywanie startu i lądowania,
* osobne sekwencje efektów dla A → B i B → A,
* do **15 efektów czasowych na kierunek**,
* efekt płomienia na diodzie WS2812B,
* dźwięk z DFPlayer Mini,
* konfiguracja przez Wi-Fi,
* zapis ustawień w pamięci ESP32,
* logi i diagnostyka przez WWW,
* automatyczny **Deep Sleep** po 5 minutach bez ładowarki.

## Sprzęt

* ESP32-C3 Super Mini
* DFPlayer Mini
* karta microSD
* 1 × WS2812B / ARGB
* akumulator LiPo ~3,7 V
* przetwornica 3,7 V → 5 V
* ładowanie bezprzewodowe
* tranzystor do wykrywania napięcia ładowarki

## Połączenia

| Funkcja                |   GPIO |
| ---------------------- | -----: |
| Wykrywanie ładowarki   |  GPIO0 |
| ARGB                   |  GPIO1 |
| RX ESP32 ← TX DFPlayer | GPIO20 |
| TX ESP32 → RX DFPlayer | GPIO21 |

Między GPIO21 a RX DFPlayera zalecany jest rezystor około **1 kΩ**.

Wszystkie moduły muszą mieć wspólną masę.

### GPIO0

```text
LOW  = balon na ładowarce
HIGH = balon poza ładowarką
```

Na GPIO ESP32 nie wolno podawać bezpośrednio **5 V**.

## Działanie

1. Balon stoi na ładowarce — `GPIO0 = LOW`.
2. Oderwanie od ładowarki powoduje zmianę `LOW → HIGH`.
3. Sterownik rozpoczyna program efektów dla aktualnego kierunku.
4. Po zakończeniu programu oczekuje na lądowanie.
5. Po wykryciu `GPIO0 = LOW` trasa zostaje zaliczona.
6. Kierunek zmienia się na przeciwny.

Jeżeli balon wyląduje przed zakończeniem programu, trasa nie zostaje zaliczona i kierunek pozostaje bez zmian.

## Efekt płomienia

Sterownik wykorzystuje jedną diodę ARGB.

**Spokojny płomień**

* aktywny podczas postoju i pomiędzy efektami,
* czerwono-pomarańczowe migotanie,
* bez dźwięku.

**Mocny płomień**

* większa jasność,
* więcej żółtego i białego,
* jednocześnie odtwarzany jest dźwięk.

## DFPlayer

Plik na karcie microSD:

```text
001.mp3
```

Podczas mocnego efektu utwór jest odtwarzany i zapętlany.

## Wi-Fi

ESP32 tworzy własną sieć:

```text
SSID:  BALON
Hasło: balon1234
```

Panel sterowania:

```text
http://192.168.4.1
```

Internet nie jest wymagany.

W panelu można:

* ustawiać czasy efektów,
* dodawać i usuwać efekty,
* kopiować konfigurację A → B / B → A,
* sprawdzać stan sterownika,
* przeglądać logi.

## Ustawianie efektów

Każdy efekt posiada:

```text
START
CZAS TRWANIA
KONIEC
```

Przykład:

```text
START:         0:15
CZAS TRWANIA:  0:08
KONIEC:        0:23
```

Pole `KONIEC` jest obliczane automatycznie.

## Deep Sleep

Jeżeli przez **5 minut** nie zostanie wykryta ładowarka (`GPIO0 = HIGH`), ESP32 przechodzi w Deep Sleep.

Przed uśpieniem:

* zatrzymuje DFPlayer,
* wyłącza ARGB,
* wyłącza Wi-Fi i serwer [WWW](http://WWW).

Wybudzenie następuje po wykryciu:

```text
GPIO0 = LOW
```

czyli po ponownym pojawieniu się balonu na ładowarce.

## Struktura projektu

```text
BALON_FINAL_AUTO_fire/
├── BALON_FINAL_AUTO_fire.ino
└── web_ui.h
```

`BALON_FINAL_AUTO_fire.ino` — logika sterownika.

`web_ui.h` — panel [WWW](http://WWW).

## Uruchomienie

1. Wgraj `001.mp3` na kartę microSD.
2. Podłącz ESP32, DFPlayer, ARGB i czujnik ładowarki.
3. Wgraj `BALON_FINAL_AUTO_fire.ino`.
4. Połącz się z Wi-Fi `BALON`.
5. Otwórz `192.168.4.1`.
6. Ustaw czasy efektów i zapisz konfigurację.

Po konfiguracji sterownik może pracować całkowicie autonomicznie.



# Sterownik wyzwalający start balonu

Osobny sterownik odpowiada za **ręczne uruchomienie przejazdu balonu oraz bazowanie mechanizmu**.

## Połączenia

| Funkcja            |      Pin |
| ------------------ | -------: |
| Przycisk START     |       D2 |
| Przycisk bazowania | A3 / D17 |
| LED czerwona       |       D3 |
| LED zielona        |       D4 |
| Sygnał BAZOWANIE   |      D10 |
| Sygnał START       |      D13 |

Wyjścia `START` i `BAZOWANIE` są aktywowane stanem **LOW**.

Sterownik generuje impuls:

```text
LOW  przez 500 ms
HIGH — stan spoczynkowy
```

## Uruchomienie balonu

W stanie gotowości:

* zielona dioda świeci,
* czerwona dioda jest wyłączona.

Po naciśnięciu przycisku **START**:

1. zielona dioda gaśnie,
2. czerwona dioda zapala się,
3. sterownik wysyła impuls na wyjście `START`,
4. rozpoczyna odmierzanie czasu lotu,
5. po zakończeniu czasu czerwona dioda gaśnie,
6. ponownie zapala się zielona dioda.

Sterownik posiada osobne czasy dla obu kierunków:

```cpp
CZAS_LOTU_1
CZAS_LOTU_2
```

Po każdym uruchomieniu kierunek jest automatycznie przełączany:

```text
Lot 1 → Lot 2 → Lot 1 → Lot 2...
```

## Bazowanie

Po naciśnięciu przycisku bazowania:

1. zielona dioda gaśnie,
2. wysyłany jest impuls na wyjście `BAZOWANIE`,
3. czerwona dioda miga przez około 10 sekund,
4. po zakończeniu ponownie zapala się zielona dioda.

Migająca czerwona dioda informuje, że trwa procedura bazowania.

## Sygnalizacja

```text
Zielona LED          → sterownik gotowy
Czerwona LED         → trwa lot
Migająca czerwona    → trwa bazowanie
```


# Sterownik balonu

Sterownik modułu balonu do makiety oparty na **ESP32-C3 Super Mini**.

Układ automatycznie wykrywa start i lądowanie balonu, wykonuje zaprogramowane efekty świetlne i dźwiękowe oraz obsługuje dwa kierunki jazdy:

* **A → B**
* **B → A**

Konfiguracja odbywa się przez lokalny panel WWW dostępny przez Wi-Fi.

### Funkcje

* automatyczne wykrywanie startu i lądowania,
* osobne sekwencje efektów dla A → B i B → A,
* do **15 efektów czasowych na kierunek**,
* efekt płomienia na diodzie WS2812B,
* dźwięk z DFPlayer Mini,
* konfiguracja przez Wi-Fi,
* zapis ustawień w pamięci ESP32,
* logi i diagnostyka przez WWW,
* automatyczny **Deep Sleep** po 5 minutach bez ładowarki.

### Sprzęt

* ESP32-C3 Super Mini
* DFPlayer Mini
* karta microSD
* 1 × WS2812B / ARGB
* akumulator LiPo ~3,7 V
* przetwornica 3,7 V → 5 V
* ładowanie bezprzewodowe
* tranzystor do wykrywania napięcia ładowarki

### Połączenia

| Funkcja                |   GPIO |
| ---------------------- | -----: |
| Wykrywanie ładowarki   |  GPIO0 |
| ARGB                   |  GPIO1 |
| RX ESP32 ← TX DFPlayer | GPIO20 |
| TX ESP32 → RX DFPlayer | GPIO21 |

Między GPIO21 a RX DFPlayera zalecany jest rezystor około **1 kΩ**.

Wszystkie moduły muszą mieć wspólną masę.

###### GPIO0

```text
LOW  = balon na ładowarce
HIGH = balon poza ładowarką
```

Na GPIO ESP32 nie wolno podawać bezpośrednio **5 V**.

### Działanie

1. Balon stoi na ładowarce — `GPIO0 = LOW`.
2. Oderwanie od ładowarki powoduje zmianę `LOW → HIGH`.
3. Sterownik rozpoczyna program efektów dla aktualnego kierunku.
4. Po zakończeniu programu oczekuje na lądowanie.
5. Po wykryciu `GPIO0 = LOW` trasa zostaje zaliczona.
6. Kierunek zmienia się na przeciwny.

Jeżeli balon wyląduje przed zakończeniem programu, trasa nie zostaje zaliczona i kierunek pozostaje bez zmian.

### Efekt płomienia

Sterownik wykorzystuje jedną diodę ARGB.

**Spokojny płomień**

* aktywny podczas postoju i pomiędzy efektami,
* czerwono-pomarańczowe migotanie,
* bez dźwięku.

**Mocny płomień**

* większa jasność,
* więcej żółtego i białego,
* jednocześnie odtwarzany jest dźwięk.

### DFPlayer

Plik na karcie microSD:

```text
001.mp3
```

Podczas mocnego efektu utwór jest odtwarzany i zapętlany.

### Wi-Fi

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

### Ustawianie efektów

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

### Deep Sleep

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

### Struktura projektu

```text
BALON_FINAL_AUTO_fire/
├── BALON_FINAL_AUTO_fire.ino
└── web_ui.h
```

`BALON_FINAL_AUTO_fire.ino` — logika sterownika.

`web_ui.h` — panel [WWW](http://WWW).

### Uruchomienie

1. Wgraj `001.mp3` na kartę microSD.
2. Podłącz ESP32, DFPlayer, ARGB i czujnik ładowarki.
3. Wgraj `BALON_FINAL_AUTO_fire.ino`.
4. Połącz się z Wi-Fi `BALON`.
5. Otwórz `192.168.4.1`.
6. Ustaw czasy efektów i zapisz konfigurację.

Po konfiguracji sterownik może pracować całkowicie autonomicznie.




# Sterownik wyzwalający start balonu

Sterownik obsługuje ręczne uruchamianie przejazdu balonu między punktami **A i B** oraz bazowanie mechanizmu.

## Połączenia

| Funkcja | Pin |
|---|---|
| Przycisk A | A3 / D17 |
| Przycisk B | D2 |
| Przycisk bazowania | A2 / D16 |
| LED czerwona A | D12 |
| LED zielona A | D11 |
| LED czerwona B | D3 |
| LED zielona B | D4 |
| BAZOWANIE | D10 |
| START | D13 |

Wyjścia `START` i `BAZOWANIE` są aktywne stanem `LOW` przez 500 ms.

## Działanie

Po uruchomieniu aktywny jest **punkt A**:

```text
A → zielony
B → czerwony
```

Naciśnięcie aktywnego przycisku uruchamia przejazd. Podczas jazdy jego zielona LED gaśnie, a czerwona świeci.

Po zakończeniu przejazdu aktywny zostaje drugi punkt:

```text
A → B → A → B...
```

Czasy dla obu kierunków są ustawiane osobno:

```cpp
int CZAS_LOTU_1 = 1000;
int CZAS_LOTU_2 = 1000;
```

## Bazowanie

Po naciśnięciu przycisku bazowania:

1. wysyłany jest impuls `BAZOWANIE`,
2. zielone LED gasną,
3. czerwone LED A i B migają przez około 10 s,
4. po zakończeniu zawsze aktywny zostaje **punkt A**.

## Sygnalizacja

```text
Zielona LED  → punkt aktywny
Czerwona LED → punkt nieaktywny / trwa przejazd
Migające czerwone A+B → bazowanie
```

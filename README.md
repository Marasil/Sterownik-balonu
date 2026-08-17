# Sterownik balonu

Sterownik modułu balonu do makiety, zbudowany na **ESP32-C3 Super Mini**.

Projekt steruje efektami świetlnymi i dźwiękowymi balonu poruszającego się pomiędzy dwiema stacjami ładowania: **A** i **B**.  
Sterownik automatycznie rozpoznaje start oraz lądowanie, realizuje osobne sekwencje efektów dla obu kierunków, udostępnia konfigurację przez własną sieć Wi-Fi i przechodzi w **Deep Sleep**, jeżeli przez dłuższy czas nie wykryje ładowarki.

---

## 1. Najważniejsze funkcje

Sterownik realizuje:

- automatyczne wykrywanie startu i lądowania,
- obsługę dwóch kierunków:
  - **A → B**,
  - **B → A**,
- automatyczną zmianę kierunku po poprawnie zakończonej trasie,
- maksymalnie **15 przedziałów efektów dla każdego kierunku**,
- sterowanie jedną diodą ARGB jako efektem płomienia,
- odtwarzanie i zapętlanie pliku `001.mp3` z DFPlayer Mini podczas aktywnego efektu,
- dwa poziomy efektu płomienia:
  - spokojny płomień bez dźwięku,
  - mocny płomień z dźwiękiem,
- konfigurację czasów przez stronę WWW,
- panel statusu i diagnostyki dostępny bez kabla USB,
- podgląd logów programu przez WWW,
- licznik czasu do uśpienia,
- Deep Sleep po 5 minutach bez wykrycia ładowania,
- wybudzenie po ponownym wykryciu ładowarki,
- zapis konfiguracji w pamięci nieulotnej ESP32.

---

# 2. Sprzęt

## Główne elementy

- **ESP32-C3 Super Mini**
- **DFPlayer Mini**
- karta microSD z plikiem dźwiękowym
- 1 × dioda adresowalna ARGB / WS2812B
- akumulator **LiPo około 3,7 V**
- przetwornica podwyższająca napięcie z około **3,7 V do 5 V**
- układ ładowania bezprzewodowego
- tranzystor wykrywający obecność napięcia 5 V z odbiornika ładowarki

---

# 3. Zasilanie

Podstawowe zasilanie modelu:

```text
Akumulator LiPo ~3,7 V
        │
        ▼
Przetwornica STEP-UP
     3,7 V → 5 V
        │
        ├── ESP32-C3
        ├── DFPlayer Mini
        └── pozostałe elementy układu
```

Balon ląduje na ładowarkach bezprzewodowych znajdujących się na obu końcach trasy.

Gdy na wejściu układu ładowania pojawia się **5 V**, tranzystor zmienia stan sygnału przekazywanego do ESP32.

ESP32 nie otrzymuje 5 V bezpośrednio na GPIO0.

---

# 4. Połączenia ESP32-C3

| Funkcja | Pin ESP32-C3 | Uwagi |
|---|---:|---|
| Wykrywanie ładowarki | GPIO0 | `LOW` = ładowarka wykryta |
| Dioda ARGB | GPIO1 | DATA diody WS2812B |
| RX ESP32 dla DFPlayera | GPIO20 | połączony z TX DFPlayera |
| TX ESP32 dla DFPlayera | GPIO21 | połączony z RX DFPlayera |
| Masa | GND | wspólna masa wszystkich modułów |

### Schemat sygnałów

<img width="943" height="536" alt="Esp 32" src="https://github.com/user-attachments/assets/bdcb397c-abca-4d74-ab44-fac75988aeaf" />


Zalecany jest rezystor około **1 kΩ** pomiędzy GPIO21 ESP32 a wejściem RX DFPlayera.

---

# 5. Znaczenie GPIO0

GPIO0 jest najważniejszym sygnałem stanu modelu.

```text
GPIO0 = LOW   → balon znajduje się na ładowarce
GPIO0 = HIGH  → balon nie znajduje się na ładowarce
```

Pin pracuje jako:

```cpp
INPUT_PULLUP
```

Dlatego brak aktywnego sygnału daje stan HIGH, a wykrycie ładowania powoduje stan LOW.

Program filtruje krótkie zakłócenia i drgania sygnału, aby przypadkowa zmiana GPIO0 nie była traktowana jako start lub lądowanie.

---

# 6. Logika działania trasy

Sterownik pracuje jako prosta maszyna stanów.

## 6.1. Balon na ładowarce

Jeżeli:

```text
GPIO0 = LOW
```

sterownik uznaje, że balon znajduje się na jednej ze stacji.

System jest gotowy do rozpoczęcia następnej trasy.

---

## 6.2. Start

Start jest wykrywany przy zmianie:

```text
LOW → HIGH
```

czyli w momencie oderwania balonu od ładowarki.

W tej chwili:

1. zapamiętywany jest czas startu,
2. wybierany jest aktualny kierunek,
3. uruchamiane jest odmierzanie czasów efektów,
4. rozpoczyna się wykonywanie programu dla danego kierunku.

---

## 6.3. Lot

Podczas lotu program porównuje czas od startu z zapisaną tabelą efektów.

Każdy efekt określa:

```text
START
CZAS TRWANIA
```

Strona WWW automatycznie oblicza:

```text
KONIEC = START + CZAS TRWANIA
```

Przykład:

```text
START:        0:15
CZAS TRWANIA: 0:08
KONIEC:       0:23
```

Efekt będzie więc aktywny od 15. do 23. sekundy lotu.

---

## 6.4. Prawidłowe lądowanie

Trasa jest uznawana za prawidłowo zakończoną, jeżeli:

1. balon wystartował,
2. wykonany został cały program czasowy,
3. po zakończeniu programu pojawiło się:

```text
GPIO0 = LOW
```

Po poprawnym zakończeniu trasy sterownik zmienia kierunek.

```text
A → B
B → A
A → B
B → A
...
```

---

## 6.5. Przedwczesne lądowanie

Jeżeli GPIO0 przejdzie w LOW przed zakończeniem programu:

- dźwięk jest zatrzymywany,
- aktywny efekt jest przerywany,
- trasa nie jest zaliczana,
- kierunek nie zmienia się.

Przykład:

```text
planowany kierunek: A → B
       │
       ├── start
       │
       ├── przedwczesne lądowanie
       │
       └── następna próba nadal A → B
```

---

# 7. Kierunki A → B i B → A

Oba kierunki mają osobne zestawy czasów.

Dzięki temu efekty mogą występować w innych momentach podczas lotu:

```text
A → B
Efekt 1
Efekt 2
Efekt 3
...

B → A
Efekt 1
Efekt 2
Efekt 3
...
```

Każdy kierunek może posiadać od **0 do 15 efektów**.

Kolejność kierunku zmienia się automatycznie tylko po poprawnie ukończonej trasie.

---

# 8. Efekt płomienia

Do generowania płomienia wykorzystywana jest jedna dioda ARGB.

Sterownik posiada dwa tryby.

## Spokojny płomień

Gdy główny efekt czasowy nie jest aktywny:

- dźwięk jest wyłączony,
- dioda nadal świeci,
- płomień jest delikatny,
- dominuje czerwień i pomarańcz,
- jasność losowo zmienia się, symulując naturalne migotanie.

Spokojny płomień jest aktywny również podczas postoju na ładowarce.

## Mocny płomień

Podczas aktywnego efektu:

- płomień staje się znacznie jaśniejszy,
- kolor przesuwa się w stronę pomarańczowego, żółtego i białego,
- jest mniej czerwieni,
- jednocześnie uruchamiany jest DFPlayer.

Po zakończeniu efektu sterownik wraca do spokojnego płomienia.

---

# 9. DFPlayer Mini

DFPlayer Mini odpowiada za dźwięk efektu.

## Połączenie

```text
DFPlayer TX ─────────► GPIO20 ESP32
DFPlayer RX ◄───────── GPIO21 ESP32
                        │
                     ~1 kΩ
```

Wszystkie moduły muszą mieć wspólną masę.

## Plik audio

Sterownik wykorzystuje utwór numer:

```text
001
```

Plik powinien znajdować się na karcie microSD jako:

```text
001.mp3
```

Podczas aktywnego efektu utwór jest uruchamiany w pętli.

Jeżeli efekt trwa dłużej niż sam plik audio, utwór rozpocznie się ponownie.

Po zakończeniu efektu odtwarzanie jest zatrzymywane.

---

# 10. Wi-Fi

ESP32 tworzy własny punkt dostępowy.

```text
SSID:  BALON
Hasło: balon1234
```

Do konfiguracji nie jest wymagany router ani dostęp do Internetu.

Po połączeniu telefonu, tabletu lub komputera z siecią należy otworzyć:

```text
http://192.168.4.1
```

Telefon może wyświetlić informację:

```text
Brak dostępu do Internetu
```

Jest to normalne. Sieć służy wyłącznie do lokalnej komunikacji ze sterownikiem.

Wi-Fi pozostaje aktywne podczas normalnej pracy modelu aż do wejścia ESP32 w Deep Sleep.

---

# 11. Panel WWW

Strona WWW jest głównym interfejsem konfiguracji i diagnostyki modelu.

## Panel stanu

Wyświetlane są między innymi:

- aktualny stan sterownika,
- stan GPIO0,
- informacja o wykryciu ładowarki,
- następny kierunek,
- stan efektu płomienia,
- liczba ukończonych tras od uruchomienia,
- czas pozostały do uśpienia.

---

# 12. Edytor efektów

Dla każdego kierunku znajduje się osobny edytor:

```text
A → B
B → A
```

Nie wpisuje się ręcznie liczby efektów.

Liczba efektów jest obliczana automatycznie na podstawie dodanych pozycji.

## Dodawanie efektu

Przycisk:

```text
+ Dodaj efekt
```

tworzy nowy wpis.

Domyślnie nowy efekt:

- rozpoczyna się kilka sekund po zakończeniu poprzedniego,
- otrzymuje domyślny czas trwania.

Następnie można zmienić wartości ręcznie.

---

## Wprowadzanie czasu

Dostępne pola:

```text
START
CZAS TRWANIA
KONIEC — AUTO
```

Najwygodniejszy format:

```text
minuty:sekundy
```

Przykłady:

```text
0:05
0:30
1:15
2:40
```

Można również wpisać samą liczbę sekund:

```text
15
```

co oznacza 15 sekund.

---

## Automatyczne obliczenia

Użytkownik wpisuje:

```text
START        = 1:20
CZAS TRWANIA = 0:15
```

Strona automatycznie pokaże:

```text
KONIEC = 1:35
```

Nie trzeba samodzielnie przeliczać końca efektu.

Strona pokazuje również czas zakończenia całego programu dla danego kierunku.

---

## Dostępne operacje

Edytor umożliwia:

- dodawanie efektu,
- usuwanie efektu,
- duplikowanie efektu,
- sortowanie efektów według czasu startu,
- czyszczenie całej trasy,
- kopiowanie konfiguracji:
  - A → B do B → A,
  - B → A do A → B.

Przed zapisaniem program sprawdza poprawność czasów.

---

# 13. Log sterownika przez WWW

Podczas pracy modelu dostęp do kabla USB może być niemożliwy.

Dlatego sterownik posiada własny bufor logów dostępny przez stronę WWW.

Komunikaty, które normalnie pojawiają się w Serial Monitorze, są również zapisywane w pamięci RAM i wyświetlane w panelu.

Przykładowe informacje:

```text
BALON PODNIESIONY
KIERUNEK: A -> B
START PROGRAMU

EFEKT: START - mocny plomien + dzwiek

EFEKT: STOP - spokojny plomien

PROGRAM EFEKTOW ZAKONCZONY
OCZEKIWANIE NA LADOWANIE

TRASA ZALICZONA
NASTEPNY KIERUNEK: B -> A
```

Bufor logów ma ograniczony rozmiar. Po zapełnieniu najstarsze informacje są usuwane, dzięki czemu log nie zajmuje coraz większej ilości pamięci RAM.

---

# 14. Oszczędzanie energii

Model jest zasilany z akumulatora, dlatego sterownik posiada automatyczne usypianie.

## Odliczanie

Jeżeli:

```text
GPIO0 = HIGH
```

i przez **5 minut** nie pojawi się stan LOW, sterownik rozpoczyna procedurę uśpienia.

Na stronie WWW widoczny jest licznik:

```text
05:00
04:59
04:58
...
00:00
```

Każde ponowne pojawienie się:

```text
GPIO0 = LOW
```

zeruje licznik.

---

# 15. Deep Sleep

Po 5 minutach bez wykrycia ładowarki ESP32 przechodzi w tryb **Deep Sleep**.

Przed uśpieniem:

1. zatrzymywany jest DFPlayer,
2. dioda ARGB zostaje całkowicie zgaszona,
3. zatrzymywany jest serwer WWW,
4. wyłączane jest Wi-Fi,
5. konfigurowane jest wybudzenie przez GPIO0,
6. ESP32 przechodzi w Deep Sleep.

W stanie Deep Sleep sieć:

```text
BALON
```

nie jest widoczna.

Jest to zachowanie prawidłowe.

---

# 16. Wybudzenie

ESP32 zostaje wybudzone, gdy:

```text
GPIO0 = LOW
```

czyli gdy ponownie zostanie wykryta ładowarka.

Po wybudzeniu procesor uruchamia program od początku:

```text
Deep Sleep
    │
    │ GPIO0 = LOW
    ▼
WYBUDZENIE
    │
    ▼
setup()
    │
    ├── wczytanie ustawień
    ├── inicjalizacja GPIO
    ├── uruchomienie ARGB
    ├── uruchomienie DFPlayera
    ├── uruchomienie Wi-Fi
    └── uruchomienie panelu WWW
```

---

# 17. Pamięć nieulotna

ESP32 wykorzystuje pamięć `Preferences` / NVS.

Aktualna wersja programu zapisuje w pamięci:

- konfigurację efektów dla A → B,
- konfigurację efektów dla B → A,
- następny kierunek.

Dzięki temu konfiguracja nie znika po:

- odłączeniu baterii,
- resecie,
- wejściu w Deep Sleep.

Licznik wykonanych tras jest informacją bieżącej sesji i nie jest traktowany jako trwały licznik eksploatacyjny.

---

# 18. Struktura projektu

Projekt składa się z dwóch głównych plików:

```text
BALON_FINAL_AUTO_fire/
│
├── BALON_FINAL_AUTO_fire.ino
└── web_ui.h
```

## `BALON_FINAL_AUTO_fire.ino`

Zawiera logikę urządzenia:

- obsługę GPIO0,
- maszynę stanów,
- kierunki A → B i B → A,
- efekty czasowe,
- DFPlayer,
- ARGB,
- pamięć ustawień,
- Wi-Fi,
- API strony WWW,
- logowanie,
- Deep Sleep.

## `web_ui.h`

Zawiera wyłącznie interfejs strony WWW:

- HTML,
- CSS,
- JavaScript,
- panel statusu,
- timer,
- edytor efektów,
- log sterownika.

Rozdzielenie interfejsu od programu głównego ułatwia późniejsze modyfikowanie wyglądu strony bez mieszania kodu HTML z logiką sterowania balonem.

---

# 19. Uruchomienie projektu

## Krok 1 — karta microSD

Umieść na karcie plik:

```text
001.mp3
```

i włóż kartę do DFPlayer Mini.

## Krok 2 — podłączenie sprzętu

Sprawdź:

```text
GPIO0  → sygnał wykrycia ładowarki
GPIO1  → DATA ARGB
GPIO20 ← TX DFPlayer
GPIO21 → RX DFPlayer
GND    → wspólna masa
```

## Krok 3 — wgranie programu

Otwórz:

```text
BALON_FINAL_AUTO_fire.ino
```

w Arduino IDE.

Plik:

```text
web_ui.h
```

musi znajdować się w tym samym folderze szkicu.

## Krok 4 — połączenie z Wi-Fi

Po uruchomieniu ESP32 połącz urządzenie z:

```text
BALON
```

Hasło:

```text
balon1234
```

## Krok 5 — konfiguracja

Otwórz:

```text
192.168.4.1
```

Ustaw czasy dla obu kierunków i wybierz:

```text
ZAPISZ USTAWIENIA
```

---

# 20. Przykładowa konfiguracja

### A → B

```text
Efekt 1
START:        0:02
CZAS TRWANIA: 0:06
KONIEC:       0:08

Efekt 2
START:        0:15
CZAS TRWANIA: 0:08
KONIEC:       0:23

Efekt 3
START:        0:30
CZAS TRWANIA: 0:12
KONIEC:       0:42

Efekt 4
START:        0:50
CZAS TRWANIA: 0:05
KONIEC:       0:55
```

W tym przykładzie program dla kierunku A → B kończy się po:

```text
0:55
```

Jeżeli balon wyląduje przed tym momentem, trasa nie zostanie zaliczona.

---

# 21. Cykl pracy

Pełny cykl wygląda następująco:

```text
        ┌─────────────────┐
        │ BALON NA STACJI │
        │    GPIO0 LOW    │
        └────────┬────────┘
                 │
                 │ oderwanie
                 ▼
        ┌─────────────────┐
        │      START      │
        │ LOW → HIGH      │
        └────────┬────────┘
                 │
                 ▼
        ┌─────────────────┐
        │       LOT       │
        │ efekty czasowe  │
        └────────┬────────┘
                 │
                 ▼
        ┌─────────────────┐
        │ KONIEC PROGRAMU │
        │ czeka na LOW    │
        └────────┬────────┘
                 │
                 │ GPIO0 LOW
                 ▼
        ┌─────────────────┐
        │   LĄDOWANIE     │
        │ trasa zaliczona │
        └────────┬────────┘
                 │
                 ▼
        zmiana A→B / B→A
```

Jeżeli lądowanie nastąpi przed zakończeniem programu:

```text
LOT
 │
 ├── GPIO0 LOW za wcześnie
 │
 ▼
TRASA NIEZALICZONA
 │
 ▼
KIERUNEK BEZ ZMIANY
```

---

# 22. Najważniejsze parametry programu

W głównym pliku można łatwo znaleźć podstawowe parametry:

```cpp
NAZWA_WIFI
HASLO_WIFI

GLOSNOSC_DFPLAYERA
NUMER_UTWORU
JASNOSC_LED

MAKS_EFEKTOW
FILTR_CZUJNIKA_MS
PLOMIEN_CO_MS
CZAS_DO_USPIENIA_MS
```

Obecne główne wartości:

```text
Wi-Fi:             BALON
Hasło:             balon1234
Utwór:             001.mp3
Maks. efektów:     15 / kierunek
Deep Sleep:        po 5 minutach bez LOW
Wybudzenie:        GPIO0 = LOW
```

---

# 23. Ważne uwagi konstrukcyjne

- GPIO ESP32-C3 pracują z poziomami logicznymi **3,3 V**.
- Na GPIO0 nie należy podawać bezpośrednio 5 V.
- Sygnał obecności 5 V z układu ładowania jest przekazywany przez tranzystor.
- Wszystkie moduły sterowane sygnałami muszą mieć wspólną masę.
- DFPlayer i dioda ARGB mogą generować krótkie skoki poboru prądu, dlatego zasilanie 5 V powinno mieć odpowiedni zapas prądowy.
- Deep Sleep ogranicza pobór ESP32, ale elementy podłączone bezpośrednio do zasilania mogą nadal pobierać prąd.
- Przy zmianach sprzętowych należy zachować logikę:
  - `LOW = ładowarka`,
  - `HIGH = brak ładowarki`.

---

# 24. Podsumowanie

**Sterownik balonu** jest autonomicznym sterownikiem makiety działającym bez konieczności stałego połączenia z komputerem.

Po skonfigurowaniu czasów przez Wi-Fi urządzenie samodzielnie:

1. wykrywa obecność balonu na stacji,
2. wykrywa start,
3. realizuje właściwą sekwencję dla kierunku A → B lub B → A,
4. steruje płomieniem i dźwiękiem,
5. wykrywa lądowanie,
6. odrzuca niepełne trasy,
7. przełącza kierunek po prawidłowej trasie,
8. udostępnia bieżące informacje i logi przez WWW,
9. po długim braku ładowarki przechodzi w Deep Sleep,
10. wybudza się automatycznie po ponownym wykryciu ładowania.

Cały model może dzięki temu pracować autonomicznie na makiecie z zasilaniem bateryjnym.

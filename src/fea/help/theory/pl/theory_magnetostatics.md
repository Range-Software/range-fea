# Magnetostatyka - Podręcznik teorii i przewodnik użytkownika

Ten dokument opisuje solver magnetostatyki (`RSolverMagnetostatics`) programu
Range FEA, obsługujący typ problemu **Magnetostatyka** (*Magneto-statics*):
obliczane prawo, przyjmowane dane wejściowe, elementy interfejsu graficznego,
które nim sterują, kompletny samouczek i wartość uzyskiwanego wyniku.

> **Stan.** Solver bezpośrednio, za pomocą **prawa Biota-Savarta**, oblicza pole
> magnetyczne prądu wyznaczonego przez zadanie elektrostatyki. Nie ma układu
> równań ani warunków brzegowych: pole jest określone wyłącznie przez prąd, a
> warunek jego zanikania z dala od przewodnika jest wbudowany w prawo. Dla
> zadanej gęstości prądu wynik jest dokładny z kontrolowanym błędem kwadratury
> znacznie mniejszym niż `1e-4` wartości maksymalnej pola. Solver działa w
> **próżni** - materiałów magnetycznych nie można odwzorować - i oblicza tylko
> pole modelowanego prądu, więc droga prądu powinna być zamknięta wewnątrz
> modelu. Ograniczenia wymienia punkt 5.

**Spis treści**

1. [Podstawy teoretyczne](#1-podstawy-teoretyczne)
2. [Interfejs graficzny](#2-interfejs-graficzny)
3. [Samouczek - pole przewodnika z prądem](#3-samouczek---pole-przewodnika-z-prądem)
4. [Sprawdzanie modelu](#4-sprawdzanie-modelu)
5. [Ograniczenia](#5-ograniczenia)

---

## 1. Podstawy teoretyczne

### 1.1 Wynik - pole magnetyczne

Wynikiem jest **indukcja magnetyczna** `B` w każdym węźle modelu, w teslach,
jako wektor o trzech składowych.

Jest ona **obliczana, a nie rozwiązywana**. Gdy gęstość prądu jest znana, pole
magnetyczne w próżni wynika z niej przez całkę, więc solver nie składa żadnej
macierzy i nie wykonuje żadnej iteracji. Każdy węzeł modelu otrzymuje wartość,
niezależnie od tego, czy leży w przewodniku, na jego powierzchni czy w dowolnym
innym miejscu siatki.

### 1.2 Prawo podstawowe

W próżni statyczne pole magnetyczne spełnia prawo Ampère'a i nie ma źródeł:

```
curl(B) = u0 * J
div(B)  = 0
```

wraz z warunkiem, że `B` zanika z dala od wszystkich prądów. Jedyne pole
spełniające wszystkie trzy warunki jest dane **prawem Biota-Savarta**

```
B(x) = u0/(4*pi) * INT_V  J(y) x (x - y) / |x - y|^3  dV(y)
```

| Symbol | Znaczenie | Jednostki | Źródło |
|---|---|---|---|
| `B` | indukcja magnetyczna | `T` | obliczana |
| `J` | gęstość prądu | `A/m^2` | z zadania elektrostatyki, zob. 1.4 |
| `u0` | przenikalność magnetyczna próżni | `H/m` | wbudowana, `1.25663706212e-6` |
| `x` | punkt obliczeniowy - węzeł modelu | `m` | |
| `y` | punkt źródłowy - wewnątrz elementu przewodzącego | `m` | |

Trzy właściwości wynikają z samego prawa, a nie z działania solvera:

- **`div(B) = 0` jest spełnione dokładnie**, wszędzie, niezależnie od siatki.
- **Pole dalekie jest poprawne bez brzegu.** `B` maleje z odległością zgodnie z
  prawem; nie ma zewnętrznej powierzchni, obszaru powietrza do siatkowania ani
  warunku do ustawienia.
- **`curl(B) = u0 * J` jest spełnione tam, gdzie prąd jest zamknięty**, czyli
  tam, gdzie `div(J) = 0`, a droga prądu tworzy pętle. Punkt 1.4 wyjaśnia, co
  to oznacza dla modelu elektrostatycznego, w którym prąd wpływa do ciała i z
  niego wypływa.

Przenikalność materiału nigdzie nie występuje. Nie ma właściwości materiału
przenikalności względnej, namagnesowania ani rozróżnienia między `B` a `H` -
solver działa w próżni, niezależnie od materiału modelu. Punkt 1.9 wyjaśnia,
dlaczego nie da się tego zmienić w ramach tego samego podejścia.

**Nie jest wymagana żadna właściwość materiału.** Typ problemu magnetostatyki
nie deklaruje żadnej, a kontrola konfiguracji nie ma czego sprawdzać w jego
imieniu. Zadanie elektrostatyki, od którego zależy ten typ problemu, ma własne
trzy wymagania - zob. podręcznik elektrostatyki.

### 1.3 Dyskretyzacja

Solver elektrostatyki wyznacza gęstość prądu, która jest **stała w każdym
elemencie**. Całka z punktu 1.2 rozpada się więc na całkę dla każdego elementu,
każdą ze stałym `J_e`, które można wyciągnąć przed całkę:

```
B(x) = u0/(4*pi) * SUM_e  J_e x G_e(x)
G_e(x) = INT_e  (x - y) / |x - y|^3  dV(y)
```

`G_e` zależy tylko od geometrii elementu i od punktu obliczeniowego. Wszystkie
typy elementów akceptowane przez solver mają płaskie ściany, a dla nich `G_e`
ma postać zamkniętą, więc solver całkuje każdy element **dokładnie**, a nie w
przybliżeniu:

- **Czworościan.** Funkcja podcałkowa jest gradientem `1/|x - y|` względem `y`,
  więc całka objętościowa staje się sumą po czterech ścianach,
  `G = SUM_f n_f * INT_f 1/R dA`. Całka z `1/R` po płaskim trójkącie jest znana
  w postaci zamkniętej: logarytm wzdłuż każdej krawędzi i kąt bryłowy, pod
  jakim trójkąt jest widoczny z `x`,
  `INT 1/R dA = SUM_edges P_e * I_e - |d| * omega`.
- **Trójkąt** encji powierzchni o grubości `t`. Prąd jest prądem
  powierzchniowym `K = J*t` w `A/m`. Część styczna `G` jest ponownie sumą
  logarytmów krawędzi, `SUM_e m_e * I_e`, a część normalna jest kątem bryłowym
  ze znakiem.
- **Czworokąt.** Dzielony na dwa trójkąty.
- **Odcinek** encji linii o polu przekroju `A`. Prąd wynosi `I = A * (J . t)`
  wzdłuż odcinka, a pole prostego skończonego przewodu jest klasyczne:

  ```
  B = u0*I/(4*pi) * (a x b) * (|a| + |b|) / ( |a|*|b| * (|a|*|b| + a.b) )
  ```

  gdzie `a` i `b` to wektory od punktu obliczeniowego do obu końców.

Tutaj `I_e` jest całką z `1/R` wzdłuż krawędzi `e`, `P_e` odległością punktu
obliczeniowego od tej krawędzi w płaszczyźnie, `m_e` zewnętrzną normalną
krawędzi w płaszczyźnie, `d` wysokością punktu obliczeniowego nad płaszczyzną
ściany, a `omega` kątem bryłowym ściany widzianej z punktu obliczeniowego.

**Elementy odległe** nie wymagają postaci zamkniętej. Funkcja podcałkowa jest
tam gładka, a reguła kwadratury jest praktycznie dokładna przy ułamku kosztu:

| Odległość punktu obliczeniowego od środka elementu | Reguła |
|---|---|
| poniżej 2 x najdłuższa krawędź elementu | postać zamknięta |
| od 2 do 6 x | reguła drugiego stopnia - 3 punkty na trójkącie, 4 na czworościanie |
| powyżej 6 x | reguła punktu środkowego - element jako prąd punktowy |

Odcinki zawsze używają postaci zamkniętej, która kosztuje tyle co jeden punkt
kwadratury. Łączny błąd reguł jest mierzony w testach jednostkowych względem
postaci zamkniętej zsumowanej po wszystkich elementach całego przewodnika i we
wszystkich węzłach pozostaje poniżej `1e-4` wartości maksymalnej pola -
wewnątrz przewodnika, na jego powierzchni i poza nim.

**Pole w samym przewodniku.**

- Dla **prądu objętościowego** pole jest ciągłe i skończone wszędzie, także w
  węzłach, na krawędziach i ścianach elementów przewodzących. Węzeł na
  powierzchni litego przewodnika otrzymuje rzeczywiste pole powierzchniowe.
- Encja **linii** lub **powierzchni** reprezentuje przewodnik o zerowym
  przekroju, a pole takiego przewodnika jest osobliwe: rośnie jak `1/r` w
  kierunku przewodu i logarytmicznie w kierunku krawędzi płata, a na płacie
  doznaje skoku o `u0 * K x n`. W węźle leżącym na takim przewodniku solver
  zwraca wartość skończoną: odcinek nie wnosi nic do punktu na własnej prostej,
  płat wnosi średnią pól po obu stronach, a logarytm krawędzi płata jest
  pomijany w węźle leżącym na tej krawędzi. Wartości te są regularyzowane, a nie
  fizyczne. Pole cienkiego przewodnika odczytuj w odległości kilku rozmiarów
  przekroju, gdzie model cienki jest poprawny.

### 1.4 Źródło - gęstość prądu

Pole magnetyczne jest wyznaczane tylko na podstawie jednego wejścia:
**gęstości prądu** pozostawionej przez zadanie *Electro-statics* umieszczone
wcześniej w przebiegu zadań problemu. Nie ma nic do wprowadzenia ani
przypisania.

Solver odczytuje **elementową** gęstość prądu zapisaną przez zadanie
elektrostatyki i używa jej bez zmian - nie ma uśredniania do węzłów. Elementami
przewodzącymi prąd są:

| Encja | Warunek | Źródło |
|---|---|---|
| Objętość | czworościany | `J` |
| Powierzchnia | grubość `> 0` | prąd powierzchniowy `J * thickness` (`thickness` = grubość) |
| Linia | pole przekroju `> 0` | prąd `J * cross area` (`cross area` = pole przekroju) |

Elementy o zerowej gęstości prądu są pomijane - dotyczy to wszystkich elementów,
których zadanie elektrostatyki nie obliczało. Pomijane są także elementy, których
gęstość prądu jest mniejsza niż `1e-10` największej w modelu: obszar słabo
przewodzący - powietrze o przewodności `5e-15 S/m` obok miedzi - dodawałby
inaczej źródło na element i nic mierzalnego do pola. Jeśli żaden element nie
przewodzi prądu - zadanie elektrostatyki nie zostało uruchomione, potencjały są
równe lub żaden materiał nie ma przewodności elektrycznej - dziennik wyświetla
ostrzeżenie, a pole jest wszędzie zerowe.

**Prąd musi się zamykać.** Prawo Biota-Savarta daje pole prądu, który mu podano,
i nic więcej. Fizyczny prąd płynie w zamkniętych pętlach, ale model
elektrostatyczny zwykle zawiera tylko część pętli: pręt między dwiema
elektrodami, przy czym prąd wpływa jednym końcem, a wypływa drugim. Przewody
zasilające i reszta obwodu nie należą do modelu, podobnie jak ich pole. Solver
oblicza wtedy pole modelowanego przewodnika tak, jakby prąd pojawiał się na
jednym końcu i znikał na drugim:

- **w pobliżu przewodnika** jest to zwykle doskonałe przybliżenie, ponieważ pole
  jest tam zdominowane przez pobliski prąd;
- **z dala od niego**, w odległościach porównywalnych z samym przewodnikiem,
  brakująca część pętli ma takie samo znaczenie jak modelowana, a wynik nie jest
  polem żadnego rzeczywistego obwodu.

Gdy pole dalekie ma znaczenie, uwzględnij w modelu elektrostatycznym drogę
powrotną, aby prąd tworzył pętlę. Cienki przewodnik powrotny można odwzorować
encją linii z polem przekroju.

### 1.5 Warunki brzegowe, początkowe i otoczenia

Nie ma **żadnych** i żadne nie są potrzebne.

| Rodzaj warunku | Oferowane dla magnetostatyki |
|---|---|
| Warunki brzegowe | brak |
| Warunki początkowe | brak |
| Warunki otoczenia | brak |
| Właściwości materiału | brak |

Pole jest całkowicie określone przez prąd i przez wymaganie, aby zanikało w
dużej odległości, które zawiera w sobie prawo Biota-Savarta. Nie ma brzegu
zewnętrznego do zamknięcia ani niczego, co trzeba by na nim narzucić.

Gdy zadanie elektrostatyki i zadanie magnetostatyki są w jednym przebiegu zadań -
co zawsze ma miejsce, zob. 1.6 - karty warunków nadal oferują warunki
**elektrostatyczne**, ponieważ karty są wypełniane na podstawie typów problemów
całego przebiegu. *Electric potential* i *Charge density* należą do zadania
elektrostatyki i wpływają na pole magnetyczne przez gęstość prądu, którą
wytwarzają.

### 1.6 Sprzężenie z innymi solverami

| Kierunek | Zmienna | Skutek |
|---|---|---|
| Elektrostatyka -> magnetostatyka | Current density | jedyne źródło pola magnetycznego |

**Magnetostatyka wymaga elektrostatyki.** Zależność jest zadeklarowana w typie
problemu, a interfejs ją wymusza: zaznaczenie *Magneto-statics* w wyborze
problemu automatycznie zaznacza *Electro-statics*, a odznaczenie
*Electro-statics* odznacza również *Magneto-statics*. Domyślny przebieg zadań
umieszcza zadanie elektrostatyki jako pierwsze, czyli w kolejności wymaganej
przez sprzężenie.

Nic nie korzysta z pola magnetycznego. Jest ono końcem swojego łańcucha - żaden
solver go nie odczytuje i nie ma siły Lorentza, prądu indukowanego ani wkładu
magnetycznego do żadnej innej fizyki.

Ponieważ solver zgłasza zbieżność bezwarunkowo, grupa zadań zawierająca zadanie
magnetostatyki zawsze kończy się po jednej iteracji.

### 1.7 Obliczanie pola

Każdy element przewodzący prąd wnosi wkład do każdego węzła, więc nakład pracy
rośnie jak **liczba węzłów razy liczba elementów przewodzących prąd**. Obliczenia
są wykonywane równolegle dla węzłów, przetwarzanych w blokach, dzięki czemu
każdy element jest pobierany z pamięci raz na blok, a nie raz na węzeł.

Orientacyjnie: pręt złożony ze `120 000` czworościanów obliczany w swoich
`24 000` węzłach zajmuje około pięciu sekund na 14 wątkach. Podwojenie gęstości
siatki w każdym kierunku zwiększa ośmiokrotnie liczbę węzłów i liczbę elementów,
a czas około sześćdziesięciu czterech razy; model z kilkuset tysiącami
elementów wymaga minut, a nie sekund.

Liczą się tylko elementy przewodzące prąd. Duża siatka wokół małego przewodnika -
obszar powietrza, zob. 2.8 - dodaje węzły, ale nie źródła.

Konfiguracja solvera macierzowego nie jest używana.

### 1.8 Wyniki

| Wynik | Dotyczy | Znaczenie |
|---|---|---|
| Magnetic field `[T]` | węzeł | wektor pola |

To całe wyjście. `process()` nic nie robi, więc nie ma wielkości pochodnych -
oddzielnie zapisanego modułu pola, strumienia ani gęstości energii. Widok 3D może
w zwykły sposób wyświetlać moduł i składowe zmiennej wektorowej, a do
wyświetlania składowych dostępne są `Magnetic field in X direction` i jej
odpowiedniki `Y` oraz `Z`.

Pole magnetyczne jest zapisywane jako zmienna **węzłowa**, w przeciwieństwie do
wyników pola elektrostatycznego, które są zmiennymi elementowymi. **Każdy węzeł**
modelu ma wartość, także węzły encji, które nie przewodzą prądu, więc siatka
wokół przewodnika pokazuje pole w otaczającej go przestrzeni.

Po zapisaniu rekordu dziennik solvera wyświetla statystyki pola magnetycznego i
wartości w ewentualnych punktach monitorowania.

### 1.9 Czym solver jest, a czym nie jest

**Jest całką, a nie równaniem pola.** Obliczenie prawa Biota-Savarta nie wymaga
brzegu ani siatki poza przewodnikiem i jest dokładne dla zadanego prądu. Ceną
jest to, że działa ono tylko w ośrodku **jednorodnym**. W obecności materiałów
magnetycznych pole prądu zależy od położenia żelaza i żadna całka z samego prądu
nie może tego wyrazić; taki problem rozwiązuje się w sformułowaniu
potencjałowym - zwykle z magnetycznym potencjałem wektorowym na elementach
krawędziowych - co jest innym solverem, a nie rozszerzeniem tego.

**Jest napędzany obliczonym prądem przewodzenia.** Nie ma możliwości
bezpośredniego zadania prądu, magnesu trwałego ani narzuconego pola; prąd musi
zostać wyznaczony przez rozwiązanie elektrostatyczne na przewodzącym ciele, a
jego dokładność określa dokładność wyniku magnetycznego. Podręcznik
elektrostatyki opisuje, czym ta gęstość prądu jest, a czym nie jest. Gęstość
prądu stała w każdym elemencie jest przybliżeniem pierwszego rzędu rzeczywistej
i jest zbieżna przy zagęszczaniu siatki.

**Oblicza pole modelowanego prądu.** Zob. 1.4 - prąd powinien zamykać się w
modelu, gdy ma znaczenie pole z dala od przewodnika.

### 1.10 Analiza statyczna i solver czasowy

Magnetostatyka jest zadeklarowana jako typ problemu **bez** solvera czasowego,
dokładnie tak jak elektrostatyka. Grupa *Solver czasowy* nie pojawia się na
karcie `Problem`, obliczenia tworzą jeden rekord i nie ma nic do
konfigurowania.

Ponieważ przebieg magnetostatyki zawsze zawiera także zadanie elektrostatyki, a
żadne z nich nie obsługuje solvera czasowego, dodanie któregokolwiek z nich do
przebiegu usuwa solver czasowy z całego przebiegu. Zadanie cieplne umieszczone
za nimi może być tylko ustalone. Podręcznik elektrostatyki omawia to we własnym
punkcie 1.10.

---

## 2. Interfejs graficzny

Wszystko, co istotne, znajduje się w panelu **Solver** (karty `Problem`,
`Materiał`, `Wyniki`), menu **Problem** i panelu **Model**. Karty warunków nie
zawierają niczego dla tego typu problemu.

### 2.1 Wybór problemu

`Problem` -> `Przebieg zadań problemów` (`Ctrl+P`) otwiera okno przebiegu zadań
problemu. Dodaj zadanie i wybierz **Magneto-statics** - „Calculate magnetic
field” (oblicz pole magnetyczne).

**Electro-statics zostanie zaznaczone automatycznie.** Wybór wymusza zależność z
punktu 1.6 w obu kierunkach: zaznaczenie *Magneto-statics* zaznacza
*Electro-statics*, a odznaczenie *Electro-statics* odznacza *Magneto-statics*.
Domyślny przebieg zadań umieszcza zadanie elektrostatyki jako pierwsze.

Magnetostatyka łączy się z każdą inną fizyką z wyjątkiem *Modal analysis*. W
praktyce jedynym sensownym łańcuchem jest ten, na którym się opiera:

- *Electro-statics*, a następnie *Magneto-statics* - pole magnetyczne obliczonego
  prądu przewodzenia.

### 2.2 Karta Problem

Dla przebiegu elektrostatyka plus magnetostatyka ta karta jest **pusta**. Żaden
z tych typów problemów nie ma solvera czasowego ani własnej grupy ustawień.

### 2.3 Karta Materiał

Magnetostatyka **nic** nie wymaga od materiału. Zadanie elektrostatyki, od
którego zależy, wymaga trzech właściwości - przenikalności względnej,
przewodności elektrycznej i gęstości - a encja, której materiałowi brakuje
którejkolwiek z nich, jest pomijana przez *to* zadanie, co pozostawia ją bez
gęstości prądu, a więc bez wkładu do pola magnetycznego.

Materiał ma więc nadal znaczenie, wyłącznie przez elektrostatyczną połowę
łańcucha. Przypisz materiał ze wszystkimi trzema właściwościami, taki jak
dostarczony *Copper* (miedź), i sprawdź w podręczniku elektrostatyki, do czego
każda z nich służy.

Nie ma właściwości przenikalności względnej do przypisania. Solver działa w
próżni.

### 2.4 Karta Warunki brzegowe

Nic na tej karcie nie należy do zadania magnetostatyki. Oferowane warunki -
*Electric potential* i *Charge density* - są elektrostatyczne i wyznaczają
gęstość prądu, która z kolei wytwarza pole magnetyczne. Przypisz je zgodnie z
opisem w podręczniku elektrostatyki.

Żaden warunek brzegowy magnetostatyki nie istnieje ani nie jest potrzebny - zob.
1.5.

### 2.5 Warunki początkowe i otoczenia

Żadna z tych kart nie oferuje niczego dla magnetostatyki. Wyświetlane warunek
początkowy *Electric potential* i warunek otoczenia *Temperature* należą do
zadania elektrostatyki.

### 2.6 Konfiguracja solvera macierzowego

Solver magnetostatyki nie rozwiązuje żadnego układu równań i nie korzysta z
konfiguracji solvera macierzowego. Pozycja **CG** konfiguruje tylko solver
elektrostatyki.

### 2.7 Punkty monitorowania

`Problem` -> `Zdefiniuj punkty monitorowania` umieszcza sondy w podanych
współrzędnych i wybiera rejestrowaną zmienną. *Magnetic field* jest dostępne na
liście razem ze zmiennymi elektrostatycznymi i odczytuje pole węzłowe w miejscu
sondy.

### 2.8 Wyniki i rekordy

Karta `Wyniki` wyświetla **Magnetic field** jako wektor w węzłach obok wyników
elektrostatycznych wytworzonych przez pierwsze zadanie. Karta `Rekordy` panelu
`Model` zawiera dokładnie jeden rekord: oba typy problemów są statyczne.

`Raport` -> `Plik dziennika solvera` wyświetla pełne wyjście: najpierw statystyki
elektrostatyki, następnie liczbę elementów przewodzących prąd i węzłów, dla
których obliczono całkę Biota-Savarta, a potem statystyki pola magnetycznego.

**Wyświetlanie pola wokół przewodnika.** Pole jest obliczane w każdym węźle, więc
pokazuje je wszystko, co zostało siatkowane wokół przewodnika. Zasiatkuj
otaczającą przestrzeń jako oddzielną encję objętości - obszar powietrza - i
nadaj jej materiał bez przewodności elektrycznej: zadanie elektrostatyki ją
pominie, więc nie będzie przewodzić prądu, ale jej węzły otrzymają pole
magnetyczne. Obszar kosztuje węzły, ale nie źródła (1.7), i nie wymaga
szczególnego rozmiaru ani brzegu, ponieważ nic nie jest na nim rozwiązywane.

Dostarczony materiał *Air* ma przewodność elektryczną, choć bardzo małą, więc
zadanie elektrostatyki uwzględniłoby ten obszar we własnym rozwiązaniu. Użyj
kopii tego materiału z usuniętą właściwością przewodności elektrycznej. Kontrola
konfiguracji ostrzeże wtedy, że materiałowi encji brakuje wymaganych
właściwości; dla obszaru powietrza jest to oczekiwane i obliczenia są
kontynuowane.

---

## 3. Samouczek - pole przewodnika z prądem

**Cel.** Wykonać od początku do końca łańcuch elektrostatyka i magnetostatyka dla
miedzianego pręta z prądem, sprawdzić pole magnetyczne obliczeniem ręcznym i
zobaczyć, gdzie ujawniają się przybliżenia z punktu 1.4.

Ten samouczek zakłada zasiatkowany pręt z podręcznika elektrostatyki -
prostopadłościan o długości `0.1 m` i kwadratowym przekroju `0.01 m`, podzielony
na czworościany, z dwiema ścianami końcowymi oznaczonymi jako oddzielne encje
powierzchni. Jeśli ten model jest nadal skonfigurowany, zmienia się tylko
przebieg zadań.

### Krok 1 - wybierz problemy

1. `Problem` -> `Przebieg zadań problemów` (`Ctrl+P`).
2. Zaznacz **Magneto-statics**. Zobacz, jak **Electro-statics** zaznacza się
   samo.
3. Zatwierdź przyciskiem `OK`.

Przebieg zadań zawiera teraz dwa zadania, z elektrostatyką jako pierwszą. Karta
`Problem` pozostaje pusta.

### Krok 2 - przypisz materiał

Wybierz encję objętości i przypisz **Copper** na karcie `Materiał` lub materiał
z trzema właściwościami potrzebnymi zadaniu elektrostatyki:

- **Relative permittivity** (przenikalność względna) = `2.8`
- **Electrical conductivity** (przewodność elektryczna) = `5.882e7` `S/m`
- **Density** (gęstość) = `8960` `kg/m^3`

Zadanie magnetostatyki nie odczytuje żadnej z nich. Potrzebuje ich zadanie
elektrostatyki, a bez gęstości prądu nie ma pola magnetycznego.

### Krok 3 - wymuś przepływ prądu

1. Wybierz encję powierzchni na jednym końcu, otwórz kartę `Warunki brzegowe` i
   zaznacz **Electric potential**. Ustaw go na `0` V.
2. Wybierz encję powierzchni na drugim końcu, zaznacz **Electric potential** i
   ustaw go na `0.01` V.

Prąd płynący przez pręt wynosi

```
I = sigma * dV * A / L = 5.882e7 * 0.01 * 1e-4 / 0.1 = 588 A
```

Pręt bez różnicy potencjałów nie przewodzi prądu i daje pole magnetyczne
dokładnie równe zeru - to pierwsza rzecz do sprawdzenia, jeśli wynik jest
pusty.

### Krok 4 - rozwiąż

1. `Rozwiązanie` -> `Uruchom solver` (`Ctrl+R`).
2. Śledź postęp w `Raport` -> `Plik dziennika solvera`.

Dziennik pokazuje zadanie elektrostatyki z jego zmiennymi, a następnie zadanie
magnetostatyki: liczbę elementów przewodzących prąd, liczbę węzłów i statystyki
pola magnetycznego. Oba są wykonywane raz.

### Krok 5 - przejrzyj wynik

- Karta `Wyniki`: wyświetl **Magnetic field**. Jest to wektor w węzłach, więc
  widok 3D oferuje jego moduł i trzy składowe.
- Pole **krąży wokół pręta**, prostopadle do prądu, zgodnie z regułą prawej
  dłoni.
- Jest **zerowe na osi** pręta i rośnie w kierunku powierzchni, gdzie jest
  największe - pośrodku każdej dłuższej ściany, a nie w narożnikach.
- Jest **słabsze w pobliżu końców** pręta niż w jego środku. To otwarta droga
  prądu z punktu 1.4: w pobliżu końca brakuje połowy tego, co byłoby dłuższym
  przewodnikiem.

### Krok 6 - sprawdź rząd wielkości

Dla okrągłego przewodnika o promieniu `R` pole na powierzchni wynosi
`u0*I/(2*pi*R)`. Pręt kwadratowy o boku `a` nie jest okrągły, ale środek ściany
w odległości `a/2` od osi jest temu bliski: przy `a/2 = 0.005 m`

```
u0*I/(2*pi*r) = 1.2566e-6 * 588 / (2*pi*0.005) = 0.0235 T
```

Obliczone pole pośrodku dłuższej ściany, w połowie długości pręta, wynosi około
`0.020 T`, czyli około 15 % mniej - kwadratowy przekrój umieszcza znaczną część
prądu dalej od punktu niż przekrój okrągły. W narożniku, w odległości
`a/sqrt(2)` od osi, jest jeszcze mniejsze, około `0.019 T`.

Dokładniejsza kontrola wymaga punktu obliczeniowego z dala od pręta, gdzie
przekrój nie ma już znaczenia. Dodaj obszar powietrza zgodnie z opisem w 2.8 i
odczytaj pole w odległości `r = 0.02 m` od osi, w połowie długości. Dla prostego
skończonego przewodnika o długości `L`

```
B = u0*I/(4*pi*r) * 2 * (L/2) / sqrt( (L/2)^2 + r^2 )
  = 1e-7 * 588 / 0.02 * 2 * 0.05 / sqrt(0.0025 + 0.0004) = 5.46e-3 T
```

a obliczona wartość powinna zgadzać się z dokładnością do około `0.1 %`, przy
czym różnica wynika z kwadratowego przekroju pręta. Ta wartość jest polem
samego pręta, jak wyjaśnia 1.4 - rzeczywisty obwód dodałby pole swoich
przewodów.

### Rozwiązywanie problemów

| Objaw | Przyczyna |
|---|---|
| Pole magnetyczne jest wszędzie zerowe, a dziennik ostrzega, że żaden element nie przewodzi prądu | brak gęstości prądu - sprawdź, czy zadanie elektrostatyki zostało uruchomione jako pierwsze, czy potencjały się różnią i czy materiał ma przewodność elektryczną |
| **Magneto-statics** nie da się odznaczyć samodzielnie | oczekiwane - wymaga *Electro-statics*, a odznaczenie go odznacza oba |
| Brak grupy *Solver czasowy* | oczekiwane - żaden z typów problemów nie ma solvera czasowego, zob. 1.10 |
| Pole na końcach przewodnika jest słabsze | oczekiwane - droga prądu jest otwarta, zob. 1.4 |
| Pole z dala od przewodnika nie odpowiada obwodowi | przewody i reszta obwodu nie należą do modelu, zob. 1.4 |
| Pole na przewodniku liniowym lub cienkim powierzchniowym jest nieregularne | oczekiwane - pole przewodnika o zerowym przekroju jest tam osobliwe, zob. 1.3 |
| Obliczenia trwają długo | koszt to liczba węzłów razy liczba elementów przewodzących prąd, zob. 1.7 |

---

## 4. Sprawdzanie modelu

**Przewodnik prostoliniowy.** Dla długiego przewodnika o promieniu `R`, przez
który równomiernie płynie całkowity prąd `I`, pole jest obwodowe i

```
outside, r > R    B = u0 * I / ( 2 * pi * r )
inside,  r < R    B = u0 * I * r / ( 2 * pi * R^2 )
```

(`outside` = na zewnątrz, `inside` = wewnątrz), przy `u0 = 1.2566e-6`. Pole jest
największe na powierzchni przewodnika, `B = u0*I/(2*pi*R)`, i maleje poza nim
jak `1/r`. Dla prądu `1 A` w przewodniku o promieniu `5 mm` to maksimum wynosi
`4e-5 T`, mniej więcej tyle co pole ziemskie.

**Przewodnik skończony.** Dla prostego przewodnika o długości `L`, w odległości
`r` od jego osi i w połowie długości,

```
B = u0 * I / ( 4 * pi * r ) * L / sqrt( (L/2)^2 + r^2 )
```

co przy rosnącym `L` dąży do wzoru dla długiego przewodnika. Z dala od
przewodnika kształt przekroju przestaje mieć znaczenie, więc każdy przekrój można
sprawdzić tym wzorem.

**Przewód.** Encja linii z polem przekroju przewodzi prąd wzdłuż odcinków, a
solver używa dla każdego z nich postaci zamkniętej dla prostego przewodu. Prosta
encja linii odtwarza więc wzór dla przewodnika skończonego dokładnie, z
dokładnością do zaokrągleń, w dowolnej odległości.

**Solenoid.** Dla długiego solenoidu o `n` zwojach na metr, przez który płynie
`I`, pole wewnątrz jest jednorodne i skierowane wzdłuż osi,

```
B = u0 * n * I
```

a na zewnątrz bliskie zeru. Uzwojenie można zamodelować jako encję linii
podążającą za zwojami, z polem przekroju drutu, a zadanie elektrostatyki wymusza
przepływ prądu przez nie na podstawie potencjałów na jego dwóch końcach.

**Dywergencja.** Dla każdego pola magnetycznego `div(B) = 0` wszędzie. Prawo
Biota-Savarta spełnia to dokładnie, więc nie testuje to tego solvera; testowałoby
jedynie interpolację między węzłami.

**Zbieżność z siatką.** Samo całkowanie jest dokładne (1.3); od siatki zależy
gęstość prądu, którą zadanie elektrostatyki oblicza jako stałą w każdym
elemencie. Zagęszczenie siatki w przewodniku i porównanie pola w tym samym punkcie
jest kontrolą tej części.

---

## 5. Ograniczenia

### Ograniczenia modelowania

- **Tylko próżnia.** W sformułowaniu nie występuje żadna przenikalność. Nie ma
  właściwości materiału przenikalności względnej, rozróżnienia między `B` a `H`,
  namagnesowania ani nasycenia, więc żelaza, ferrytów i ekranów nie da się w ogóle
  odwzorować. Zob. 1.9, dlaczego wymaga to innego solvera.
- **Tylko pole modelowanego prądu.** Pole przewodów zasilających i reszty obwodu
  nie jest uwzględniane. Gdy droga prądu w modelu jest otwarta, pole z dala od
  przewodnika nie jest polem rzeczywistego obwodu - zob. 1.4.
- **Brak magnesów trwałych i narzuconego pola.** Jedynym źródłem jest prąd
  przewodzenia obliczony przez zadanie elektrostatyki. Magnes, pole tła i
  wymuszenie zewnętrzne nie mają reprezentacji.
- **Prądu nie można zadać bezpośrednio.** Prąd jest wyznaczany przez
  rozwiązanie elektrostatyczne na przewodzącym ciele. Cewkę można zamodelować
  jako encję linii podążającą za uzwojeniem, ale jej prąd nadal ustala się przez
  potencjały.
- **Cienkie przewodniki są osobliwe.** Pole w węźle na encji linii lub na
  krawędzi encji powierzchni jest wartością regularyzowaną, a nie fizyczną - zob.
  1.3.
- **Typy elementów.** Prąd przewodzą czworościany, trójkąty, czworokąty i
  dwuwęzłowe odcinki; inne typy elementów nie wnoszą niczego.
- **Koszt.** Nakład pracy rośnie jak liczba węzłów razy liczba elementów
  przewodzących prąd - zob. 1.7.
- **Brak wyników pochodnych.** `process()` nic nie oblicza, więc wektor pola
  magnetycznego jest całym wyjściem. Nie ma strumienia, energii, siły ani modułu
  pola zapisanego jako osobna zmienna.
- **Nic nie korzysta z wyniku.** Żaden inny solver nie odczytuje pola
  magnetycznego. Nie ma siły Lorentza działającej na przewodnik, prądu
  indukowanego ani sprzężenia zwrotnego z fizyką elektrostatyczną, cieplną lub
  strukturalną.
- **Tylko statyka.** Nie ma solvera czasowego, a dodanie zadania magnetostatyki -
  które wnosi ze sobą zadanie elektrostatyki - usuwa solver czasowy z całego
  przebiegu zadań, więc każda inna fizyka połączona za nim jest również
  ustalona.

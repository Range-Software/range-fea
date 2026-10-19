# Przepływ przez ośrodki porowate - Podręcznik teoretyczny i przewodnik użytkownika

Ten dokument opisuje solver przepływu przez ośrodki porowate (`RSolverPotential`)
w Range FEA, który obsługuje typ problemu **Przepływ przez ośrodki porowate**
(*Flow through porous media*): równania, które rozwiązuje, znaczenie każdej
przyjmowanej danej, części graficznego interfejsu, które nim sterują, oraz dwa
szczegółowo omówione samouczki.

**Spis treści**

1. [Podstawy teoretyczne](#1-podstawy-teoretyczne)
2. [Graficzny interfejs użytkownika](#2-graficzny-interfejs-użytkownika)
3. [Samouczek - przepływ wymuszony ciśnieniem przez filtr piaskowy](#3-samouczek---przepływ-wymuszony-ciśnieniem-przez-filtr-piaskowy)
4. [Samouczek - wznoszące przesiąkanie przez pionową kolumnę](#4-samouczek---wznoszące-przesiąkanie-przez-pionową-kolumnę)
5. [Sprawdzanie modelu](#5-sprawdzanie-modelu)
6. [Ograniczenia](#6-ograniczenia)

---

## 1. Podstawy teoretyczne

### 1.1 Główna niewiadoma - pole ciśnienia

Główną niewiadomą jest **ciśnienie** `p` płynu porowego w każdym węźle, w
paskalach. Każdy węzeł ma jeden stopień swobody, więc siatka z `N` węzłami daje
układ `N` równań - równie tani jak model cieplny lub elektrostatyczny na tej samej
siatce. Prędkość płynu jest odtwarzana z pola ciśnienia po rozwiązaniu.

Pole ciśnienia opisane tym równaniem jest określone tylko **z dokładnością do
stałej addytywnej**: dodanie tej samej wartości do ciśnienia wszędzie nie zmienia
żadnej prędkości. Każdy model potrzebuje więc co najmniej jednego warunku
brzegowego *Pressure (explicit)*, zob. 1.7. To, czy ciśnienie traktuje się jako
bezwzględne, czy nadciśnienie, zależy od użytkownika - solver pracuje wyłącznie z
jego gradientem.

### 1.2 Równania podstawowe

Solver implementuje **prawo Darcy'ego** dla powolnego przepływu jednego
nieściśliwego płynu wypełniającego pory sztywnego ciała porowatego:

```
v = -( k / mu ) * ( grad(p) - rho * g )
```

wraz z zasadą zachowania masy płynu nieściśliwego

```
div( v ) = 0
```

co razem daje równanie potencjału (typu Laplace'a) dla ciśnienia:

```
div( ( k / mu ) * ( grad(p) - rho * g ) ) = 0
```

| Symbol | Znaczenie | Jednostki | Źródło |
|---|---|---|---|
| `p` | ciśnienie płynu porowego | `Pa` | rozwiązywane |
| `v` | prędkość Darcy'ego (prędkość filtracji) | `m/s` | odtwarzana, zob. 1.8 |
| `k` | przepuszczalność właściwa ciała porowatego | `m^2` | właściwość materiału *Permeability to fluids* |
| `mu` | lepkość dynamiczna płynu porowego | `kg/(m*s)` | właściwość materiału *Dynamic viscosity* |
| `rho` | gęstość płynu porowego | `kg/m^3` | właściwość materiału *Density* |
| `g` | przyspieszenie ziemskie | `m/s^2` | warunek otoczenia, zob. 1.6 |

Stosunek `k/mu` pełni rolę, jaką w modelu cieplnym pełni przewodność. Bez
grawitacji równanie jest równaniem Laplace'a dla ciśnienia, a przepływ biegnie
wprost zgodnie ze spadkiem ciśnienia. Z grawitacją człon `rho*g` sprawia, że płyn w
spoczynku przenosi hydrostatyczny gradient ciśnienia: `grad(p) = rho*g` daje
`v = 0`.

**Prędkość jest prędkością Darcy'ego.** Jest to objętościowe natężenie przepływu na
jednostkę **całkowitego** przekroju, fazy stałej i porów razem - a nie prędkość
płynu wewnątrz porów. Średnia prędkość w porach jest większa o odwrotność
porowatości `n`:

```
v_pore = v / n
```

Porowatość nie występuje w przepływie ustalonym, więc solver jej nie potrzebuje i
nie przyjmuje. Użyj jej ręcznie do oszacowania czasu przepływu.

**Przepuszczalność i współczynnik filtracji.** Mechanika gruntów i hydrogeologia
często podają zamiast przepuszczalności właściwej *współczynnik filtracji* `K` w
`m/s` - współczynnik prawa Darcy'ego zapisanego dla wysokości hydraulicznej. Obie
wielkości wiążą właściwości płynu:

```
k = K * mu / ( rho * |g| )
```

Dla wody w 20 °C wartość `K = 1e-4 m/s` odpowiada `k = 1.02e-11 m^2`. Inżynieria
naftowa podaje przepuszczalność w darcy: `1 D = 9.869e-13 m^2`.

**Wymagane właściwości materiału.** Problem przepływu przez ośrodki porowate
wymaga **trzech** z nich:

| Właściwość | Jednostki | Zastosowanie |
|---|---|---|
| Permeability to fluids | `m^2` | macierz sztywności - przepuszczalność fazy stałej |
| Dynamic viscosity | `kg/(m*s)` | macierz sztywności - lepkość **płynu** |
| Density | `kg/m^3` | człon grawitacyjny - gęstość **płynu** |

**Gęstość i lepkość dotyczą płynu porowego, nie ciała porowatego.** Materiał dla
tego typu problemu opisuje więc ośrodek *nasycony*: przepuszczalność szkieletu i
gęstość oraz lepkość płynu, który go wypełnia. Tak zbudowane są dostarczane
materiały o nazwie *... (water saturated)* (zob. 2.3). Materiał podający gęstość
objętościową suchego gruntu zafałszuje ciśnienie hydrostatyczne w stosunku obu
gęstości.

Encja, której materiałowi brakuje którejkolwiek z trzech właściwości, **nie jest
rozwiązywana**. Kontrola ustawień ostrzega o tym przed uruchomieniem obliczeń.
Jedynym wyjątkiem jest encja z warunkiem brzegowym tego typu problemu: zostaje
włączona do układu niezależnie od materiału, a jej przepuszczalność jest wtedy po
prostu zerowa. Tak dopływ otrzymuje również powierzchnia brzegowa bez materiału.

Każda właściwość jest tabelą w funkcji temperatury. Zadanie przepływu przez ośrodki
porowate samo w sobie nie ma pola temperatury, więc każda tabela jest obliczana w
`293.15 K`. Tabele są uwzględniane tylko wtedy, gdy zadanie *Heat transfer*
umieszczone wcześniej w tym samym przebiegu zadań obliczyło temperatury elementów -
tak cieplejszy płyn o mniejszej lepkości płynie szybciej.

### 1.3 Dyskretyzacja metodą elementów skończonych

Słabe sformułowanie równania ciśnienia daje dla każdego elementu

```
K = INT( B^T * ( k / mu ) * B )                  sztywność
f = INT( B^T * ( k / mu ) * rho * g )            obciążenie grawitacyjne
  + INT_S( N^T * v_in )                          dopływ przez brzeg
```

gdzie `B` jest budowana z pochodnych funkcji kształtu, `N` to funkcje kształtu, a
`v_in` jest prędkością dopływu zadaną na powierzchni brzegowej. Każda macierz
elementu ma wymiar `n x n` dla elementu o `n` węzłach. Nie ma macierzy masy ani
retencji: przepływ jest ustalony, zob. 1.10.

**Miary elementów.** Jak wszędzie w Range FEA, encje niższego wymiaru otrzymują
miarę geometryczną, aby mogły reprezentować rzeczywiste ciała:

| Encja | Miara | Gdzie się ją ustawia |
|---|---|---|
| Objętość | 1 | - |
| Powierzchnia | grubość powierzchni | właściwości geometryczne encji |
| Linia | przekrój linii | właściwości geometryczne encji |
| Punkt | - | punkty nie przewodzą przepływu |

**Powierzchnia** z grubością przewodzi przepływ we własnej płaszczyźnie - tak
buduje się model dwuwymiarowy (przekrój zapory lub warstwy wodonośnej). Jej
obciążenie grawitacyjne korzysta ze składowej `g` leżącej w płaszczyźnie
powierzchni. Powierzchnia o **zerowej grubości** - zwykła powierzchnia brzegowa
modelu objętościowego - nie ma sztywności i jedynie przyjmuje dopływ.

**Linia** z przekrojem przewodzi przepływ wzdłuż swojego kierunku, dla
jednowymiarowego modelu kolumny lub rury z wypełnieniem. **Punkt** nic nie wnosi.
Element punktowy, którego węzeł nie należy do żadnego innego obliczanego elementu,
zostawia pusty wiersz w macierzy - nie przypisuj materiału porowatego encjom
punktowym.

### 1.4 Warunki brzegowe

| Warunek brzegowy | Typ | Dotyczy | Składowe |
|---|---|---|---|
| Pressure (explicit) | jawny (Dirichleta) | powierzchnia | Pressure `[Pa]` |
| Velocity (inflow) | naturalny (zadany strumień) | powierzchnia | Velocity `[m/s]` |
| Volumetric flow rate (inflow) | naturalny (zadany strumień) | powierzchnia | Volumetric flow rate `[m^3/s]` |

Powierzchnia brzegowa bez żadnego z nich jest **nieprzepuszczalna**: żaden płyn
przez nią nie przechodzi. To naturalny warunek równania, więc ściany, spąg warstwy
wodonośnej i płaszczyzny symetrii nie potrzebują żadnego warunku.

**Pressure (explicit)** jest jedynym warunkiem jawnym. Każdy węzeł powierzchni jest
usuwany z układu równań i utrzymywany na zadanym ciśnieniu. To miejsce, w którym
płyn opuszcza model (wylot, drenaż, swobodna woda zbiornika) lub wchodzi do niego
pod znanym ciśnieniem. Każdy model potrzebuje co najmniej jednego. Nowo przypisany
warunek zaczyna od `1000 Pa`.

**Velocity (inflow)** zadaje normalną prędkość Darcy'ego przez powierzchnię.
Wartość **dodatnia** oznacza dopływ do ciała porowatego, **ujemna** odpływ (pobór).
Wartość jest strumieniem na jednostkę pola, więc całkowite natężenie przepływu
rośnie z polem powierzchni.

**Volumetric flow rate (inflow)** zadaje **całkowite** natężenie przepływu przez
powierzchnię w `m^3/s`. Solver dzieli je przez pole całej encji i stosuje wynik
jako równomierną prędkość dopływu, więc natężenie nie zmienia się przy
przesiatkowaniu lub powiększeniu powierzchni. Wartość dodatnia znów oznacza
dopływ.

Żaden z warunków dopływu nie ustala ciśnienia: ciśnienie na wlocie jest częścią
wyniku. Tam, gdzie powierzchnia dopływu i powierzchnia z *Pressure (explicit)*
mają wspólne węzły, w tych węzłach wygrywa ciśnienie.

Powierzchnia dopływu **nie potrzebuje materiału ani grubości**: sam warunek włącza
ją do układu. Jeżeli powierzchnia ma materiał porowaty i grubość, przewodzi
dodatkowo przepływ we własnej płaszczyźnie.

Warunek *Pressure (implicit)* solvera przepływu nieściśliwego nie jest oferowany
dla tego typu problemu.

Wszystkie składowe przyjmują tabelę w funkcji czasu, która działa przy włączonym
solverze czasowym, zob. 1.10.

### 1.5 Grawitacja

Grawitacja wchodzi przez warunek otoczenia *Gravitational acceleration*. **Każda
encja ma go domyślnie z `g = (0, 0, -9.80665) m/s^2`**, więc grawitacja jest
skierowana wzdłuż ujemnej osi Z, o ile się jej nie zmieni. Warto znać dwie
konsekwencje:

- W modelu, którego oś Z jest skierowana w górę, płyn w spoczynku automatycznie
  otrzymuje ciśnienie hydrostatyczne rosnące o `rho*|g|` na metr głębokości.
  Zwykle o to właśnie chodzi.
- Model zbudowany z inną osią skierowaną w górę musi mieć warunek zmieniony na
  każdej encji porowatej, w przeciwnym razie płyn „spada" w bok. Ustaw wszystkie
  trzy składowe na zero, aby wyłączyć grawitację - na przykład w modelu poziomym,
  w którym liczy się tylko różnica ciśnień.

Grawitacja w połączeniu z powierzchnią *Pressure (explicit)*, która nie jest
pozioma, wymaga uwagi: równomierne ciśnienie na pionowej ścianie **nie jest**
hydrostatyczne, więc przy tej ścianie płyn krąży. Zadaj ciśnienie w funkcji
wysokości (podziel ścianę na kilka encji) albo wyłącz grawitację, jeśli nie jest
częścią zagadnienia.

### 1.6 Warunki otoczenia, początkowe i sprzężenia

#### Warunki otoczenia

| Warunek otoczenia | Składowe | Działanie |
|---|---|---|
| Gravitational acceleration | g w X, Y, Z `[m/s^2]` | człon grawitacyjny prawa Darcy'ego, zob. 1.5 |
| Temperature | Temperature `[K]` | oferowany, ale nieodczytywany - zob. niżej |

Warunek otoczenia *Temperature* jest oferowany, ponieważ właściwości materiałów są
tabelami w funkcji temperatury. Solver go **nie odczytuje**: temperatury elementów
pobiera wyłącznie z danych współdzielonych pozostawionych przez zadanie
*Heat transfer*, a w przeciwnym razie oblicza każdą tabelę w `293.15 K`.

#### Warunki początkowe

| Warunek początkowy | Działanie |
|---|---|
| Pressure | oferowany, nieodczytywany |
| Velocity | oferowany, nieodczytywany |

Przepływ jest ustalony, więc wynik nie zależy od pola początkowego i solver nie
odczytuje żadnego z warunków początkowych. Oba są oferowane, ponieważ typ problemu
dzieli je z problemem przepływu nieściśliwego.

#### Sprzężenie z innymi solverami

| Kierunek | Zmienna | Działanie |
|---|---|---|
| Ciepło -> przepływ przez ośrodki porowate | Temperature | wybiera wiersz każdej zależnej od temperatury tabeli materiału |

Ciśnienie i prędkość nie są wykorzystywane przez żaden inny solver. Przepływ przez
ośrodki porowate można łączyć z każdą inną fizyką oprócz *Incompressible viscous
flow* i *Modal analysis*.

Solver bezwarunkowo zgłasza zbieżność, więc grupa zadań zawierająca tylko zadanie
przepływu przez ośrodki porowate zawsze kończy się po jednej iteracji.

### 1.7 Rozwiązywanie układu

Solver składa i rozwiązuje

```
K * p = f
```

`K` jest symetryczna i dodatnio określona, gdy tylko zadane jest co najmniej jedno
ciśnienie, dlatego rozwiązuje się ją metodą **gradientów sprzężonych** z
prekondycjonowaniem Jacobiego. Składanie jest zrównoleglone po elementach z
buforami dla każdego wątku, które na końcu są jednokrotnie scalane, więc wynik nie
zależy od liczby wątków.

Przepuszczalności rzeczywistych materiałów sięgają od około `1e-9 m^2` (żwir) do
`1e-20 m^2` (nienaruszony ił), co zbliżyłoby wyrazy macierzy do granic zaokrągleń
solvera iteracyjnego. Przed rozwiązaniem każde równanie jest więc dzielone przez
największe `k/mu` w modelu. Podzielenie wszystkich równań przez tę samą liczbę nie
zmienia rozwiązania; jedynie utrzymuje dobre uwarunkowanie iteracji.

Model **bez warunku brzegowego Pressure (explicit)** nie ma jednoznacznego
rozwiązania. Solver to wykrywa i zatrzymuje się z błędem *Flow through porous
media requires at least one 'Pressure (explicit)' boundary condition*.

Inne sposoby dojścia do osobliwego lub pustego układu, wszystkie bez komunikatu:

- encja, której materiałowi brakuje jednej z trzech wymaganych właściwości, nie
  jest rozwiązywana, a jej węzły wypadają z układu;
- encja o zerowej przepuszczalności lub zerowej lepkości nie ma sztywności, więc
  jej węzły pozostają w układzie, ale nie są z niczym połączone;
- powierzchnia o zerowej grubości lub linia o zerowym przekroju nie przewodzi
  przepływu.

### 1.8 Wyniki pochodne

Po rozwiązaniu prędkość elementu jest odtwarzana z węzłowego pola ciśnienia.
Gradient ciśnienia jest uśredniany po punktach całkowania elementu i łączony z
członem grawitacyjnym; dla elementów liniowych i powierzchniowych jest obracany do
współrzędnych globalnych:

```
v = -( k / mu ) * ( grad(p) - rho * g )
```

| Wynik | Dotyczy | Znaczenie |
|---|---|---|
| Pressure `[Pa]` | węzeł | rozwiązane pole ciśnienia porowego |
| Velocity `[m/s]` | element | wektor prędkości Darcy'ego |

Prędkość jest **gęstością** w tym samym sensie co wyniki polowe innych solverów:
nie zależy od rozmiaru elementu, jest stała w elemencie liniowym i zbiega przy
zagęszczaniu siatki. Powierzchnie brzegowe o zerowej grubości pokazują zerową
prędkość - wyświetlaj wynik na objętości.

Całkowite natężenie przepływu nie jest zapisywane jako wynik. Oblicz je ręcznie z
prędkości i pola przekroju albo z warunków dopływu, zob. 5.

Po zapisaniu rekordu dziennik solvera wypisuje statystyki obu zmiennych i wartości
w punktach monitorowania.

### 1.9 Zakres stosowalności prawa Darcy'ego

Prawo Darcy'ego obowiązuje dla powolnego, laminarnego przepływu, w którym siły
lepkości przeważają nad bezwładnością płynu. Typowe kryterium wykorzystuje liczbę
Reynoldsa utworzoną ze średniej średnicy ziarna `d`:

```
Re = rho * |v| * d / mu  <  1 ... 10
```

Przepływ wód podziemnych w gruntach, przesiąkanie przez zapory i nasypy, przepływ
przez filtry, złoża usypane i większość porowatych materiałów technicznych
spełniają je z dużym zapasem. Przepływ przez gruby żwir, narzut kamienny lub bardzo
otwarte pianki przy dużych prędkościach nie; prawo Darcy'ego zaniża wtedy stratę
ciśnienia (zakres Forchheimera), a ten solver poda zbyt dużą prędkość.

### 1.10 Przepływ ustalony a solver czasowy

Typ problemu dopuszcza solver czasowy, ale sam przepływ jest **ustalony**: płyn i
ciało porowate są nieściśliwe i nie ma członu retencji. Przy włączonym solverze
czasowym każdy krok czasowy rozwiązuje osobny stan ustalony z wartościami warunków
w danej chwili. To właściwy model, gdy warunki brzegowe zmieniają się wolno w
porównaniu z czasem, w jakim ustala się ciśnienie - poziom zbiornika zmieniający
się w ciągu godzin nad filtrem piaskowym - i pozwala zadaniu przepływu przez
ośrodki porowate dzielić przebieg zadań z nieustalonym zadaniem cieplnym.

Konsolidacja, odwadnianie gruntu nienasyconego, przepływ gazu ściśliwego i każde
inne zjawisko, w którym ilość płynu zmagazynowanego w porach zmienia się w czasie,
leżą poza modelem, zob. rozdział 6.

---

## 2. Graficzny interfejs użytkownika

Wszystko, co istotne, znajduje się w panelu **Solver** (karty `Problem`,
`Warunki brzegowe`, `Warunki początkowe`, `Warunki otoczenia`, `Materiał`,
`Wyniki`), w menu **Problem** i w panelu **Model**.

### 2.1 Wybór problemu

`Problem` -> `Przebieg zadań problemów` (`Ctrl+P`) otwiera okno przebiegu zadań.
Dodaj zadanie i wybierz **Flow through porous media**.

Typu problemu nie można łączyć z *Incompressible viscous flow* ani z *Modal
analysis*. Zadanie *Heat transfer* umieszczone przed nim sprawia, że tabele
materiału podążają za obliczoną temperaturą.

### 2.2 Karta Problem

Karta pokazuje grupę *Solver czasowy*, ponieważ typ problemu obsługuje solver
czasowy - co oznacza tu obliczenie nieustalone, wyjaśnia 1.10. Przepływ przez
ośrodki porowate nie ma własnej grupy ustawień. Wszystko przypisuje się encjom na
kartach warunków.

### 2.3 Karta Materiał

Karta `Materiał` pokazuje materiał przypisany do wybranej encji. Przepływ przez
ośrodki porowate wymaga **wszystkich trzech** z poniższych właściwości:

| Właściwość | Jednostki | Uwagi |
|---|---|---|
| Permeability to fluids | `m^2` | przepuszczalność właściwa fazy stałej, zob. tabela niżej |
| Dynamic viscosity | `kg/(m*s)` | płynu porowego, `1.003e-3` dla wody w 20 °C |
| Density | `kg/m^3` | płynu porowego, `998.2` dla wody w 20 °C |

Dostarczana biblioteka zawiera gotowe do użycia materiały porowate nasycone wodą.
Każdy zawiera zależne od temperatury gęstość i lepkość wody oraz typową
przepuszczalność:

| Materiał | Przepuszczalność `[m^2]` | Współczynnik filtracji w 20 °C `[m/s]` |
|---|---|---|
| Gravel (water saturated) | `1e-9` | `1e-2` |
| Coarse Sand (water saturated) | `1e-10` | `1e-3` |
| Fine Sand (water saturated) | `1e-12` | `1e-5` |
| Sandstone (water saturated) | `1e-13` | `1e-6` |
| Silt (water saturated) | `1e-14` | `1e-7` |
| Clay (water saturated) | `1e-17` | `1e-10` |

Przepuszczalność rzeczywistego gruntu może różnić się od tych wartości o rząd
wielkości lub więcej - są to wartości wyjściowe, a nie dane konkretnej lokalizacji.
Aby zamodelować inny płyn, skopiuj jeden z nich i zastąp gęstość i lepkość. Zwykły
materiał *Sand* jest suchym materiałem sypkim bez lepkości, dlatego jest oznaczony
jako nieprawidłowy dla tego typu problemu.

### 2.4 Karta Warunki brzegowe

Karta jest podzielona na listę dostępnych warunków brzegowych u góry i edytor
wybranego warunku u dołu. Najpierw wybierz encję powierzchniową w drzewie `Model`;
lista zaoferuje wtedy warunki z rozdziału 1.4. Zaznacz jeden, aby go przypisać, a
następnie edytuj jego składowe w dolnym drzewie.

- **Pressure (explicit)** utrzymuje ciśnienie na wylocie. Model bez niego nie
  zostanie rozwiązany.
- **Velocity (inflow)** i **Volumetric flow rate (inflow)** wprowadzają do modelu
  znane natężenie przepływu. Wartości dodatnie wpływają, ujemne wypływają.
- Powierzchnie bez warunku są nieprzepuszczalne.

### 2.5 Warunki początkowe i otoczenia

Karta `Warunki otoczenia` oferuje *Gravitational acceleration*, który każda encja
już ma z wartością `(0, 0, -9.80665) m/s^2` (zob. 1.5), oraz *Temperature*, który
ten solver ignoruje.

Karta `Warunki początkowe` oferuje *Pressure* i *Velocity*, których ten solver nie
odczytuje.

### 2.6 Konfiguracja solvera macierzowego

`Problem` -> `Konfiguruj solver macierzowy problemów` konfiguruje solvery
iteracyjne. Solver przepływu przez ośrodki porowate używa pozycji **CG**. Jeżeli
rozwiązanie nie jest zbieżne, szukaj najpierw encji o zerowej przepuszczalności lub
odizolowanego elementu punktowego, a nie w ustawieniach solvera.

### 2.7 Punkty monitorowania

`Problem` -> `Zdefiniuj punkty monitorowania` umieszcza sondy w zadanych
współrzędnych i wybiera rejestrowaną zmienną - *Pressure* lub *Velocity*.
`Raport` -> `Punkty monitorowania` pokazuje wartości.

### 2.8 Wyniki i rekordy

Karta `Wyniki` wyświetla obliczone zmienne i steruje widokiem 3D. Ciśnienie jest
skalarem węzłowym, prędkość wektorem elementowym. Wyświetl prędkość jako pole
wektorowe na objętości, aby zobaczyć drogi przepływu.

Karta `Rekordy` panelu `Model` wyświetla rekordy wyników: jeden dla obliczenia
ustalonego, po jednym na każdy wyjściowy krok czasowy przy włączonym solverze
czasowym.

`Raport` -> `Plik dziennika solvera` pokazuje pełne wyjście solvera, łącznie ze
statystykami obu zmiennych.

---

## 3. Samouczek - przepływ wymuszony ciśnieniem przez filtr piaskowy

**Cel.** Obliczyć przepływ wody przez poziomy filtr piaskowy wymuszony znaną
różnicą ciśnień i sprawdzić natężenie przepływu z prawem Darcy'ego.

Samouczek zakłada objętościowy model prostopadłościanu z siatką - użyj `Plik` ->
`Nowy model`, narysuj prostopadłościan przez `Geometria` -> `Utwórz element` lub
zaimportuj geometrię i wygeneruj siatkę czworościanów; samouczek *Draw cube* to
omawia. Oznacz obie powierzchnie końcowe jako osobne encje powierzchniowe, aby
można im było przypisać warunki.

Niech filtr ma `1 m` długości wzdłuż osi X i przekrój `0.1 m` na `0.1 m`.

### Krok 1 - wybór problemu

1. `Problem` -> `Przebieg zadań problemów` (`Ctrl+P`).
2. Dodaj zadanie i wybierz **Flow through porous media**.
3. Potwierdź przyciskiem `OK`.

Pozostaw *Solver czasowy* wyłączony - wynikiem jest jeden stan ustalony.

### Krok 2 - przypisanie materiału

1. Wybierz encję objętościową w drzewie `Model`.
2. Otwórz kartę `Materiał` i przypisz **Coarse Sand (water saturated)**.

Materiał zawiera przepuszczalność piasku `1e-10 m^2` oraz gęstość i lepkość wody.

### Krok 3 - wyłączenie grawitacji

Filtr jest poziomy i liczy się tylko różnica ciśnień, więc usuń domyślną
grawitację:

1. Przy wybranej encji objętościowej otwórz kartę `Warunki otoczenia`.
2. Wybierz **Gravitational acceleration** i ustaw wszystkie trzy składowe na `0`.

Przy pozostawionej grawitacji woda przenosiłaby także ciśnienie hydrostatyczne na
wysokości `0.1 m` filtra, a równomierne ciśnienia na pionowych ścianach końcowych
powodowałyby krążenie przy nich - zob. 1.5.

### Krok 4 - zadanie obu końców

1. Wybierz encję powierzchniową w `x = 0`.
2. Otwórz kartę `Warunki brzegowe`, zaznacz **Pressure (explicit)** i ustaw
   `10000` Pa - około jednego metra słupa wody.
3. Wybierz encję powierzchniową w `x = 1 m`, zaznacz **Pressure (explicit)** i
   ustaw `0` Pa.

Cztery powierzchnie boczne nie otrzymują warunku: są nieprzepuszczalnymi ścianami.

### Krok 5 - rozwiązanie

1. `Rozwiązanie` -> `Uruchom solver` (`Ctrl+R`).
2. Śledź postęp w `Raport` -> `Plik dziennika solvera`.

Obliczenie to jedno rozwiązanie metodą gradientów sprzężonych.

### Krok 6 - przegląd wyników

- Karta `Wyniki`: wyświetl **Pressure**. Spada liniowo od `10000` Pa do `0` Pa
  wzdłuż filtra, z płaskimi izobarami prostopadłymi do jego osi.
- Wyświetl **Velocity**. Jest skierowana wzdłuż +X i jednakowa w każdym elemencie.
- Punkt monitorowania na *Pressure* w połowie filtra wskazuje `5000` Pa.

### Krok 7 - obliczenie ręczne

Przy `k = 1e-10 m^2` i `mu = 1.003e-3 kg/(m*s)` dla wody w 20 °C:

```
v = ( k / mu ) * dp / L   = 9.97e-8 * 10000 / 1     = 9.97e-4  m/s
Q = v * A                 = 9.97e-4 * 0.01          = 9.97e-6  m^3/s  (0.6 l/min)
```

Wynik **Velocity** musi pokazywać `9.97e-4 m/s`. Liczba Reynoldsa przy ziarnie
`1 mm` wynosi `998 * 9.97e-4 * 1e-3 / 1.003e-3 = 1`, na granicy zakresu Darcy'ego
(1.9). Przy porowatości `0.35` sama woda porusza się z prędkością
`9.97e-4 / 0.35 = 2.8e-3 m/s` i przechodzi przez filtr w około sześć minut.

### Rozwiązywanie problemów

| Objaw | Przyczyna |
|---|---|
| *Flow through porous media requires at least one 'Pressure (explicit)' boundary condition* | nigdzie nie zadano ciśnienia na wylocie |
| *Entity has material assigned which is missing required properties* | materiałowi brakuje lepkości - często suchy materiał gruntowy, np. *Sand* |
| Ciśnienie zmienia się z wysokością, a prędkość ma składową Z | domyślna grawitacja jest nadal włączona, zob. krok 3 |
| Prędkość jest wszędzie zerowa | oba końce mają to samo ciśnienie lub przepuszczalność jest zerowa |

---

## 4. Samouczek - wznoszące przesiąkanie przez pionową kolumnę

**Cel.** Przetłoczyć wodę w górę przez pionową kolumnę piasku ze znanym natężeniem
przepływu i zobaczyć, jak ciśnienie na wlocie dzieli się na część hydrostatyczną i
część tarcia.

Użyj prostopadłościanu o podstawie `0.1 m` na `0.1 m` i wysokości `1 m` wzdłuż osi
**Z**, z dolną powierzchnią (`z = 0`) i górną powierzchnią (`z = 1 m`) jako
osobnymi encjami powierzchniowymi.

### Krok 1 - problem i materiał

1. Wybierz **Flow through porous media** jak w samouczku 3.
2. Przypisz objętości **Coarse Sand (water saturated)**.

### Krok 2 - pozostawienie grawitacji

Kolumna stoi wzdłuż osi Z, więc domyślna grawitacja `(0, 0, -9.80665) m/s^2` jest
skierowana prawidłowo. Pozostaw warunek *Gravitational acceleration* bez zmian.

### Krok 3 - warunki brzegowe

1. Wybierz górną powierzchnię, zaznacz **Pressure (explicit)** i ustaw `0` Pa:
   swobodna woda nad kolumną.
2. Wybierz dolną powierzchnię, zaznacz **Volumetric flow rate (inflow)** i ustaw
   `1e-6` m^3/s. Na polu `0.01 m^2` odpowiada to prędkości dopływu `1e-4 m/s`.

Górna powierzchnia jest pozioma, więc równomierne ciśnienie na niej jest zgodne z
grawitacją.

### Krok 4 - rozwiązanie i przegląd

`Rozwiązanie` -> `Uruchom solver` (`Ctrl+R`), następnie wyświetl **Pressure** i
**Velocity**.

- Prędkość wynosi `1e-4 m/s` w górę (+Z) w każdym elemencie.
- Ciśnienie rośnie liniowo od `0` Pa u góry do maksimum na dnie.

### Krok 5 - podział ciśnienia na wlocie

Ciśnienie na dnie musi podnieść słup wody i przetłoczyć go przez piasek:

```
p_hydrostatic = rho * g * L            = 998.2 * 9.80665 * 1       = 9789 Pa
p_friction    = mu * v * L / k         = 1.003e-3 * 1e-4 * 1 / 1e-10 = 1003 Pa
p_bottom      = p_hydrostatic + p_friction                         = 10792 Pa
```

Punkt monitorowania na dolnej powierzchni potwierdza sumę. Ustaw natężenie
przepływu na zero i rozwiąż ponownie: prędkość znika, a ciśnienie na dnie spada do
czysto hydrostatycznych `9789 Pa` - solver dokładnie odtwarza płyn w spoczynku.

Zwiększaj natężenie przepływu, aż część tarcia osiągnie ciężar piasku pod
wyporem, około `(rho_s - rho) * g * (1 - n) * L`: to początek fluidyzacji
(kurzawki), której sztywny szkielet tego modelu nie potrafi odwzorować - solver po
prostu liczy coraz większe ciśnienie.

### Rozwiązywanie problemów

| Objaw | Przyczyna |
|---|---|
| Ciśnienie na dnie to tylko część tarcia | grawitacja jest wyłączona na objętości |
| Ciśnienie na dnie to tylko część hydrostatyczna | dopływ jest na niewłaściwej powierzchni lub ma wartość zero |
| Prędkość jest skierowana w bok | kolumnę zbudowano wzdłuż innej osi niż Z; zmień warunek grawitacji |

---

## 5. Sprawdzanie modelu

**Jednorodna kolumna.** Dla ciała o długości `L` utrzymywanego na `p1` i `p2` na
dwóch przeciwległych powierzchniach, z jednego materiału i z wyłączoną grawitacją,
ciśnienie wzdłuż ciała jest liniowe oraz

```
p(x) = p1 + ( p2 - p1 ) * x / L
v    = ( k / mu ) * ( p1 - p2 ) / L
```

Wynik prędkości musi się zgadzać z dokładnością do tolerancji solvera, a ciśnienie
w płaszczyźnie środkowej musi być średnią obu zadanych wartości.

**Hydrostatyka.** Ciało z grawitacją, z warunkiem *Pressure (explicit)* tylko na
poziomej górnej powierzchni i nieprzepuszczalnymi ścianami w pozostałych miejscach,
musi wykazywać zerową prędkość i ciśnienie rosnące o `rho*|g|` na metr głębokości.
Sprawdza to jednocześnie gęstość i warunek grawitacji.

**Bilans przepływu.** W modelu ustalonym cały płyn, który wpływa, musi wypłynąć.
Dla dopływu zadanego przez *Volumetric flow rate (inflow)* prędkość w przekroju
pomnożona przez jego pole musi odtworzyć zadane natężenie. Odczytaj prędkość w
kolumnie punktem monitorowania.

**Warstwy szeregowe.** Dla dwóch warstw o długości `L1`, `L2` i przepuszczalności
`k1`, `k2` połączonych szeregowo między dwoma zadanymi ciśnieniami przepływ stawia
opór jak dwa rezystory połączone szeregowo:

```
v  = ( p1 - p2 ) / ( mu * ( L1 / k1 + L2 / k2 ) )
pi = p1 - v * mu * L1 / k1          ciśnienie na granicy warstw
```

W odróżnieniu od solvera elektrostatycznego ten dzieli ciśnienie w stosunku
**przepuszczalności**, tak jak zrobiłaby to sieć rezystorów.

**Przepływ radialny do studni.** Dla napiętej warstwy wodonośnej o miąższości `H`
między studnią o promieniu `rw` utrzymywaną na `pw` a promieniem zewnętrznym `R`
utrzymywanym na `pR` ciśnienie jest logarytmiczne, a wydatek studni wynosi

```
p(r) = pw + ( pR - pw ) * ln( r / rw ) / ln( R / rw )
Q    = 2 * pi * H * ( k / mu ) * ( pR - pw ) / ln( R / rw )
```

Sprawdza to solver na niejednorodnym polu i pokazuje, czy siatka przy studni jest
wystarczająco gęsta.

Zagęść siatkę co najmniej raz i upewnij się, że ciśnienie zbiega, a nie dryfuje.

---

## 6. Ograniczenia

- **Przepływ ustalony, nieściśliwy, jednofazowy.** Nie ma członu retencji, więc
  konsolidacja, przepływ gazu ściśliwego i nieustalona odpowiedź napiętej warstwy
  wodonośnej nie są modelowane. Obliczenie nieustalone to ciąg stanów ustalonych,
  zob. 1.10.
- **Tylko ośrodki w pełni nasycone.** Nie ma swobodnego zwierciadła (krzywej
  depresji) ani strefy nienasyconej. Przesiąkanie przez nasyp, którego zwierciadło
  wody trzeba wyznaczyć, leży poza modelem, chyba że obszar nasycony zostanie
  jawnie objęty siatką.
- **Sztywne ciało porowate.** Szkielet się nie odkształca, więc nie ma sprzężenia z
  analizą naprężeń, naprężenia efektywnego, konsolidacji ani fluidyzacji.
- **Izotropowa przepuszczalność.** Przepuszczalność jest skalarem. Grunty
  warstwowe o różnej przepuszczalności poziomej i pionowej trzeba modelować jako
  osobne warstwy.
- **Tylko zakres Darcy'ego.** Przepływ bezwładnościowy (Forchheimera) i burzliwy w
  bardzo gruboziarnistych ośrodkach nie jest modelowany, zob. 1.9.
- **Warunki brzegowe tylko na powierzchniach.** Ciśnienia i dopływy zadaje się na
  encjach powierzchniowych; nie ma powierzchni wysięku, studni ani źródła
  liniowego, ani ciśnienia zadanego na linii lub w punkcie.
- **Brak wyniku całkowitego natężenia przepływu.** Natężenia przepływu przez
  przekrój trzeba obliczyć z prędkości ręcznie.
- **Warunki początkowe nie są odczytywane**, a warunek otoczenia *Temperature* nie
  ma wpływu; tabele materiału podążają za temperaturą tylko poprzez zadanie
  cieplne umieszczone wcześniej w przebiegu zadań.
- **Model potrzebuje zadanego ciśnienia.** Bez niego solver zatrzymuje się z
  błędem.

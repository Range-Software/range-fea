# Elektrostatyka - Podręcznik teorii i przewodnik użytkownika

Ten dokument opisuje solver elektrostatyczny (`RSolverElectrostatics`) programu
Range FEA, obsługujący typ problemu **Electro-statics**: rozwiązywane równania,
znaczenie wszystkich przyjmowanych danych wejściowych, elementy interfejsu
graficznego, które nim sterują, oraz dwa kompletne samouczki.

**Spis treści**

1. [Podstawy teoretyczne](#1-podstawy-teoretyczne)
2. [Interfejs graficzny](#2-interfejs-graficzny)
3. [Samouczek - potencjał i prąd w przewodniku](#3-samouczek---potencjał-i-prąd-w-przewodniku)
4. [Samouczek - nagrzewanie rezystancyjne](#4-samouczek---nagrzewanie-rezystancyjne)
5. [Sprawdzanie modelu](#5-sprawdzanie-modelu)
6. [Ograniczenia](#6-ograniczenia)

---

## 1. Podstawy teoretyczne

### 1.1 Niewiadoma podstawowa - pole potencjału elektrycznego

Niewiadomą podstawową jest **potencjał elektryczny** `V` w każdym węźle, w
woltach. Każdy węzeł ma jeden stopień swobody, więc siatka o `N` węzłach daje
układ `N` równań. Pole elektryczne, gęstość prądu i wszystko, co z nich wynika,
wyznacza się z pola potencjału po rozwiązaniu.

Potencjał jest skalarem, dzięki czemu model elektrostatyczny jest tak samo tani
jak model cieplny na tej samej siatce: jedna niewiadoma na węzeł zamiast trzech
i macierz o jednej dziewiątej liczby wpisów modelu konstrukcyjnego.

Domyślny potencjał węzła, którego nie dotyczy żaden warunek ani żaden
wcześniejszy wynik, wynosi `0 V`.

Pole potencjału jest określone tylko **z dokładnością do stałej addytywnej**.
Nic w równaniach nie ustala poziomu zerowego, więc każdy model potrzebuje co
najmniej jednego warunku brzegowego *Electric potential* - zob. 1.7. Jest to
elektrostatyczny odpowiednik konstrukcji bez podparć lub modelu cieplnego bez
zadanej temperatury.

### 1.2 Równanie podstawowe

Solver realizuje **równanie pola elektrostatycznego** - równanie Poissona dla
potencjału w liniowym, izotropowym dielektryku:

```
div( e0 * er * grad(V) ) = -rho
```

a przy braku gęstości ładunku w całym obszarze sprowadza się ono do równania
Laplace'a

```
div( e0 * er * grad(V) ) = 0
```

Pole elektryczne jest ujemnym gradientem potencjału, a gęstość prądu wynika z
prawa Ohma:

```
E = -grad(V)
J = sigma * E
```

| Symbol | Znaczenie | Jednostki | Źródło |
|---|---|---|---|
| `V` | potencjał elektryczny | `V` | rozwiązywany |
| `rho` | gęstość ładunku | `C/m^3`, zob. 1.5 | warunek Charge density |
| `e0` | przenikalność elektryczna próżni | `F/m` | wbudowana, `8.854187817e-12` |
| `er` | względna przenikalność elektryczna | `-` | właściwość materiału |
| `sigma` | konduktywność elektryczna | `S/m` | właściwość materiału |
| `E` | pole elektryczne | `V/m` | wyznaczane, zob. 1.8 |
| `J` | gęstość prądu | `A/m^2` | wyznaczana, zob. 1.8 |

Przenikalność próżni jest stałą solvera, a nie właściwością materiału ani
ustawieniem - z materiałem wprowadza się tylko przenikalność **względną**.

**Pole jest wyznaczane przez przenikalność, a nie przez konduktywność.** To
najważniejsza rzecz, jaką trzeba wiedzieć o tym solverze. Macierz sztywności
jest budowana wyłącznie z `e0*er`; konduktywność nigdy nie wchodzi do układu
równań. Jest używana dopiero potem, aby przekształcić obliczone pole w gęstość
prądu, rezystywność i ciepło Joule'a. Konsekwencje opisano w 1.9: w ciele z
jednego materiału nie ma to znaczenia, ale w ciele z **dwóch** potencjał dzieli
się proporcjonalnie do przenikalności, a nie proporcjonalnie do rezystancji.

**Wymagane właściwości materiału.** Problem elektrostatyczny wymaga **trzech**
z nich:

| Właściwość | Jednostki | Zastosowanie |
|---|---|---|
| Relative permittivity | `-` | macierz sztywności |
| Electrical conductivity | `S/m` | gęstość prądu, rezystywność, ciepło Joule'a |
| Density | `kg/m^3` | nic - wymagana, nigdy nie odczytywana |

(*Relative permittivity* = względna przenikalność elektryczna, *Electrical
conductivity* = konduktywność elektryczna, *Density* = gęstość.) Encja, której
materiałowi brakuje którejkolwiek z trzech, **nie jest rozwiązywana**, mimo że
ten solver w ogóle nie odczytuje gęstości. Kontrola konfiguracji ostrzega o tym
przed rozpoczęciem obliczeń. Jedynym wyjątkiem jest encja z elektrostatycznym
warunkiem brzegowym: taka encja jest włączana do układu niezależnie od
materiału, a jej przenikalność i konduktywność wynoszą wtedy po prostu zero.

Zerowa przenikalność nie jest nieszkodliwą wartością domyślną - element z
`er = 0` nie ma sztywności, więc nic nie wnosi do układu i pozostawia swoje
węzły niepowiązane. Jeśli encja ma być rozwiązywana, nadaj jej rzeczywistą
przenikalność: `1` dla próżni lub, w przybliżeniu, dla powietrza.

Każda właściwość jest tablicą w funkcji temperatury, ale samo zadanie
elektrostatyczne nie ma pola temperatury, w którym można by ją odczytać, więc
każda tablica jest obliczana dla `293.15 K`. Tablice są uwzględniane tylko
wtedy, gdy zadanie *Heat transfer* umieszczone wcześniej w tym samym przebiegu
zadań obliczyło temperatury elementów, które są udostępniane kolejnym zadaniom.

Względna przenikalność jest **bezwymiarowa** - interfejs pokazuje ją jako `N/A`,
podobnie jak emisyjność i liczbę Poissona - a jej wartość jest używana jako
zwykły mnożnik `e0`. Wpisz `1` dla próżni, `1.00059` dla powietrza, `2.1` dla
PTFE, około `4` dla szkła.

### 1.3 Dyskretyzacja metodą elementów skończonych

Postać słaba równania pola daje dla każdego elementu

```
K = INT( B^T * e0 * er * B )        stiffness
f = INT( N^T * rho )                charge source
```

(`stiffness` = sztywność, `charge source` = źródło ładunku.) Tu `B` jest
budowane z pochodnych funkcji kształtu, a `N` to same funkcje kształtu. Każda
macierz elementu ma wymiar `n x n` dla elementu o `n` węzłach - jeden wiersz na
węzeł, a nie trzy. Nie ma macierzy mas ani pojemności: problem jest statyczny, a
w 1.10 wyjaśniono, co to oznacza dla przebiegu zadań.

**Miary elementów.** Jak wszędzie w Range FEA, encjom niższego wymiaru nadaje
się miarę geometryczną, aby mogły reprezentować rzeczywiste ciała:

| Encja | Miara | Gdzie jest ustawiana |
|---|---|---|
| Objętość | 1 | - |
| Powierzchnia | grubość powierzchni | właściwości geometryczne encji |
| Linia | pole przekroju linii | właściwości geometryczne encji |
| Punkt | objętość punktu | właściwości geometryczne encji |

**Powierzchnia** przenosi pole w swojej płaszczyźnie przez przekrój równy jej
grubości. Nie ma gradientu potencjału w kierunku grubości: element powierzchni
ma grubość jednego węzła, więc z założenia jest ekwipotencjalny w kierunku
grubości. Powierzchnia o **zerowej grubości** nie ma więc żadnej sztywności i
niczego nie łączy - w przeciwieństwie do solvera cieplnego, gdzie powierzchnia o
zerowej grubości nadal przenosi konwekcję, tutaj jest po prostu bierna. Nadaj
encji powierzchni grubość zawsze, gdy ma przewodzić, i pamiętaj, że ściana o
zerowej grubości nie otrzymuje też **żadnych wyników pochodnych** (1.8).

**Linia** przenosi pole wzdłuż swojego kierunku przez swoje pole przekroju.
**Punkt** nie ma kierunku, wzdłuż którego mógłby je przenosić, więc nie wnosi
**żadnej sztywności**: zawiera tylko źródło ładunku. Element punktowy, którego
węzeł nie jest współdzielony z żadnym innym obliczanym elementem, tworzy więc w
macierzy pusty wiersz, z którym metoda gradientów sprzężonych sobie nie radzi -
punkt jest miejscem do wprowadzenia ładunku lub zadania potencjału, a nie
przewodnikiem.

### 1.4 Źródła i warunki brzegowe

| Warunek brzegowy | Typ | Dotyczy | Składowe |
|---|---|---|---|
| Electric potential | jawny (Dirichlet) | punkt, linia, powierzchnia, objętość | Electric potential `[V]` |
| Charge density | naturalny, źródło | punkt, linia, powierzchnia, objętość | Charge density, zob. 1.5 |

Te dwa warunki stanowią całe słownictwo tego typu problemu. Nie ma zadanego
strumienia, ładunku powierzchniowego, warunku pola dalekiego lub otwartego
brzegu ani warunku symetrii - zob. rozdział 6.

**Electric potential** (potencjał elektryczny) jest jedynym jawnym warunkiem
problemu elektrostatycznego. Każdy węzeł encji, do której go przypisano, jest
usuwany z układu równań i utrzymywany na podanej wartości; jego wkład jest
przenoszony na prawą stronę sąsiednich równań. Każdy model potrzebuje co
najmniej jednego takiego warunku. Nowo przypisany warunek ma wartość
początkową `1 V`.

Tam, gdzie encje się nakładają, **wygrywa encja niższego wymiaru**: potencjał
zadany na powierzchni zastępuje we wspólnych węzłach potencjał zadany na
objętości za nią, linia zastępuje powierzchnię, a punkt zastępuje wszystko.
Potencjały nie są łączone - w węźle jest tylko jeden stopień swobody, więc
obowiązuje po prostu ostatni odczytany warunek.

**Charge density** (gęstość ładunku) jest członem źródłowym równania Poissona. W
przeciwieństwie do warunku *Heat* solvera cieplnego jej wartość **nie** jest
rozkładana na miarę encji: wprowadzona liczba jest stosowana jako gęstość do
każdego elementu encji w takiej postaci, w jakiej jest. Całkowity ładunek
umieszczony w encji objętości jest więc wartością pomnożoną przez objętość tej
encji i rośnie, gdy encja zostanie powiększona - nie pozostaje stały, jak
wartość *Heat* w watach.

| Encja | Co oznacza wartość |
|---|---|
| Objętość | ładunek na jednostkę objętości |
| Powierzchnia | ładunek na jednostkę objętości przekroju ważonego grubością |
| Linia | ładunek na jednostkę objętości przekroju |
| Punkt | węzłowe źródło ładunku |

Obie składowe przyjmują tablicę w funkcji czasu. Ponieważ sam problem jest
statyczny, przebieg czasowy ma znaczenie tylko wtedy, gdy przebieg zadań
współdzieli inne, zależne od czasu zadanie - a jak wyjaśnia 1.10, takie
połączenie samo w sobie jest ograniczone.

### 1.5 Wartość gęstości ładunku

Składowa *Charge density* jest **objętościową gęstością ładunku** w `C/m^3`,
zgodnie z jej etykietą w interfejsie. Jest gęstością, a nie wartością
całkowitą, co czyni ją niezależną od siatki - i, jak zauważono w 1.4, odróżnia
ją od warunku *Heat* solvera cieplnego, który przyjmuje wartość całkowitą w
watach i sam ją rozkłada.

Znak jest zgodny ze zwykłą konwencją. Całkowanie przez części
`div(e0*er*grad(V)) = -rho` pozostawia ładunek po prawej stronie ze znakiem
dodatnim,

```
every entity type      f = +INT( N^T * rho )
```

(`every entity type` = każdy typ encji), więc **dodatnia** gęstość ładunku
podnosi potencjał wokół siebie, a ujemna go obniża, jednakowo na encjach
objętości, powierzchni, linii i punktu.

Pamiętaj, że samo źródło ładunku nie tworzy rozwiązywalnego modelu. Potencjał
nadal jest określony tylko z dokładnością do stałej, dopóki coś go nie ustali,
więc model sterowany ładunkiem także potrzebuje co najmniej jednego warunku
*Electric potential* - choćby na zewnętrznym brzegu siatkowanego obszaru. Zob.
1.7.

### 1.6 Warunki otoczenia, początkowe i sprzężenia

#### Warunki otoczenia

| Warunek otoczenia | Składowe | Działanie |
|---|---|---|
| Temperature | Temperature `[K]` | oferowany, ale nie odczytywany - zob. niżej |

Warunek otoczenia *Temperature* (temperatura) jest oferowany dla problemu
elektrostatycznego, ponieważ właściwości materiałów są tablicami w funkcji
temperatury. Solver elektrostatyczny go **nie** odczytuje: temperatury elementów
pobiera wyłącznie z udostępnionych danych pozostawionych przez zadanie *Heat
transfer*, a w przeciwnym razie oblicza każdą tablicę dla `293.15 K`. Przypisanie
go w modelu czysto elektrostatycznym niczego nie zmienia.

#### Warunki początkowe

| Warunek początkowy | Działanie |
|---|---|
| Electric potential | początkowe pole potencjału, stosowane przy pierwszym uruchomieniu |

Potencjał początkowy jest odczytywany **tylko przy pierwszym uruchomieniu**.
Ponieważ rozwiązanie jest bezpośrednie, a nie krokowe, nie zmienia ono zbieżnego
wyniku - ustawia jedynie punkt startowy iteracji gradientów sprzężonych oraz
wartość każdego węzła, który solver pomija w układzie.

Warunki otoczenia, warunki początkowe i warunki brzegowe są odczytywane w tej
kolejności i każde nadpisują poprzednie, więc **warunek brzegowy ma
pierwszeństwo przed warunkiem początkowym, a ten przed warunkiem otoczenia**.

#### Sprzężenie z innymi solverami

Solver elektrostatyczny wytwarza dwa wyniki, z których korzystają inne solvery.

| Kierunek | Zmienna | Działanie |
|---|---|---|
| Elektrostatyka -> wymiana ciepła | Joule heat | dodawane do członu źródłowego każdego elementu |
| Elektrostatyka -> wymiana ciepła w płynach | Joule heat | dodawane do członu źródłowego każdego elementu |
| Elektrostatyka -> magnetostatyka | Current density | źródło pola magnetycznego |
| Wymiana ciepła -> elektrostatyka | Temperature | wybiera wiersz każdej tablicy materiałowej zależnej od temperatury |

**Joule heat** (ciepło Joule'a) jest pobierane przez zadanie *Heat transfer*
umieszczone w przebiegu zadań problemów **po** zadaniu elektrostatycznym, gdzie
jest dodawane do członu źródłowego elementu dokładnie tak, jak zrobiłby to
warunek *Heat rate (unit volume)*. Nagrzewanie rezystancyjne nie wymaga więc
żadnej konfiguracji poza uporządkowaniem obu zadań - przeprowadza przez to
samouczek 4.

**Magneto-statics** (magnetostatyka) to oddzielny typ problemu, który *wymaga*
zadania elektrostatycznego: odczytuje zapisaną przez ten solver gęstość prądu w
elementach - jednakowo na encjach objętości, powierzchni i linii - i oblicza
pole magnetyczne tego prądu z prawa Biota-Savarta. Najpierw dodaj zadanie
elektrostatyczne.

Ponieważ solver bezwarunkowo zgłasza zbieżność, grupa zadań zawierająca tylko
zadanie elektrostatyczne zawsze kończy się po jednej iteracji. Iterowanie ma
sens tylko wtedy, gdy coś, od czego zależy rozwiązanie, samo jest opóźnione -
na przykład silnie zależna od temperatury przenikalność zasilana przez zadanie
cieplne.

### 1.7 Rozwiązywanie układu

Solver składa i rozwiązuje

```
K * V = f
```

`K` jest symetryczna i dodatnio określona, gdy zadano co najmniej jeden
potencjał, więc rozwiązuje się ją metodą **gradientów sprzężonych** z
prekondycjonowaniem Jacobiego. Składanie jest zrównoleglone po elementach z
buforami dla poszczególnych wątków, łączonymi raz na końcu, więc wynik nie zależy
od liczby wątków.

Model **bez warunku brzegowego Electric potential** jest osobliwy: potencjał
jest określony tylko z dokładnością do stałej, macierz ma jądro nietrywialne, a
rozwiązanie nie będzie zbieżne. Każdy model potrzebuje co najmniej jednego
zadanego potencjału, a model, który ma przewodzić prąd, potrzebuje dwóch o
różnych wartościach.

Warto znać trzy dalsze sposoby otrzymania osobliwego lub pustego układu,
ponieważ wszystkie trzy przebiegają bez komunikatu:

- encja, której materiałowi brakuje jednej z trzech wymaganych właściwości, nie
  jest rozwiązywana, a jej węzły wypadają z układu;
- encja z `er = 0` nie ma sztywności, więc jej węzły pozostają w układzie, ale
  nie są z niczym połączone;
- powierzchnia o zerowej grubości lub linia o zerowym polu przekroju również nie
  ma sztywności.

### 1.8 Wyniki pochodne

Po rozwiązaniu wyniki elementowe są wyznaczane z węzłowego pola potencjału. Pole
elektryczne jest ujemnym gradientem potencjału, uśrednionym po punktach
całkowania i obróconym do współrzędnych globalnych dla elementów liniowych i
powierzchniowych:

```
E = -grad(V)
J = sigma * E
w = e0 * er * |E|^2 / 2          electric energy
r = |E| / |J|                    electrical resistivity
q = sigma * |E|^2                Joule heat
```

(`electric energy` = energia elektryczna, `electrical resistivity` =
rezystywność elektryczna, `Joule heat` = ciepło Joule'a.)

| Wynik | Dotyczy | Znaczenie |
|---|---|---|
| Electric potential `[V]` | węzeł | rozwiązane pole potencjału |
| Electric field `[V/m]` | element | wektor pola |
| Current density `[A/m^2]` | element | `sigma*E`, zero tam, gdzie konduktywność wynosi zero |
| Electric energy `[J/m^3]` | element | gęstość energii pola |
| Electrical resistivity `[V*m/A]` | element | `1/sigma` wszędzie tam, gdzie płynie prąd, `0` w pozostałych miejscach |
| Joule heat `[W/m^3]` | element | rozpraszanie rezystancyjne, przekazywane do solvera cieplnego |

Wszystkie te wyniki są **gęstościami**: pochodne funkcji kształtu są uśredniane
po punktach całkowania elementu i nie są ważone wyznacznikiem jakobianu, a
grubość powierzchni i pole przekroju linii pozostają w sztywności, gdzie ich
miejsce, zamiast wchodzić do wyznaczanego pola. Pole, gęstość prądu i energia
elektryczna są więc niezależne od rozmiaru elementu, w którym je obliczono, dzięki
czemu można je porównywać między elementami i z obliczeniami ręcznymi, a przy
zagęszczaniu siatki są zbieżne.

**Electrical resistivity** jest wynikiem, po który warto sięgnąć przy
sprawdzaniu modelu, ponieważ oba czynniki, z których się składa, się
upraszczają: `|E|/|sigma*E|` wynosi dokładnie `1/sigma`, niezależnie od pola. To
najszybsze potwierdzenie, że konduktywność przeznaczona dla encji rzeczywiście
dotarła do jej elementów. Jest zapisywana jako `0` w każdym elemencie, w którym
gęstość prądu spada poniżej tolerancji numerycznej, co obejmuje każdy element o
zerowej konduktywności.

**Ciepło Joule'a** jest gęstością rozpraszania `sigma*|E|^2` w `W/m^3`. Dokładnie
tego oczekuje zadanie *Heat transfer*, gdy dodaje tę wartość do swojego członu
źródłowego i całkuje ją po elemencie, więc moc przechodząca do sprzężonych
obliczeń cieplnych jest mocą faktycznie rozpraszaną w modelu. Podobnie jak
pozostałe wyniki jest gęstością, więc nie zależy od rozmiaru elementu i jest
zbieżna przy zagęszczaniu siatki.

Są jeszcze dwa miejsca, w których wyniki pochodne pozostają po prostu zerowe:
**elementy punktowe** nigdy nie są poddawane obróbce wyników, a **powierzchnia o
zerowej grubości** lub **linia o zerowym polu przekroju** jest pomijana, więc obie
zachowują zerowe pole niezależnie od tego, jak duży jest na nich gradient
potencjału.

Po zapisaniu rekordu dziennik solvera wypisuje statystyki wszystkich sześciu
zmiennych oraz wartości w punktach monitorowania.

### 1.9 Czym solver jest, a czym nie jest

Nazwa jest dokładna: jest to solver **elektrostatyczny**. Oblicza pole rozkładu
ładunku i potencjału w dielektryku. Podawana przez niego gęstość prądu to
wartość, jaką dla tego pola daje prawo Ohma - wielkość pochodna, obliczana po
fakcie, która nigdy nie wpływa na samo pole.

Problem **ustalonego przepływu prądu** opisuje natomiast równanie

```
div( sigma * grad(V) ) = 0
```

z konduktywnością w miejscu, w którym ten solver umieszcza przenikalność. Oba
dają to samo pole potencjału w ciele jednorodnym, ponieważ oba sprowadzają się do
równania Laplace'a, a stała się upraszcza. Rozchodzą się, gdy tylko ciało składa
się z więcej niż jednego materiału:

| | Ten solver | Solver przepływu prądu |
|---|---|---|
| Potencjał dzieli się proporcjonalnie do | `1/er` każdego obszaru | `1/sigma` każdego obszaru |
| Właściwy dla | kondensatora, dielektryka, izolatora | sieci rezystorów, szyny zbiorczej, ścieżki przewodzącej |

Jednomateriałowy przewodnik z zadanymi potencjałami na końcach jest więc
modelowany poprawnie: potencjał jest liniowy, a podzielenie przez długość daje
pole. Dwa połączone szeregowo przewodniki o różnej konduktywności - już nie;
podział następuje według przenikalności, co raczej nie jest zamierzone. Taki
przypadek modeluj jako jeden materiał lub oblicz podział ręcznie.

### 1.10 Analiza statyczna i solver czasowy

Electro-statics jest zadeklarowany jako typ problemu **bez** solvera czasowego.
Ma to dwie widoczne konsekwencje.

W modelu czysto elektrostatycznym grupa *Solver czasowy* w ogóle nie pojawia się
na karcie `Problem`, obliczenia tworzą jeden rekord i nie ma nic do
skonfigurowania.

Bardziej zaskakujące jest to, że dodanie zadania elektrostatycznego do
przebiegu zadań **wyłącza solver czasowy dla całego przebiegu**. Solver czasowy
jest oferowany tylko wtedy, gdy *każdy* typ problemu w przebiegu go obsługuje,
więc przejściowa analiza cieplna, do której dodano zadanie elektrostatyczne,
staje się analizą stanu ustalonego, bez żadnego ostrzeżenia poza zniknięciem
grupy. Jeśli dla grzejnika rezystancyjnego potrzebna jest przejściowa odpowiedź
cieplna, uruchom zadanie elektrostatyczne osobno i przenieś jego rozpraszanie do
przejściowego modelu cieplnego jako warunek *Heat*, zamiast umieszczać oba zadania
w jednym przebiegu.

---

## 2. Interfejs graficzny

Wszystko, co istotne, znajduje się w panelu **Solver** (karty `Problem`,
`Warunki brzegowe`, `Warunki początkowe`, `Warunki otoczenia`, `Materiał`,
`Wyniki`), menu **Problem** i panelu **Model**.

### 2.1 Wybór problemu

`Problem` -> `Przebieg zadań problemów` (`Ctrl+P`) otwiera okno przebiegu zadań
problemu. Dodaj zadanie i wybierz **Electro-statics** - „Calculate electric
field, current density and generated Joule's heat” (oblicz pole elektryczne,
gęstość prądu i wytworzone ciepło Joule'a).

Electro-statics można swobodnie łączyć z każdą inną fizyką z wyjątkiem *Modal
analysis*. Warto znać następujące łańcuchy:

- *Electro-statics*, a następnie *Heat transfer* - nagrzewanie rezystancyjne;
- *Electro-statics*, a następnie *Magneto-statics* - pole magnetyczne
  obliczonego prądu, co jest jedynym sposobem przeprowadzenia analizy
  magnetostatycznej;
- *Electro-statics*, następnie *Heat transfer*, następnie *Stress analysis* -
  naprężenia cieplne wywołane nagrzewaniem rezystancyjnym.

Pamiętaj o 1.10: każdy przebieg zawierający zadanie elektrostatyczne jest
rozwiązywany jako przebieg w stanie ustalonym.

### 2.2 Karta Problem

W modelu czysto elektrostatycznym ta karta jest **pusta**. Nie ma solvera
czasowego, ponieważ typ problemu go nie obsługuje, a elektrostatyka nie ma
własnej grupy ustawień. Wszystko przypisuje się do poszczególnych encji na
kartach warunków.

Jeśli karta pokazuje grupę *Solver czasowy*, umieściło ją tam inne zadanie w
przebiegu - i zgodnie z 1.10 zniknie ona ponownie, gdy tylko zostanie dodane
zadanie elektrostatyczne.

### 2.3 Karta Materiał

Karta `Materiał` zawiera materiał przypisany do wybranej encji. Elektrostatyka
wymaga **wszystkich trzech** poniższych właściwości:

| Właściwość | Jednostki | Komentarz |
|---|---|---|
| Relative permittivity | `N/A`, bezwymiarowa | `er`, `1` dla próżni, `1.00059` dla powietrza, około `4` dla szkła |
| Electrical conductivity | `S/m` | `sigma`, `5.882e7` dla miedzi, `5e-15` dla powietrza |
| Density | `kg/m^3` | wymagana, aby encja była rozwiązywana, nigdy nie odczytywana |

Encja, której brakuje którejkolwiek z nich, nie jest rozwiązywana, a kontrola
konfiguracji ostrzega przed rozpoczęciem obliczeń komunikatem *Entity ... has
material assigned which is missing required properties* (encja ... ma
przypisany materiał, któremu brakuje wymaganych właściwości). Ponieważ ten
solver nigdy nie używa gęstości, wymaganie spełnia dowolna wartość dodatnia.

Dołączona biblioteka materiałów zawiera już wszystkie trzy właściwości dla
metali, powietrza i szkieł, więc na początek wystarczy przypisać *Copper*
(miedź) lub *Air* (powietrze).

Każda właściwość jest tablicą w funkcji temperatury, ale zależność od
temperatury jest uwzględniana tylko wtedy, gdy zadanie cieplne wcześniej w
przebiegu wytworzyło temperatury elementów - zob. 1.6.

### 2.4 Karta Warunki brzegowe

Karta jest podzielona na listę menedżera dostępnych warunków brzegowych u góry i
edytor wybranego warunku u dołu. Najpierw wybierz encję w drzewie `Model`;
menedżer zaoferuje wtedy warunki właściwe dla danego typu encji i wybranego
problemu. Zaznacz warunek, aby go przypisać, a następnie edytuj jego składowe w
dolnym drzewie.

Oferowane są tylko dwa warunki z punktu 1.4. Uwagi dotyczące ich użycia:

- **Electric potential** jest warunkiem, który utrzymuje wartość, zamiast ją
  wymuszać, i jest oferowany dla każdego typu encji. Przypisz go do ściany,
  krawędzi lub punktu o znanym potencjale. Model bez żadnego z nich nie zostanie
  rozwiązany.
- **Charge density** jest członem źródłowym, a dodatnia wartość podnosi
  potencjał wokół siebie. Przed użyciem przeczytaj 1.5: wartość jest gęstością
  i, w przeciwieństwie do warunku *Heat* solvera cieplnego, nie jest rozkładana
  na encję.
- Obie składowe są tablicami w funkcji czasu, co ma znaczenie tylko w przebiegu
  zadań sterowanym przez inne, zależne od czasu zadanie.

### 2.5 Warunki początkowe i otoczenia

Karta `Warunki początkowe` oferuje *Electric potential*, który ustawia pole
startowe rozwiązania iteracyjnego. Nie zmienia on zbieżnego wyniku.

Karta `Warunki otoczenia` oferuje *Temperature*, który ten solver ignoruje -
zob. 1.6.

### 2.6 Konfiguracja solvera macierzowego

`Problem` -> `Konfiguruj solver macierzowy problemów` konfiguruje solvery
iteracyjne. Solver elektrostatyczny używa pozycji **CG**.

Jeśli rozwiązanie nie jest zbieżne, przyczyną jest zwykle model bez zadanego
potencjału lub encja o zerowej przenikalności, a nie ustawienie solvera. Zanim
zwiększysz liczbę iteracji, sprawdź warunki brzegowe i materiał.

### 2.7 Punkty monitorowania

`Problem` -> `Zdefiniuj punkty monitorowania` umieszcza sondy w podanych
współrzędnych i wybiera rejestrowaną zmienną - dla problemu elektrostatycznego
zwykle jest to *Electric potential*, a jest to najszybszy sposób odczytania
wartości z modelu jako liczby, a nie koloru. `Raport` -> `Punkty monitorowania`
pokazuje wartości.

### 2.8 Wyniki i rekordy

Karta `Wyniki` zawiera listę obliczonych zmiennych i steruje widokiem 3D.
Potencjał elektryczny jest skalarem węzłowym; pole elektryczne i gęstość prądu
są wektorami elementowymi; energia elektryczna, rezystywność elektryczna i
ciepło Joule'a są skalarami elementowymi.

Karta `Rekordy` panelu `Model` zawiera listę rekordów wyników. Analiza
elektrostatyczna jest statyczna, więc jest dokładnie **jeden**.

`Raport` -> `Plik dziennika solvera` wyświetla pełne wyjście solvera, w tym
statystyki wszystkich sześciu zmiennych po zapisaniu rekordu.

---

## 3. Samouczek - potencjał i prąd w przewodniku

**Cel.** Obliczyć rozkład potencjału wzdłuż miedzianego pręta utrzymywanego na
dwóch różnych potencjałach i odczytać pole, prąd i rezystancję.

Ten samouczek zakłada zasiatkowany model objętościowy - dobrze sprawdza się
długi prostopadłościan. Użyj `Plik` -> `Nowy model`, narysuj prostopadłościan za
pomocą `Geometria` -> `Utwórz element` lub zaimportuj geometrię i podziel ją na
czworościany; samouczek *Rysowanie sześcianu* omawia te czynności. Oznacz obie
ściany końcowe jako oddzielne encje powierzchni, aby można było przypisać im
warunki.

Przyjmij pręt o długości `0.1 m` i przekroju `0.01 m` na `0.01 m`, co ułatwia
śledzenie obliczeń ręcznych na końcu.

### Krok 1 - wybierz problem

1. `Problem` -> `Przebieg zadań problemów` (`Ctrl+P`).
2. Dodaj zadanie i wybierz **Electro-statics**.
3. Zatwierdź przyciskiem `OK`.

Karta `Problem` pozostaje pusta - nie ma nic do ustawienia.

### Krok 2 - przypisz materiał

1. Wybierz encję objętości w drzewie `Model`.
2. Otwórz kartę `Materiał` i przypisz **Copper** lub materiał ze wszystkimi
   trzema właściwościami:
   - **Relative permittivity** = `2.8`
   - **Electrical conductivity** = `5.882e7` `S/m`
   - **Density** = `8960` `kg/m^3`

Ten solver nie używa gęstości, ale musi ona być obecna, aby encja w ogóle była
rozwiązywana. Pole jest wyznaczane przez przenikalność; konduktywność jedynie
przekształca potem pole w prąd.

### Krok 3 - zadaj potencjały na obu końcach

1. Wybierz encję powierzchni na jednym końcu.
2. Otwórz kartę `Warunki brzegowe` i zaznacz **Electric potential**.
3. Ustaw wartość `0` V - jest to masa.
4. Wybierz encję powierzchni na drugim końcu, ponownie zaznacz **Electric
   potential** i ustaw `10` V.

To ten krok czyni model rozwiązywalnym. Jeden zadany potencjał to minimum; pole
tworzą dwa potencjały o różnych wartościach.

Nie dodawaj warunku *Charge density*. Przewodnik z zadanymi potencjałami na
końcach nie ma ładunku przestrzennego, więc człon źródłowy powinien być zerowy,
a potencjał jest linią prostą między obu końcami - dzięki temu ręczne sprawdzenie
w kroku 6 jest dokładne.

### Krok 4 - rozwiąż

1. `Rozwiązanie` -> `Uruchom solver` (`Ctrl+R`).
2. Kontrola konfiguracji zgłasza brakujące materiały przed rozpoczęciem
   obliczeń.
3. Śledź postęp w `Raport` -> `Plik dziennika solvera`.

Obliczenia to jedno rozwiązanie metodą gradientów sprzężonych, które na siatce
tej wielkości kończy się w kilka sekund.

### Krok 5 - przejrzyj wyniki

- Karta `Wyniki`: wyświetl **Electric potential**. Powinien maleć liniowo od
  `10` V do `0` V wzdłuż pręta, z płaskimi płaszczyznami ekwipotencjalnymi
  prostopadłymi do jego osi. Jakiekolwiek zakrzywienie w jednorodnym pręcie
  oznacza, że siatka nie jest połączona tak, jak się zakłada.
- **Electrical resistivity** powinna wynosić `1/5.882e7 = 1.7e-8` `V*m/A` w
  każdym elemencie pręta. To najlepsze pojedyncze sprawdzenie, że materiał
  dotarł do elementów: jest dokładne, niezależne od siatki i błędne w momencie,
  gdy błędna jest konduktywność.
- **Electric field** jest skierowane wzdłuż pręta i powinno wynosić równomiernie
  `10/0.1 = 100` `V/m`, a **Current density** `sigma*E = 5.88e9` `A/m^2`. Obie
  wielkości są gęstościami, więc liczby obowiązują niezależnie od rozmiaru
  elementu - krok 6 je wyprowadza.
- Punkt monitorowania dla *Electric potential* w połowie długości pręta powinien
  wskazywać `5` V.

### Krok 6 - oblicz wartości ręcznie

Pierwsze dwa wiersze poniżej to wartości, które wyniki **Electric field** i
**Current density** powinny już pokazywać, więc stanowią bezpośrednie
sprawdzenie rozwiązania. Ostatnie trzy to wartości całkowite, a nie gęstości,
których nie podaje żaden wynik elementowy, i trzeba je obliczyć. Dla
jednorodnego pręta o długości `L` i przekroju `A` przy różnicy potencjałów `U`:

```
E = U / L                       = 10 / 0.1            = 100      V/m
J = sigma * E                   = 5.882e7 * 100       = 5.88e9   A/m^2
I = J * A                       = 5.88e9 * 1e-4       = 5.88e5   A
R = L / ( sigma * A )           = 0.1 / (5.882e7*1e-4) = 1.7e-5  ohm
P = U^2 / R                     = 100 / 1.7e-5        = 5.88e6   W
```

Prąd jest ogromny, ponieważ pełny miedziany pręt o takim przekroju jest bardzo
dobrym przewodnikiem, a `10` V na nim to zwarcie - przydatne przypomnienie, że
model elektrostatyczny bez oporu obliczy sytuację, która odparowałaby
rzeczywistą część.

### Krok 7 - opcjonalnie zagęść siatkę

Zagęść siatkę raz i ponownie rozwiąż. Każdy wynik - **potencjał**, **pole
elektryczne**, **gęstość prądu**, **energia elektryczna**, **rezystywność** i
**ciepło Joule'a** - musi zachować swoją wartość. Wszystkie są gęstościami, więc
wartość dryfująca przy zagęszczaniu siatki świadczy o problemie z modelem, a nie
o drobniejszej siatce, i warto go wyjaśnić, zanim przejdzie się dalej.

### Rozwiązywanie problemów

| Objaw | Przyczyna |
|---|---|
| Rozwiązanie nie jest zbieżne | nigdzie nie ma warunku *Electric potential*, więc potencjał jest określony tylko z dokładnością do stałej |
| *Entity has material assigned which is missing required properties* | brakuje jednej z trzech wymaganych właściwości - często gęstości, której solver nigdy nie używa |
| Cały model ma jeden potencjał | zadano tylko jeden potencjał lub drugi ma tę samą wartość |
| Potencjał wynosi zero wszędzie poza zadanymi ścianami | encja ma `er = 0`, więc nie ma sztywności i niczego nie łączy |
| Rezystywność wynosi wszędzie zero | konduktywność elektryczna wynosi zero, więc nie płynie prąd, a rezystywność jest zapisywana jako zero |
| Encja powierzchni w ogóle nie pokazuje pola | jej grubość wynosi zero, więc nie jest ani składana, ani poddawana obróbce wyników |

---

## 4. Samouczek - nagrzewanie rezystancyjne

**Cel.** Wykorzystać ciepło Joule'a obliczone z pola elektrycznego do nagrzania
pręta przez połączenie w łańcuch zadania elektrostatycznego i zadania cieplnego.

### Krok 1 - zbuduj przebieg zadań

1. `Problem` -> `Przebieg zadań problemów` (`Ctrl+P`).
2. Dodaj zadanie **Electro-statics**.
3. Dodaj zadanie **Heat transfer** **po** nim.
4. Zatwierdź przyciskiem `OK`.

Kolejność ma znaczenie: solver cieplny odczytuje ciepło Joule'a pozostawione
przez solver elektrostatyczny, więc zadanie elektrostatyczne musi być wykonane
jako pierwsze. Do połączenia ich nie jest potrzebny żaden warunek ani ustawienie.

Zwróć uwagę, co dzieje się z kartą `Problem`: grupa *Solver czasowy*, którą
pokazałoby samo zadanie cieplne, zniknęła, ponieważ elektrostatyka nie ma
solvera czasowego, a przebieg przyjmuje najbardziej restrykcyjne ustawienie -
zob. 1.10. Ten łańcuch działa tylko w stanie ustalonym.

### Krok 2 - uzupełnij materiał

Zadanie cieplne potrzebuje czterech własnych właściwości oprócz trzech
potrzebnych zadaniu elektrostatycznemu, więc materiał encji musi zawierać
wszystkie poniższe:

| Właściwość | Wartość dla miedzi | Potrzebna dla |
|---|---|---|
| Relative permittivity | `2.8` | elektrostatyki |
| Electrical conductivity | `5.882e7` `S/m` | elektrostatyki |
| Density | `8960` `kg/m^3` | obu |
| Thermal conductivity | `401` `W/(m*K)` | wymiany ciepła |
| Heat capacity | `385` `J/(kg*K)` | wymiany ciepła |
| Emissivity | `0` | wymiany ciepła |

Dołączony materiał *Copper* już je zawiera. Encja, której brakuje właściwości
wymaganej przez **którekolwiek** z zadań, jest przez to zadanie pomijana.

### Krok 3 - zachowaj elektryczne warunki brzegowe

Dwa warunki **Electric potential** z samouczka 3 przenoszą się bez zmian. Obniż
potencjał wymuszający do mniej gwałtownej wartości - `0.01` V na pręcie nadal
daje duży prąd i utrzymuje źródło ciepła w zakresie, w którym wynik cieplny jest
czytelny.

### Krok 4 - dodaj cieplne warunki brzegowe

Cieplna połowa modelu potrzebuje własnego odpływu, dokładnie tak jak samodzielny
model cieplny:

1. Wybierz encję powierzchni obejmującą boki pręta.
2. Otwórz kartę `Warunki brzegowe` i zaznacz **Simple convection** (konwekcja
   prosta).
3. Ustaw **Convection coefficient** (współczynnik konwekcji) = `10`
   `W/(m^2*K)` i **Fluid temperature** (temperatura płynu) = `293.15` K.

Bez powierzchni z konwekcją lub zadanej temperatury model cieplny ma źródło, ale
nie ma odpływu, i stan ustalony nie istnieje. Zauważ, że ściana używana do
konwekcji nie potrzebuje grubości dla solvera cieplnego, ale ściana o zerowej
grubości nie przenosi też pola elektrycznego - pozostaw warunki elektryczne na
ścianach końcowych, a konwekcję na bokach, a oba nie będą się zakłócać.

### Krok 5 - rozwiąż

`Rozwiązanie` -> `Uruchom solver` (`Ctrl+R`). Dziennik pokazuje najpierw zadanie
elektrostatyczne ze statystykami jego sześciu zmiennych, a następnie zadanie
cieplne ze statystykami temperatury, strumienia ciepła i współczynnika
przejmowania ciepła.

### Krok 6 - przejrzyj wyniki

- Karta `Wyniki`: wyświetl **Joule heat**. Jest to pomost między dwiema
  fizykami - wszędzie tam, gdzie jest niezerowe, solver cieplny widział źródło.
- Wyświetl **Temperature**. Pręt powinien być najcieplejszy tam, gdzie ciepło
  Joule'a jest największe, a temperatura powinna spadać w kierunku temperatury
  płynu przy ścianach z konwekcją.
- Wyświetl **Electric potential**, aby potwierdzić, że elektryczna połowa modelu
  to nadal liniowe pole z samouczka 3.

### Krok 7 - sprawdź bilans energii

Ciepło Joule'a jest gęstością rozpraszania w `W/m^3`, więc całkowita moc
dostarczana przez elektryczną połowę modelu do cieplnej to ta gęstość pomnożona
przez objętość, na którą działa. Dla jednorodnego pręta powinna ona wynieść
`P = U^2/R` z samouczka 3, przy `R = L/(sigma*A)`.

W stanie ustalonym powierzchnia z konwekcją musi oddawać dokładnie tę moc, co daje
to samo globalne sprawdzenie, które opisuje podręcznik wymiany ciepła:

```
P = h * A_s * ( Ts_mean - Tf )
```

Odczytaj średnią temperaturę powierzchni ze statystyk w dzienniku solvera i
potwierdź, że bilans się zamyka. To jedyne sprawdzenie obejmujące obie fizyki
jednocześnie: wychwytuje błędną konduktywność, błędny współczynnik konwekcji oraz
encję, która w którejkolwiek połowie po cichu pozostała nierozwiązana.

### Rozwiązywanie problemów

| Objaw | Przyczyna |
|---|---|
| Temperatura jest jednolita i wynosi `293.15` K | ciepło Joule'a wynosi zero - sprawdź konduktywność elektryczną oraz to, czy zadanie elektrostatyczne jest wykonywane jako pierwsze |
| Rozwiązanie cieplne nie jest zbieżne | cieplna połowa nie ma zadanej temperatury ani powierzchni z konwekcją |
| Grupa *Solver czasowy* zniknęła | oczekiwane - zadanie elektrostatyczne sprawia, że cały przebieg działa w stanie ustalonym, zob. 1.10 |
| Przyrost temperatury zmienia się przy zagęszczaniu siatki | nieoczekiwane - każdy wynik elektrostatyczny jest gęstością, więc sprawdź cieplną połowę modelu i jakość siatki |
| Pojawiają się tylko wyniki elektrostatyczne | zadaniu cieplnemu brakuje jednej z jego czterech wymaganych właściwości materiału |

---

## 5. Sprawdzanie modelu

Solver nie ma wbudowanego zestawu testów weryfikacyjnych, więc przed zaufaniem
nowemu modelowi warto go sprawdzić względem czegoś, co można obliczyć ręcznie.
Poniższe sprawdzenia dotyczą **potencjału**, który jest zarazem niewiadomą
podstawową i wielkością, w której wyrażone są wszystkie rozwiązania w postaci
zamkniętej; wyniki pola elektrycznego i gęstości prądu powinny zgadzać się z
wynikającymi z niego gradientami.

**Jednorodny pręt.** Dla ciała o długości `L` utrzymywanego na `V1` i `V2` na
dwóch przeciwległych ścianach, z jednym materiałem i bez ładunku, potencjał jest
liniowy wzdłuż pręta i

```
V(x) = V1 + ( V2 - V1 ) * x / L
E    = ( V1 - V2 ) / L          [V/m]
```

Umieść punkt monitorowania w płaszczyźnie środkowej i potwierdź, że wskazuje
średnią obu zadanych wartości, a drugi w jednej czwartej długości dla wartości
ćwiartkowej. Wychwytuje to niepołączoną siatkę, warunek przypisany do niewłaściwej
encji i zerową przenikalność. Zagęść siatkę raz i potwierdź, że wartości się nie
zmieniają.

**Jednorodny ładunek przestrzenny.** Powyższy pręt sprawdza tylko sztywność. Aby
sprawdzić także człon źródłowy, weź to samo ciało, utrzymuj **obie** ściany
końcowe na `0` V i przypisz objętości jednorodną *Charge density* `rho`.
Potencjał jest wtedy parabolą, przy `eps = e0*er`:

```
V(x)  = rho * x * ( L - x ) / ( 2 * eps )
Vmax  = rho * L^2 / ( 8 * eps )          at the mid-plane
```

(`at the mid-plane` = w płaszczyźnie środkowej.) Punkt monitorowania w
płaszczyźnie środkowej daje bezpośrednio `Vmax`. Sprawdzane są jednocześnie dwie
rzeczy: wielkość, która potwierdza, że przenikalność i gęstość ładunku są
odczytywane zgodnie z oczekiwaniami, oraz **znak** - dodatnia gęstość ładunku
musi dawać dodatnie wybrzuszenie potencjału między dwiema uziemionymi ścianami.
Powtórz z ładunkiem na encji powierzchni lub linii oraz na punkcie i potwierdź,
że biegunowość nie zmienia się wraz z typem encji.

**Rezystywność.** Każdy element, przez który płynie prąd, musi zapisywać
rezystywność elektryczną równą dokładnie `1/sigma`. Odczytaj ją ze statystyk w
dzienniku solvera i porównaj z konduktywnością w materiale. Jest dokładna, a nie
przybliżona, niezależna od siatki i stanowi najszybsze potwierdzenie, że materiał
dotarł do elementów, do których miał dotrzeć.

**Pole współosiowe.** Dla pierścienia między promieniami `a` i `b` z
powierzchnią wewnętrzną na `V1` i zewnętrzną na `V2` potencjał jest
logarytmiczny:

```
V(r) = V2 + ( V1 - V2 ) * ln( b / r ) / ln( b / a )
```

Zamodeluj pierścień, zadaj obie powierzchnie i porównaj punkty monitorowania na
kilku promieniach. Weryfikuje to solver na polu niejednorodnym, czego pręt nie
potrafi, i pokazuje, czy siatka jest wystarczająco gęsta w pobliżu przewodnika
wewnętrznego.

**Dielektryki szeregowe.** Dla dwóch warstw o grubościach `d1`, `d2` i względnych
przenikalnościach `er1`, `er2` ułożonych między dwoma zadanymi potencjałami
potencjał na granicy warstw wynosi

```
Vi = ( V1*er1/d1 + V2*er2/d2 ) / ( er1/d1 + er2/d2 )
```

Porównaj z obliczonym potencjałem na granicy. Bezpośrednio potwierdza to, co
opisuje punkt 1.9: podział następuje według **przenikalności**. Uruchomienie tego
samego modelu z dwiema różnymi konduktywnościami i tą samą przenikalnością oraz
obserwacja, że potencjał na granicy się nie przesuwa, dowodzi, że jest to solver
elektrostatyczny, a nie solver przepływu prądu.

**Rezystancja i rozpraszanie.** Wyniki elementowe są gęstościami, więc wartość
całkowita - prąd, rezystancja, moc - musi pochodzić z potencjału. Dla
jednorodnego pręta:

```
R = L / ( sigma * A )      [ohm]
I = U / R                  [A]
P = U * I = U^2 / R        [W]
```

Używaj ich zawsze, gdy wartość całkowita ma opuścić model - przy doborze
przewodnika lub dla kontroli poprawności względem pomiaru. `P` jest także
wartością, do której musi się scałkować wynik **Joule heat** po objętości pręta,
co sprawdza, czy elektryczna i cieplna połowa obliczeń sprzężonych są zgodne.

Przy każdym z tych sprawdzeń zagęść siatkę co najmniej raz i potwierdź, że
potencjał jest zbieżny, a nie dryfuje.

---

## 6. Ograniczenia

### Ograniczenia modelowania

- **Elektrostatyka, a nie przepływ prądu.** Sztywność jest budowana z
  przenikalności; konduktywność nigdy nie wchodzi do układu równań. Potencjał w
  ciele z dwóch materiałów dzieli się więc proporcjonalnie do przenikalności, a
  nie rezystancji. Przewodnik jednomateriałowy jest modelowany poprawnie; sieć
  rezystorów - nie, zob. 1.9.
- **Tylko analiza statyczna.** Nie ma solvera czasowego, pojemności ani
  indukcyjności, więc stany przejściowe ładowania, pola przemienne, prąd
  przesunięcia, prądy wirowe i efekt naskórkowości wykraczają poza model. Dodanie
  zadania elektrostatycznego do przebiegu zadań usuwa także solver czasowy z
  całego przebiegu, więc zadanie cieplne połączone za nim może działać tylko w
  stanie ustalonym - zob. 1.10.
- **Tylko liniowe, izotropowe dielektryki.** Przenikalność i konduktywność
  elementu są skalarami i są stałe podczas rozwiązania. Materiały anizotropowe,
  ferroelektryczne i zależne od pola nie są modelowane, nie ma też przebicia
  dielektryka.
- **Tylko dwa warunki brzegowe.** Zadany potencjał i gęstość ładunku stanowią
  całe słownictwo. Nie ma zadanego ładunku powierzchniowego, zadanego strumienia
  normalnego, warunku symetrii ani traktowania otwartego brzegu czy elementów
  nieskończonych - nieograniczony problem polowy trzeba obciąć, siatkując
  wystarczająco dużo otaczającego powietrza i zadając potencjał na jego
  zewnętrznym brzegu.
- **Brak swobodnej redystrybucji ładunku.** Ładunek nie przemieszcza się na
  powierzchnię przewodnika; solver nie ma żadnego pojęcia przewodnika poza
  nadanymi mu właściwościami materiału. Przewodnik modeluj przez zadanie jego
  potencjału, a nie przez nadanie mu dużej konduktywności.
- **Brak sprzężenia zwrotnego od prądu.** Gęstość prądu jest wyznaczana z pola w
  obróbce wyników, więc konduktywność zmieniająca się wraz z polem lub z
  temperaturą wytwarzaną przez rozpraszanie nie ma wpływu w ramach jednego
  rozwiązania.
- **Powierzchnie są ekwipotencjalne w kierunku grubości.** Encja powierzchni
  przenosi pole tylko w swojej płaszczyźnie. Powierzchnia o zerowej grubości lub
  linia o zerowym polu przekroju nie ma żadnej sztywności i jest pomijana
  zarówno przy składaniu, jak i w obróbce wyników.
- **Elementy punktowe nie przewodzą.** Punkt przenosi źródło ładunku i może mieć
  zadany potencjał, nic więcej, i nigdy nie jest poddawany obróbce wyników.
  Punkt, którego węzeł nie należy do żadnego innego obliczanego elementu,
  pozostawia w macierzy pusty wiersz.
- **Zależność materiału od temperatury nie jest odczytywana.** Tablice są
  obliczane dla `293.15 K`, chyba że zadanie cieplne wcześniej w przebiegu
  wytworzyło temperatury elementów; warunek otoczenia *Temperature* nie ma
  wpływu na ten solver.
- **Model potrzebuje zadanego potencjału.** Bez niego macierz ma jądro
  nietrywialne, a rozwiązanie nie będzie zbieżne.

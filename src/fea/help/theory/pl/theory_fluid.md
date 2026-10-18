# Przepływ płynów - Podręcznik teorii i przewodnik użytkownika

Ten dokument opisuje solver przepływu płynów programu Range FEA - `RSolverFluid`,
obsługujący typ problemu **Incompressible viscous flow**: rozwiązywane
równania, znaczenie wszystkich przyjmowanych danych wejściowych, elementy
interfejsu graficznego, które nim sterują, oraz kompletny samouczek.

Dwa typy problemów rozwiązywane na polu przepływu mają własne podręczniki:
[Wymiana ciepła w płynach](theory_fluid_heat.md) (`RSolverFluidHeat`) oraz
[Dyspersja zanieczyszczeń](theory_fluid_particle.md) (`RSolverFluidParticle`).

**Spis treści**

1. [Podstawy teoretyczne](#1-podstawy-teoretyczne)
2. [Interfejs graficzny](#2-interfejs-graficzny)
3. [Samouczek - ustalony przepływ przez kanał](#3-samouczek---ustalony-przepływ-przez-kanał)
4. [Sprawdzanie modelu](#4-sprawdzanie-modelu)
5. [Ograniczenia](#5-ograniczenia)

---

## 1. Podstawy teoretyczne

### 1.1 Niewiadome podstawowe - pola prędkości i ciśnienia

Niewiadomymi podstawowymi **Incompressible viscous flow** (nieściśliwego
przepływu lepkiego) są wektor **prędkości** `v` w `m/s` i **ciśnienie** `p` w
`Pa`, oba w każdym węźle. Każdy węzeł ma **cztery** stopnie swobody - `vx`, `vy`,
`vz` i `p` - więc siatka o `N` węzłach daje układ `4N` równań.

To czyni model przepływu najkosztowniejszym typem fizyki w Range FEA: cztery
niewiadome na węzeł wobec trzech dla modelu konstrukcyjnego i jednej dla modelu
cieplnego, macierz ani symetryczna, ani dodatnio określona, a do tego iteracja
nieliniowa z punktu 1.6 wokół każdego rozwiązania liniowego.

Dwa towarzyszące typy problemów dodają po jednym skalarze, rozwiązywanym na polu
przepływu wytworzonym przez solver płynu:

| Typ problemu | Solver | Niewiadoma podstawowa | Jednostki | Stopnie swobody na węzeł |
|---|---|---|---|---|
| Incompressible viscous flow | `RSolverFluid` | prędkość i ciśnienie | `m/s`, `Pa` | 4 |
| Heat transfer in fluids | `RSolverFluidHeat` | temperatura | `K` | 1 |
| Contaminant dispersion | `RSolverFluidParticle` | stężenie cząstek | `kg/m^3` | 1 |

Oba typy towarzyszące **wymagają** zadania *Incompressible viscous flow* w tym
samym problemie: odczytują pole prędkości z jego wyniku i nigdy same go nie
obliczają. Każdy z nich jest opisany we własnym podręczniku -
[Wymiana ciepła w płynach](theory_fluid_heat.md) oraz
[Dyspersja zanieczyszczeń](theory_fluid_particle.md).

Domyślnym stanem węzła, którego nie dotyczy żaden warunek ani żaden wcześniejszy
wynik, jest zerowa prędkość i zerowe ciśnienie.

### 1.2 Równania podstawowe

Solver realizuje **równania Naviera-Stokesa dla płynu nieściśliwego** - płynu
newtonowskiego o gęstości `rho` i lepkości dynamicznej `mu`. Obie są
właściwościami materiału obliczanymi dla każdego elementu w jego temperaturze -
w 1.9 opisano, skąd pochodzi ta temperatura i jak wywołuje wypór w przepływie.
Bilans pędu i masy ma postać

```
rho * ( dv/dt + (v . grad) v ) = -grad(p) + div( mu * ( grad(v) + grad(v)^T ) ) + rho * g

div(v) = 0
```

a przy wyłączonym solverze czasowym człon nieustalony znika, pozostawiając
bilans stanu ustalonego

```
rho * (v . grad) v = -grad(p) + div( mu * ( grad(v) + grad(v)^T ) ) + rho * g

div(v) = 0
```

| Symbol | Znaczenie | Jednostki | Źródło |
|---|---|---|---|
| `v` | prędkość | `m/s` | rozwiązywana |
| `p` | ciśnienie | `Pa` | rozwiązywane |
| `rho` | gęstość | `kg/m^3` | właściwość materiału |
| `mu` | lepkość dynamiczna | `kg/(m*s)`, czyli `Pa*s` | właściwość materiału |
| `g` | przyspieszenie ziemskie | `m/s^2` | warunek otoczenia Gravitational acceleration |

Ciśnienie nie jest tu wielkością termodynamiczną. Nieściśliwość pozostawia je
jako mnożnik Lagrange'a wymuszający `div(v) = 0`, co ma dwie praktyczne
konsekwencje, które warto przyswoić przed odczytaniem jakiegokolwiek wyniku:

- Znaczenie mają tylko **różnice ciśnień**. Model, w którym żaden warunek nigdzie
  nie ustala ciśnienia, wyznacza pole ciśnienia tylko z dokładnością do stałej
  addytywnej, a macierz jest osobliwa względem tej stałej - zob. 1.7.
- Ciśnienie reaguje natychmiast w całym obszarze. W tym sformułowaniu nie ma fali
  akustycznej ani prędkości dźwięku, więc ciśnienie przyłożone na wylocie jest
  odczuwalne na wlocie w tym samym rozwiązaniu.

**Liczba Reynoldsa** zbudowana z długości charakterystycznej `L` i prędkości `V`,

```
Re = rho * V * L / mu
```

jest tą jedną liczbą, która decyduje o tym, czy model przepływu będzie się
dobrze zachowywał. Poniżej mniej więcej `Re = 1` człon konwekcyjny jest
pomijalny, a problem niemal liniowy; do kilku tysięcy przepływ jest laminarny, a
solver jest w swoim żywiole; znacznie powyżej tej wartości rzeczywisty przepływ
jest turbulentny, a ten solver nie modeluje turbulencji - zob. rozdział 5.

### 1.3 Dyskretyzacja metodą elementów skończonych i stabilizacja

Prędkość i ciśnienie są interpolowane **tymi samymi** liniowymi funkcjami
kształtu w każdym elemencie - interpolacja równego rzędu `P1/P1` i `Q1/Q1`. Takie
połączenie samo w sobie nie spełnia warunku inf-sup (Babuški-Brezziego) i
dałoby pole ciśnienia w szachownicę, więc sformułowanie jest **stabilizowane**.
Dodawane są trzy człony stabilizujące, każdy z własnym parametrem na poziomie
elementu:

| Człon | Cel |
|---|---|
| **SUPG** - streamline upwind Petrov-Galerkin | stabilizuje człon konwekcyjny, który w przeciwnym razie dawałby oscylacje między węzłami w przepływie zdominowanym przez konwekcję |
| **PSPG** - pressure stabilising Petrov-Galerkin | dopuszcza prędkość i ciśnienie równego rzędu, usuwając szachownicowy tryb ciśnienia |
| **LSIC** - least-squares on the incompressibility constraint | tłumi błąd dywergencji, poprawiając zachowanie masy element po elemencie |

(SUPG = metoda Petrova-Galerkina z upwindingiem wzdłuż linii prądu, PSPG = metoda
Petrova-Galerkina stabilizująca ciśnienie, LSIC = najmniejsze kwadraty na więzie
nieściśliwości.) Parametry SUPG i PSPG są budowane z elementowej liczby
Reynoldsa

```
h   = element length along the flow direction
Re  = rho * |v| * h / ( 2 * mu )            element Reynolds number

tau = h * Re / ( 6 * |v| )      for  0 < Re <= 3
tau = h / ( 2 * |v| )           for  Re > 3
```

(`element length along the flow direction` = długość elementu w kierunku
przepływu, `element Reynolds number` = elementowa liczba Reynoldsa, `for` = dla.)
Upwinding jest więc silny tam, gdzie w elemencie dominuje konwekcja, i zanika
tam, gdzie dominuje dyfuzja. Parametr LSIC wynosi `|v| * h / 2`. Te parametry
należą do solvera przepływu; solver dyspersji używa własnego parametru SUPG
ograniczonego krokiem czasowym, zob. podręcznik
[Dyspersja zanieczyszczeń](theory_fluid_particle.md).

Parametr SUPG używa **lokalnej** prędkości elementu i długości elementu, więc
dostosowuje się element po elemencie. Parametr PSPG używa natomiast **globalnej
prędkości strumienia** i globalnej skali długości elementu, co utrzymuje
jednolitą stabilizację ciśnienia w całej siatce. Prędkość strumienia jest
pobierana z warunków wlotowych przy pierwszym przejściu każdego rozwiązania -
każdego kroku czasowego w obliczeniach przejściowych - a nie z rozwiązywanego
pola,

```
V_stream = sum( |Q| over inflow surfaces ) / sum( area of inflow surfaces )
```

(`over inflow surfaces` = po powierzchniach wlotowych, `area of inflow surfaces`
= pole powierzchni wlotowych.) Jest to więc średnia ważona polem prędkość
wlotowa, z wartością zastępczą `1 m/s`, gdy model w ogóle nie ma warunku
wlotowego. Skala długości elementu jest wyprowadzana z objętości elementu:
`cbrt(6*V/pi)` dla czworościanu, `cbrt(V)` dla sześciościanu.

**Obsługiwane elementy.** Równania przepływu są składane **tylko na elementach
objętościowych** i tylko na **liniowych czworościanach** (`TETRA1`) i
**liniowych sześciościanach** (`HEXA1`). Każdy inny typ elementu objętościowego
zatrzymuje obliczenia z błędem podającym ten typ. Elementy powierzchniowe biorą
udział tylko tam, gdzie mają warunek *Pressure (implicit)*, wnosząc wtedy
obciążenie ciśnieniem i nic więcej. Encje punktów i linii nie wnoszą niczego - w
przeciwieństwie do modelu cieplnego nie są przewodnikami niższego wymiaru, po
prostu ich nie ma.

To najbardziej wyraźna praktyczna różnica w stosunku do innych fizyk w Range FEA:
model płynu musi być **siatkowaną objętością**. Wygeneruj siatkę czworościenną za
pomocą `Geometria` -> `Objętość` -> `Generuj siatkę czworościenną`, zanim
zaczniesz oczekiwać, że rozwiązanie przepływu cokolwiek zrobi.

### 1.4 Warunki brzegowe

| Warunek brzegowy | Typ | Dotyczy | Składowe |
|---|---|---|---|
| Wall | jawny | punkt, linia, powierzchnia | brak |
| Wall (frictionless) | jawny | punkt, linia, powierzchnia | brak |
| Velocity (inflow) | jawny | powierzchnia | Velocity `[m/s]` |
| Volumetric flow rate (inflow) | jawny | powierzchnia | Volumetric flow rate `[m^3/s]` |
| Pressure (explicit) | jawny | powierzchnia | Pressure `[Pa]` |
| Pressure (implicit) | naturalny, obciążenie | powierzchnia | Pressure `[Pa]` |

**Wall** (ściana) jest warunkiem braku poślizgu. W każdym węźle encji wszystkie
trzy składowe prędkości są usuwane z układu i utrzymywane na zerze. Warunek nie
ma składowych - nie ma niczego do wprowadzenia, zaznaczenie go stanowi całą
konfigurację. Jest to warunek dla każdego brzegu stałego, którego dotyka płyn.

**Wall (frictionless)** (ściana bez tarcia) jest warunkiem poślizgu lub symetrii:
płyn może płynąć wzdłuż powierzchni, ale nie przez nią. Jest stosowany przez
znalezienie **dominującej osi globalnej** normalnej każdego elementu powierzchni i
utrzymywanie na zerze tylko tej jednej składowej prędkości, przy pozostawieniu
dwóch pozostałych swobodnymi. Zwróć uwagę, co to oznacza: warunek jest dokładny
tylko dla powierzchni, której normalna leży wzdłuż `x`, `y` lub `z`. Na
powierzchni odchylonej od osi globalnych wiąże niewłaściwy kierunek, a im bliżej
normalna jest kąta 45 stopni między dwiema osiami, tym gorsze przybliżenie.
Używaj go dla płaskich płaszczyzn symetrii modelu wyrównanego z osiami
globalnymi, a gdzie indziej używaj *Wall*.

**Velocity (inflow)** (prędkość wlotowa) zadaje prędkość o podanej wartości
skierowaną wzdłuż **średniej normalnej** encji powierzchni, do wnętrza płynu.
Znak jest pobierany z orientacji elementów encji względem objętości za nimi,
więc wartość dodatnia oznacza przepływ wpływający do obszaru, a ujemna -
wypływający z niego. Wprowadza się prędkość skalarną, a nie wektor - kierunek
wynika z geometrii.

**Volumetric flow rate (inflow)** (objętościowe natężenie przepływu na wlocie)
zadaje to samo na podstawie całkowitego natężenia przepływu: solver dzieli
natężenie przez pole encji i stosuje wynikową prędkość średnią wzdłuż tej samej
średniej normalnej.

```
v = Q / A          A = area of the entity
```

(`area of the entity` = pole encji.) Sięgaj po ten warunek zawsze, gdy faktycznie
znaną wielkością jest wydajność - parametr pompy, kanał w `m^3/s` - a po
*Velocity (inflow)*, gdy znasz prędkość. Oba utrzymują wszystkie trzy składowe
prędkości w każdym węźle encji, więc oba czynią z wlotu jednorodny przepływ
tłokowy; żaden nie może wytworzyć rozwiniętego profilu na ścianie wlotowej.
Jeśli profil w tym miejscu ma znaczenie, nadaj modelowi odcinek wlotowy.

**Pressure (explicit)** (ciśnienie jawne) usuwa ciśnieniowy stopień swobody z
każdego węzła encji i utrzymuje go na podanej wartości. Jest to ciśnieniowy
odpowiednik zadanej prędkości i jeden ze sposobów ustalenia stałej addytywnej z
punktu 1.2.

**Pressure (implicit)** (ciśnienie niejawne) jest jego naturalnym odpowiednikiem:
zamiast ustalać ciśnienie węzłowe, przykłada **obciążenie ciśnieniem** do
elementów powierzchni i pozostawia tam prędkość swobodną, więc to płyn decyduje,
ile wypływa i z jakim profilem. Obciążenie obejmuje wysokość hydrostatyczną,

```
t = -( p + rho * g * h ) * n
```

gdzie `h` jest wysokością węzła mierzoną **wzdłuż kierunku grawitacji od
najniższego punktu** modelu, a `n` jest normalną do powierzchni. Kierunek
grawitacji jest średnią warunków otoczenia *Gravitational acceleration*
przypisanych do powierzchni z tym warunkiem. Bez grawitacji człon wysokości
znika, a obciążenie jest samym wprowadzonym ciśnieniem.

Jest to warunek wylotowy, po który należy sięgnąć niemal w każdym modelu:
pozwala rozwinąć się profilowi wypływu, zamiast wymuszać płaski, a wprowadzenie
`0` czyni z wylotu odniesienie przy ciśnieniu atmosferycznym. *Pressure
(explicit)* jest wyborem sztywniejszym, bardziej narzucającym.

Każda składowa każdego z tych warunków jest **tablicą w funkcji czasu**, więc wlot
można stopniowo zwiększać przez pierwsze sekundy obliczeń przejściowych, a
natężeniu przepływu nadać cykl pracy.

**Pierwszeństwo.** Warunki prędkości i warunki ciśnienia są niezależne - wiążą
różne stopnie swobody, więc powierzchnia wlotowa może mieć warunek prędkości, a
wylotowa warunek ciśnienia bez wzajemnego zakłócania. Wśród warunków tego samego
rodzaju na tej samej encji obowiązuje ostatni odczytany, a *Wall* zeruje prędkość
swoich węzłów po zastosowaniu wszystkich innych warunków prędkości. Encja, która
jest jednocześnie wlotem i ścianą, jest więc ścianą.

### 1.5 Warunki otoczenia i początkowe

#### Warunki otoczenia

| Warunek otoczenia | Składowe | Działanie |
|---|---|---|
| Gravitational acceleration | `gx`, `gy`, `gz` `[m/s^2]` | siła masowa `rho*g` na encji oraz człon wysokości warunku *Pressure (implicit)* |
| Temperature | Temperature `[K]` | temperatura tła zadania *Heat transfer in fluids*; nie jest odczytywana przez sam solver przepływu - zob. 1.9 |

**Gravitational acceleration** (przyspieszenie ziemskie) ma wartość domyślną
`(0, 0, -9.80665) m/s^2`, czyli grawitację wzdłuż ujemnej osi `Z`. Przypisz je
do encji objętości dla siły masowej oraz do powierzchni wylotowej, aby wysokość
w *Pressure (implicit)* była mierzona we właściwym kierunku.

Grawitacja wchodzi do równania pędu jako siła masowa `rho*g`. W zamkniętym
obszarze o jednorodnej gęstości jednorodne pole grawitacji wytwarza
hydrostatyczny gradient ciśnienia i żadnego ruchu - co jest poprawną odpowiedzią.
Tam, gdzie gęstość zmienia się z temperaturą, zmienia się wraz z nią siła masowa,
a różnica wywołuje **przepływ wyporowy** - konwekcję naturalną, zob. 1.9. Wynik
ciśnienia zawsze zawiera część hydrostatyczną `rho*(g . x)`; nic nie jest
odejmowane.

#### Warunki początkowe

| Warunek początkowy | Działanie |
|---|---|
| Velocity | początkowe pole prędkości, stosowane przy pierwszym uruchomieniu |
| Pressure | początkowe pole ciśnienia, stosowane przy pierwszym uruchomieniu |

Początkowe pola temperatury i stężenia towarzyszących typów problemów są opisane
w ich własnych podręcznikach.

Warunki początkowe są odczytywane **tylko przy pierwszym uruchomieniu**; potem
pola są prowadzone od poprzedniego wyniku. To właśnie umożliwia przebieg pracy z
ponownym uruchomieniem z punktu 1.7: obliczenia przejściowe uruchomione z
**Uruchom solver ponownie / kontynuuj** przejmują zbieżne pole stanu ustalonego,
zamiast znów zaczynać od spoczynku.

Tam, gdzie warunek brzegowy i warunek początkowy się spotykają, pierwszeństwo ma
warunek brzegowy.

### 1.6 Iteracja nieliniowa

Człon konwekcyjny `(v . grad) v` jest kwadratowy względem niewiadomej, więc
równania przepływu są **nieliniowe** i nie można ich rozwiązać w jednym
przejściu. Solver je linearyzuje i iteruje: każde przejście składa residuum
bilansu pędu i masy w bieżącym polu, rozwiązuje **przyrost** i go dodaje,

```
J * dx = -R(x)
x <- x + dx
```

gdzie `x` zawiera węzłowe prędkości i ciśnienia. Lewa strona jest przybliżonym,
a nie dokładnym jakobianem, co czyni ze schematu zmodyfikowaną iterację Newtona:
w pobliżu rozwiązania zbiega wolniej niż pełna metoda Newtona, ale rozwiązanie, do
którego zbiega, jest wyznaczone przez **residuum** `R`, czyli przez same
zdyskretyzowane równania Naviera-Stokesa.

**Krok jest tłumiony.** Ponieważ macierz nie jest dokładną pochodną residuum,
pełny krok `dx` może przestrzelić i pozostawić pole dalej od rozwiązania niż na
początku - residuum następnego przejścia wychodzi wtedy wyższe niż poprzednie.
Solver dokładnie to obserwuje i stosuje tylko część obliczonego kroku,

```
x <- x + omega * dx
```

zaczynając od `omega = 1`. Przejście, które pogorszyło residuum, zmniejsza ją o
połowę, do dolnej granicy `0.1`; przejście, które je poprawiło, pozwala `omega`
odrastać za każdym razem o jedną czwartą, więc wycofanie jest szybkie, a powrót
rozważny. Obowiązująca wartość jest wypisywana w dzienniku jako `Relaxation`
(relaksacja) i pozostaje równa `1` w obliczeniach, które nigdy nie
przestrzeliwują - dobrze zachowujący się model przepływu w ogóle nie zauważa, że
ten mechanizm istnieje. `omega` jest ustawiana z powrotem na `1` na początku każdego
rozwiązania, co w obliczeniach przejściowych oznacza każdy krok czasowy.

Wynikają z tego dwie konsekwencje, które kształtują cały przebieg pracy z modelem
przepływu:

- **Jedno przejście nigdy nie wystarcza.** Pojedyncza iteracja zadania przepływu
  zwraca to, co akurat dał pierwszy przyrost, co nie jest rozwiązaniem
  czegokolwiek. Zadanie przepływu musi być umieszczone w **grupie zadań
  problemów** z liczbą iteracji - setkami dla obliczeń w stanie ustalonym,
  dziesiątkami na krok czasowy dla obliczeń przejściowych.
- **Liczba jest górną granicą, a nie celem.** Grupa ma także **wartość
  zbieżności** i kończy swoje iteracje, gdy tylko zadanie przepływu zgłosi
  zbieżność poniżej tej wartości. Ustaw liczbę wystarczająco wysoko dla
  najtrudniejszego kroku i pozwól, by wartość zbieżności zatrzymywała każdy z
  nich, gdy się ustabilizuje.

Po każdej iteracji solver zapisuje cztery liczby do pliku zbieżności i do
dziennika:

| Wielkość | Znaczenie |
|---|---|
| `Residual` | norma wektora residuum, odległość od rozwiązania |
| `Convergence-R` | zmiana residuum od poprzedniej iteracji |
| `Convergence-V` | względna wielkość przyrostu prędkości, `\|\|dv\|\| / \|\|v\|\|` |
| `Convergence-P` | względna wielkość przyrostu ciśnienia, `\|\|dp\|\| / \|\|p\|\|` |

To z `Convergence-V` i `Convergence-P` porównywana jest wartość zbieżności. Obie
są bezwymiarowe i obie maleją, gdy iteracja się stabilizuje: mierzą, jak duży
krok pole nadal wykonuje względem wielkości samego pola, więc wartość `1e-5`
oznacza, że w danym przejściu rozwiązanie przesunęło się o jedną
stutysięczną siebie.

Mały krok sam w sobie nie jest jednak rozwiązaniem. Niemal osobliwy układ - model
bez odniesienia ciśnienia lub siatka, która nie radzi sobie z wymaganą liczbą
Reynoldsa - wykonuje maleńkie kroki, ponieważ **nie może się ruszyć**, a nie
dlatego, że dotarł do celu, a jego residuum stoi w miejscu lub rośnie. Test ma
więc dwie części, a grupa zatrzymuje się tylko wtedy, gdy obie są spełnione:

| Część | Warunek |
|---|---|
| Pole przestało się zmieniać | `Convergence-V` i `Convergence-P` są obie poniżej wartości zbieżności |
| Pole faktycznie zmalało | residuum spadło do **jednej dziesiątej** wartości z pierwszego przejścia tego rozwiązania |

Druga część jest stała i nie podlega konfiguracji - jest kontrolą poprawności
pierwszej, a nie drugim pokrętłem do strojenia. Dziennik wypisuje ją jako
`Residual ratio` (stosunek residuów) obok wartości docelowej, więc widać, jak
blisko zatrzymania znalazły się obliczenia, które nie zatrzymały się wcześniej. W
obliczeniach przejściowych każdy krok czasowy jest osobnym rozwiązaniem, więc
stosunek jest mierzony od nowa od początku każdego kroku.

`Raport` -> `Zbieżność solvera` rysuje wszystkie cztery liczby. Zbieżne
obliczenia w stanie ustalonym pokazują residuum spadające o kilka rzędów
wielkości, a następnie wypłaszczające się, przy czym `Convergence-V` i
`Convergence-P` monotonicznie dążą do zera. Residuum, które wypłaszcza się
wysoko, oscyluje lub rośnie, oznacza, że obliczenia nie osiągnęły zbieżności - a
grupa nie zatrzyma się wcześniej w takich obliczeniach, niezależnie od
przyrostów.

Wartość zbieżności `0` wyłącza wcześniejsze zakończenie i wykonuje pełną liczbę
iteracji, tak jak solver działał, zanim ta wartość istniała. Sięgaj po nią, gdy
chcesz stałej ilości pracy na krok niezależnie od zachowania residuum.

### 1.7 Analiza w stanie ustalonym

Przy wyłączonym solverze czasowym solver iteruje ustalony bilans pędu i masy do
punktu stałego. Układ liniowy każdej iteracji nie jest symetryczny, więc jest
rozwiązywany metodą **GMRES** z prekondycjonowaniem Jacobiego, a nie metodą
gradientów sprzężonych używaną przez fizyki symetryczne.

Model przepływu w stanie ustalonym potrzebuje co najmniej:

- siatkowanej **objętości** z przypisaną gęstością i lepkością dynamiczną;
- warunku **Wall** na każdym brzegu stałym;
- warunku **wlotowego** lub różnicy ciśnień, aby wymusić przepływ;
- **odniesienia ciśnienia** - powierzchni *Pressure (explicit)* lub *Pressure
  (implicit)*. Bez niego ciśnienie jest wyznaczone tylko z dokładnością do stałej,
  a iteracja się nie ustabilizuje.

Model, którego brzegi są w całości ścianami i zadanymi prędkościami, ma jeszcze
jedną pułapkę: zadany wpływ i wypływ muszą się **równoważyć**, ponieważ płyn
nieściśliwy nie może się gromadzić. Zadanie 5 m^3/s na wlocie i 4 m^3/s na
wylocie to żądanie niemożliwego, a rozwiązanie nie będzie zbieżne. Pozostawienie
wylotu jako *Pressure (implicit)* całkowicie omija ten problem, co jest głównym
powodem, by go preferować.

Zwykły przebieg pracy polega na doprowadzeniu do zbieżności najpierw obliczeń w
stanie ustalonym i użyciu ich jako pola startowego obliczeń przejściowych,
ponieważ obliczenia przejściowe rozpoczęte od spoczynku spędzają pierwsze kroki
czasowe na rozwiązywaniu stanu przejściowego rozruchu, a nie interesującej
fizyki.

### 1.8 Analiza przejściowa

Przy włączonym solverze czasowym solver całkuje w czasie równania nieustalone
schematem theta, w którym `alpha` jest współczynnikiem przybliżenia schematu
czasowego:

| Przybliżenie | `alpha` | Wynikowy schemat | Stabilność |
|---|---|---|---|
| Backward difference (stable) | `1` | krok w pełni niejawny | bezwarunkowa |
| Central difference (accurate) | `0.5` | Crank-Nicolson | bezwarunkowa, możliwe oscylacje |
| Forward difference (fast) | `0` | krok jawny | warunkowa |

**Backward difference** jest ustawieniem, którego należy używać dla modelu
przepływu. Czas pochłania iteracja nieliniowa w każdym kroku czasowym, a schemat
niejawny pozwala dobrać krok czasowy do fizyki, a nie do siatki.

Nawet przy schemacie niejawnym krok czasowy modelu przepływu jest ograniczony
dokładnością, poprzez **liczbę Couranta**

```
C = |v| * dt / h
```

gdzie `h` jest rozmiarem elementu w kierunku przepływu. Liczba Couranta bliska
`1` oznacza, że płyn przechodzi w jednym kroku przez mniej więcej jeden element,
co jest właściwym rzędem dla rozdzielenia stanu przejściowego; przy znacznie
większej wartości solver rozmywa transport, który ma rozdzielać, niezależnie od
stabilności schematu. Dobierz `dt` na podstawie siatki i oczekiwanej prędkości:

```
dt ~ h / |v|
```

Każdy krok czasowy wykonuje pełną iterację nieliniową z punktu 1.6, więc koszt
obliczeń przejściowych to liczba kroków czasowych pomnożona przez liczbę iteracji
na krok. Od dziesięciu do pięćdziesięciu iteracji na krok to typowy zakres, gdy
pole przepływu zostało już doprowadzone do zbieżności w obliczeniach w stanie
ustalonym. Krok, którego residuum nie spadło do końca jego iteracji, jest krokiem
o niezbieżnej odpowiedzi, a błąd przenosi się na każdy następny krok.

### 1.9 Wymiana ciepła i dyspersja zanieczyszczeń

Na polu przepływu wytworzonym przez ten solver rozwiązywane są dwa typy
problemów, każdy z własnym podręcznikiem:

- **Heat transfer in fluids** (`RSolverFluidHeat`) przenosi temperaturę wraz z
  przepływem i sprzęga się z solverem cieplnym siatkowanego ciała stałego - zob.
  [Wymiana ciepła w płynach](theory_fluid_heat.md);
- **Contaminant dispersion** (`RSolverFluidParticle`) przenosi stężenie
  zanieczyszczenia wraz z przepływem - zob.
  [Dyspersja zanieczyszczeń](theory_fluid_particle.md).

Oba odczytują prędkość z wyniku przepływu. Dyspersja zanieczyszczeń nigdy nie
zmienia przepływu. Wymiana ciepła w płynach go zmienia, poprzez właściwości
materiału.

#### Właściwości zależne od temperatury i wypór

W **każdym przejściu** solver przepływu oblicza gęstość i lepkość dynamiczną
każdego elementu z tablic materiałowych dla temperatury elementu pozostawionej
przez ostatnie przejście wymiany ciepła. Bez zadania wymiany ciepła w problemie
temperatura ta wynosi wszędzie domyślne `293.15 K`. Warunek otoczenia
*Temperature* nie jest odczytywany bezpośrednio przez solver przepływu; trafia do
przepływu tylko przez zadanie wymiany ciepła.

To gęstość w sile masowej `rho(T)*g` nadaje przepływowi wypór: ciepły płyn jest
lżejszy od otaczającego i unosi się, zimny opada. Konwekcja naturalna, smugi
cieplne i stratyfikacja wynikają więc z grupy zadań *Incompressible viscous flow*
i *Heat transfer in fluids*, pod warunkiem że materiał ma **tablicę gęstości
zależnej od temperatury**. Dostarczone materiały *Water* (woda) i *Air*
(powietrze) ją mają. Materiał, którego gęstość jest pojedynczą wartością, nie
daje wyporu przy żadnej różnicy temperatur.

Jak oba zadania wymieniają dane:

- **Stan ustalony.** Umieść oba zadania w jednej grupie, zadanie przepływu jako
  pierwsze, z kilkoma iteracjami i wartością zbieżności. Pierwsze przejście
  przepływu widzi temperaturę początkową - warunek początkowy *Temperature* lub
  `293.15 K` - więc nie ma jeszcze wyporu. Od drugiej iteracji każde przejście
  przepływu widzi temperaturę z poprzedzającego je przejścia cieplnego. Model
  napędzany wyłącznie wyporem potrzebuje wystarczająco wielu iteracji, aby oba
  ustabilizowały się razem; grupa z jedną iteracją nigdy ich nie sprzęga.
- **Stan przejściowy.** Każdy krok czasowy ponownie wykonuje grupę, a każde
  przejście przepływu widzi najnowszą temperaturę. Przy jednej iteracji na krok
  czasowy przepływ używa temperatury z poprzedniego kroku - opóźnienie o jeden
  krok, które jest dopuszczalne, dopóki krok czasowy rozdziela cieplny stan
  przejściowy. Kilka iteracji na krok je usuwa.

Grupa kończy się, gdy zadanie przepływu osiągnie zbieżność, ponieważ
niesprzężone zadanie *Heat transfer in fluids* zgłasza zbieżność w każdym
przejściu. Zmiana temperatury przesuwa residuum przepływu przez `rho(T)` i
`mu(T)`, więc przepływ, który osiągnął zbieżność, jest też takim, na który
temperatura już nie wpływa.

Sformułowanie zachowuje `rho(T)` w każdym członie bilansu pędu i `div(v) = 0` dla
masy. Dla małych różnic gęstości konwekcji naturalnej w cieczy lub w gazie w
zakresie kilkudziesięciu kelwinów daje to ten sam wynik co przybliżenie
Boussinesqa. Nie jest to sformułowanie ściśliwe: rozszerzalność cieplna płynu
nie wchodzi do bilansu masy, więc gaz ogrzany o setki kelwinów wykracza poza jego
zakres.

### 1.10 Wyniki pochodne

| Wynik | Dotyczy | Wytwarzany przez | Znaczenie |
|---|---|---|---|
| Velocity `[m/s]` | węzeł | przepływ | rozwiązane pole prędkości, trzy składowe |
| Pressure `[Pa]` | węzeł | przepływ | rozwiązane pole ciśnienia |

Wyniki towarzyszących typów problemów są wymienione w ich własnych
podręcznikach.

Prędkość jest zapisywana jako węzłowy **wektor**, więc karta `Wyniki` może
wyświetlać jej wartość jako kolor, poszczególne składowe lub strzałki. Warto znać
dwa narzędzia wyświetlania przeznaczone właśnie dla wyników przepływu:

- **płaszczyznę przekroju** przez objętość, ponieważ interesująca struktura
  przepływu znajduje się wewnątrz obszaru, a nie na jego powierzchni;
- **linie prądu**, tworzone za pomocą `Geometria` -> `Linia prądu`, które śledzą
  pole prędkości i są najszybszym sposobem dostrzeżenia recyrkulacji lub strefy
  martwej.

Po każdym rekordzie dziennik solvera wypisuje statystyki prędkości i ciśnienia
oraz wartości w punktach monitorowania.

### 1.11 Stan ustalony czy przejściowy?

| | Stan ustalony | Stan przejściowy |
|---|---|---|
| Rozwiązuje | bilans ustalony, iterowany | równania nieustalone, całkowane w czasie |
| Odpowiada na | „jak wygląda rozwinięty przepływ” | „jak przepływ do tego dochodzi i co ze sobą niesie” |
| Iteracje | setki, w jednej grupie zadań | dziesiątki na krok czasowy |
| Rekordy | jeden | jeden na każdy zapisany krok czasowy |
| Wymaga odniesienia ciśnienia | tak | tak |

**Używaj analizy w stanie ustalonym do** rozwiniętego pola przepływu w kanale,
kolektorze lub przejściu wymiennika ciepła, spadku ciśnienia na elemencie oraz
jako pola startowego dowolnych obliczeń przejściowych. Od tego zaczyna się niemal
każdy model przepływu.

**Używaj analizy przejściowej do** rozruchu i wyłączania, pulsującego lub
cyklicznego wlotu, odrywania się wirów oraz - przede wszystkim - do **dyspersji
zanieczyszczeń**, która jest problemem transportu w czasie i nie ma sensownej
odpowiedzi ustalonej dla zdarzenia uwolnienia.

Dwuetapowy przebieg pracy z samouczka *Dyspersja zanieczyszczeń w płynach* jest
wzorcowy: doprowadź przepływ do zbieżności w stanie ustalonym, a następnie
uruchom ponownie z włączonym solverem czasowym i zadaniem dyspersji - zob.
podręcznik [Dyspersja zanieczyszczeń](theory_fluid_particle.md).

---

## 2. Interfejs graficzny

Wszystko, co istotne, znajduje się w panelu **Solver** (karty `Problem`,
`Warunki brzegowe`, `Warunki początkowe`, `Warunki otoczenia`, `Materiał`,
`Wyniki`), menu **Problem** i panelu **Model**.

### 2.1 Wybór problemu

`Problem` -> `Przebieg zadań problemów` (`Ctrl+P`) otwiera okno przebiegu zadań
problemu. Dodaj zadanie i wybierz **Incompressible viscous flow** -
„Steady-state and transient flow of newtonian fluids” (ustalony i nieustalony
przepływ płynów newtonowskich).

Zadanie przepływu należy umieścić w **grupie zadań**, która ma dwie wartości
sterujące iteracją nieliniową z punktu 1.6. Kliknij dwukrotnie którąkolwiek z
nich, aby ją edytować:

| Wartość | Znaczenie |
|---|---|
| **Liczba iteracji** | maksymalna liczba przejść, jaką może wykonać jedno rozwiązanie - lub jeden krok czasowy |
| **Zbieżność** | grupa kończy przejścia, gdy zadanie przepływu ustabilizuje się poniżej tej wartości **i** jego residuum zmaleje; `0` zawsze wykonuje pełną liczbę iteracji |

Wartość zbieżności domyślnie wynosi `1e-5`, co jest bezpieczną względną wielkością
kroku dla modelu przepływu. Umieszczenie zadania przepływu poza grupą, gdzie jest
wykonywane dokładnie raz, nie jest przydatne - zob. 1.6.

Warto znać następujące połączenia:

- samo *Incompressible viscous flow* w grupie kilkuset iteracji - ustalone pole
  przepływu;
- *Incompressible viscous flow* i *Contaminant dispersion* w jednej grupie -
  dyspersja na polu przepływu, klasyczne połączenie przejściowe, zob.
  [Dyspersja zanieczyszczeń](theory_fluid_particle.md);
- *Incompressible viscous flow* i *Heat transfer in fluids* w jednej grupie -
  ogrzewany lub chłodzony przepływ, zob.
  [Wymiana ciepła w płynach](theory_fluid_heat.md).

Przepływ wyklucza *Flow through porous media* i *Modal analysis*; oba
towarzyszące typy problemów wymagają obecności zadania przepływu.

### 2.2 Karta Problem

**Solver czasowy** - wyświetlany dla wszystkich trzech typów problemów płynów,
ponieważ wszystkie zależą od czasu. W obliczeniach przepływu w stanie ustalonym
pozostaw go wyłączonym.

| Pole | Znaczenie |
|---|---|
| Włącz | włącza i wyłącza przejściowe całkowanie w czasie |
| Przybliżenie | theta schematu czasowego: wsteczny, centralny lub w przód, zob. 1.8 |
| Czas początkowy | pierwsza wartość czasu w sekundach; ponowne uruchomienie od wyniku stanu ustalonego umieszcza ten wynik w czasie początkowym, ponowne uruchomienie przerwanych obliczeń przejściowych kontynuuje od ich ostatniego kroku |
| Czas końcowy | `start time + time-step size * number of time-steps` (czas początkowy + rozmiar kroku czasowego × liczba kroków czasowych), tylko do odczytu |
| Rozmiar kroku czasowego | `dt` w sekundach, zob. wskazówki dotyczące liczby Couranta w 1.8 |
| Liczba kroków czasowych | liczba kroków do obliczenia |
| Częstotliwość zapisu | zapisuje plik wyników co N kroków; `0` zapisuje tylko ostatni krok |

Typ problemu przepływu nie ma własnej grupy ustawień; grupa *Ustawienia dyspersji
zanieczyszczeń* jest opisana w podręczniku
[Dyspersja zanieczyszczeń](theory_fluid_particle.md).
Wszystko inne przypisuje się do poszczególnych encji na kartach warunków, a liczba
iteracji - która ma tu większe znaczenie niż gdziekolwiek indziej w Range FEA -
znajduje się w oknie przebiegu zadań problemu, a nie na tej karcie.

### 2.3 Karta Materiał

Karta `Materiał` zawiera materiał przypisany do wybranej encji. Typ problemu
przepływu wymaga:

| Właściwość | Jednostki | Komentarz |
|---|---|---|
| Density | `kg/m^3` | `rho`, około `1000` dla wody, `1.2` dla powietrza |
| Dynamic viscosity | `kg/(m*s)` | `mu`, około `1.0e-3` dla wody, `1.8e-5` dla powietrza |

(*Density* = gęstość, *Dynamic viscosity* = lepkość dynamiczna.) Towarzyszące
typy problemów wymieniają swoje właściwości w swoich podręcznikach. Encja, której
brakuje którejkolwiek właściwości wymaganej przez typ problemu w przebiegu zadań,
nie jest rozwiązywana, a kontrola konfiguracji ostrzega o tym przed rozpoczęciem
obliczeń. Dostarczona baza materiałów zawiera **Water** i **Air**, co zwykle
wyczerpuje ten krok.

Zwróć uwagę, że wymagana jest lepkość **dynamiczna** `mu` w `Pa*s`, a nie lepkość
kinematyczna `nu = mu/rho` w `m^2/s`. Wpisanie dla wody `1.0e-6`, bo to znajoma
liczba, jest najczęstszym błędem materiałowym w modelu przepływu i sprawia, że
płyn jest tysiąc razy mniej lepki, niż powinien - co zwykle objawia się
obliczeniami, które nie osiągają zbieżności.

Każda właściwość jest tablicą w funkcji temperatury, obliczaną dla temperatury
elementu z poprzedniego uruchomienia.

### 2.4 Karta Warunki brzegowe

Karta jest podzielona na listę menedżera dostępnych warunków brzegowych u góry i
edytor wybranego warunku u dołu. Najpierw wybierz encję w drzewie `Model`;
menedżer zaoferuje wtedy warunki właściwe dla danego typu encji i wybranych typów
problemów. Zaznacz warunek, aby go przypisać, a następnie edytuj jego składowe w
dolnym drzewie.

Oferowane są warunki z punktu 1.4. Uwagi dotyczące ich użycia:

- **Wall** należy umieścić na każdym brzegu stałym i nie przyjmuje on żadnej
  wartości. Encja powierzchni siatkowanej objętości bez żadnego warunku *nie*
  jest ścianą - jest brzegiem niezwiązanym, co rzadko jest zamierzone. Ściany to
  pierwsza rzecz do sprawdzenia w modelu, który zachowuje się dziwnie.
- **Wall (frictionless)** jest przeznaczony dla płaszczyzn symetrii wyrównanych z
  osiami globalnymi. Na pochylonej powierzchni wiąże niewłaściwy kierunek - zob.
  1.4.
- **Velocity (inflow)** i **Volumetric flow rate (inflow)** wymuszają przepływ
  przez powierzchnię wzdłuż jej średniej normalnej. Wybierz ten, który odpowiada
  liczbie, którą faktycznie znasz. Wartość ujemna wymusza przepływ w przeciwną
  stronę, co jest jednym ze sposobów określenia wylotu.
- **Pressure (implicit)** jest preferowanym warunkiem wylotowym: pozwala rozwinąć
  się profilowi wypływu i, wprowadzony jako `0`, czyni z wylotu odniesienie
  ciśnienia. **Pressure (explicit)** zamiast tego ustala ciśnienie węzłowe.
- Warunki *Heat transfer in fluids* i *Contaminant dispersion* pojawiają się, gdy
  ich zadanie znajdzie się w przebiegu zadań, i są opisane w ich własnych
  podręcznikach.
- Każda składowa jest tablicą w funkcji czasu. Kliknij **Edytuj wartości zależne
  od czasu**, aby otworzyć edytor składowej, w którym wartości wprowadza się
  względem czasu; między dwiema podanymi chwilami wartość jest interpolowana
  liniowo, a po ostatniej chwili zachowywana jest ostatnia wartość. W ten sposób
  modeluje się zdarzenie uwolnienia, stopniowo zwiększany wlot lub cykl pracy.

### 2.5 Warunki początkowe i otoczenia

Karta `Warunki początkowe` oferuje dla problemu przepływu *Velocity* i
*Pressure*. Ustawiają one pole startowe pierwszego uruchomienia i to dzięki nim
obliczenia przejściowe mogą zaczynać się od czegoś innego niż spoczynek.

Karta `Warunki otoczenia` oferuje *Gravitational acceleration*, z wartością
domyślną `(0, 0, -9.80665) m/s^2`. Przypisz je do objętości dla siły masowej oraz
do każdej powierzchni z *Pressure (implicit)*, aby wysokość hydrostatyczna tego
warunku była mierzona we właściwym kierunku.

### 2.6 Konfiguracja solvera macierzowego

`Problem` -> `Konfiguruj solver macierzowy problemów` konfiguruje solvery
iteracyjne. Solver przepływu używa pozycji **GMRES**, ponieważ jego macierz nie
jest symetryczna.

Zanim cokolwiek tu zmienisz, rozróżnij dwie zagnieżdżone iteracje: iteracje
**solvera macierzowego** rozwiązują jeden zlinearyzowany układ, natomiast
iteracje **grupy zadań** sterują nieliniowością z punktu 1.6. Model przepływu,
który nie osiąga zbieżności, prawie zawsze potrzebuje więcej iteracji grupy
zadań, lepszej siatki lub poprawionego zestawu warunków brzegowych - a nie
większej liczby iteracji GMRES.

### 2.7 Punkty monitorowania

`Problem` -> `Zdefiniuj punkty monitorowania` umieszcza sondy w podanych
współrzędnych i wybiera rejestrowaną zmienną - *Velocity* lub *Pressure* dla
przepływu. W obliczeniach przejściowych przebieg pokazuje `Raport` ->
`Punkty monitorowania`.

### 2.8 Wyniki i rekordy

Karta `Wyniki` zawiera listę obliczonych zmiennych i steruje widokiem 3D.
Prędkość jest wektorem węzłowym, a ciśnienie skalarem węzłowym.

Karta `Rekordy` panelu `Model` zawiera listę rekordów wyników:

- **analiza w stanie ustalonym** - jeden rekord;
- **analiza przejściowa** - jeden rekord na każdy zapisany krok czasowy.
  Animacja rekordów odtwarza przepływ, a czerwony przycisk nagrywania u dołu
  drzewa zapisuje animację do pliku wideo.

`Raport` -> `Zbieżność solvera` rysuje historię zbieżności opisaną w 1.6 i jest
raportem, który należy otworzyć jako pierwszy po każdych obliczeniach przepływu.
`Raport` -> `Plik dziennika solvera` wyświetla pełne wyjście solvera, w tym
statystyki prędkości i ciśnienia po każdym rekordzie.

---

## 3. Samouczek - ustalony przepływ przez kanał

**Cel.** Obliczyć rozwinięte pole przepływu i spadek ciśnienia płynu
przepływającego przez kanał oraz potwierdzić, że wynik jest zbieżny.

Ten samouczek zakłada zasiatkowany model objętościowy - dostarczony model
**Channel.tmsh** jest dokładnie tym przypadkiem i używa go także samouczek
*Dyspersja zanieczyszczeń w płynach*. Jeśli budujesz własny model, narysuj
prostopadłościan, podziel go na czworościany i oznacz ścianę wlotową, ścianę
wylotową i pozostałe ściany jako trzy oddzielne encje powierzchni, aby można było
przypisać im warunki.

### Krok 1 - wybierz problem

1. `Problem` -> `Przebieg zadań problemów` (`Ctrl+P`).
2. Dodaj **grupę zadań** i ustaw jej **Liczba iteracji** na `2000`, klikając
   dwukrotnie wartość.
3. Pozostaw jej **Zbieżność** na domyślnym `1e-5`.
4. Dodaj zadanie **Incompressible viscous flow** wewnątrz grupy.
5. Zatwierdź przyciskiem `OK`.

Te dwie wartości są sednem tego kroku. Obliczenia wykonują do 2000 przejść i
zatrzymują się, gdy tylko pole ustabilizuje się poniżej `1e-5` - zob. 1.6. Ustaw
liczbę hojnie: jest to pułap, a obliczenia zwykle kończy wartość zbieżności.

### Krok 2 - wygeneruj siatkę

`Geometria` -> `Objętość` -> `Generuj siatkę czworościenną`.

Model przepływu jest rozwiązywany tylko na elementach objętościowych. Bez siatki
objętościowej solver nie ma czego składać.

### Krok 3 - przypisz materiał

1. Wybierz encję objętości w drzewie `Model`.
2. Otwórz kartę `Materiał` i przypisz **Water** lub wprowadź:
   - **Density** = `1000` `kg/m^3`
   - **Dynamic viscosity** = `1.0e-3` `kg/(m*s)`

Sprawdź, czy lepkość jest dynamiczna w `Pa*s`, a nie kinematyczna - zob. 2.3.

### Krok 4 - pozostaw analizę w stanie ustalonym

Otwórz kartę `Problem` i pozostaw **Włącz** niezaznaczone w grupie *Solver
czasowy*.

### Krok 5 - przypisz ściany

1. Wybierz encję powierzchni obejmującą brzegi stałe kanału.
2. Otwórz kartę `Warunki brzegowe` i zaznacz **Wall**.

Nie ma wartości do wprowadzenia. Jest to warunek braku poślizgu i to on sprawia,
że przepływ rozwija profil, zamiast przesuwać się jak tłok.

### Krok 6 - wymuś przepływ

1. Wybierz encję powierzchni wlotowej.
2. Zaznacz **Volumetric flow rate (inflow)** i wprowadź wydajność w `m^3/s` -
   lub zamiast tego zaznacz **Velocity (inflow)** i wprowadź prędkość w `m/s`.

Zanim przejdziesz dalej, oszacuj liczbę Reynoldsa na podstawie średniej prędkości
i szerokości kanału:

```
Re = rho * V * L / mu
```

Poniżej kilku tysięcy model jest w zakresie, z którym ten solver sobie radzi.
Znacznie powyżej spodziewaj się, że obliczenia nie osiągną zbieżności, i zanim
poświęcisz im czas, przeczytaj rozdział 5.

### Krok 7 - otwórz wylot

1. Wybierz encję powierzchni wylotowej.
2. Zaznacz **Pressure (implicit)** i ustaw **Pressure** = `0` `Pa`.

To ten krok czyni model rozwiązywalnym. Nadaje polu ciśnienia odniesienie i
pozwala rozwinąć się profilowi wypływu, zamiast wymuszać płaski. Model z wlotem i
ścianami, ale bez żadnego warunku ciśnienia, ma osobliwe ciśnienie i nie osiągnie
zbieżności.

### Krok 8 - rozwiąż

1. `Rozwiązanie` -> `Uruchom solver` (`Ctrl+R`).
2. Kontrola konfiguracji zgłasza brakujące materiały lub brakujące warunki
   brzegowe przed rozpoczęciem obliczeń.
3. Śledź postęp w `Raport` -> `Plik dziennika solvera`.

Może to chwilę potrwać - model przepływu dowolnej wielkości to rzeczywista praca,
a dziennik wypisuje jeden blok na iterację. Obserwuj w dzienniku `Convergence
target` (cel zbieżności): przejście, które go osiąga, jest oznaczane, a grupa
zgłasza wtedy *All sub-tasks have converged* (wszystkie podzadania osiągnęły
zbieżność) i się zatrzymuje.

### Krok 9 - sprawdź zbieżność

`Raport` -> `Zbieżność solvera`.

Zrób to, zanim spojrzysz na jakikolwiek obraz. Residuum powinno spaść o kilka
rzędów wielkości, a następnie się wypłaszczyć, a `Convergence-V` i
`Convergence-P` powinny zbliżać się do zera. Jeśli residuum nadal gwałtownie
spada w ostatniej iteracji, obliczenia były zbyt krótkie - zwiększ liczbę
iteracji i uruchom ponownie. Jeśli oscyluje lub rośnie, odpowiedź nie jest
rozwiązaniem i należy zajrzeć do rozdziału 5.

### Krok 10 - przejrzyj wyniki

- Karta `Wyniki`: wyświetl **Velocity**. Profil w poprzek kanału powinien być
  gładki, zerowy przy ścianach i najszybszy w pobliżu środka.
- Utwórz płaszczyznę przekroju przez objętość - interesująca struktura przepływu
  znajduje się wewnątrz obszaru, a nie na jego powierzchni.
- Wyświetl **Pressure**. Powinno monotonicznie maleć od wlotu do wylotu i
  wynosić `0` na wylocie, gdzie je ustawiono.
- Utwórz **linie prądu** za pomocą `Geometria` -> `Linia prądu`, aby od razu
  zobaczyć recyrkulację lub strefy martwe.
- Sprawdź bilans masy: średnia prędkość na wylocie pomnożona przez pole wylotu
  powinna równać się objętościowemu natężeniu przepływu zadanemu na wlocie. To
  najszybsze globalne sprawdzenie, czy model robi to, o co prosiłeś - zob.
  rozdział 4.

### Rozwiązywanie problemów

| Objaw | Przyczyna |
|---|---|
| Residuum nigdy nie spada | nigdzie nie ma warunku ciśnienia, więc ciśnienie jest nieokreślone |
| Residuum oscyluje lub rośnie | liczba Reynoldsa jest zbyt wysoka dla siatki lub lepkość wprowadzono jako kinematyczną |
| Przepływ przesuwa się bez profilu | brak warunku *Wall* na brzegach stałych |
| Wynik wygląda jak pierwsza iteracja | zadanie nie zostało umieszczone w grupie zadań, więc wykonało się raz |
| Obliczenia zatrzymały się daleko przed liczbą iteracji | osiągnęły zbieżność - zarówno przyrosty, jak i residuum spełniły test z 1.6 |
| Obliczenia nigdy nie zatrzymują się wcześniej, niezależnie od tego, jak luźna jest wartość zbieżności | residuum nie maleje - odczytaj `Residual ratio` w dzienniku i traktuj pole przepływu jako niezbieżne |
| *Failed to calculate element scales. Unsupported element type* | siatka zawiera elementy objętościowe inne niż liniowe czworościany lub sześciościany |
| Nic nie zostało rozwiązane | nie wygenerowano siatki objętościowej lub materiałowi brakuje gęstości lub lepkości |
| Zadany wpływ i wypływ się nie równoważą | płyn nieściśliwy nie może się gromadzić - użyj *Pressure (implicit)* na wylocie |

---

## 4. Sprawdzanie modelu

Solver nie ma wbudowanego zestawu testów weryfikacyjnych, więc przed zaufaniem
nowemu modelowi warto go sprawdzić względem czegoś, co można obliczyć ręcznie.
Cztery tanie sprawdzenia, w kolejności przydatności:

**Bilans masy.** W każdym zbieżnym modelu nieściśliwym to, co wpływa, musi
wypływać:

```
sum( v . n * A ) over the inlets  =  sum( v . n * A ) over the outlets
```

(`over the inlets` = po wlotach, `over the outlets` = po wylotach.) Odczytaj
średnią prędkość na wylocie ze statystyk w dzienniku solvera, pomnóż przez pole
wylotu i porównaj z zadanym wpływem. To jedno globalne sprawdzenie wychwytuje
niezbieżne obliczenia, brakującą ścianę i warunek brzegowy na niewłaściwej
encji, a nic nie kosztuje.

**Hydrostatyka.** Wyłącz każdy wpływ, pozostaw obszar zamknięty ścianami,
przypisz warunek otoczenia *Gravitational acceleration* i rozwiąż. Płyn musi
przejść w stan spoczynku, a ciśnienie musi zmieniać się liniowo z głębokością:

```
p(h) = p_ref + rho * g * h
```

Każdy pozostały ruch jest numeryczny. Weryfikuje to w jednym obliczeniu siłę
masową grawitacji, odniesienie ciśnienia i jednostki materiału, a jest to
najszybszy pierwszy model do zbudowania w nowej konfiguracji.

**Płaski przepływ Poiseuille'a.** Dla ustalonego przepływu laminarnego między
dwiema równoległymi płytami odległymi o `H` profil jest paraboliczny, prędkość na
osi wynosi `1.5` średniej, a gradient ciśnienia wynosi

```
dp/dx = 12 * mu * V_mean / H^2
```

Zamodeluj prosty kanał ze ścianami u góry i u dołu, zadanym wpływem i wylotem
*Pressure (implicit)* i porównaj z tymi wartościami obliczoną prędkość na osi i
spadek ciśnienia. To najbardziej rygorystyczne dostępne sprawdzenie: weryfikuje
jednocześnie człon lepki, ścianę bez poślizgu i jednostki lepkości. Przed
pomiarem nadaj modelowi wystarczający odcinek wlotowy, aby profil się rozwinął,
ponieważ warunek wlotowy narzuca profil płaski.

**Przepływ Hagena-Poiseuille'a.** To samo sprawdzenie w okrągłej rurze o
średnicy `D` i długości `L`, gdzie prędkość na osi wynosi `2` średniej, a

```
dp = 128 * mu * Q * L / ( pi * D^4 )
```

obowiązuje, dopóki przepływ jest laminarny, czyli poniżej około `Re = 2300`.

Przy każdym z tych sprawdzeń zagęść siatkę co najmniej raz i potwierdź, że wynik
jest zbieżny, a nie dryfuje - i zanim cokolwiek porównasz, potwierdź, że historia
zbieżności się wypłaszczyła. Niezbieżny model przepływu nie przejdzie żadnego z
tych sprawdzeń z tego samego powodu.

---

## 5. Ograniczenia

### Ograniczenia modelowania

- **Brak modelu turbulencji.** Solver całkuje laminarne równania
  Naviera-Stokesa. Człony SUPG, PSPG i LSIC są stabilizacją numeryczną, a nie
  domknięciem turbulencji - nie odwzorowują mieszania ani dodatkowej dyssypacji
  przepływu turbulentnego. Powyżej mniej więcej `Re = 2300` w rurze rzeczywisty
  przepływ jest turbulentny, a ten solver go nie modeluje: obliczenia mogą nadal
  osiągnąć zbieżność, ale odpowiedź opisuje przepływ laminarny, który nie
  istnieje. Nie ma modelu `k`-`e`, funkcji przyściennej ani sformułowania
  wielkowirowego.
- **Tylko płyny nieściśliwe i newtonowskie.** Bilans masy ma postać
  `div(v) = 0`, a naprężenie lepkie jest liniowe względem prędkości
  odkształcenia. Gęstość i lepkość mogą zależeć od temperatury, ale nie od
  ciśnienia. Przepływ ściśliwy, efekty liczby Macha, swobodne powierzchnie,
  przepływy wielofazowe i zachowanie nienewtonowskie wykraczają poza
  sformułowanie.
- **Wypór bez rozszerzalności płynu.** Gęstość zależna od temperatury wywołuje
  konwekcję naturalną przez `rho(T)*g` (zob. 1.9), ale rozszerzalność cieplna
  nigdy nie wchodzi do bilansu masy. Jest to dokładne dla małych różnic gęstości
  konwekcji naturalnej, ale nie dla gazu ogrzanego o setki kelwinów. Przepływ
  wyporowy jest laminarny jak każdy inny przepływ tutaj - smuga lub wnęka powyżej
  liczby Rayleigha rzędu `1e8` do `1e9` jest w rzeczywistości turbulentna.
- **Temperatura trafia do przepływu tylko z zadania wymiany ciepła.** Bez niego
  właściwości materiału są obliczane dla `293.15 K`; sam warunek otoczenia
  *Temperature* ich nie zmienia.
- **Dyspersja zanieczyszczeń jest jednokierunkowa.** Przepływ unosi
  zanieczyszczenie; stężenie nigdy nie zmienia przepływu. Ograniczenia
  towarzyszących typów problemów są wymienione w ich własnych podręcznikach.
- **Tylko elementy objętościowe, tylko liniowe czworościany i sześciościany.**
  Model przepływu musi być siatkowaną objętością z elementów `TETRA1` lub
  `HEXA1`; cokolwiek innego zatrzymuje obliczenia. Powierzchnie biorą udział
  tylko po to, by przenosić obciążenie *Pressure (implicit)*, a encje punktów i
  linii nie wnoszą niczego.
- **Ściana bez tarcia jest wyrównana z osiami.** Wiąże jedną dominującą składową
  globalną normalnej każdego elementu, więc jest dokładna tylko na powierzchni,
  której normalna leży wzdłuż `x`, `y` lub `z`.
- **Wloty są jednorodne.** *Velocity (inflow)* i *Volumetric flow rate (inflow)*
  narzucają płaski profil wzdłuż średniej normalnej powierzchni. Rozwinięty
  profil wlotowy trzeba uzyskać przez odcinek wlotowy w siatce.

### Ograniczenia solvera

- **Iteracja jest zmodyfikowanym schematem Newtona**, więc zbieżność w pobliżu
  rozwiązania jest liniowa, a nie kwadratowa. Spodziewaj się setek iteracji w
  obliczeniach w stanie ustalonym i ustaw liczbę iteracji grupy zadań na tyle
  wysoko, aby obliczenia kończyła wartość zbieżności, a nie liczba iteracji.
  Tłumienie z punktu 1.6 utrzymuje spadek w takich obliczeniach, ale nie może
  uczynić zbieżności kwadratową.
- **Schemat różnic centralnych solvera przepływu nie jest schematem
  Cranka-Nicolson.** Ważenie theta jest stosowane do macierzy, ale nie do
  residuum, więc *Central difference (accurate)* rozwiązuje residuum wstecznego
  schematu Eulera z macierzą o połowie sztywności. Odpowiedź jest odpowiedzią
  różnicy wstecznej, osiąganą wolniej. Dopóki nie zostanie to poprawione, używaj
  dla przepływu *Backward difference (stable)*. Nie dotyczy to solvera dyspersji
  zanieczyszczeń - jest liniowy, a jego schemat jest prawdziwym schematem
  Cranka-Nicolson.
- **Parametry stabilizacji nie są różniczkowane.** `Tsupg`, `Tpspg`, `Tlsic` i
  długość elementu zależą od prędkości i wszystkie wchodzą do macierzy jako stałe
  obliczone w bieżącym polu. Jest to zwykła praktyka dla sformułowania
  stabilizowanego i jeden z powodów, dla których jakobian nie jest dokładny.
- **Część testu zbieżności dotycząca residuum jest stała.** Rozwiązanie uznaje się
  za zbieżne dopiero wtedy, gdy jego residuum spadnie do jednej dziesiątej
  wartości z początku tego rozwiązania, a tej jednej dziesiątej nie można
  zmienić. Jest to celowo zachowawcze: wolno zbiegający model może nigdy tego nie
  spełnić i wtedy wykona pełną liczbę iteracji. `Residual ratio` w dzienniku
  podaje, jak blisko znalazły się takie obliczenia.
- **Wymagane jest odniesienie ciśnienia.** Bez powierzchni *Pressure (explicit)*
  i bez powierzchni *Pressure (implicit)* ciśnienie jest wyznaczone tylko z
  dokładnością do stałej, a iteracja się nie ustabilizuje.
- **Zadane przepływy muszą się równoważyć.** Model, którego brzegi są w całości
  ścianami i zadanymi prędkościami, musi mieć wpływ dokładnie równy wypływowi;
  płyn nieściśliwy nie może przyjąć różnicy.
- **Koszt.** Cztery niewiadome na węzeł, niesymetryczna macierz i iteracja
  nieliniowa wokół każdego rozwiązania liniowego czynią model przepływu zdecydowanie
  najkosztowniejszym typem problemu w Range FEA. Odpowiednio dobierz wielkość
  siatki i doprowadź do zbieżności pole stanu ustalonego przed rozpoczęciem
  jakichkolwiek obliczeń przejściowych.

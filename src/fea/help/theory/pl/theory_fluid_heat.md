# Wymiana ciepła w płynach - Podręcznik teorii i przewodnik użytkownika

Ten dokument opisuje solver wymiany ciepła w płynach programu Range FEA -
`RSolverFluidHeat`, obsługujący typ problemu **Wymiana ciepła w płynach**
(*Heat transfer in fluids*): rozwiązywane równanie, znaczenie wszystkich
przyjmowanych danych wejściowych, sprzężenie z solverem cieplnym ciała stałego
oraz elementy interfejsu graficznego, które nim sterują.

Wymiana ciepła w płynach jest rozwiązywana na polu przepływu zadania
**Przepływ lepki nieściśliwy** (*Incompressible viscous flow*). Solver
przepływu, jego warunki brzegowe, iteracja nieliniowa i całkowanie w czasie są
opisane w podręczniku [Przepływ płynów](theory_fluid.md), a solver cieplny ciał
stałych w podręczniku [Wymiana ciepła](theory_heat.md).

**Spis treści**

1. [Podstawy teoretyczne](#1-podstawy-teoretyczne)
2. [Interfejs graficzny](#2-interfejs-graficzny)
3. [Ograniczenia](#3-ograniczenia)

---

## 1. Podstawy teoretyczne

### 1.1 Niewiadoma podstawowa - temperatura

Niewiadomą podstawową jest **temperatura** `T` w `K`, jeden stopień swobody na
węzeł.

| Typ problemu | Solver | Niewiadoma podstawowa | Jednostki | Stopnie swobody na węzeł |
|---|---|---|---|---|
| Heat transfer in fluids | `RSolverFluidHeat` | temperatura | `K` | 1 |

Ten typ problemu **wymaga** zadania *Incompressible viscous flow* w tym samym
problemie: odczytuje pole prędkości z jego wyniku i nigdy nie oblicza go sam.

### 1.2 Równanie podstawowe

**Heat transfer in fluids** rozwiązuje adwekcję-dyfuzję temperatury na polu
przepływu,

```
rho * c * ( dT/dt + v . grad(T) ) = div( k * grad(T) ) + q
```

z tą samą stabilizacją SUPG członu konwekcyjnego co solver przepływu i tym samym
schematem czasowym theta. Prędkość `v` jest **pobierana z wyniku przepływu**, a
nie rozwiązywana tutaj - dlatego ten typ problemu wymaga zadania
*Incompressible viscous flow*.

| Symbol | Znaczenie | Jednostki | Źródło |
|---|---|---|---|
| `T` | temperatura | `K` | rozwiązywana |
| `v` | prędkość | `m/s` | wynik przepływu |
| `k` | przewodność cieplna | `W/(m*K)` | właściwość materiału |
| `c` | pojemność cieplna | `J/(kg*K)` | właściwość materiału |
| `rho` | gęstość | `kg/m^3` | właściwość materiału |
| `q` | gęstość źródła ciepła | `W/m^3` | warunek Heat, ciepło Joule'a, promieniowanie |

Różnica w stosunku do zwykłego typu problemu **Heat transfer** polega właśnie na
członie `v . grad(T)`: energia jest unoszona przez poruszający się płyn, a nie
tylko przewodzona. Oznacza to również, że ten solver nie ma warunków brzegowych
konwekcji - nie ma czego korelować, ponieważ ciepło przechodzące ze ściany do
płynu jest rozwiązywane przez siatkę, a nie modelowane.

### 1.3 Warunki brzegowe

| Warunek brzegowy | Typ | Dotyczy | Składowe |
|---|---|---|---|
| Temperature | jawny (Dirichlet) | punkt, linia, powierzchnia, objętość | Temperature `[K]` |
| Heat | naturalny, źródło | punkt, linia, powierzchnia, objętość | Heat `[W]` |

**Temperature** (temperatura) utrzymuje węzły swojej encji w podanej wartości.
Przypisz go do powierzchni wlotu, aby ustawić temperaturę płynu wpływającego do
obszaru, lub do ściany, aby zamodelować ogrzewany lub chłodzony brzeg.

**Heat** (ciepło) jest źródłem i przyjmuje **całkowitą moc w `W`** dostarczaną do
encji, rozkładaną przed złożeniem na miarę encji - tak samo jak w solverze
cieplnym. *Heat rate (unit area)* i *Heat rate (unit volume)* są oferowane przez
interfejs dla tego typu problemu, ale solver cieplny płynów **ich nie
odczytuje**; jako źródła używaj tutaj *Heat*.

Każda składowa jest **tablicą w funkcji czasu**.

Warunki przepływu - ściany, wloty i wyloty - należą do zadania przepływu i są
opisane w podręczniku [Przepływ płynów](theory_fluid.md).

### 1.4 Sprzężona wymiana ciepła

Ściana między siatkowanym płynem a siatkowanym ciałem stałym, na której
przypisano warunek **Forced convection** (konwekcja wymuszona) zwykłego solvera
*Heat transfer*, sprzęga oba obszary. Po uruchomieniu solvera cieplnego solver
cieplny płynów utrzymuje węzły ściany w temperaturze ciała stałego - do tego
czasu ściana jest izolowana - i zwraca dla każdego elementu ściany parę
`(h, Tf)`, która odtwarza strumień ciepła przechodzący przez ścianę w jego
rozwiązaniu. Solver cieplny stosuje tę parę jako warunek konwekcji. Strumień
jest residuum równań płynu w węzłach ściany, zgodnym z dyskretyzacją, dzięki
czemu pozostaje dokładny w cienkiej warstwie przyściennej cieplnej. Temperatura
ściany jest relaksowana między przebiegami współczynnikiem Aitkena, co
stabilizuje wymianę w kilku przebiegach. Podręcznik
[Wymiana ciepła](theory_heat.md) opisuje ten schemat w części *Sprzężona
wymiana ciepła*. Ciepło promieniowania i ciepło Joule'a pochodzące z innych
solverów są dodawane do członu źródłowego tak samo jak w solverze cieplnym.

### 1.5 Zbieżność

Bez sprzężenia solver cieplny płynów zgłasza zbieżność bezwarunkowo - przy
ustalonym polu przepływu jest liniowy, więc jedno rozwiązanie jest odpowiedzią
końcową. Grupa zawierająca zarówno zadanie przepływu, jak i zadanie cieplne
płynów kończy się więc, gdy zbieżność osiągnie zadanie przepływu. Przy
sprzężonych ścianach solver zgłasza natomiast względną zmianę swojego pola
temperatury, więc grupa iteruje, aż temperatura ściany się ustabilizuje.

### 1.6 Sprzężenie zwrotne z przepływem - wypór

Temperatura obliczona przez ten solver jest przekazywana z powrotem do
przepływu. W każdym przebiegu solver przepływu oblicza gęstość i lepkość
dynamiczną każdego elementu przy najnowszej temperaturze, więc dla materiału,
którego gęstość zależy od temperatury, siła masowa `rho(T)*g` wywołuje
**konwekcję naturalną** - ciepły płyn unosi się, zimny opada. Dostarczone
materiały *Water* i *Air* mają gęstość zależną od temperatury.

W obliczeniach ustalonych pierwszy przebieg przepływu widzi tylko temperaturę
początkową, więc wypór pojawia się od drugiej iteracji grupy; nadaj grupie kilka
iteracji i wartość zbieżności. W obliczeniach przejściowych z jedną iteracją na
krok czasowy przepływ widzi temperaturę z poprzedniego kroku. Podręcznik
[Przepływ płynów](theory_fluid.md) opisuje ten schemat w punkcie 1.9.

### 1.7 Warunki początkowe

| Warunek początkowy | Skutek |
|---|---|
| Temperature | początkowe pole temperatury |

Warunki początkowe są odczytywane **tylko przy pierwszym obliczeniu**; później
pole jest rozwijane od poprzedniego wyniku. Warunek brzegowy ma pierwszeństwo
przed warunkiem początkowym tam, gdzie oba się spotykają.

### 1.8 Wyniki

| Wynik | Dotyczy | Znaczenie |
|---|---|---|
| Temperature `[K]` | węzeł | rozwiązane pole temperatury |
| Heat flux `[W/m^2]` | element | strumień przewodzenia `-k*grad(T)` |

---

## 2. Interfejs graficzny

Wszystko, co istotne, znajduje się w panelu **Solver** (karty `Problem`,
`Warunki brzegowe`, `Warunki początkowe`, `Materiał`, `Wyniki`), menu
**Problem** i panelu **Model**.

### 2.1 Wybór problemu

`Problem` -> `Przebieg zadań problemów` (`Ctrl+P`) otwiera okno przebiegu zadań
problemu. Warto znać następujące kombinacje:

- *Incompressible viscous flow* i *Heat transfer in fluids* w jednej grupie:
  ogrzewany lub chłodzony przepływ. Nadaj grupie kilka iteracji, gdy przepływ
  jest wywoływany lub wyraźnie zmieniany przez wypór, aby przepływ i temperatura
  ustabilizowały się razem (zob. 1.6);
- *Heat transfer in fluids* i oddzielne zadanie *Heat transfer* na siatkowanym
  ciele stałym, najpierw zadanie cieplne płynów, w jednej grupie z kilkoma
  iteracjami: sprzężona wymiana ciepła przez ściany z warunkiem *Forced
  convection*.

Ten typ problemu wymaga obecności zadania przepływu. Wartości grupy zadań -
**Liczba iteracji** i **Zbieżność** - są opisane w podręczniku
[Przepływ płynów](theory_fluid.md).

### 2.2 Karta Problem

**Solver czasowy** - wyświetlany, ponieważ ten typ problemu zależy od czasu. W
obliczeniach ustalonych pozostaw go wyłączonego. Jego pola są opisane w
podręczniku [Przepływ płynów](theory_fluid.md).

Ten typ problemu nie ma własnej grupy ustawień. Wszystko inne przypisuje się do
encji na kartach warunków.

### 2.3 Karta Materiał

| Właściwość | Jednostki | Komentarz |
|---|---|---|
| Density | `kg/m^3` | `rho`, około `1000` dla wody, `1.2` dla powietrza |
| Dynamic viscosity | `kg/(m*s)` | `mu`, około `1.0e-3` dla wody, `1.8e-5` dla powietrza |
| Thermal conductivity | `W/(m*K)` | `k`, około `0.6` dla wody, `0.025` dla powietrza |
| Heat capacity | `J/(kg*K)` | `c`, około `4180` dla wody, `1005` dla powietrza |

Encja, której brakuje którejkolwiek wymaganej właściwości, nie jest
rozwiązywana, a kontrola konfiguracji ostrzega o tym przed rozpoczęciem
obliczeń. Dostarczona baza materiałów zawiera **Water** (woda) i **Air**
(powietrze), co zwykle wyczerpuje ten krok. Każda właściwość jest tablicą w
funkcji temperatury, obliczaną przy temperaturze elementu z poprzedniego
obliczenia.

### 2.4 Karta Warunki brzegowe

**Temperature** i **Heat** pojawiają się, gdy w przebiegu zadań znajduje się
zadanie *Heat transfer in fluids* - zob. punkt 1.3. Każda składowa jest tablicą
w funkcji czasu; kliknij **Edytuj wartości zależne od czasu**, aby wprowadzić
wartości w funkcji czasu.

### 2.5 Warunki początkowe

Karta `Warunki początkowe` oferuje *Temperature*. Ustawia ona pole początkowe
pierwszego obliczenia.

### 2.6 Konfiguracja solvera macierzowego

`Problem` -> `Konfiguruj solver macierzowy problemów` konfiguruje solvery
iteracyjne. Solver cieplny płynów używa pozycji **GMRES**, ponieważ jego macierz
nie jest symetryczna.

### 2.7 Punkty monitorowania i wyniki

`Problem` -> `Zdefiniuj punkty monitorowania` może rejestrować *Temperature*.
Karta `Wyniki` wyświetla temperaturę jako skalar w węzłach, a strumień ciepła
jako wektor w elementach.

---

## 3. Ograniczenia

- **Wypór bez rozszerzalności płynu.** Temperatura wpływa na przepływ przez
  `rho(T)` i `mu(T)` (zob. 1.6), ale rozszerzalność cieplna płynu nigdy nie
  wchodzi do bilansu masy `div(v) = 0`. Jest to dokładne dla małych różnic
  gęstości w konwekcji naturalnej, ale nie dla gazu ogrzanego o setki kelwinów.
  Przepływ wyporowy jest laminarny, jak każdy przepływ w Range FEA.
- **Sprzężenie jest sekwencyjne, a nie monolityczne.** Przepływ i temperatura są
  rozwiązywane jedno po drugim i wymieniają dane między przebiegami. Ustalony
  przepływ wyporowy wymaga grupy z kilkoma iteracjami; przy jednej iteracji na
  krok czasowy przepływ przejściowy opóźnia się o jeden krok względem
  temperatury.
- **Warunki `Heat rate` są ignorowane.** *Heat rate (unit area)* i *Heat rate
  (unit volume)* są oferowane przez interfejs dla *Heat transfer in fluids*, ale
  `RSolverFluidHeat` ich nie odczytuje; dociera do niego tylko warunek *Heat*.
  Zwykły typ problemu *Heat transfer* odczytuje wszystkie trzy.
- **Brak warunków brzegowych konwekcji.** Ciepło przechodzące ze ściany do
  siatkowanego płynu jest rozwiązywane, a nie korelowane, więc warunki
  *Simple*, *Forced* i *Natural convection* należą do zwykłego solvera
  cieplnego. Sprzężoną wymianę ciepła konfiguruje się jako zadanie *Heat
  transfer* na ciele stałym, którego ściany z *Forced convection* odczytują
  obliczony stan płynu.
- **Brak modelu turbulencji.** Przepływ jest laminarny (zob. podręcznik
  [Przepływ płynów](theory_fluid.md)), więc turbulentna wymiana ciepła nie jest
  odwzorowywana.

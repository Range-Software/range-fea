# Wymiana ciepła - Podręcznik teorii i przewodnik użytkownika

Ten dokument opisuje solver cieplny (`RSolverHeat`) programu Range FEA,
obsługujący typ problemu **Heat transfer**: rozwiązywane równania, znaczenie
wszystkich przyjmowanych danych wejściowych, elementy interfejsu graficznego,
które nim sterują, oraz dwa kompletne samouczki.

**Spis treści**

1. [Podstawy teoretyczne](#1-podstawy-teoretyczne)
2. [Interfejs graficzny](#2-interfejs-graficzny)
3. [Samouczek - ustalona wymiana ciepła](#3-samouczek---ustalona-wymiana-ciepła)
4. [Samouczek - nieustalona wymiana ciepła](#4-samouczek---nieustalona-wymiana-ciepła)
5. [Sprawdzanie modelu](#5-sprawdzanie-modelu)
6. [Ograniczenia](#6-ograniczenia)

---

## 1. Podstawy teoretyczne

### 1.1 Niewiadoma podstawowa - pole temperatury

Niewiadomą podstawową jest **temperatura** `T` w każdym węźle, w kelwinach.
Każdy węzeł ma jeden stopień swobody, więc siatka o `N` węzłach daje układ `N`
równań. Strumień ciepła wyznacza się z pola temperatury po rozwiązaniu.

Temperatura jest skalarem, dzięki czemu model cieplny jest wyraźnie tańszy niż
model konstrukcyjny na tej samej siatce: jedna niewiadoma na węzeł zamiast
trzech i macierz o jednej dziewiątej liczby wpisów.

Domyślna temperatura węzła, którego nie dotyczy żaden warunek ani żaden
wcześniejszy wynik, wynosi `293.15 K`.

### 1.2 Równanie podstawowe

Solver realizuje **liniowe przewodzenie ciepła** z brzegami konwekcyjnymi.
Bilans energii ciała o przewodności `k`, gęstości `rho` i cieple właściwym `c`
ma postać

```
rho * c * dT/dt = div( k * grad(T) ) + q
```

a przy wyłączonym solverze czasowym człon nieustalony znika, pozostawiając
bilans stanu ustalonego

```
div( k * grad(T) ) + q = 0
```

Ciepło opuszcza powierzchnię przez konwekcję zgodnie z prawem stygnięcia
Newtona,

```
qs = h * ( T - Tf )
```

| Symbol | Znaczenie | Jednostki | Źródło |
|---|---|---|---|
| `T` | temperatura | `K` | rozwiązywana |
| `q` | gęstość źródła ciepła | `W/m^3` | warunek Heat, ciepło Joule'a, promieniowanie |
| `k` | przewodność cieplna | `W/(m*K)` | właściwość materiału |
| `c` | pojemność cieplna | `J/(kg*K)` | właściwość materiału |
| `rho` | gęstość | `kg/m^3` | właściwość materiału |
| `h` | współczynnik konwekcji | `W/(m^2*K)` | warunek konwekcji, zob. 1.5 |
| `Tf` | temperatura płynu | `K` | warunek konwekcji |
| `Q` | strumień ciepła | `W/m^2` | wyznaczany, zob. 1.9 |

Nie ma członu adwekcji. Problem wymiany ciepła przenosi energię przez
przewodzenie wewnątrz ciała i przez konwekcję przez jego brzeg; nie przenosi
energii z płynącym ośrodkiem. Do tego służy *Heat transfer in fluids*.

**Wymagane właściwości materiału.** Problem cieplny wymaga **czterech** z nich:

| Właściwość | Jednostki | Zastosowanie |
|---|---|---|
| Thermal conductivity | `W/(m*K)` | macierz przewodzenia |
| Heat capacity | `J/(kg*K)` | macierz pojemności, tylko analiza nieustalona |
| Density | `kg/m^3` | macierz pojemności, tylko analiza nieustalona |
| Emissivity | `-` | tylko radiacyjna wymiana ciepła |

(*Thermal conductivity* = przewodność cieplna, *Heat capacity* = pojemność
cieplna, *Density* = gęstość, *Emissivity* = emisyjność.) Encja, której
materiałowi brakuje którejkolwiek z czterech, **nie jest rozwiązywana**, nawet w
czystym obliczeniu przewodzenia w stanie ustalonym, które nigdy nie sięga po
gęstość ani emisyjność. Kontrola konfiguracji ostrzega o tym przed rozpoczęciem
obliczeń. Jedynym wyjątkiem jest encja z cieplnym warunkiem brzegowym: taka encja
jest włączana do układu niezależnie od materiału, a jej przewodność i pojemność
wynoszą wtedy po prostu zero.

Każda właściwość jest tablicą w funkcji temperatury. Wartość używana dla elementu
jest odczytywana z tablicy dla **temperatury elementu z poprzedniego
uruchomienia**, więc przewodność zależna od temperatury jest opóźniona o jedną
iterację. Punkt 1.6 wyjaśnia, jak doprowadzić ją do zbieżności.

### 1.3 Dyskretyzacja metodą elementów skończonych

Postać słaba bilansu energii daje dla każdego elementu

```
K = INT( B^T * k * B )  +  INT( N^T * h * N )      conduction + convection
M = INT( rho * c * N^T * N )                       capacity matrix
f = INT( N^T * q )  +  h * Tf * A / n              sources + convection
```

(`conduction` = przewodzenie, `convection` = konwekcja, `capacity matrix` =
macierz pojemności, `sources` = źródła.) Tu `B` jest budowane z pochodnych
funkcji kształtu, a `N` to same funkcje kształtu. Każda macierz elementu ma
wymiar `n x n` dla elementu o `n` węzłach - jeden wiersz na węzeł, a nie trzy.

Człony konwekcji są składane **tylko na elementach powierzchni** i tylko tam,
gdzie przypisano konwekcyjny warunek brzegowy. Sztywność konwekcji ma postać
spójną `INT( N^T * h * N )`, natomiast obciążenie konwekcyjne jest rozkładane
równo na węzły elementu. Na rozsądnej siatce obie postacie są wystarczająco
zgodne, a różnica zanika przy zagęszczaniu siatki.

**Miary elementów.** Jak wszędzie w Range FEA, encjom niższego wymiaru nadaje
się miarę geometryczną, aby mogły reprezentować rzeczywiste ciała:

| Encja | Miara | Gdzie jest ustawiana |
|---|---|---|
| Objętość | 1 | - |
| Powierzchnia | grubość powierzchni | właściwości geometryczne encji |
| Linia | pole przekroju linii | właściwości geometryczne encji |
| Punkt | objętość punktu | właściwości geometryczne encji |

**Powierzchnia** przewodzi ciepło w swojej płaszczyźnie przez przekrój równy jej
grubości i magazynuje energię proporcjonalnie do tej grubości. Nie ma gradientu
temperatury w kierunku grubości: element powierzchni ma grubość jednego węzła,
więc z założenia jest izotermiczny w kierunku grubości. Powierzchnia o **zerowej
grubości** niczego więc nie przewodzi ani nie magazynuje, ale nadal przenosi
konwekcję - a dokładnie tego oczekuje się od ściany brzegowej siatkowanej
objętości, która istnieje tylko po to, by nieść warunek konwekcji.

**Linia** przewodzi wzdłuż swojego kierunku przez swoje pole przekroju. **Punkt**
nie ma kierunku, wzdłuż którego mógłby przewodzić, więc nie wnosi żadnego
przewodzenia: ma tylko pojemność, proporcjonalną do objętości punktu, i źródło
ciepła. Punkt jest skupioną masą cieplną, a nie przewodnikiem.

Macierz pojemności jest składana tylko wtedy, gdy jest potrzebna, czyli w analizie
nieustalonej.

### 1.4 Źródła i warunki brzegowe

| Warunek brzegowy | Typ | Dotyczy | Składowe |
|---|---|---|---|
| Temperature | jawny (Dirichlet) | punkt, linia, powierzchnia, objętość | Temperature `[K]` |
| Heat | naturalny, źródło | punkt, linia, powierzchnia, objętość | Heat `[W]`, zob. niżej |
| Simple convection | naturalny (Robin) | powierzchnia | Convection coefficient `[W/(m^2*K)]`, Fluid temperature `[K]` |
| Forced convection | naturalny, korelacja | powierzchnia | Density, Dynamic viscosity, Fluid temperature, Heat capacity, Hydraulic diameter, Thermal conductivity, Velocity |
| Natural convection | naturalny, korelacja | powierzchnia | Density, Dynamic viscosity, Fluid temperature, Heat capacity, Hydraulic diameter, Thermal conductivity, Thermal expansion coefficient |
| Heat rate (unit area) | naturalny, źródło | powierzchnia | Heat per unit area `[W/m^2]` |
| Heat rate (unit volume) | naturalny, źródło | objętość | Heat per unit volume `[W/m^3]` |

**Temperature** (temperatura) jest jedynym jawnym warunkiem problemu cieplnego.
Każdy węzeł encji, do której go przypisano, jest usuwany z układu równań i
utrzymywany na podanej wartości; jego wkład jest przenoszony na prawą stronę
sąsiednich równań. Model w stanie ustalonym potrzebuje co najmniej jednego
takiego warunku lub co najmniej jednej powierzchni z konwekcją - zob. 1.7.

Tam, gdzie encje się nakładają, **wygrywa encja niższego wymiaru**: temperatura
zadana na powierzchni zastępuje we wspólnych węzłach temperaturę zadaną na
objętości za nią, linia zastępuje powierzchnię, a punkt zastępuje wszystko. W
przeciwieństwie do więzów przemieszczeń solvera konstrukcyjnego temperatury nie
są łączone - w węźle jest tylko jeden stopień swobody, więc obowiązuje po prostu
ostatni odczytany warunek.

**Heat** (ciepło) jest źródłem ciepła, a jego wartością jest **całkowita moc w
`W`** dostarczana do encji, do której go przypisano, niezależnie od wymiaru tej
encji. Solver potrzebuje gęstości źródła, więc wartość całkowita jest przed
składaniem rozkładana na miarę encji:

| Encja | Gęstość stosowana przez solver |
|---|---|
| Objętość | wartość / objętość encji, `W/m^3` |
| Powierzchnia | wartość / pole encji, `W/m^2` |
| Linia | wartość / długość encji, `W/m` |
| Punkt | wartość / liczba elementów punktowych, `W` na każdy |

Dostarczana moc jest więc dokładnie wprowadzoną liczbą - nie zmienia się wraz z
rozmiarem encji, zagęszczaniem siatki ani skalą modelu. Udział otrzymują tylko
elementy, które solver może obliczyć, więc encja bez właściwości materiału jest
pomijana, a jej warunek zgłaszany. Powierzchniowe źródło ciepła nie jest
skalowane grubością powierzchni.

**Heat rate (unit area)** i **Heat rate (unit volume)** zadają ten sam człon
źródłowy jako gęstość zamiast wartości całkowitej: `W/m^2` na pole encji
powierzchni i `W/m^3` na objętość encji objętości. Sięgaj po nie, gdy źródło jest
naturalnie gęstością - grzejnik o mocy podanej na metr kwadratowy - a po *Heat*,
gdy jest naturalnie wartością całkowitą, jak element o znanej mocy w watach.
Wszystkie sumują się na encji, która ma ich kilka, i wszystkie przyjmują tablicę
w funkcji czasu, więc grzejnik można włączać i wyłączać podczas obliczeń
nieustalonych.

Warunki **konwekcji** dotyczą powierzchni i dodają zarówno człon sztywności, jak
i obciążenie, więc działają jak warunek brzegowy Robina, a nie jak stały
strumień: podniesienie temperatury ściany zwiększa ciepło, które ściana traci.
Wszystkie trzy odmiany prowadzą do tej samej pary `(h, Tf)`; różnią się tylko
tym, skąd pochodzi `h`:

- **Simple convection** (konwekcja prosta) pobiera `h` i `Tf` wprost z dwóch
  wprowadzonych wartości. Używaj jej zawsze, gdy współczynnik konwekcji jest
  znany z poradnika, pomiaru lub wcześniejszych obliczeń. Jest jedyną z trzech
  odmian w pełni pod Twoją kontrolą.
- **Forced convection** (konwekcja wymuszona) ma dwa tryby. Na ścianie między
  siatkowanym ciałem stałym a siatkowanym płynem, gdy zadanie *Heat transfer in
  fluids* wytworzyło już wynik, sprzęga oba rozwiązania bezpośrednio - nie
  stosuje się żadnej korelacji, ponieważ rozwiązany przepływ już niesie
  konwekcję. Solver ciepła w płynach przekazuje dla każdego elementu ściany parę
  `(h, Tf)`, która odtwarza strumień ciepła przechodzący przez ścianę w jego
  własnym rozwiązaniu; zob. *Sprzężona wymiana ciepła* w 1.6. Wszędzie indziej -
  na powierzchni niegraniczącej z żadnym siatkowanym płynem lub w pierwszym
  przejściu obliczeń sprzężonych, zanim zadanie ciepła w płynach zostało
  wykonane - `h` jest obliczane z korelacji dla opływu zewnętrznego płaskiej
  płyty z użyciem właściwości płynu, średnicy hydraulicznej oraz wprowadzonych
  *Fluid temperature* i *Velocity*. Dziennik podaje, który z trybów jest używany.
- **Natural convection** (konwekcja naturalna) oblicza `h` z korelacji dla płyt
  poziomych z użyciem właściwości płynu, średnicy hydraulicznej i różnicy
  temperatur między ścianą a płynem.

Jeśli do tej samej powierzchni przypisano więcej niż jeden warunek konwekcji, nie
sumują się one: najpierw odczytywana jest konwekcja prosta, wymuszona ją
nadpisuje, a naturalna nadpisuje obie.

### 1.5 Korelacje konwekcji

Obie obliczane odmiany budują zwykłe liczby bezwymiarowe z właściwości płynu
wprowadzonych wraz z warunkiem brzegowym, ze średnicą hydrauliczną `d` jako
długością charakterystyczną:

```
Pr = c * mu / k                                        Prandtl
Re = rho * v * d / mu                                  Reynolds
Gr = | rho^2 * g * beta * (Ts - Tf) * d^3 / mu^2 |     Grashof,  g = 9.81
Ra = Gr * Pr                                           Rayleigh
```

i przeliczają liczbę Nusselta na współczynnik konwekcji jako

```
h = Nu * k / d
```

| Odmiana | Korelacja | Liczba Nusselta |
|---|---|---|
| Wymuszona | opływ zewnętrzny, płaska płyta | `Nu = 0.036 * Re^(4/5) * Pr^(1/3)` |
| Naturalna | płyta pozioma, `Ra < 2e7` | `Nu = 0.54 * Ra^(1/4)` |
| Naturalna | płyta pozioma, `Ra >= 2e7` | `Nu = 0.14 * Ra^(1/3)` |

`k`, `c`, `rho`, `mu` i `beta` w tych wyrażeniach są właściwościami **płynu**
wprowadzonymi wraz z warunkiem brzegowym, a nie właściwościami materiału ciała
stałego.

Oba warunki są tworzone z właściwościami **suchego powietrza w temperaturze 20
stopni Celsjusza i pod ciśnieniem 101.325 kPa**, więc płyn jest już opisany i do
wprowadzenia pozostają tylko przepływ i geometria:

| Składowa | Wartość domyślna |
|---|---|
| Density | `1.2041 kg/m^3` |
| Dynamic viscosity | `1.8205e-5 kg/(m*s)` |
| Heat capacity | `1005 J/(kg*K)` |
| Thermal conductivity | `0.02514 W/(m*K)` |
| Thermal expansion coefficient | `3.4112e-3 1/K`, wartość dla gazu doskonałego `1/T` |
| Fluid temperature | `293.15 K` |
| Hydraulic diameter | `1 m` |

Dla każdego innego płynu je zastąp - są to właściwości płynu, a nie materiał
ściany. *Natural convection* jest więc gotowa do rozwiązania zaraz po
przypisaniu; *Forced convection* nadal potrzebuje **prędkości**, której żadna
wartość domyślna nie odgadnie - chyba że powierzchnia graniczy z siatkowanym
płynem, gdzie żadna z tych wartości nie jest odczytywana.

Każda z tych liczb dzieli przez `mu`, `k` lub `d`, a `Re` i `Pr` spadają do zera,
jeśli zerują się `rho`, `c` lub `v`, co dałoby `h = 0` i powierzchnię, która
nigdy nie stygnie. Wartość zerowa jest więc traktowana jako błąd, a nie
maskowana: solver zatrzymuje się i podaje składową oraz encję, na przykład

```
Value of 'Velocity' configured in 'Forced convection' boundary condition on
entity 'Surface' is zero - the convection correlation can not be evaluated.
```

(„Wartość 'Velocity' skonfigurowana w warunku brzegowym 'Forced convection' na
encji 'Surface' wynosi zero - nie można obliczyć korelacji konwekcji”.)

Prędkość obliczona przez solver przepływu nigdy nie jest podawana do korelacji.
Ściana z warunkiem braku poślizgu utrzymuje płyn w spoczynku, więc prędkość tuż
przy niej wynosi zero, a korelacja potrzebowałaby prędkości przepływu
swobodnego i temperatury średniej, których ogólny trójwymiarowy przepływ nie
definiuje. Ściana granicząca z siatkowanym płynem jest zamiast tego sprzęgana z
rozwiązaniem płynu.

Warto pamiętać o trzech konsekwencjach:

- Korelacja jest **stała**. Solver nie bierze pod uwagę orientacji powierzchni,
  tego, czy przepływ jest wewnętrzny czy zewnętrzny, ani tego, czy warstwa
  przyścienna jest laminarna czy turbulentna. Wszystko wchodzi przez średnicę
  hydrauliczną, która jest długością charakterystyczną korelacji - to jedyny
  sposób wpływu na geometrię.
- Zerowa średnica hydrauliczna dałaby `h = 0`, czyli ścianę izolowaną, więc
  solver zatrzymuje się, zamiast ją zaakceptować.
- Konwekcja naturalna zależy od **temperatury ściany**, która jest pobierana z
  temperatury elementu z **poprzedniego** rozwiązania. Przy pierwszym
  uruchomieniu analizy w stanie ustalonym jest to temperatura początkowa, więc
  współczynnik jest obliczany dla niewłaściwej różnicy temperatur. Umieść
  zadanie w grupie zadań problemów z kilkoma iteracjami, aby osiągnąć zbieżność,
  lub użyj *Simple convection* ze współczynnikiem obliczonym samodzielnie.
  Zanikająca różnica temperatur daje `Nu = 1`.

### 1.6 Warunki otoczenia, początkowe i sprzężenia

#### Warunki otoczenia

| Warunek otoczenia | Składowe | Działanie |
|---|---|---|
| Temperature | Temperature `[K]` | temperatura tła encji, tylko pierwsze uruchomienie |

#### Warunki początkowe

| Warunek początkowy | Działanie |
|---|---|
| Temperature | początkowe pole temperatury, stosowane przy pierwszym uruchomieniu |

Warunki otoczenia, warunki początkowe i warunki brzegowe są odczytywane w tej
kolejności i każde nadpisują poprzednie, więc **warunek brzegowy ma
pierwszeństwo przed warunkiem początkowym, a ten przed warunkiem otoczenia**.
Wartości otoczenia i początkowe są odczytywane **tylko przy pierwszym
uruchomieniu**; potem pole jest prowadzone od poprzedniego wyniku.

#### Sprzężenie z innymi solverami

Solver cieplny zarówno korzysta z wyników, jak i wytwarza wyniki udostępniane
reszcie przebiegu zadań.

| Kierunek | Zmienna | Działanie |
|---|---|---|
| Radiacyjna wymiana ciepła -> ciepło | Heat - radiation part | dodawane do członu źródłowego elementów **powierzchni** |
| Elektrostatyka -> ciepło | Joule heat | dodawane do członu źródłowego każdego elementu |
| Naprężenia -> ciepło | Displacement | siatka jest odkształcana na czas rozwiązania cieplnego, a potem przywracana |
| Ciepło -> naprężenia | Temperature | wymusza człon rozszerzalności cieplnej solvera konstrukcyjnego |
| Ciepło -> dowolny solver | Temperature | wybiera wiersz każdej tablicy materiałowej zależnej od temperatury |
| Ciepło w płynach -> ciepło | Ściana `(h, Tf)` | steruje ścianami *Forced convection* graniczącymi z siatkowanym płynem |
| Ciepło -> ciepło w płynach | Temperatura ciała stałego | utrzymuje te same ściany w temperaturze ciała stałego w rozwiązaniu płynu |

**Radiative heat transfer** (radiacyjna wymiana ciepła) to oddzielny typ
problemu z własnym solverem. Wymaga zadania *Heat transfer*, rozwiązuje układ
radiosity zamkniętej przestrzeni na łatach powierzchni z użyciem współczynników
widzenia i emisyjności materiału, a wynik przekazuje z powrotem jako
powierzchniowe źródło ciepła. Dodaj oba zadania do jednej grupy zadań problemów
i pozwól grupie iterować: solver radiacyjny zgłasza zbieżność i kończy grupę, gdy
ciepło jego łat przestaje się zmieniać.

**Joule heat** (ciepło Joule'a) przychodzi w ten sam sposób z zadania
*Electro-statics* umieszczonego przed zadaniem cieplnym, więc nagrzewanie
rezystancyjne nie wymaga dalszej konfiguracji.

O ile nie jest sprzężony z płynem, solver cieplny bezwarunkowo zgłasza
zbieżność, więc grupa zadań zawierająca tylko zadanie cieplne zawsze kończy się
po jednej iteracji. Iterowanie ma sens, gdy coś, od czego zależy rozwiązanie
cieplne, samo jest opóźnione - ciepło promieniowania, współczynnik konwekcji
naturalnej lub silnie zależna od temperatury przewodność.

#### Sprzężona wymiana ciepła

Solver cieplny rozwiązuje **tylko ciała stałe**. Objętość, której materiałem jest
płyn - gaz lub ciecz albo materiał o nieokreślonym stanie, który ma lepkość
dynamiczną, co obejmuje każdy płyn z bazy materiałów - nigdy nie jest częścią
rozwiązania cieplnego, niezależnie od swoich właściwości i warunków. To samo
dotyczy encji punktu, linii lub powierzchni leżącej wewnątrz płynu, takiej jak
wlot; warunek na niej, który odczytuje tylko solver cieplny, jest zgłaszany w
dzienniku. Obszar płynu należy do typu problemu *Heat transfer in fluids*, który
przenosi ciepło wraz z przepływem.

Oba spotykają się na ścianach z warunkiem **Forced convection**. Gdy oba zadania
są w jednej grupie zadań problemów - zadanie ciepła w płynach jako pierwsze -
każde przejście grupy wykonuje następujące czynności:

1. Solver ciepła w płynach utrzymuje węzły ściany w temperaturze ostatnio
   obliczonej przez solver cieplny w ciele stałym (w pierwszym przejściu, gdy
   takiej temperatury jeszcze nie ma, ściana jest izolowana), rozwiązuje płyn i
   pobiera strumień ciepła wpływający do płynu w każdym węźle ściany. Ten
   strumień jest residuum równań płynu w węźle, tym samym, które widziałoby
   jedno rozwiązanie obu obszarów, więc cienka termiczna warstwa przyścienna nie
   pogarsza dokładności.
2. Dla każdego elementu ściany przekazuje parę `(h, Tf)`. `h = k * G` jest
   przewodnością pierwszego elementu płynu za ścianą - dla czworościanu `k`
   podzielone przez wysokość elementu nad ścianą - a `Tf` jest ustawiane tak, aby
   `q = h * (Tw - Tf)` było strumieniem z kroku 1.
3. Solver cieplny stosuje je na ścianie tak, jak zastosowałby warunek *Simple
   convection*, rozwiązuje ciało stałe i przekazuje z powrotem temperaturę
   ściany.

Para `(h, Tf)` jest środkiem numerycznym, który uzgadnia oba rozwiązania - `h`
nie jest współczynnikiem przejmowania ciepła z poradnika i zmienia się wraz z
siatką. Zbieżne są temperatura i strumień przez ścianę, które są wartościami
problemu sprzężonego. Nie jest potrzebna żadna informacja o prędkości płynu przy
ścianie, dlatego ściana z warunkiem braku poślizgu - gdzie przepływ jest w
spoczynku - przenosi ciepło dokładnie tak, jak powinna.

Temperatura ściany przekazywana do płynu jest relaksowana **współczynnikiem
Aitkena** obliczanym z dwóch ostatnich przejść. Bez niego słabo przewodzące ciało
stałe obok dobrze rozwiązanego płynu może wymagać setek przejść; z nim grupa
zwykle stabilizuje się po kilku. Oba solvery, gdy są sprzężone, zgłaszają jako
swoją zbieżność względną zmianę swojego pola temperatury, więc ustaw wartość
zbieżności grupy zadań - `1e-6` to rozsądny początek - i zapewnij jej wystarczającą
liczbę iteracji. Zerowa wartość zbieżności wykonuje wszystkie iteracje.

### 1.7 Analiza w stanie ustalonym

Przy wyłączonym solverze czasowym solver składa i rozwiązuje

```
K * T = f
```

`K` jest symetryczna i dodatnio określona, gdy zadano co najmniej jedną
temperaturę, więc rozwiązuje się ją metodą **gradientów sprzężonych** z
prekondycjonowaniem Jacobiego.

Model w stanie ustalonym **bez warunku brzegowego Temperature i bez powierzchni z
konwekcją** jest cieplnym odpowiednikiem konstrukcji bez podparć: energię można
wprowadzić, ale nie ma dokąd odpłynąć, macierz jest osobliwa, a rozwiązanie nie
będzie zbieżne. Każdy model w stanie ustalonym potrzebuje co najmniej jednej
drogi do znanej temperatury.

### 1.8 Analiza nieustalona

Przy włączonym solverze czasowym solver całkuje w czasie

```
M * dT/dt + K * T = f
```

schematem theta, w którym `alpha` jest współczynnikiem przybliżenia schematu
czasowego:

```
( M + alpha*dt*K ) * T_n+1 = dt*f + ( M - (1-alpha)*dt*K ) * T_n
```

Ustawienie *Przybliżenie* solvera czasowego wybiera `alpha`:

| Przybliżenie | `alpha` | Wynikowy schemat | Stabilność |
|---|---|---|---|
| Backward difference (stable) | `1` | `(M + dt*K)*T_n+1 = dt*f + M*T_n`, krok niejawny | bezwarunkowa |
| Central difference (accurate) | `0.5` | Crank-Nicolson | bezwarunkowa, możliwe oscylacje |
| Forward difference (fast) | `0` | `M*T_n+1 = dt*f + (M - dt*K)*T_n`, krok jawny | warunkowa |

**Backward difference** (różnica wsteczna) to bezpieczny wybór domyślny dla
modelu cieplnego. Kosztuje jedno rozwiązanie macierzowe na krok, ale krok jest
ograniczony przez oczekiwaną dokładność, a nie przez rozmiar najmniejszego
elementu. **Central difference** (różnica centralna) ma dokładność drugiego
rzędu i jest lepszym wyborem, gdy krok czasowy jest wystarczająco mały, by
rozdzielić stan przejściowy, choć przy gwałtownym włączeniu może oscylować.
**Forward difference** (różnica w przód) umieszcza całą macierz przewodzenia na
poprzednim kroku czasowym, co czyni każdy krok tanim, a obliczenia kruchymi.

Wariant w przód jest stabilny tylko wtedy, gdy krok czasowy pozostaje poniżej
w przybliżeniu

```
dt < rho * c * h^2 / ( 2 * k )
```

gdzie `h` jest najmniejszym rozmiarem elementu. Ponieważ dyfuzyjność cieplna
`k/(rho*c)` metalu jest rzędu `1e-5 m^2/s`, element o rozmiarze milimetra
wymusza już krok czasowy znacznie poniżej jednej dziesiątej sekundy. Poza bardzo
drobną rozdzielczością czasową używaj schematu niejawnego.

Niestabilne obliczenia nieustalone łatwo rozpoznać: pole temperatury oscyluje z
kroku na krok z rosnącą amplitudą, często między fizycznie niemożliwymi
wartościami, zamiast się stabilizować. Lekarstwem jest mniejszy krok czasowy lub
schemat niejawny.

### 1.9 Wyniki pochodne

Po rozwiązaniu strumień ciepła w elemencie jest wyznaczany z pola temperatury
jako średnia po punktach całkowania z

```
Q = -k * grad(T)
```

obliczana w elemencie i obracana do współrzędnych globalnych dla elementów
liniowych i powierzchniowych. Na powierzchni z konwekcją strumień konwekcyjny

```
Qh = h * ( Tf - T )
```

jest dodawany wzdłuż normalnej do powierzchni, więc wektor strumienia ściany z
konwekcją wskazuje kierunek, w którym ciepło faktycznie odpływa.

Strumień jest **gęstością** w `W/m^2`, niezależną od rozmiaru elementu, w którym
go obliczono, więc można go porównywać między elementami i z obliczeniami
ręcznymi, a przy zagęszczaniu siatki jest zbieżny.

Zapisywany jest także **współczynnik przejmowania ciepła**. Jest to współczynnik
faktycznie użyty przez solver na każdym elemencie powierzchni, co jest jedynym
sposobem sprawdzenia, co wytworzyły korelacje *Forced convection* i *Natural
convection* - a na powierzchni z konwekcją naturalną zmienia się on od elementu
do elementu, ponieważ zależy od lokalnej temperatury ściany. Na ścianie
sprzężonej z płynem jest przewodnością pierwszego elementu płynu, która zależy od
siatki - zob. *Sprzężona wymiana ciepła* w 1.6. Na każdym elemencie bez warunku
konwekcji wynosi zero.

| Wynik | Dotyczy | Znaczenie |
|---|---|---|
| Temperature `[K]` | węzeł | rozwiązane pole temperatury |
| Heat flux `[W/m^2]` | element | wektor strumienia przewodzenia plus część konwekcyjna na powierzchniach z konwekcją |
| Heat transfer coefficient `[W/(m^2*K)]` | element | użyty współczynnik konwekcji, zero tam, gdzie nie ma konwekcji |

Po każdym rekordzie dziennik solvera wypisuje statystyki wszystkich trzech oraz
wartości w punktach monitorowania.

### 1.10 Stan ustalony czy nieustalony?

| | Stan ustalony | Stan nieustalony |
|---|---|---|
| Rozwiązuje | `K*T = f` | `M*T' + K*T = f` w czasie |
| Odpowiada na | „jak gorąco będzie w końcu” | „ile czasu potrzeba, aby to osiągnąć” |
| Używa gęstości i pojemności cieplnej | nie, ale nadal muszą być obecne | tak |
| Wymaga zadanej temperatury lub powierzchni z konwekcją | tak | nie, wystarczy pojemność |
| Rekordy | jeden | jeden na każdy zapisany krok czasowy |

**Używaj analizy w stanie ustalonym do** końcowego rozkładu temperatury części
pracującej w sposób ciągły, doboru radiatora, mostka cieplnego w ścianie lub
pola temperatury zasilającego analizę naprężeń cieplnych. Jest to ustawienie
domyślne i obejmuje większość prac.

**Używaj analizy nieustalonej do** nagrzewania i stygnięcia, cyklu pracy, szoku
cieplnego, hartowania lub każdego przypadku, w którym chodzi o stałą czasową
części.

---

## 2. Interfejs graficzny

Wszystko, co istotne, znajduje się w panelu **Solver** (karty `Problem`,
`Warunki brzegowe`, `Warunki początkowe`, `Warunki otoczenia`, `Materiał`,
`Wyniki`), menu **Problem** i panelu **Model**.

### 2.1 Wybór problemu

`Problem` -> `Przebieg zadań problemów` (`Ctrl+P`) otwiera okno przebiegu zadań
problemu. Dodaj zadanie i wybierz **Heat transfer** - „Steady-state and transient
Heat-transfer” (ustalona i nieustalona wymiana ciepła).

Heat transfer można swobodnie łączyć z każdą inną fizyką z wyjątkiem *Modal
analysis*. Warto znać następujące łańcuchy:

- *Heat transfer*, a następnie *Stress analysis* - naprężenia cieplne;
- *Heat transfer* i *Radiative heat transfer* w jednej iterującej grupie zadań -
  przewodzenie plus promieniowanie w zamkniętej przestrzeni;
- *Electro-statics*, a następnie *Heat transfer* - nagrzewanie rezystancyjne.

Aby iterować sprzężoną parę, dodaj grupę zadań i ustaw jej liczbę iteracji;
grupa kończy się wcześniej, gdy każde zadanie w niej zgłosi zbieżność.

### 2.2 Karta Problem

**Solver czasowy** - wyświetlany dla wymiany ciepła, ponieważ jest to typ
problemu zależny od czasu. W analizie w stanie ustalonym pozostaw go wyłączonym.

| Pole | Znaczenie |
|---|---|
| Włącz | włącza i wyłącza nieustalone całkowanie w czasie |
| Przybliżenie | theta schematu czasowego: wsteczny, centralny lub w przód, zob. 1.8 |
| Czas początkowy | pierwsza wartość czasu w sekundach; ignorowana przy ponownym uruchomieniu solvera |
| Czas końcowy | `start time + time-step size * number of time-steps` (czas początkowy + rozmiar kroku czasowego × liczba kroków czasowych), tylko do odczytu |
| Rozmiar kroku czasowego | `dt` w sekundach |
| Liczba kroków czasowych | liczba kroków do obliczenia |
| Częstotliwość zapisu | zapisuje plik wyników co N kroków; `0` zapisuje tylko ostatni krok |

Problem cieplny nie ma własnej grupy ustawień. Wszystko inne przypisuje się do
poszczególnych encji na kartach warunków.

### 2.3 Karta Materiał

Karta `Materiał` zawiera materiał przypisany do wybranej encji. Wymiana ciepła
wymaga **wszystkich czterech** poniższych właściwości:

| Właściwość | Jednostki | Komentarz |
|---|---|---|
| Thermal conductivity | `W/(m*K)` | `k`, około `50` dla stali konstrukcyjnej, `15` dla nierdzewnej, `200` dla aluminium |
| Heat capacity | `J/(kg*K)` | `c`, około `460` dla stali |
| Density | `kg/m^3` | `rho`, około `7850` dla stali |
| Emissivity | `-` | używana tylko przez radiacyjną wymianę ciepła, ale mimo to wymagana |

Encja, której brakuje którejkolwiek z nich, nie jest rozwiązywana, a kontrola
konfiguracji ostrzega o tym przed rozpoczęciem obliczeń. Jeśli nie wykonujesz
radiacyjnej wymiany ciepła, do spełnienia wymagania wystarczy emisyjność `0`.

Materiał musi być też **ciałem stałym**. Objętość, której materiał jest gazem lub
cieczą - albo nie ma ustawionego stanu i ma lepkość dynamiczną - jest
pozostawiana typowi problemu *Heat transfer in fluids*, nawet jeśli ma wszystkie
cztery właściwości.

Każda właściwość jest tablicą w funkcji temperatury, więc przewodność zależna od
temperatury jest uwzględniana - obliczana dla temperatury elementu z poprzedniego
uruchomienia, przez co przy silnej zależności warto iterować zadanie.

### 2.4 Karta Warunki brzegowe

Karta jest podzielona na listę menedżera dostępnych warunków brzegowych u góry i
edytor wybranego warunku u dołu. Najpierw wybierz encję w drzewie `Model`;
menedżer zaoferuje wtedy warunki właściwe dla danego typu encji i wybranego
problemu. Zaznacz warunek, aby go przypisać, a następnie edytuj jego składowe w
dolnym drzewie.

Oferowane są warunki z punktu 1.4. Uwagi dotyczące ich użycia:

- **Temperature** jest jedynym warunkiem, który utrzymuje wartość, zamiast ją
  wymuszać. Przypisz go do ściany, krawędzi lub punktu o znanej temperaturze.
- **Heat** jest oferowany dla każdego typu encji i przyjmuje całkowitą moc w
  `W`; solver sam rozkłada ją na encję. Jest to warunek, po który należy sięgnąć,
  gdy znana jest moc w watach tego, co wytwarza ciepło.
- **Heat rate (unit volume)** na objętości i **Heat rate (unit area)** na
  powierzchni zadają to samo źródło jako gęstość, dla źródła naturalnie
  określonego na jednostkę objętości lub pola.
- Trzy warunki **konwekcji** dotyczą tylko powierzchni. Przypisz *Simple
  convection*, chyba że masz konkretny powód, by pozwolić solverowi wyznaczyć
  współczynnik z korelacji; jest to wariant przewidywalny. *Forced convection*
  na ścianie graniczącej z siatkowanym płynem sprzęga ciało stałe z rozwiązaniem
  ciepła w płynach i nie odczytuje żadnej ze swoich składowych; w pozostałych
  miejscach wyznacza z nich współczynnik z korelacji.
- Każda składowa jest tablicą w funkcji czasu, więc zadanej temperaturze lub
  źródłu ciepła można nadać przebieg czasowy dla obliczeń nieustalonych. W ten
  sposób modeluje się cykl pracy lub stopniowo włączany grzejnik.

### 2.5 Warunki początkowe i otoczenia

Karta `Warunki początkowe` oferuje *Temperature*, który ustawia pole startowe
obliczeń nieustalonych. Przypisz go do całego modelu i nadaj mu temperaturę
początkową części.

Karta `Warunki otoczenia` także oferuje *Temperature*, jako wartość tła dla
encji. Oba są odczytywane tylko przy pierwszym uruchomieniu, a warunek brzegowy
ma pierwszeństwo przed obydwoma.

### 2.6 Konfiguracja solvera macierzowego

`Problem` -> `Konfiguruj solver macierzowy problemów` konfiguruje solvery
iteracyjne. Solver cieplny używa pozycji **CG**.

Jeśli rozwiązanie w stanie ustalonym nie jest zbieżne, przyczyną jest zwykle
model bez zadanej temperatury i bez powierzchni z konwekcją, a nie ustawienie
solvera. Zanim zwiększysz liczbę iteracji, sprawdź warunki brzegowe.

### 2.7 Punkty monitorowania

`Problem` -> `Zdefiniuj punkty monitorowania` umieszcza sondy w podanych
współrzędnych i wybiera rejestrowaną zmienną - dla problemu cieplnego
*Temperature*. W obliczeniach nieustalonych przebieg pokazuje `Raport` ->
`Punkty monitorowania`, co jest najszybszym sposobem odczytania z modelu krzywej
nagrzewania lub stałej czasowej.

### 2.8 Wyniki i rekordy

Karta `Wyniki` zawiera listę obliczonych zmiennych i steruje widokiem 3D.
Temperatura jest skalarem węzłowym, strumień ciepła wektorem elementowym, a
współczynnik przejmowania ciepła skalarem elementowym, który poza powierzchniami
z konwekcją wynosi zero.

Karta `Rekordy` panelu `Model` zawiera listę rekordów wyników:

- **analiza w stanie ustalonym** - jeden rekord;
- **analiza nieustalona** - jeden rekord na każdy zapisany krok czasowy.
  Animacja rekordów odtwarza nagrzewanie.

`Raport` -> `Plik dziennika solvera` wyświetla pełne wyjście solvera, w tym
statystyki temperatury, strumienia ciepła i współczynnika przejmowania ciepła po
każdym rekordzie.

---

## 3. Samouczek - ustalona wymiana ciepła

**Cel.** Obliczyć rozkład temperatury pręta utrzymywanego w wysokiej
temperaturze na jednym końcu i chłodzonego otaczającym powietrzem oraz odczytać
temperaturę na zimnym końcu.

Ten samouczek zakłada zasiatkowany model objętościowy - dobrze sprawdza się
długi prostopadłościan. Użyj `Plik` -> `Nowy model`, narysuj prostopadłościan za
pomocą `Geometria` -> `Utwórz element` lub zaimportuj geometrię i podziel ją na
czworościany; samouczek *Rysowanie sześcianu* omawia te czynności. Oznacz gorącą
ścianę końcową i pozostałe ściany jako oddzielne encje powierzchni, aby można
było przypisać im warunki.

### Krok 1 - wybierz problem

1. `Problem` -> `Przebieg zadań problemów` (`Ctrl+P`).
2. Dodaj zadanie i wybierz **Heat transfer**.
3. Zatwierdź przyciskiem `OK`.

### Krok 2 - przypisz materiał

1. Wybierz encję objętości w drzewie `Model`.
2. Otwórz kartę `Materiał` i przypisz materiał ze wszystkimi czterema
   właściwościami, dla stali konstrukcyjnej:
   - **Thermal conductivity** = `50` `W/(m*K)`
   - **Heat capacity** = `460` `J/(kg*K)`
   - **Density** = `7850` `kg/m^3`
   - **Emissivity** = `0`

Pojemność cieplna i gęstość nie są używane w rozwiązaniu w stanie ustalonym, ale
muszą być obecne, aby encja w ogóle była rozwiązywana.

### Krok 3 - pozostaw analizę w stanie ustalonym

Otwórz kartę `Problem` i pozostaw **Włącz** niezaznaczone w grupie *Solver
czasowy*. Solver raz składa i rozwiązuje wtedy `K*T = f`.

### Krok 4 - zadaj gorący koniec

1. Wybierz encję powierzchni na gorącym końcu.
2. Otwórz kartę `Warunki brzegowe` i zaznacz **Temperature**.
3. Ustaw wartość `353.15` K, czyli 80 stopni Celsjusza.

Pamiętaj, że wszystkie temperatury w Range FEA są bezwzględne, w kelwinach.

### Krok 5 - schłodź resztę

1. Wybierz encję powierzchni obejmującą pozostałe ściany.
2. Otwórz kartę `Warunki brzegowe` i zaznacz **Simple convection**.
3. Ustaw **Convection coefficient** (współczynnik konwekcji) = `10`
   `W/(m^2*K)` i **Fluid temperature** (temperatura płynu) = `293.15` K.

Współczynnik około `10` jest typowy dla powierzchni w nieruchomym powietrzu;
wymuszony przepływ powietrza daje bliżej `50`, a woda setki lub tysiące.

To ten krok czyni model rozwiązywalnym. Bez niego pręt ma dopływ ciepła, ale nie
ma odpływu, i stan ustalony nie istnieje.

### Krok 6 - rozwiąż

1. `Rozwiązanie` -> `Uruchom solver` (`Ctrl+R`).
2. Kontrola konfiguracji zgłasza brakujące materiały lub brakujące warunki
   brzegowe przed rozpoczęciem obliczeń.
3. Śledź postęp w `Raport` -> `Plik dziennika solvera`.

### Krok 7 - przejrzyj wyniki

- Karta `Wyniki`: wyświetl **Temperature**. Pole powinno płynnie maleć od
  zadanego gorącego końca w kierunku temperatury płynu.
- Najzimniejszy punkt powinien pozostać powyżej temperatury płynu. Temperatura
  gdziekolwiek poniżej `Tf` lub powyżej zadanych `353.15` K oznacza, że coś
  dostarcza energię w złym kierunku - sprawdź znak każdego źródła ciepła.
- **Heat flux** pokazuje, którędy przepływa energia, w `W/m^2`. Pomnóż wartość na
  ścianach z konwekcją przez ich pole i porównaj z mocą wpływającą do pręta.
- **Heat transfer coefficient** powinien wynosić `10` na ścianach z konwekcją i
  `0` wszędzie indziej. Jest to najszybszy sposób potwierdzenia, że warunek
  konwekcji dotarł do zamierzonych elementów, i jedyny sposób sprawdzenia, co
  obliczył warunek oparty na korelacji.
- Punkt monitorowania dla *Temperature* na zimnym końcu podaje odpowiedź jako
  liczbę, a nie kolor.

### Krok 8 - opcjonalnie dodaj źródło ciepła

Aby zamodelować grzejnik osadzony w pręcie, wybierz encję objętości, otwórz kartę
`Warunki brzegowe` i zaznacz **Heat**. Aby dostarczyć do pręta łącznie `P` watów,
wprowadź `P` - solver rozłoży tę wartość na objętość encji.

Użyj zamiast tego **Heat rate (unit volume)**, gdy moc źródła jest podana na
jednostkę objętości; aby dostarczyć tę samą wartość całkowitą, wprowadź `P / V`
`W/m^3` - zob. 1.4.

Temperatura w stanie ustalonym musi wtedy wzrosnąć, aż powierzchnia z konwekcją
będzie mogła oddawać dokładnie tę moc - co jest sprawdzeniem bilansu energii z
rozdziału 5.

### Krok 9 - opcjonalnie naprężenia cieplne

Aby przenieść wynik do analizy konstrukcyjnej:

1. Dodaj zadanie **Stress analysis** *po* zadaniu cieplnym w przebiegu zadań
   problemów i nadaj encji potrzebne właściwości konstrukcyjne.
2. Przypisz encji konstrukcyjnej warunek otoczenia **Temperature** z
   temperaturą odniesienia stanu beznaprężeniowego, na przykład `293.15` K.

Solver naprężeń pobiera obliczone temperatury elementów i automatycznie dodaje
człon rozszerzalności cieplnej.

### Rozwiązywanie problemów

| Objaw | Przyczyna |
|---|---|
| Rozwiązanie nie jest zbieżne | brak warunku Temperature i brak powierzchni z konwekcją, więc stan ustalony jest nieokreślony |
| *Entity has material assigned which is missing required properties* | brakuje jednej z czterech wymaganych właściwości - często emisyjności |
| Cały model ma jedną temperaturę | nie dopływa ciepło lub nic nie wytwarza gradientu względem zadanej wartości |
| Temperatury są absurdalnie wysokie | do warunku *Heat*, który przyjmuje wartość całkowitą w watach, wprowadzono gęstość w `W/m^3` lub współczynnik konwekcji jest o wiele za mały |
| Ściana między ciałem stałym a płynem nie stygnie | zadanie ciepła w płynach jest wykonywane po zadaniu cieplnym lub grupa wykonuje jedną iterację - umieść zadanie ciepła w płynach jako pierwsze i pozwól grupie iterować |
| Płyn jest ignorowany przez rozwiązanie cieplne | zamierzone - materiał płynny jest rozwiązywany tylko przez *Heat transfer in fluids* |
| Temperatury wszędzie wynoszą około `293.15` K | encja nie została rozwiązana - sprawdź w dzienniku brakującą właściwość materiału |

---

## 4. Samouczek - nieustalona wymiana ciepła

**Cel.** Rozpocząć z tym samym prętem w temperaturze pokojowej, włączyć gorący
koniec i ustalić, ile czasu zajmuje nagrzewanie.

### Krok 1 - wybierz problem

To samo zadanie **Heat transfer** co w samouczku 3. Jeśli model jest nadal
skonfigurowany, trzeba dodać tylko ustawienia solvera czasowego i warunek
początkowy.

### Krok 2 - przypisz materiał

Te same cztery właściwości. Tutaj znaczenie mają **pojemność cieplna** i
**gęstość**: razem z przewodnością określają dyfuzyjność cieplną `k/(rho*c)`,
która decyduje o tym, jak szybko przemieszcza się front.

### Krok 3 - ustaw temperaturę początkową

1. Wybierz encję objętości.
2. Otwórz kartę `Warunki początkowe` i zaznacz **Temperature**.
3. Ustaw `293.15` K.

Bez tego model i tak rozpoczyna od domyślnych `293.15` K, ale jawne podanie
wartości sprawia, że model sam się dokumentuje, i pozwala rozpocząć od
dowolnego innego stanu.

### Krok 4 - skonfiguruj solver czasowy

Otwórz kartę `Problem` i w grupie *Solver czasowy* ustaw:

| Pole | Wartość |
|---|---|
| Włącz | zaznaczone |
| Przybliżenie | `Backward difference (stable)` |
| Czas początkowy | `0` |
| Rozmiar kroku czasowego | `1` |
| Liczba kroków czasowych | `300` |
| Częstotliwość zapisu | `10` |

*Backward difference* jest schematem niejawnym - zob. tabela w 1.8 - i należy go
tu użyć. Pozostaje stabilny przy kroku czasowym dobranym do fizyki, a nie do
siatki.

Dobierz krok czasowy do oczekiwanej stałej czasowej, dążąc do około
pięćdziesięciu kroków w interesującej części krzywej. Rozdział 5 podaje wzór do
pierwszego oszacowania. Zapisywanie co dziesiątego kroku utrzymuje mały plik
wyników bez utraty kształtu krzywej.

### Krok 5 - zachowaj warunki brzegowe

Zadana **Temperature** na gorącym końcu i **Simple convection** na pozostałych
ścianach przenoszą się bez zmian. Każda składowa jest tablicą w funkcji czasu,
więc gorącemu końcowi można zamiast tego nadać narastanie: otwórz edytor
składowej i dodaj punkty czasowe, aby temperatura rosła przez pierwsze kilka
sekund, zamiast zmienić się skokowo.

### Krok 6 - dodaj punkt monitorowania

`Problem` -> `Zdefiniuj punkty monitorowania`, umieść punkt na zimnym końcu i
ustaw jego zmienną na **Temperature**. Krzywą nagrzewania o wiele łatwiej
odczytać jako przebieg niż jako sekwencję kolorowych obrazów.

### Krok 7 - rozwiąż

`Rozwiązanie` -> `Uruchom solver` (`Ctrl+R`). Dziennik wypisuje jeden blok na
krok czasowy:

```
time step:         1 of 300       | time =  0.000000e+00 [sec] | dt =  1.000000e+00 [sec]
```

(`time step: 1 of 300` = krok czasowy 1 z 300.) Następnie wypisywane są
iteracje solvera macierzowego i statystyki temperatury dla tego kroku.

### Krok 8 - przejrzyj wyniki

- Panel `Model`, karta `Rekordy`: jeden rekord na zapisany krok. Przeglądaj je
  lub animuj, aby obserwować, jak front ciepła wnika w pręt.
- Karta `Wyniki`: wyświetl **Temperature**. Ustal zakres wyświetlania dla
  wszystkich rekordów, aby kolory znaczyły to samo w każdej klatce; w przeciwnym
  razie animacja sama zmienia skalę i postęp jest niewidoczny.
- `Raport` -> `Punkty monitorowania` rysuje krzywą nagrzewania na zimnym końcu.
  Odczytaj z niej stałą czasową: czas pokrycia około 63 procent całkowitego
  przyrostu.
- Ostatni rekord powinien zbliżać się do wyniku w stanie ustalonym z samouczka 3.
  Jeśli tak nie jest, obliczenia były zbyt krótkie.

### Rozwiązywanie problemów

| Objaw | Przyczyna |
|---|---|
| Temperatura oscyluje i rośnie z kroku na krok | schemat jawny ze zbyt dużym krokiem czasowym - zmień przybliżenie na *Backward difference (stable)* lub zmniejsz `dt` |
| Wynik prawie nie zmienia się w ciągu całych obliczeń | krok czasowy jest znacznie mniejszy od stałej czasowej lub obliczenia są zbyt krótkie |
| Pierwszy krok przeskakuje do wyniku końcowego | krok czasowy jest znacznie większy od stałej czasowej, więc schemat działa praktycznie jak stan ustalony |
| Zapisano tylko jeden rekord | częstotliwość zapisu wynosi `0`, co zapisuje tylko ostatni krok |
| Stan nieustalony nigdy nie osiąga stanu ustalonego | często jest to poprawne - porównaj z obliczeniami w stanie ustalonym, zanim zaczniesz podejrzewać solver |

---

## 5. Sprawdzanie modelu

Solver nie ma wbudowanego zestawu testów weryfikacyjnych, więc przed zaufaniem
nowemu modelowi warto go sprawdzić względem czegoś, co można obliczyć ręcznie.
Cztery tanie sprawdzenia, w kolejności przydatności:

**Ściana płaska.** Dla płyty o grubości `L` i przewodności `k` utrzymywanej na
`T1` i `T2` na swoich dwóch ścianach temperatura jest liniowa w poprzek płyty, a
strumień wynosi

```
q = k * ( T1 - T2 ) / L          [W/m^2]
```

Zamodeluj płytę elementami objętościowymi, zadaj obie ściany, potwierdź, że
płaszczyzna środkowa ma dokładnie wartość pośrednią i że obliczony **Heat flux**
odpowiada `q`. Wychwytuje to błędne jednostki, błędną przewodność i siatkę, która
nie jest połączona tak, jak się zakłada. Zagęść siatkę raz i potwierdź, że obie
wartości się nie zmieniają.

**Ściana z konwekcją.** Dla tej samej płyty utrzymywanej na `T1` na jednej
ścianie i chłodzonej na drugiej płynem o temperaturze `Tf` ze współczynnikiem
`h` chłodzona powierzchnia ustala się na

```
Ts = ( (k/L)*T1 + h*Tf ) / ( k/L + h )
```

Porównaj z obliczoną temperaturą powierzchni. Weryfikuje to warunek konwekcji,
czyli tę część modelu cieplnego, w której najłatwiej o błąd.

**Skupiona stała czasowa.** Do sprawdzenia nieustalonego weź ciało o objętości
`V` i polu powierzchni `A` bez zadanej temperatury, z temperaturą początkową
`T0`, chłodzone wyłącznie przez konwekcję. Dopóki liczba Biota `h*V/(A*k)`
pozostaje poniżej około `0.1`, ciało jest niemal izotermiczne i

```
tau  = rho * c * V / ( h * A )
T(t) = Tf + ( T0 - Tf ) * exp( -t / tau )
```

Odczytaj `tau` z przebiegu w punkcie monitorowania i porównaj. Weryfikuje to
jednocześnie macierz pojemności, warunek konwekcji i krok czasowy - i jest
najszybszym sposobem doboru kroku czasowego dla rzeczywistego modelu, ponieważ
`tau/50` to dobre pierwsze przybliżenie.

**Bilans energii.** Dla modelu w stanie ustalonym ze znaną mocą wejściową `P`
powierzchnia z konwekcją musi oddawać dokładnie `P` w temperaturze końcowej:

```
P = h * A * ( Ts_mean - Tf )
```

Odczytaj średnią temperaturę powierzchni ze statystyk w dzienniku solvera i
sprawdź bilans. Jest to jedyne globalne sprawdzenie, które wychwytuje wartość
Heat wprowadzoną jako wartość całkowita zamiast gęstości.

Przy każdym z tych sprawdzeń zagęść siatkę co najmniej raz i potwierdź, że wynik
jest zbieżny, a nie dryfuje.

---

## 6. Ograniczenia

### Ograniczenia modelowania

- **Tylko liniowe przewodzenie.** Przewodność, pojemność i gęstość elementu są
  stałe podczas rozwiązania, odczytywane z tablicy materiałowej dla temperatury
  z poprzedniego uruchomienia. Właściwość silnie zależna od temperatury wymaga do
  zbieżności iterującej grupy zadań.
- **Brak przemian fazowych.** Nie ma ciepła utajonego ani sformułowania
  entalpowego, więc topnienie, krzepnięcie, wrzenie i kondensacja nie są
  modelowane. Tablica materiałowa nie może odwzorować plateau ciepła utajonego.
- **Brak adwekcji.** Energia przemieszcza się przez przewodzenie wewnątrz ciała i
  przez konwekcję przez jego brzeg. Poruszający się ośrodek nie unosi ze sobą
  energii; do tego służy *Heat transfer in fluids*.
- **Konwekcja jest modelem brzegowym.** Odmiany oparte na korelacji używają po
  jednej stałej korelacji - zewnętrznej płaskiej płyty dla przepływu
  wymuszonego, płyt poziomych dla konwekcji naturalnej - ze średnicą
  hydrauliczną jako długością charakterystyczną. Orientacja powierzchni, reżim
  przepływu ani to, czy przepływ jest wewnętrzny czy zewnętrzny, nie są
  wykrywane. Gdy współczynnik ma znaczenie, oblicz go samodzielnie i wprowadź za
  pomocą *Simple convection*.
- **Współczynnik konwekcji naturalnej jest opóźniony.** Jest obliczany dla
  temperatury ściany z poprzedniego rozwiązania, więc jednoprzebiegowe obliczenia
  w stanie ustalonym w praktyce używają początkowej różnicy temperatur.
- **Promieniowanie tylko przez oddzielny typ problemu.** Sam solver cieplny nie ma
  członu emisyjności powierzchni; promieniowanie w zamkniętej przestrzeni wymaga
  zadania *Radiative heat transfer*, jego macierzy współczynników widzenia i
  iteracji między nimi.
- **Powierzchnie są izotermiczne w kierunku grubości.** Encja powierzchni
  przewodzi tylko w swojej płaszczyźnie. Ścianę, w której gradient w kierunku
  grubości ma znaczenie, modeluj elementami objętościowymi.
- **Elementy punktowe nie przewodzą.** Punkt jest skupioną pojemnością i źródłem
  ciepła, niczym więcej.
- **Model w stanie ustalonym potrzebuje odpływu.** Bez zadanej temperatury i bez
  powierzchni z konwekcją macierz jest osobliwa, a rozwiązanie nie będzie
  zbieżne.

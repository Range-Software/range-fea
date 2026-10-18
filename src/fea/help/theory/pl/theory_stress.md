# Analiza naprężeń - Podręcznik teorii i przewodnik użytkownika

Ten dokument opisuje solver konstrukcyjny (`RSolverStress`) programu Range FEA,
obsługujący zarówno typ problemu **Stress analysis**, jak i **Modal analysis**:
rozwiązywane równania, znaczenie wszystkich przyjmowanych danych wejściowych,
elementy interfejsu graficznego, które nim sterują, oraz dwa kompletne
samouczki.

**Spis treści**

1. [Podstawy teoretyczne](#1-podstawy-teoretyczne)
2. [Interfejs graficzny](#2-interfejs-graficzny)
3. [Samouczek - statyczna analiza naprężeń](#3-samouczek---statyczna-analiza-naprężeń)
4. [Samouczek - analiza modalna](#4-samouczek---analiza-modalna)
5. [Sprawdzanie modelu](#5-sprawdzanie-modelu)
6. [Ograniczenia](#6-ograniczenia)

---

## 1. Podstawy teoretyczne

### 1.1 Niewiadoma podstawowa - pole przemieszczeń

Niewiadomą podstawową jest **wektor przemieszczenia** `u = (ux, uy, uz)` w każdym
węźle, w metrach. Każdy węzeł ma trzy stopnie swobody, więc siatka o `N` węzłach
daje układ `3N` równań. Odkształcenia, naprężenia i siły reakcji wyznacza się z
pola przemieszczeń po rozwiązaniu.

### 1.2 Równania podstawowe

Solver realizuje **liniową, izotropową sprężystość przy małych
odkształceniach**. Definiują ją trzy zależności.

**Równowaga** ciała pod obciążeniem objętościowym `b` i obciążeniem
powierzchniowym `t`:

```
div(sigma) + b = 0
```

Zależność **odkształcenie-przemieszczenie** przy małych odkształceniach:

```
eps_xx = dux/dx        gamma_xy = dux/dy + duy/dx
eps_yy = duy/dy        gamma_yz = duy/dz + duz/dy
eps_zz = duz/dz        gamma_xz = dux/dz + duz/dx
```

Zależność **konstytutywna** z członem cieplnym:

```
sigma = D * ( eps - alpha * dT )
```

| Symbol | Znaczenie | Jednostki | Źródło |
|---|---|---|---|
| `u` | przemieszczenie | `m` | rozwiązywane |
| `sigma` | naprężenie Cauchy'ego | `Pa` | wyznaczane |
| `eps` | odkształcenie inżynierskie | `-` | wyznaczane |
| `E` | moduł sprężystości | `Pa` | właściwość materiału |
| `v` | liczba Poissona | `-` | właściwość materiału |
| `rho` | gęstość | `kg/m^3` | właściwość materiału |
| `alpha` | współczynnik rozszerzalności cieplnej | `1/K` | właściwość materiału |
| `dT` | przyrost temperatury ponad stan beznaprężeniowy | `K` | zob. 1.5 |

Wszystkie cztery właściwości materiału są **wymagane** - encja, której brakuje
którejkolwiek z nich, nie jest rozwiązywana. Dwa współczynniki tłumienia
Rayleigha analizy dynamicznej (zob. 1.7) są opcjonalne.

### 1.3 Macierz sprężystości

**Elementy objętościowe** używają pełnej trójwymiarowej izotropowej macierzy
sprężystości w notacji Voigta, z wektorem naprężeń uporządkowanym jako
`[sigma_xx, sigma_yy, sigma_zz, tau_yz, tau_xz, tau_xy]`:

```
        E                | 1-v   v    v      0        0        0     |
D = -----------------  * |  v   1-v   v      0        0        0     |
    (1+v)*(1-2v)         |  v    v   1-v     0        0        0     |
                         |  0    0    0   (1-2v)/2    0        0     |
                         |  0    0    0      0     (1-2v)/2    0     |
                         |  0    0    0      0        0     (1-2v)/2 |
```

**Elementy powierzchniowe** używają dwuwymiarowej postaci **płaskiego stanu
odkształcenia** z wektorem naprężeń `[sigma_xx, sigma_yy, tau_xy]`, obliczanej w
lokalnej płaszczyźnie elementu i obracanej z powrotem do współrzędnych
globalnych:

```
        E                | 1-v   v      0     |
D = -----------------  * |  v   1-v     0     |
    (1+v)*(1-2v)         |  0    0   (1-2v)/2 |
```

Jest to płaski stan **odkształcenia**, a nie płaski stan naprężenia. Cienka
płyta obciążona w swojej płaszczyźnie wyjdzie więc sztywniejsza, niż
przewidywałaby idealizacja płaskiego stanu naprężenia.

**Elementy liniowe** są traktowane jako **pręty kratownicowe**: składana jest
tylko sztywność osiowa `E * A` wzdłuż kierunku elementu, gdzie `A` jest polem
przekroju linii. Nie ma sztywności na zginanie, ścinanie ani skręcanie, nawet dla
typów elementów nazwanych „beam” (belka).

### 1.4 Dyskretyzacja metodą elementów skończonych

Postać słaba równowagi daje dla każdego elementu

```
K = INT( B^T * D * B )        stiffness matrix
M = INT( rho * N^T * N )      consistent mass matrix
f = load vector, see 1.5
```

(`stiffness matrix` = macierz sztywności, `consistent mass matrix` = spójna
macierz mas, `load vector, see 1.5` = wektor obciążeń, zob. 1.5.) Tu `B` jest
macierzą odkształcenie-przemieszczenie zbudowaną z pochodnych funkcji kształtu.
Każda macierz elementu ma wymiar `3n x 3n` dla elementu o `n` węzłach, ze
stopniami swobody przeplatanymi jako `[ux1, uy1, uz1, ux2, uy2, uz2, ...]`.

**Miary elementów.** Jak wszędzie w Range FEA, encjom niższego wymiaru nadaje
się miarę geometryczną, aby mogły reprezentować rzeczywiste ciała:

| Encja | Miara | Gdzie jest ustawiana |
|---|---|---|
| Objętość | 1 | - |
| Powierzchnia | grubość powierzchni | właściwości geometryczne encji |
| Linia | pole przekroju linii | właściwości geometryczne encji |
| Punkt | objętość punktu | właściwości geometryczne encji |

Powierzchnia o zerowej grubości lub linia o zerowym polu przekroju nie wnosi
sztywności ani masy - nadal może jednak przenosić obciążenia, a dokładnie tego
oczekuje się od obciążonej ściany siatkowanej bryły.

Macierz mas jest składana tylko wtedy, gdy jest potrzebna, czyli w dynamicznej
analizie przejściowej lub w analizie modalnej.

### 1.5 Obciążenia, warunki brzegowe i warunki otoczenia

#### Warunki brzegowe

| Warunek brzegowy | Typ | Dotyczy | Składowe |
|---|---|---|---|
| Displacement | jawny (Dirichlet) | punkt, linia, powierzchnia, objętość | Displacement in X, Y, Z `[m]` |
| Normal displacement | jawny, układ lokalny | powierzchnia | Displacement `[m]` |
| Roller displacement | jawny, układ lokalny | punkt, linia, powierzchnia | Displacement `[m]` |
| Force | naturalny | punkt, linia, powierzchnia | Force in X, Y, Z `[N]` |
| Force (unit area) | naturalny | powierzchnia | Force per unit area in X, Y, Z `[N/m^2]` |
| Pressure | naturalny | powierzchnia | Pressure `[Pa]` |
| Weight | naturalny | punkt, linia, powierzchnia | Weight `[kg]` |

**Displacement** (przemieszczenie) jest warunkiem o składowych **opcjonalnych**:
każdą z trzech składowych można włączyć lub wyłączyć osobno. Włączona składowa
wiąże ten kierunek globalny każdego węzła encji z podaną wartością; wyłączona
składowa pozostawia ten kierunek całkowicie swobodny. Wyłączenie *Displacement in
Y* i *Displacement in Z* tworzy więc podporę, która utrzymuje X i pozwala węzłowi
przesuwać się w płaszczyźnie YZ.

Użyj zamiast tego *Roller displacement*, gdy kierunki swobodne nie są kierunkami
globalnymi, lecz płaszczyzną styczną zakrzywionej powierzchni.

**Normal displacement** (przemieszczenie normalne) i **Roller displacement**
(przemieszczenie przesuwne) działają w **układzie lokalnym** budowanym dla
każdego związanego węzła. Domyślnie układ podąża za geometrią:

- na **powierzchni** lokalną osią X jest uśredniona normalna sąsiednich elementów
  powierzchni;
- na **linii** układ lokalny jest budowany z kierunku elementu;
- na **punkcie** nie ma geometrii, za którą można by podążać, więc zawsze używany
  jest kierunek wprowadzony wraz z warunkiem brzegowym.

Zaznaczenie **Użyj wprowadzonego kierunku lokalnego** zmienia to dla powierzchni
lub linii i pobiera lokalną oś X z kierunku wprowadzonego poniżej. Używaj tej
opcji, gdy kierunek, który chcesz zablokować, nie jest tym, który akurat daje
siatka - na przykład dla podpory przesuwnej, która ma się przesuwać po
fasetowanej powierzchni wzdłuż osi globalnej.

Więzy z różnych encji **łączą się** we wspólnym węźle. Solver zbiera wszystkie
więzy przemieszczeń działające na węzeł, z których każdy jest stwierdzeniem
`d . u = v` o jednym kierunku, i redukuje ten zbiór do co najwyżej trzech
wzajemnie prostopadłych kierunków utrzymywanych metodą Grama-Schmidta,
przeprowadzając zadane wartości przez te same operacje. Te kierunki utrzymywane
stają się wiodącymi osiami układu węzła; to, co pozostaje, uzupełnia układ i
pozostaje swobodne. Węzeł może więc być utrzymywany w jednym kierunku przez
globalne *Displacement*, w drugim przez *Roller displacement* na pochylonej
ścianie, a w trzecim pozostawać swobodny - oba warunki nie nadpisują się
wzajemnie, a składowa sformułowana globalnie nigdy nie jest po cichu
odczytywana w czyimś układzie lokalnym.

Dwie encje, które zadają **różne wartości w tym samym kierunku**, nie mogą być
spełnione jednocześnie. Solver wykrywa to podczas ortogonalizacji i zatrzymuje
się z błędem podającym węzeł, zamiast rozstrzygać to na korzyść warunku, który
akurat został odczytany jako ostatni.

Macierze elementów i wektory obciążeń związanego węzła są obracane do tego
układu, stosowany jest więz, a wynikowe przemieszczenie jest obracane z
powrotem. W tym układzie:

- **Normal displacement** blokuje wszystkie trzy składowe lokalne, a zadany
  skalar jest stosowany wzdłuż lokalnej normalnej. Węzeł przemieszcza się
  dokładnie o podaną wartość wzdłuż normalnej, a poza tym jest unieruchomiony.
- **Roller displacement** na powierzchni lub punkcie blokuje tylko lokalną
  składową normalną, więc węzeł może swobodnie przesuwać się w płaszczyźnie
  stycznej. Na encji linii blokuje zamiast tego dwie składowe prostopadłe do
  linii, więc węzeł może swobodnie przesuwać się wzdłuż linii.

**Force** (siła) jest siłą całkowitą w niutonach rozkładaną na encję, do której
ją przypisano - dzieloną przez długość encji dla linii, przez pole encji dla
powierzchni i przez liczbę punktów encji punktowej. Zagęszczanie siatki nie
zmienia więc obciążenia całkowitego.

**Force (unit area)** (siła na jednostkę pola) jest obciążeniem powierzchniowym w
`N/m^2` przykładanym w podanym kierunku globalnym i całkowanym po polu elementu.

**Pressure** (ciśnienie) działa wzdłuż **normalnej** do powierzchni. Solver
ustala dla każdej powierzchni brzegowej, czy jej normalna wskazuje do wnętrza
sąsiedniego obliczanego elementu objętościowego, i odpowiednio odwraca znak, tak
aby dodatnie ciśnienie zawsze naciskało na ciało. Powierzchnia, która nie
ogranicza żadnego elementu objętościowego, nigdy nie jest klasyfikowana jako
skierowana do wnętrza, więc tam obciążenie działa wzdłuż **przeciwnej** normalnej
do powierzchni - w takim przypadku sprawdź kierunek wynikowego obciążenia.

**Weight** (ciężar) jest masą w kilogramach dołączoną do encji. Jest mnożona przez
przyspieszenie ziemskie z warunku otoczenia i rozkładana na encję w taki sam
sposób jak Force. Używaj go dla niemodelowanych dołączonych mas.

#### Warunki otoczenia

| Warunek otoczenia | Składowe | Działanie |
|---|---|---|
| Gravitational acceleration | G acceleration in X, Y, Z `[m/s^2]` | ciężar własny i warunek brzegowy Weight |
| Temperature | Temperature `[K]` | temperatura odniesienia stanu beznaprężeniowego |

**Ciężar własny** jest generowany automatycznie dla każdej encji, która ma zarówno
gęstość, jak i miarę geometryczną, jako `rho * g` całkowane po elemencie. Pojawia
się, gdy tylko zostanie przypisany warunek otoczenia przyspieszenia ziemskiego;
wartość domyślna wynosi `-9.80665 m/s^2` w Z.

**Obciążenie cieplne.** Różnica temperatur wymuszająca człon rozszerzalności
cieplnej wynosi

```
dT = element temperature - environment temperature
```

(`element temperature` = temperatura elementu, `environment temperature` =
temperatura otoczenia.) **Temperatura otoczenia** pochodzi z warunku otoczenia
*Temperature* i jest stanem odniesienia bez naprężeń. **Temperatura elementu**
pochodzi z solvera wymiany ciepła uruchomionego wcześniej w tym samym przebiegu
zadań problemów i jest przekazywana przez współdzielone dane solvera.
Uruchomienie *Heat transfer*, a następnie *Stress analysis* w jednym przebiegu
zadań daje więc analizę naprężeń cieplnych bez dalszej konfiguracji. Bez solvera
cieplnego temperatura elementu pozostaje domyślna, a człon cieplny jest różnicą
względem samego warunku otoczenia.

#### Warunki początkowe

| Warunek początkowy | Działanie |
|---|---|
| Displacement | początkowe pole przemieszczeń dynamicznej analizy przejściowej |
| Structural velocity | początkowe pole prędkości dynamicznej analizy przejściowej |
| Pressure | początkowe obciążenie ciśnieniem powierzchniowym |

Warunki początkowe są odczytywane tylko przy pierwszym uruchomieniu analizy
przejściowej. Analiza dynamiczna bez nich rozpoczyna się od spoczynku. Kierunki
związane nie zachowują własnej prędkości początkowej, lecz podążają za zadanym
przemieszczeniem.

#### Sprzężenie z solverem płynu

Solver odtwarza wynik **Pressure** z poprzedniego rozwiązania i używa go jako
obciążenia powierzchniowego wszędzie tam, gdzie nie przypisano jawnego warunku
brzegowego ciśnienia. Zadanie przepływu, po którym w tym samym przebiegu zadań
następuje zadanie naprężeń, automatycznie przenosi więc ciśnienie płynu na
konstrukcję.

W przeciwnym kierunku każdy solver niekonstrukcyjny przed uruchomieniem
przesuwa siatkę o zapisany wynik **Displacement**, a potem przesuwa ją z
powrotem, więc rozwiązanie przepływowe lub cieplne następujące po rozwiązaniu
naprężeń widzi kształt odkształcony.

### 1.6 Analiza statyczna

Przy wyłączonym solverze czasowym i typie problemu ustawionym na *Stress
analysis* solver składa i rozwiązuje

```
K * u = f
```

`K` jest symetryczna i dodatnio określona, gdy więzy przemieszczeń usunęły
postacie ruchu ciała sztywnego, więc rozwiązuje się ją metodą **gradientów
sprzężonych** z prekondycjonowaniem Jacobiego i rozmiarem bloku 3.

Jeśli model jest niedostatecznie podparty, macierz jest osobliwa, a rozwiązanie
nie będzie zbieżne. Każdy model statyczny potrzebuje więzów wystarczających do
usunięcia wszystkich sześciu postaci ruchu ciała sztywnego - trzech przesunięć i
trzech obrotów.

### 1.7 Analiza przejściowa

Przy włączonym solverze czasowym *Ustawienia przejściowej analizy naprężeń*
wybierają jeden z dwóch typów analizy.

#### Analiza dynamiczna

Ustawienie domyślne. Solver całkuje równania liniowej elastodynamiki

```
M * d2u/dt2 + C * du/dt + K * u = f(t)
```

dzięki czemu odtwarzane są bezwładność, fale naprężeń, drgania, dynamiczne
wzmocnienie i rezonans.

**Tłumienie.** `C` jest tłumieniem Rayleigha, budowanym element po elemencie z
dwóch opcjonalnych właściwości materiału,

```
C = alpha_M * M + beta_K * K
```

*Rayleigh mass damping coefficient* (współczynnik tłumienia masowego Rayleigha)
`alpha_M` `[1/s]` tłumi niskie częstotliwości, a *Rayleigh stiffness damping
coefficient* (współczynnik tłumienia sztywnościowego Rayleigha) `beta_K` `[s]`
wysokie. Stopień tłumienia postaci o częstości kołowej `omega` wynosi

```
zeta(omega) = alpha_M / (2*omega) + beta_K * omega / 2
```

więc docelowy stopień tłumienia `zeta` przy dwóch częstościach `omega_1` i
`omega_2` osiąga się przy

```
alpha_M = 2*zeta * omega_1*omega_2 / (omega_1 + omega_2)
beta_K  = 2*zeta / (omega_1 + omega_2)
```

Weź obie częstości z analizy modalnej, wokół interesującego pasma. Typowe stopnie
tłumienia konstrukcji wynoszą od 0.01 do 0.05. Bez tych dwóch właściwości nie ma
żadnego tłumienia fizycznego.

**Całkowanie w czasie.** Metoda Newmarka z modyfikacją Hilbera-Hughesa-Taylora
(HHT). Przy przemieszczeniu `u`, prędkości `v` i przyspieszeniu `a` na początku
kroku każdy krok rozwiązuje

```
( a0*M + (1+alpha)*(K + a1*C) ) * u_n+1 =
      (1+alpha)*f_n+1 - alpha*f_n
    + M * ( a0*u_n + a2*v_n + a3*a_n )
    + (1+alpha) * C * ( a1*u_n + a4*v_n + a5*a_n )
    + alpha * ( C*v_n + K*u_n )
```

```
a0 = 1/(beta*dt^2)   a1 = gamma/(beta*dt)   a2 = 1/(beta*dt)
a3 = 1/(2*beta) - 1  a4 = gamma/beta - 1    a5 = dt/2 * (gamma/beta - 2)
```

a następnie aktualizuje

```
a_n+1 = a0*(u_n+1 - u_n) - a2*v_n - a3*a_n
v_n+1 = v_n + dt*( (1-gamma)*a_n + gamma*a_n+1 )
```

Macierz efektywna jest symetryczna i dodatnio określona, więc rozwiązuje się ją
tą samą metodą CG co analizę statyczną.

- Przy **HHT alpha = 0** (domyślnie) schemat jest zwykłym schematem Newmarka z
  `gamma = 1/2` i `beta = max(theta/2, 1/4)`, gdzie `theta` jest
  współczynnikiem przybliżenia schematu czasowego (`1` wsteczny, `0.5`
  centralny, `0` w przód). Wariant centralny i w przód dają schemat średniego
  przyspieszenia - bezwarunkowo stabilny, o dokładności drugiego rzędu i bez
  tłumienia numerycznego. Wariant wsteczny daje `beta = 1/2`, co również jest
  bezwarunkowo stabilne, ale wydłuża okresy.
- Przy **HHT alpha** między `-1/3` a `0` `gamma = 1/2 - alpha` oraz
  `beta = (1 - alpha)^2 / 4`. Schemat pozostaje bezwarunkowo stabilny i
  dokładny w drugim rzędzie, a tłumi wysokie częstotliwości, które zgrubna
  siatka rozdziela źle, pozostawiając niskie postacie niemal nietknięte. `-0.05`
  do `-0.1` to częsty wybór, gdy odpowiedź jest zanieczyszczona szumem wysokiej
  częstotliwości.

**Przyspieszenie początkowe.** W pierwszym kroku przyspieszenie jest
rozwiązywane z równowagi, `M*a = f - K*u - C*v`, więc obciążenie przyłożone na
początku daje poprawne przyspieszenie początkowe. Kierunki bez masy otrzymują
zero.

**Skupiona macierz mas.** Opcja *Skupiona macierz mas* zastępuje spójną macierz
mas macierzą diagonalną o tej samej masie całkowitej, skalując przekątną
macierzy spójnej (skupianie HRZ, które pozostaje dodatnie również dla elementów
kwadratowych). Nieznacznie obniża częstotliwości własne i daje gładszą
odpowiedź za ostrymi czołami fal.

**Wybór kroku czasowego.** Schemat jest stabilny dla dowolnego `dt`, ale
dokładność wymaga mniej więcej 20 lub więcej kroków na okres najwyższej postaci,
którą chcesz rozdzielić. Dla fali biegnącej przez siatkę dobrym początkiem jest
krok bliski czasowi, którego fala potrzebuje na przejście przez jeden element
(`dt ~ h/c`, `c = sqrt(E/rho)`).

**Wyniki.** Oprócz przemieszczeń i naprężeń analiza dynamiczna zapisuje węzłowe
wektory *Structural velocity* (prędkość konstrukcji) i *Structural acceleration*
(przyspieszenie konstrukcji). Przenoszą one także stan do następnego kroku
czasowego i przez ponowne uruchomienie solvera.

#### Analiza quasi-statyczna

Każdy krok czasowy jest niezależnym rozwiązaniem statycznym

```
K * u(t) = f(t)
```

z pominięciem bezwładności i tłumienia. Używaj jej dla historii obciążenia
przykładanej powoli w porównaniu z okresami konstrukcji, gdy znaczenie ma tylko
sekwencja stanów równowagi. Nie wymaga gęstości ani ograniczenia kroku
czasowego.

### 1.8 Analiza modalna

Przy typie problemu ustawionym na *Modal analysis* solver składa `K` i `M` i
rozwiązuje uogólniony problem własny

```
K * phi = lambda * M * phi        with  lambda = omega^2
```

(`with` = przy.) Każda para własna jest **postacią**: `phi` jest postacią drgań,
zapisywaną i wyświetlaną jako pole przemieszczeń, a `lambda` jest wartością
własną, z której częstotliwość własna wynika jako

```
omega = sqrt(lambda)      [rad/s]
f     = sqrt(lambda) / (2*pi)      [Hz]
```

Solver sam wykonuje to przeliczenie: ustawienia modalne i widok 3D podają
częstotliwość własną w Hz, a dziennik solvera wypisuje obok niej odpowiadającą
wartość własną. Postać, której iteracja wartości własnych nie zdołała rozwiązać,
jest podawana jako `0` Hz z ostrzeżeniem w dzienniku.

Dostępne są dwie metody, wybierane ustawieniem metody modalnej:

| Metoda modalna | Solver wartości własnych | Wyodrębnia |
|---|---|---|
| Multiple modes | iteracja podprzestrzenna z rzutem Rayleigha-Ritza | żądaną liczbę postaci |
| Only most dominant mode | odwrotna iteracja potęgowa z ilorazem Rayleigha | jedną postać |

Obie metody iterują na `K^-1 * M`, więc zbiegają do **najniższych**
częstotliwości - czyli tych, które mają znaczenie. Każdy krok jest rozwiązaniem
liniowym z `K` przy użyciu ustawień gradientów sprzężonych z konfiguracji solvera
macierzowego, więc koszt analizy modalnej to mniej więcej koszt rozwiązania
statycznego pomnożony przez liczbę wektorów podprzestrzeni i iteracji.

Iteracja podprzestrzenna przenosi blok wektorów, rzutuje `K` i `M` na rozpinaną
przez nie podprzestrzeń i dokładnie rozwiązuje powstały mały, gęsty problem
własny. Oprócz żądanej liczby postaci przenoszonych jest kilka dodatkowych
wektorów, ponieważ wyraźnie przyspieszają zbieżność poszukiwanych postaci. Dla
`p` żądanych postaci blok zawiera `min(2p, p+8)` wektorów, co jest zwykłym
kompromisem między szybkością zbieżności a kosztem iteracji. Wartości własne
wychodzą bezpośrednio jako `lambda` i są sortowane rosnąco, więc **postać 0 jest
postacią podstawową**.

**Właśnie tu idzie czas obliczeń.** Jedna iteracja podprzestrzenna wykonuje jedno
pełne rozwiązanie liniowe na każdy wektor bloku, więc żądanie 10 postaci kosztuje
18 rozwiązań na iterację, a żądanie 100 - 108. Iteracja kończy się wcześniej
tylko wtedy, gdy **każda** żądana wartość własna ustabilizuje się w granicach
wartości zbieżności, a najwyższa żądana postać stabilizuje się zawsze jako
ostatnia - więc duża liczba postaci zarazem czyni każdą iterację kosztowną i
sprawia, że wcześniejsze zakończenie jest mało prawdopodobne. Żądaj tych postaci,
które faktycznie zamierzasz oglądać.

Jeśli żądane postacie nie ustabilizowały się do ostatniej iteracji, solver
informuje o tym w dzienniku i podaje osiągniętą zmianę, zamiast przedstawiać
niezbieżne wartości bez komentarza. Najniższe postacie takiego obliczenia są
zwykle nadal wiarygodne; najwyższe - nie.

Macierz mas jest w analizie modalnej zawsze składana, więc gęstość musi być
obecna na każdej uczestniczącej encji.

**Analiza modalna ze wstępnym naprężeniem.** Przed złożeniem macierzy analiza
modalna przesuwa każdy węzeł o wynik przemieszczenia z poprzedniego rozwiązania,
a potem przywraca siatkę. Uruchomienie *Stress analysis*, a następnie *Modal
analysis* w jednym przebiegu zadań daje więc postacie **obciążonej,
odkształconej** konstrukcji, a nie nieodkształconej. Zapisana postać drgań jest
wektorem własnym dodanym do tego przemieszczenia początkowego.

Aktualizowana jest tylko geometria - macierz sztywności jest nadal liniowo
sprężysta, obliczana na przesuniętej siatce. Ujmuje to zmianę kształtu, a nie
geometryczne usztywnienie rozciąganego elementu.

Analiza modalna jest **wyłączna**: nie można jej łączyć z żadną inną fizyką poza
poprzedzającą ją analizą naprężeń.

### 1.9 Wyniki pochodne

Po rozwiązaniu naprężenia w elementach są wyznaczane z pola przemieszczeń jako
średnia po punktach całkowania z

```
sigma = D * ( B * u  -  eps_th )
```

z odkształceniem cieplnym `eps_th = alpha * dT * [1, 1, 1, 0, 0, 0]` dla
elementów objętościowych. Elementy powierzchniowe są w płaskim stanie
odkształcenia: rozszerzalność poza ich płaszczyzną jest stłumiona, co zwiększa
odkształcenie cieplne w płaszczyźnie do `eps_th = (1+v) * alpha * dT * [1, 1, 0]`.
To samo odkształcenie cieplne wchodzi do obciążenia cieplnego
`f_th = INT( B^T * D * eps_th )`. Elementy liniowe podają
`E * (eps - alpha * dT)`.

Z sześciu składowych solver tworzy dwa niezmienniki i łączy je w kwadraturze w
naprężenie von Misesa:

```
normal part  QN = sqrt( sx^2 + sy^2 + sz^2 - (sx*sy + sy*sz + sz*sx) )
shear part   QS = sqrt( 3 * (tyz^2 + txz^2 + txy^2) )
von Mises    QVM = sqrt( QN^2 + QS^2 )
```

(`normal part` = część normalna, `shear part` = część styczna.) Jest to
klasyczna definicja

```
QVM = sqrt( sx^2 + sy^2 + sz^2 - (sx*sy + sy*sz + sz*sx) + 3*(tyz^2 + txz^2 + txy^2) )
```

Dla elementów powierzchniowych używane są odpowiedniki dwuwymiarowe: `QN` z dwóch
naprężeń normalnych w płaszczyźnie i `QS = sqrt(3 * tau_xy^2)`. Dla elementów
liniowych podawana jest tylko wartość osiowa, a część styczna wynosi zero.

**Siły węzłowe** są wyznaczane element po elemencie jako wewnętrzna siła
sprężysta

```
f = K * u - f_th
```

i sumowane w węzłach. W związanym węźle jest to siła reakcji; w nieobciążonym
węźle swobodnym powinna dla zbieżnego rozwiązania statycznego wynosić blisko
zera, co czyni wynik Force przydatnym sprawdzeniem residuum. Ciało, które może
swobodnie rozszerzać się cieplnie, nie wykazuje więc ani sił, ani naprężeń.

| Wynik | Dotyczy | Znaczenie |
|---|---|---|
| Displacement `[m]` | węzeł | wektor przemieszczenia lub postać drgań w analizie modalnej |
| Von Mises stress `[Pa]` | element | `sqrt(QN^2 + QS^2)` jak wyżej |
| Normal stress `[Pa]` | element | `QN` |
| Shear stress `[Pa]` | element | `QS` |
| Normal stress in X, Y, Z `[Pa]` | element | `sigma_xx`, `sigma_yy`, `sigma_zz` |
| Shear stress in YZ, XZ, XY `[Pa]` | element | `tau_yz`, `tau_xz`, `tau_xy` |
| Force `[N]` | węzeł | wyznaczona siła węzłowa, reakcja w związanych węzłach |

Sześć składowych naprężenia jest podawanych we współrzędnych **globalnych** dla
elementów objętościowych. Elementy powierzchniowe i liniowe obliczają naprężenie
we własnym lokalnym układzie elementu, więc ich składowe są podawane w tym
układzie: powierzchnia wypełnia `sigma_xx`, `sigma_yy` i `tau_xy` w płaszczyźnie,
a linia tylko osiowe `sigma_xx`.

### 1.10 Statyczna, przejściowa czy modalna?

| | Statyczna | Dynamiczna przejściowa | Quasi-statyczna przejściowa | Modalna |
|---|---|---|---|---|
| Rozwiązuje | `K*u = f` | `M*u'' + C*u' + K*u = f(t)` w czasie | `K*u = f(t)` w każdej chwili | `K*phi = omega^2*M*phi` |
| Odpowiada na | „jak bardzo się ugina i jak duże są naprężenia” | „jak reaguje na szybko zmieniające się obciążenie” | „jak podąża za powoli zmieniającym się obciążeniem” | „przy jakich częstotliwościach chce drgać i w jakiej postaci” |
| Wymaga masy | nie | tak | nie | tak |
| Wymaga więzów przeciw ruchowi ciała sztywnego | tak | tak | tak | tak |
| Rekordy | jeden | jeden na krok czasowy | jeden na krok czasowy | jeden na wyodrębnioną postać |

**Używaj analizy statycznej do** sprawdzania wytrzymałości i sztywności,
dopuszczalnych ugięć, sił reakcji, ciężaru własnego, zbiorników ciśnieniowych,
naprężeń cieplnych. Jest to ustawienie domyślne i obejmuje większość prac.

**Używaj dynamicznej analizy przejściowej do** uderzeń, nagle przyłożonych lub
zwolnionych obciążeń, historii wstrząsów i drgań, propagacji fal oraz odpowiedzi
na wymuszenie harmoniczne w pobliżu rezonansu - wszędzie tam, gdzie znaczenie ma
bezwładność.

**Używaj quasi-statycznej analizy przejściowej do** obciążenia zmieniającego się
w czasie na tyle powoli, że konstrukcja podąża za nim w stanie równowagi.

**Używaj analizy modalnej do** częstotliwości i postaci drgań własnych, unikania
rezonansu ze znanym wymuszeniem, oceny, czy konstrukcja jest dynamicznie
wystarczająco sztywna, oraz wykrywania niezamierzonych mechanizmów - wartość
własna bliska zeru zwykle oznacza brakujący więz lub niepołączoną część.

---

## 2. Interfejs graficzny

Wszystko, co istotne, znajduje się w panelu **Solver** (karty `Problem`,
`Warunki brzegowe`, `Warunki początkowe`, `Warunki otoczenia`, `Materiał`,
`Wyniki`), menu **Problem** i panelu **Model**.

### 2.1 Wybór problemu

`Problem` -> `Przebieg zadań problemów` (`Ctrl+P`) otwiera okno przebiegu zadań
problemu. Dodaj zadanie i wybierz jedną z opcji:

- **Stress analysis** - „Stress and linear elasto-dynamics analysis including
  modal analysis” (analiza naprężeń i liniowej elastodynamiki, w tym analiza
  modalna);
- **Modal analysis** - „Stress modal analysis” (modalna analiza naprężeń).

Stress analysis można swobodnie łączyć z innymi fizykami. Dwa przydatne łańcuchy
to *Heat transfer*, a następnie *Stress analysis* dla naprężeń cieplnych, oraz
*Incompressible viscous flow*, a następnie *Stress analysis* dla obciążenia
ciśnieniem.

Modal analysis wyklucza każdy inny typ problemu z wyjątkiem poprzedzającej ją
analizy naprężeń, która zamienia ją w analizę modalną ze wstępnym naprężeniem.

### 2.2 Karta Problem

**Solver czasowy** - wyświetlany dla analizy naprężeń, ponieważ jest to typ
problemu zależny od czasu. W analizie statycznej pozostaw go wyłączonym.

| Pole | Znaczenie |
|---|---|
| Włącz | włącza i wyłącza przejściowe całkowanie w czasie |
| Przybliżenie | ustawia beta Newmarka analizy dynamicznej, zob. 1.7; zalecany jest wariant centralny |
| Czas początkowy | pierwsza wartość czasu w sekundach; ignorowana przy ponownym uruchomieniu solvera |
| Czas końcowy | `start time + time-step size * number of time-steps` (czas początkowy + rozmiar kroku czasowego × liczba kroków czasowych), tylko do odczytu |
| Rozmiar kroku czasowego | `dt` w sekundach |
| Liczba kroków czasowych | liczba kroków do obliczenia |
| Częstotliwość zapisu | zapisuje plik wyników co N kroków; `0` zapisuje tylko ostatni krok |

**Ustawienia przejściowej analizy naprężeń** - wyświetlane dla analizy naprężeń
i aktywne tylko przy włączonym solverze czasowym.

| Pole | Znaczenie | Domyślnie |
|---|---|---|
| Typ analizy | *Dynamic* (z bezwładnością) lub *Quasi-static* (rozwiązanie statyczne na każdy krok czasowy) | Dynamic |
| Tłumienie numeryczne (alfa HHT) | od `-1/3` do `0`; wartości ujemne tłumią szum wysokiej częstotliwości | 0 |
| Skupiona macierz mas | używa diagonalnej macierzy mas zamiast spójnej | wyłączone |

Ostatnie dwa pola dotyczą tylko analizy dynamicznej, a w innych przypadkach są
wyszarzone.

**Ustawienia analizy modalnej** - wyświetlane, gdy typem problemu jest *Modal
analysis*.

| Pole | Znaczenie | Domyślnie |
|---|---|---|
| Metoda modalna | *Only most dominant mode* lub *Multiple modes* | Multiple modes |
| Iteracje | iteracje solvera wartości własnych; wyłączone dla metody postaci dominującej | 100 |
| Wyodrębnij postacie | liczba postaci do wyodrębnienia; wyłączone dla metody postaci dominującej | 10 |
| Wartość zbieżności | próg zbieżności solvera wartości własnych | 1.0e-9 |

(*Only most dominant mode* = tylko postać najbardziej dominująca, *Multiple
modes* = wiele postaci.) Obie liczby są niezależne: iteracje udoskonalają cały
zbiór postaci jednocześnie, a nie wytwarzają ich po jednej.

**To pole Wyodrębnij postacie decyduje o czasie obliczeń.** Każda postać dodaje
jedno rozwiązanie liniowe do każdej iteracji, a obliczenia mogą zakończyć się
wcześniej dopiero wtedy, gdy wszystkie żądane postacie się ustabilizują, więc
koszt rośnie szybciej niż liczba postaci. Dziesięć to rozsądny punkt wyjścia i
wartość domyślna; sto to obliczenia innego rzędu, które na dużej siatce mogą
trwać godzinami. Zwiększaj tę wartość tylko wtedy, gdy wiesz, że potrzebujesz
wyższych postaci.

Jeśli obliczenia trwają dłużej, niż się spodziewasz, dziennik solvera podaje
wektor podprzestrzeni, nad którym trwa praca, więc postęp w obrębie iteracji jest
widoczny, a po każdej iteracji wypisuje osiągnięty współczynnik zbieżności obok
wymaganej wartości.

### 2.3 Karta Materiał

Karta `Materiał` zawiera materiał przypisany do wybranej encji. Analiza naprężeń
wymaga **wszystkich czterech** poniższych właściwości:

| Właściwość | Jednostki | Komentarz |
|---|---|---|
| Modulus of elasticity | `Pa` | `E`, około `2.1e11` dla stali |
| Poisson ratio | `-` | `v`, około `0.3` dla stali; musi pozostać poniżej `0.5` |
| Density | `kg/m^3` | potrzebna dla ciężaru własnego, analizy dynamicznej i modalnej |
| Thermal expansion coefficient | `1/K` | ustaw `0`, jeśli nie ma obciążenia cieplnego |

Encja, której brakuje którejkolwiek z nich, nie jest rozwiązywana, a kontrola
konfiguracji ostrzega o tym przed rozpoczęciem obliczeń.

Dwie dalsze właściwości są opcjonalne i są odczytywane tylko przez dynamiczną
analizę przejściową:

| Właściwość | Jednostki | Komentarz |
|---|---|---|
| Rayleigh mass damping coefficient | `1/s` | `alpha_M`, zob. 1.7; zero, gdy jej brak |
| Rayleigh stiffness damping coefficient | `s` | `beta_K`, zob. 1.7; zero, gdy jej brak | Każda właściwość jest tablicą w funkcji temperatury, więc
moduł zależny od temperatury jest uwzględniany, gdy najpierw uruchamiany jest solver cieplny.

### 2.4 Karta Warunki brzegowe

Karta jest podzielona na listę menedżera dostępnych warunków brzegowych u góry i
edytor wybranego warunku u dołu. Najpierw wybierz encję w drzewie `Model`;
menedżer zaoferuje wtedy warunki właściwe dla danego typu encji i wybranego
problemu. Zaznacz warunek, aby go przypisać, a następnie edytuj jego składowe w
dolnym drzewie.

Oferowane są warunki z punktu 1.5. Uwagi dotyczące ich użycia:

- **Displacement** pokazuje pole wyboru obok każdej z trzech składowych. Wiązane
  są tylko zaznaczone składowe, więc ścianę można utrzymywać w X i pozostawić
  swobodną w Y i Z. Pole wartości niezaznaczonej składowej jest wyszarzone.
- **Normal displacement** i **Roller displacement** domyślnie pobierają swój
  układ lokalny z geometrii - z uśrednionych normalnych elementów powierzchni, z
  kierunku elementu linii. Na encji **punktu** nie ma geometrii, za którą można
  by podążać, więc edytor *Kierunek lokalny* jest zawsze wyświetlany i zawsze
  używany.
- Na powierzchni lub linii drzewo warunku pokazuje pole wyboru **Użyj
  wprowadzonego kierunku lokalnego**. Zaznacz je, aby wyświetlić edytor
  *Kierunek lokalny* i pobrać układ z wprowadzonego kierunku zamiast z siatki.
  Strzałki kierunku rysowane w widoku 3D podążają za tym z dwóch wariantów, który
  obowiązuje. Jeśli więz powierzchni zachowuje się dziwnie przy niezaznaczonym
  polu, sprawdź, czy normalne powierzchni są spójne, i zsynchronizuj je
  narzędziami geometrii.
- Więzy z kilku encji spotykających się w jednym węźle są łączone, więc ściana
  może być jednocześnie utrzymywana w kierunku globalnym przez *Displacement* i
  przesuwać się po pochylonej płaszczyźnie. Jeśli dwa z nich żądają różnych
  wartości w tym samym kierunku, obliczenia zatrzymują się z błędem podającym
  węzeł.
- **Pressure** i **Force (unit area)** dotyczą tylko powierzchni.
- Każda składowa jest tablicą w funkcji czasu, więc obciążeniu można nadać
  przebieg czasowy dla obliczeń przejściowych.

### 2.5 Warunki początkowe i otoczenia

Na karcie `Warunki otoczenia` włącza się **grawitację**. Przypisz
*Gravitational acceleration* (przyspieszenie ziemskie) do encji, która ma
przenosić swój ciężar własny - zwykle do całego modelu - i ustaw wektor; wartość
domyślna wynosi `-9.80665` w Z. Przypisz *Temperature*, aby ustawić temperaturę
odniesienia stanu beznaprężeniowego dla analizy naprężeń cieplnych.

Karta `Warunki początkowe` oferuje *Displacement*, *Structural velocity* i
*Pressure*, stosowane przy pierwszym uruchomieniu analizy przejściowej.

### 2.6 Konfiguracja solvera macierzowego

`Problem` -> `Konfiguruj solver macierzowy problemów` konfiguruje solvery
iteracyjne. Solver konstrukcyjny używa pozycji **CG** zarówno dla rozwiązania
statycznego i przejściowego, jak i dla wewnętrznych rozwiązań iteracji wartości
własnych.

Jeśli rozwiązanie statyczne nie jest zbieżne, przyczyną jest zwykle
niedostatecznie podparty model, a nie ustawienie solvera. Zanim zwiększysz
liczbę iteracji, sprawdź więzy.

### 2.7 Punkty monitorowania

`Problem` -> `Zdefiniuj punkty monitorowania` umieszcza sondy w podanych
współrzędnych i wybiera rejestrowaną zmienną - zwykle *Displacement* lub *Von
Mises stress*. W obliczeniach przejściowych przebieg pokazuje `Raport` ->
`Punkty monitorowania`.

### 2.8 Wyniki i rekordy

Karta `Wyniki` zawiera listę obliczonych zmiennych i steruje widokiem 3D.
Przemieszczenie jest wektorem węzłowym; trzy miary naprężenia są skalarami
elementowymi. Dynamiczna analiza przejściowa dodaje węzłowe wektory *Structural
velocity* i *Structural acceleration*.

Aby narysować kształt odkształcony, wyświetl *Displacement* i włącz skalowanie
odkształceń w ustawieniach widoku - rzeczywiste przemieszczenia są zwykle o wiele
za małe, by były widoczne w prawdziwej skali.

Karta `Rekordy` panelu `Model` zawiera listę rekordów wyników:

- **analiza statyczna** - jeden rekord;
- **analiza przejściowa** - jeden rekord na każdy zapisany krok czasowy;
- **analiza modalna** - jeden rekord na każdą wyodrębnioną postać, numerowany od
  postaci podstawowej. Przechodzenie przez rekordy oznacza przechodzenie przez
  postacie, a ich animacja animuje postacie drgań.

Dla rekordu modalnego widok 3D opisuje rekord numerem postaci i częstotliwością
własną tej postaci w Hz.

`Raport` -> `Plik dziennika solvera` wyświetla pełne wyjście solvera, w tym
wartość własną wypisywaną dla każdej wyodrębnionej postaci oraz statystyki
przemieszczenia, naprężenia von Misesa i siły po każdym rekordzie.

---

## 3. Samouczek - statyczna analiza naprężeń

**Cel.** Obliczyć ugięcie i naprężenie wspornika obciążonego ciężarem własnym i
siłą na końcu oraz odczytać reakcję w podporze.

Ten samouczek zakłada zasiatkowany model objętościowy - dobrze sprawdza się
długi prostopadłościan. Użyj `Plik` -> `Nowy model`, narysuj prostopadłościan za
pomocą `Geometria` -> `Utwórz element` lub zaimportuj geometrię i podziel ją na
czworościany; samouczek *Rysowanie sześcianu* omawia te czynności. Oznacz
utwierdzony koniec i obciążony koniec jako oddzielne encje powierzchni, aby
można było przypisać im warunki brzegowe.

### Krok 1 - wybierz problem

1. `Problem` -> `Przebieg zadań problemów` (`Ctrl+P`).
2. Dodaj zadanie i wybierz **Stress analysis**.
3. Zatwierdź przyciskiem `OK`.

### Krok 2 - przypisz materiał

1. Wybierz encję objętości w drzewie `Model`.
2. Otwórz kartę `Materiał` i przypisz materiał ze wszystkimi czterema
   właściwościami, dla stali konstrukcyjnej:
   - **Modulus of elasticity** = `2.1e11` `Pa`
   - **Poisson ratio** = `0.3`
   - **Density** = `7850` `kg/m^3`
   - **Thermal expansion coefficient** = `1.2e-5` `1/K`

### Krok 3 - pozostaw analizę statyczną

Otwórz kartę `Problem` i pozostaw **Włącz** niezaznaczone w grupie *Solver
czasowy*. Solver raz składa i rozwiązuje wtedy `K*u = f`.

### Krok 4 - podeprzyj model

1. Wybierz encję powierzchni na utwierdzonym końcu.
2. Otwórz kartę `Warunki brzegowe` i zaznacz **Displacement**.
3. Pozostaw wszystkie trzy składowe zaznaczone i ustawione na `0`.

Całkowicie unieruchamia to ścianę i usuwa wszystkie sześć postaci ruchu ciała
sztywnego. Sprawdź, czy nic innego nie pozostało niepodparte - niepołączona część
bez więzów czyni układ osobliwym, a rozwiązanie nie będzie zbieżne.

Dla podpory, która ma się przesuwać, zamiast być w pełni utwierdzona, użyj
zamiast tego na powierzchni **Roller displacement**. Blokuje on tylko kierunek
normalny do powierzchni i pozostawia węzeł swobodny w płaszczyźnie stycznej.

### Krok 5 - przyłóż obciążenia

**Ciężar własny.** Wybierz encję objętości, otwórz kartę `Warunki otoczenia` i
zaznacz **Gravitational acceleration**. Pozostaw domyślne `-9.80665` w Z.

**Obciążenie na końcu.** Wybierz encję powierzchni na swobodnym końcu, otwórz
kartę `Warunki brzegowe` i zaznacz **Force**. Wprowadź siłę całkowitą w
niutonach, na przykład `-1000` w Z. Wartość jest sumą na całej encji, więc nie
zmienia się przy zagęszczaniu siatki.

Alternatywy, zależnie od fizycznej natury obciążenia:

- **Pressure** dla jednorodnego ciśnienia normalnego na ścianie, w `Pa`;
- **Force (unit area)** dla skierowanego obciążenia powierzchniowego w `N/m^2`;
- **Weight** dla dołączonej masy w `kg`, która nie jest modelowana jako
  geometria.

### Krok 6 - rozwiąż

1. `Rozwiązanie` -> `Uruchom solver` (`Ctrl+R`).
2. Kontrola konfiguracji zgłasza brakujące materiały lub brakujące warunki
   brzegowe przed rozpoczęciem obliczeń.
3. Śledź postęp w `Raport` -> `Plik dziennika solvera`.

### Krok 7 - przejrzyj wyniki

- Karta `Wyniki`: wyświetl **Displacement**, aby zobaczyć kształt ugięty, oraz
  **Von Mises stress**, aby zobaczyć rozkład naprężeń. Włącz skalowanie
  odkształceń w ustawieniach widoku, aby ugięcie było widoczne.
- **Normal stress** i **Shear stress** rozdzielają dwa składniki, z których
  składa się podawana wartość von Misesa.
- **Force** na utwierdzonej ścianie jest reakcją. Jej suma powinna równoważyć
  przyłożone obciążenie plus ciężar własny - szybkie i skuteczne sprawdzenie, czy
  model jest obciążony zgodnie z zamierzeniem.
- Punkt monitorowania dla *Displacement* na końcu podaje ugięcie jako liczbę, a
  nie kolor.

### Krok 8 - opcjonalnie naprężenia cieplne

Aby dodać obciążenie cieplne:

1. Dodaj zadanie **Heat transfer** *przed* zadaniem naprężeń w przebiegu zadań
   problemów i skonfiguruj jego warunki brzegowe i właściwości materiału.
2. Przypisz encji konstrukcyjnej warunek otoczenia **Temperature** z
   temperaturą odniesienia stanu beznaprężeniowego, na przykład `293.15` K.

Solver naprężeń pobiera obliczone temperatury elementów z solvera cieplnego i
automatycznie dodaje człon rozszerzalności cieplnej.

### Rozwiązywanie problemów

| Objaw | Przyczyna |
|---|---|
| Rozwiązanie nie jest zbieżne | model jest niedostatecznie podparty; jakaś część jest niepodparta lub podpory nie usuwają wszystkich sześciu postaci ruchu ciała sztywnego |
| *Entity has material assigned which is missing required properties* | brakuje jednej z czterech wymaganych właściwości |
| Przemieszczenia wynoszą wszędzie zero | żadne obciążenie nie dotarło do modelu - sprawdź, czy przypisano grawitację lub czy składowe Force są niezerowe |
| Przemieszczenia są ogromne | moduł sprężystości jest błędny o rzędy wielkości lub encja powierzchni ma zerową grubość, a więc brak sztywności |
| Obciążenie zmienia się przy zagęszczaniu siatki | *Force (unit area)* lub *Pressure* miało być siłą całkowitą albo odwrotnie |
| Ciśnienie naciska w złą stronę | normalne powierzchni nie są spójne lub powierzchnia nie ogranicza objętości |

---

## 4. Samouczek - analiza modalna

**Cel.** Wyodrębnić kilka pierwszych postaci drgań własnych tego samego
wspornika.

### Krok 1 - wybierz problem

`Problem` -> `Przebieg zadań problemów` (`Ctrl+P`), dodaj zadanie, wybierz
**Modal analysis**.

Analizy modalnej nie można łączyć z innymi fizykami z jednym wyjątkiem:
poprzedzającej ją analizy naprężeń, która czyni z niej analizę modalną ze
wstępnym naprężeniem.

### Krok 2 - przypisz materiał

Te same cztery właściwości co w samouczku statycznym. **Gęstość** ma tu większe
znaczenie niż gdziekolwiek indziej: stanowi całą macierz mas, a częstotliwości
skalują się jak `1/sqrt(rho)`.

### Krok 3 - podeprzyj model

Przypisz ten sam warunek brzegowy **Displacement** na utwierdzonym końcu.

To więzy odróżniają postacie wspornika od postaci swobodnie unoszącego się
pręta. Jeśli model pozostanie niepodparty, pierwsze sześć wartości własnych
będzie bliskich zeru - postacie ruchu ciała sztywnego - a interesujące postacie
zostaną zepchnięte dalej na liście.

### Krok 4 - skonfiguruj ustawienia modalne

Otwórz kartę `Problem` i w *Ustawieniach analizy modalnej* ustaw:

| Pole | Wartość |
|---|---|
| Metoda modalna | `Multiple modes` |
| Iteracje | `100` |
| Wyodrębnij postacie | `6` |
| Wartość zbieżności | `1.0e-9` |

Sześć postaci to rozsądny punkt wyjścia, mniej niż domyślne dziesięć. Każda
dodatkowa postać dodaje jedno rozwiązanie liniowe do każdej iteracji, więc duża
liczba postaci na dużej siatce działa wolno.

Wybierz `Only most dominant mode`, gdy potrzebna jest tylko jedna postać; używa
odwrotnej iteracji potęgowej na jednym wektorze i jest znacznie tańsza.

### Krok 5 - obciążenia nie są potrzebne

Analiza modalna nie ma wektora obciążeń - problem własny jest jednorodny.
Obciążenia przypisane do modelu są ignorowane z jednym wyjątkiem: jeśli wcześniej
uruchomiono analizę naprężeń, jej wynik przemieszczenia odkształca siatkę na
potrzeby składania macierzy modalnych.

### Krok 6 - rozwiąż

`Rozwiązanie` -> `Uruchom solver` (`Ctrl+R`). Dziennik podaje iteracje
podprzestrzenne i ich współczynnik zbieżności, a następnie jeden blok na
wyodrębnioną postać:

```
Extracting mode 6
{
    Eigen-value = ...
    Storing results
    ...
}
```

(`Extracting mode 6` = wyodrębnianie postaci 6, `Eigen-value` = wartość własna,
`Storing results` = zapisywanie wyników.) Postacie są przetwarzane od najwyższego
wyodrębnionego indeksu do postaci podstawowej, więc postać podstawowa jest
obliczana jako ostatnia i zapisywana jako rekord 1.

### Krok 7 - przejrzyj postacie

- Panel `Model`, karta `Rekordy`: jeden rekord na postać, numerowany od postaci
  podstawowej. Przechodź przez nie, aby zobaczyć każdą postać drgań.
- Karta `Wyniki`: wyświetl **Displacement**. Postać drgań jest określona tylko z
  dokładnością do współczynnika skali, więc wielkości są dowolne - liczy się
  kształt. Włącz skalowanie odkształceń i animuj rekord, aby zobrazować postać.
- Wartość własna każdej postaci jest wypisywana w dzienniku solvera i
  wyświetlana jako etykieta rekordu w widoku 3D.

### Krok 8 - odczytaj częstotliwości własne

Widok 3D opisuje każdy rekord modalny numerem postaci i jej częstotliwością
własną w Hz. Dziennik solvera podaje tę samą częstotliwość dla każdej
wyodrębnionej postaci wraz z odpowiadającą jej wartością własną:

```
Eigen-value = 1e+06, frequency = 159.155 [Hz]
```

(`frequency` = częstotliwość.) Postać, której iteracja wartości własnych nie
zdołała rozwiązać, jest podawana jako `0` Hz i opatrzona ostrzeżeniem w
dzienniku.

Jak w przypadku każdego wyniku, przy pierwszej konfiguracji modelu warto
sprawdzić częstotliwość podstawową względem obliczeń ręcznych - rozdział 5 podaje
wzór dla wspornika.

### Krok 9 - opcjonalnie postacie ze wstępnym naprężeniem

1. W przebiegu zadań problemów umieść zadanie **Stress analysis** *przed*
   zadaniem **Modal analysis**.
2. Skonfiguruj obciążenia statyczne i więzy jak w samouczku 3.

Analiza modalna działa wtedy na odkształconej siatce. W ten sposób można
zobaczyć, jak obciążenie statyczne zmienia kształt, w jakim drga konstrukcja.

### Rozwiązywanie problemów

| Objaw | Przyczyna |
|---|---|
| Pierwsze wartości własne są bliskie zeru | model jest niedostatecznie podparty lub jakaś część jest niepołączona - rzeczywisty mechanizm |
| Solver wartości własnych nie jest zbieżny | zbyt mało iteracji lub źle uwarunkowana macierz mas, ponieważ encja nie ma gęstości lub miary geometrycznej |
| Wyodrębnianie postaci jest bardzo wolne | zażądano zbyt wielu postaci; każda postać kosztuje pełne rozwiązanie liniowe |
| Obliczenia trwają godzinami lub zdają się nie kończyć | zażądano zbyt wielu postaci - każda kosztuje rozwiązanie liniowe w każdej iteracji, a obliczenia nie mogą zakończyć się wcześniej, dopóki wszystkie się nie ustabilizują. Obserwuj licznik wektorów podprzestrzeni w dzienniku, aby widzieć postęp w obrębie iteracji |
| Częstotliwość wygląda absurdalnie | iteracja podprzestrzenna nie osiągnęła zbieżności - dziennik podaje to na końcu niezbieżnych obliczeń; wyodrębnij mniej postaci, zwiększ liczbę iteracji lub sprawdź gęstość i więzy |
| Postacie drgań wyglądają identycznie | blisko położone postacie symetrycznej konstrukcji lub zbyt mało iteracji podprzestrzennych, by je rozdzielić |
| Mniej rekordów niż żądanych postaci | podprzestrzeń utraciła niezależne kierunki; dziennik podaje, ile postaci wyodrębniono, i zapisuje tylko te |

---

## 5. Sprawdzanie modelu

Solver nie ma wbudowanego zestawu testów weryfikacyjnych, więc przed zaufaniem
nowemu modelowi warto go sprawdzić względem czegoś, co można obliczyć ręcznie.
Trzy tanie sprawdzenia, w kolejności przydatności:

**Bilans reakcji.** Zsumuj wynik *Force* po związanych węzłach. Musi on
równoważyć przyłożone obciążenia plus ciężar własny `rho * V * g`. Wychwytuje to
błędne wielkości obciążeń, błędne jednostki i błędne kierunki ciśnienia.

**Ugięcie końca wspornika.** Dla belki o długości `L`, momencie bezwładności
przekroju `I` i obciążeniu na końcu `F` teoria Eulera-Bernoulliego daje

```
delta = F * L^3 / (3 * E * I)
```

Zamodeluj belkę elementami objętościowymi i porównaj. Spodziewaj się, że wynik
metody elementów skończonych na zgrubnej siatce będzie nieco sztywniejszy i przy
zagęszczaniu siatki będzie zbiegał od dołu. Nie używaj do tego sprawdzenia
elementów liniowych - nie mają sztywności na zginanie.

**Częstotliwość podstawowa wspornika.** Dla tej samej belki o polu przekroju `A`

```
f1 = (1.875^2 / (2*pi)) * sqrt( E*I / (rho * A * L^4) )
```

Porównaj z `sqrt(lambda_0) / (2*pi)` z analizy modalnej. Weryfikuje to
jednocześnie macierz mas, macierz sztywności i przeliczenie wartości własnych.

**Nagle obciążony pręt.** Pręt utwierdzony na jednym końcu i obciążony na drugim
siłą osiową `F` włączoną w chwili `t = 0` sprawdza analizę dynamiczną. Fala
naprężeń biegnie do utwierdzonego końca i z powrotem z prędkością
`c = sqrt(E/rho)`, więc swobodny koniec przemieszcza się liniowo do podwójnego
ugięcia statycznego `2 * F * L / (E * A)` w chwili `t = 2*L/c` i z powrotem do
zera w chwili `t = 4*L/c`. Bez tłumienia cykl się powtarza. Przy tłumieniu
Rayleigha zanika, a pręt zatrzymuje się na ugięciu statycznym. Użyj zerowej
liczby Poissona, aby pręt był rzeczywiście jednowymiarowy.

Przy każdym z tych sprawdzeń zagęść siatkę co najmniej raz i potwierdź, że wynik
jest zbieżny, a nie dryfuje.

---

## 6. Ograniczenia

### Ograniczenia modelowania

- **Tylko liniowa sprężystość.** Małe odkształcenia, małe przemieszczenia,
  materiał izotropowy, brak plastyczności, kontaktu, dużych deformacji i
  wyboczenia. Wyniki mają sens tylko wtedy, gdy konstrukcja pozostaje w zakresie
  liniowym.
- **Tłumienie wyłącznie Rayleigha.** Macierz tłumienia wynosi
  `alpha_M*M + beta_K*K` dla każdego materiału, więc stopień tłumienia podąża za
  krzywą z 1.7 i nie można go ustawić dla każdej postaci z osobna. Nie ma
  tłumików dyskretnych, tłumienia niezależnego od częstotliwości (histerezowego)
  ani tłumiącego warunku brzegowego.
- **Tylko niejawne całkowanie w czasie.** Każdy krok jest rozwiązaniem liniowym.
  Nie ma jawnego schematu różnic centralnych, więc bardzo krótkie zdarzenia,
  takie jak uderzenie balistyczne, wymagają wielu kosztownych kroków.
- **Elementy powierzchniowe są w płaskim stanie odkształcenia.** Cienka płyta
  obciążona w swojej płaszczyźnie będzie sztywniejsza niż w idealizacji płaskiego
  stanu naprężenia. Gdy różnica ma znaczenie, modeluj cienkie konstrukcje
  elementami objętościowymi.
- **Elementy liniowe są prętami kratownicowymi.** Składana jest tylko sztywność
  osiowa `E*A`. Nie ma zginania, ścinania ani skręcania, niezależnie od nazwy
  typu elementu, więc rama modelowana elementami liniowymi zachowa się jak
  kratownica przegubowa.
- **Wstępne naprężenie jest tylko geometryczne.** Analiza modalna ze wstępnym
  naprężeniem ponownie oblicza liniową sztywność na przesuniętej siatce. Nie
  dodaje członu sztywności geometrycznej, więc usztywnienie napiętej liny ani
  zmiękczenie ściskanego pręta nie są ujmowane.
- **Brak współczynników udziału modalnego i mas efektywnych.** Solver podaje
  tylko wartości własne i postacie drgań, więc nie ma bezpośredniej informacji o
  tym, które postacie mają znaczenie dla danego kierunku wymuszenia.

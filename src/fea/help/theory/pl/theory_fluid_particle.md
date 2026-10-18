# Dyspersja zanieczyszczeń - Podręcznik teorii i przewodnik użytkownika

Ten dokument opisuje solver dyspersji zanieczyszczeń programu Range FEA -
`RSolverFluidParticle`, obsługujący typ problemu **Dyspersja zanieczyszczeń**
(*Contaminant dispersion*): rozwiązywane równanie, znaczenie wszystkich
przyjmowanych danych wejściowych, elementy interfejsu graficznego, które nim
sterują, oraz kompletny samouczek.

Dyspersja zanieczyszczeń jest rozwiązywana na polu przepływu zadania
**Przepływ lepki nieściśliwy** (*Incompressible viscous flow*). Solver
przepływu, jego warunki brzegowe, iteracja nieliniowa i całkowanie w czasie są
opisane w podręczniku [Przepływ płynów](theory_fluid.md), na którym opiera się
niniejszy.

**Spis treści**

1. [Podstawy teoretyczne](#1-podstawy-teoretyczne)
2. [Interfejs graficzny](#2-interfejs-graficzny)
3. [Samouczek - przepływ przejściowy z dyspersją zanieczyszczeń](#3-samouczek---przepływ-przejściowy-z-dyspersją-zanieczyszczeń)
4. [Sprawdzanie modelu](#4-sprawdzanie-modelu)
5. [Ograniczenia](#5-ograniczenia)

---

## 1. Podstawy teoretyczne

### 1.1 Niewiadoma podstawowa - stężenie cząstek

Niewiadomą podstawową jest **stężenie cząstek** `C` w `kg/m^3`, jeden stopień
swobody na węzeł.

| Typ problemu | Solver | Niewiadoma podstawowa | Jednostki | Stopnie swobody na węzeł |
|---|---|---|---|---|
| Contaminant dispersion | `RSolverFluidParticle` | stężenie cząstek | `kg/m^3` | 1 |

Ten typ problemu **wymaga** zadania *Incompressible viscous flow* w tym samym
problemie: odczytuje pole prędkości z jego wyniku i nigdy nie oblicza go sam.

### 1.2 Równanie podstawowe

**Contaminant dispersion** transportuje skalarne stężenie na polu przepływu,

```
dC/dt + v . grad(C) = div( D * grad(C) ) + s_eff

s_eff = s * ( 1 - C / C_sat )     for s > 0 and a maximum saturation set
s_eff = s                         otherwise
```

(`for s > 0 and a maximum saturation set` = dla `s > 0` i ustawionego
maksymalnego nasycenia; `otherwise` = w przeciwnym razie), ze stabilizacją SUPG
i schematem czasowym theta solvera przepływu, z prędkością odczytywaną z wyniku
przepływu.

| Symbol | Znaczenie | Jednostki | Źródło |
|---|---|---|---|
| `C` | stężenie cząstek | `kg/m^3` | rozwiązywane |
| `v` | prędkość | `m/s` | wynik przepływu |
| `D` | współczynnik dyfuzji | `m^2/s` | *Ustawienia dyspersji zanieczyszczeń*, domyślnie `0` |
| `C_sat` | maksymalne nasycenie | `kg/m^3` | *Ustawienia dyspersji zanieczyszczeń*, domyślnie `0` (bez ograniczenia) |
| `s` | szybkość źródła cząstek | `kg/(m^3*s)` | warunek Particle rate |

### 1.3 Ustawienia dyspersji zanieczyszczeń

Dwa parametry tego typu problemu ustawia się raz dla modelu w grupie
*Ustawienia dyspersji zanieczyszczeń* na karcie `Problem` (punkt 2.2). Oba
domyślnie mają wartość zero, co odpowiada czystej adwekcji z nieograniczonym
źródłem.

#### Współczynnik dyfuzji

**Współczynnik dyfuzji** `D` rozprzestrzenia zanieczyszczenie zgodnie ze
spadkiem gradientu jego stężenia. Przy `D = 0` transport jest czysto adwekcyjny:
zanieczyszczenie jest unoszone przez przepływ i rozprzestrzenia się tylko dzięki
dyfuzji numerycznej stabilizacji, więc solver odpowiada na pytanie „dokąd
przepływ je zaniesie i jak długo to potrwa”, a nie „jak szeroka jest smuga”.
Dodatnie `D` sprawia, że szerokość smugi staje się wynikiem fizycznym.

Nie ma modelu turbulencji (punkt 5), więc `D` jest dyfuzyjnością
**efektywną**: mieszanie w rzeczywistym przepływie zależy głównie od
turbulencji, którą można oszacować jako `D ~ nu_t / Sc_t` z turbulentną liczbą
Schmidta `Sc_t ~ 0.7`.

| Dyfuzja molekularna (właściwość fizyczna) | `D [m^2/s]` |
|---|---|
| gazy i pary w powietrzu (para wodna `2.5e-5`, CO2 `1.6e-5`, pary rozpuszczalników około `1e-5`) | `1e-6` - `1e-4` |
| substancje rozpuszczone w wodzie (O2 `2e-9`, sole około `1.5e-9`) | `1e-10` - `1e-8` |
| cząstki aerozolu w powietrzu, ruchy Browna (10 nm `5e-8`, 0.1 um `7e-10`, 1 um `3e-11`) | `1e-11` - `1e-8` |

W skalach inżynierskich dyfuzja molekularna jest prawie zawsze pomijalna w
porównaniu z transportem przez przepływ. Zwykle wprowadza się wartość
efektywną:

| Dyfuzja efektywna (turbulentna) | `D_eff [m^2/s]` |
|---|---|
| wentylowane pomieszczenie, mieszanie powietrza wewnętrznego | `1e-3` - `1e-2` |
| przepływ w kanale lub przewodzie, około `D ~ 0.01 - 0.05 * U * L` (`U` prędkość średnia, `L` wymiar kanału) | `1e-3` - `1e-1` |
| rzeki, mieszanie poprzeczne | `1e-2` - `1` |
| atmosfera, mieszanie poziome | `1` - `100` |

To, czy dana wartość ma znaczenie, zależy od siatki. Przy prędkości średniej
`U`, długości przepływu `L`, rozmiarze elementu `h` i kroku czasowym `dt`:

- **kiedy `D` ma widoczny wpływ** - zanieczyszczenie rozszerza się o około
  `sqrt( 2 * D * L / U )`, przechodząc przez obszar. Jeśli jest to mniej niż
  `h`, `D` nic nie zmienia, więc `D` ma znaczenie dopiero powyżej około
  `U * h^2 / ( 2 * L )`. Dla kanału z `U ~ 1 m/s`, `L ~ 15 m` i `h ~ 0.1 m` jest
  to około `3e-4 m^2/s`;
- **kiedy `D` dominuje** - powyżej około `U * L` (około `15 m^2/s` dla tego
  samego kanału) dyfuzja rozmywa wszystko, a przepływ prawie nie ma znaczenia;
- **krok czasowy** - jeśli `D * dt / h^2` przekracza około `1`, przełącz na
  *Backward difference (stable)*; schemat różnic centralnych nie tłumi
  najszybszych postaci dyfuzyjnych i oscyluje.

#### Maksymalne nasycenie

**Maksymalne nasycenie** `C_sat` to największe stężenie, jakie zanieczyszczenie
może osiągnąć w płynie - stężenie pary nasyconej lub rozpuszczalność. Gdy jest
ustawione, dodatni *Particle rate* słabnie w miarę zbliżania się stężenia do tej
wartości i zatrzymuje się przy nasyceniu, `s * ( 1 - C / C_sat )`; modeluje to
parowanie lub rozpuszczanie, a `s` jest wtedy szybkością w czystym płynie, około
`k_m * C_sat * A / V`. Upusty (`s < 0`) nie są ograniczane. Ograniczenie działa
tylko na źródło: stężenie narzucone warunkiem brzegowym lub początkowym nie jest
ograniczane, a dziennik solvera ostrzega, gdy przekracza ono `C_sat`. Stężenie
nie jest też obcinane, więc niewielkie przeregulowanie przy stromym czole jest
nadal możliwe.

Ograniczenie nasycenia istnieje tylko tam, gdzie istnieje ograniczenie
termodynamiczne: ciśnienie pary lub rozpuszczalność. Dla pary w powietrzu
stężenie nasycenia wynika z ciśnienia pary nasyconej,
`C_sat = p_sat * M / ( R * T )`:

| Para w powietrzu | Temperatura | `C_sat [kg/m^3]` |
|---|---|---|
| para wodna | 0 °C | `0.0048` |
| para wodna | 20 °C | `0.017` |
| para wodna | 40 °C | `0.051` |
| para wodna | 100 °C | `0.60` |
| etanol lub toluen | 20 °C | około `0.11` |
| metanol | 20 °C | około `0.17` |
| benzen | 20 °C | około `0.32` |
| aceton | 20 °C | około `0.59` |

Dla substancji rozpuszczonej w wodzie ograniczeniem jest jej rozpuszczalność:

| Rozpuszczone w wodzie | `C_sat [kg/m^3]` |
|---|---|
| tlen w równowadze z powietrzem, 20 °C | `0.009` |
| dwutlenek węgla przy 1 atm | `1.7` |
| gips | `2.4` |
| chlorek sodu | około `360` |

Cząstki, pyły i dym nie mają nasycenia; pozostaw wartość `0` (bez
ograniczenia). Wyłącznie dla orientacji: najwyższe dopuszczalne stężenia w
środowisku pracy wynoszą około `1e-6` - `1e-5 kg/m^3` (1 - 10 mg/m^3), a dolne
granice wybuchowości pyłów około `0.02` - `0.06 kg/m^3`.
Nasycenie silnie zależy od temperatury, podczas gdy solver używa jednej stałej
wartości: w przypadku nieizotermicznym przyjmij wartość dla najzimniejszej
istotnej temperatury, przy której zaczęłaby się kondensacja.

### 1.4 Stabilizacja

Parametr SUPG solvera dyspersji jest ograniczony przez krok czasowy i uwzględnia
dyfuzję,

```
tau = [ ( 2 / dt )^2 + ( 2 * |v| / h )^2 + 9 * ( 4 * D / h^2 )^2 ]^(-1/2)
```

przy czym w analizie ustalonej człon z `dt` jest pomijany. Redukuje się on do
`h / ( 2 * |v| )` tam, gdzie przepływ jest szybki, nigdy nie przekracza `dt / 2`
i dąży do `h^2 / ( 12 * D )` - granicy parametru jednowymiarowego dokładnego w
węzłach - tam, gdzie dominuje dyfuzja. Ograniczenie krokiem czasowym ma
znaczenie w obszarach powolnych i z recyrkulacją: liczba Couranta elementu
`|v| * dt / h` jest tam bardzo mała, a nieograniczone `tau` zamieniłoby
stabilizację w szum, który wytwarza stężenie przed czołem. Ten parametr należy
do solvera dyspersji; solver przepływu używa parametru opisanego w podręczniku
[Przepływ płynów](theory_fluid.md).

Człon adwekcji Galerkina jest składany w **postaci adwekcyjnej z upustem
dywergencji**, `v . grad(C) + 0.5 * max(div(v), 0) * C`, z ciągłą prędkością
węzłową. Prędkość z solvera przepływu nigdy nie jest dokładnie bezdywergencyjna -
na grubej siatce `div(v)` rzędu dziesiątek `1/s` przy ścianach i narożnikach jest
częste. Tam, gdzie przepływ się rozszerza (`div(v) > 0`), zwykła adwekcja sama
wytwarzałaby stężenie, wzór szachownicy narastający z kroku na krok, gdy krok
czasowy jest mały; upust go znosi. Tam, gdzie przepływ się zbiega, żaden człon
nie jest dodawany: symetryczna poprawka `0.5 * div(v) * C` działałaby tam jak
źródło i gromadziła zanieczyszczenie przy ścianach. Dla prędkości dokładnie
bezdywergencyjnej upust wynosi zero.

Przy pomijalnym współczynniku dyfuzji transport jest prawie czystą adwekcją,
której Galerkin z SUPG nie utrzymuje jako monotonicznej: należy spodziewać się
przeregulowań w górę i w dół, największych przy ścianach z warunkiem przylegania,
gdzie prędkość jest zerowa. Efektywny współczynnik dyfuzji (punkt 1.3) usuwa
większość z nich.

W przeciwieństwie do solvera przepływu solver dyspersji stosuje wagę theta
konsekwentnie, więc *Central difference (accurate)* jest tu prawdziwym
schematem Cranka-Nicolson.

### 1.5 Warunki brzegowe

| Warunek brzegowy | Typ | Dotyczy | Składowe |
|---|---|---|---|
| Particle concentration | jawny (Dirichlet) | punkt, linia, powierzchnia | Particle concentration `[kg/m^3]` |
| Particle rate | naturalny, źródło | punkt, linia, powierzchnia, objętość | Particle rate `[kg/(m^3*s)]` |
| Particle flux (unit area) | naturalny (Neumann) | powierzchnia | Particle flux `[kg/(m^2*s)]` |
| Particle mass transfer | naturalny (Robin) | powierzchnia | Mass transfer coefficient `[m/s]`, Ambient particle concentration `[kg/m^3]` |

**Particle concentration** (stężenie cząstek) utrzymuje swoje węzły przy
podanym stężeniu. Przypisany do powierzchni wlotu z tablicą w funkcji czasu
pozwala zamodelować uwolnienie: zero przed zdarzeniem, stężenie uwolnienia w
jego trakcie i zero po nim. Jest to najprzydatniejszy sposób użycia tego typu
problemu, na którym opiera się samouczek z punktu 3.

**Particle rate** (szybkość emisji cząstek) jest źródłem i jest **gęstością** w
`kg/(m^3*s)`, a nie wartością całkowitą - w przeciwieństwie do warunku *Heat*
nie jest rozkładany na miarę encji. Modeluje źródło emitujące w sposób ciągły
wewnątrz obszaru.

**Particle flux (unit area)** (strumień cząstek na jednostkę powierzchni)
narzuca strumień masy `q` wpływający do obszaru przez powierzchnię,
`D * grad(C) . n = q`, gdzie `n` jest normalną zewnętrzną. W przeciwieństwie do
*Particle rate* jest to strumień na jednostkę powierzchni, więc całkowita emisja
wynosi `q * A`. Dodatnie `q` oznacza emisję - parującą warstwę, odgazowującą
ścianę - i jest ograniczane przez maksymalne nasycenie tak samo jak dodatnia
szybkość emisji cząstek, `q * ( 1 - C / C_sat )`. Ujemne `q` oznacza stałą
szybkość usuwania, niezależną od lokalnego stężenia, która może obniżyć je
poniżej zera; osadzanie proporcjonalne do stężenia to *Particle mass transfer*
z `C_inf = 0`. Na ścianie strumień wnika wyłącznie przez dyfuzję; na części brzegu,
przez którą wpływa przepływ, napływ jest traktowany jako czysty płyn niosący
strumień, więc `q` staje się całkowitym napływającym strumieniem masy, a
`C ~ q / |v . n|`. Przypisany do powierzchni wewnętrznej między dwiema
objętościami emituje `q` na jednostkę powierzchni jako źródło powierzchniowe.

**Particle mass transfer** (wymiana masy cząstek) wymienia zanieczyszczenie z
otaczającym zbiornikiem, `q = k_m * ( C_inf - C )`, gdzie `k_m` to *Mass
transfer coefficient* (współczynnik wymiany masy), a `C_inf` to *Ambient
particle concentration* (stężenie cząstek w otoczeniu). Jest to odpowiednik
konwekcji prostej w wymianie ciepła. Przy `C_inf = 0` modeluje osadzanie na
powierzchni z prędkością osadzania `k_m`; przy `C_inf > 0` - wymianę z
powietrzem otoczenia przez otwór lub rozpuszczającą się powierzchnię utrzymywaną
przy `C_inf`. Typowe wartości `k_m` to `1e-4` - `1e-2 m/s` dla osadzania cząstek
w pomieszczeniach i `1e-3` - `1e-1 m/s` dla parowania do poruszającego się
gazu. Duże `k_m` doprowadza powierzchnię do `C_inf` i zbliża się do warunku
*Particle concentration*.

Każda składowa jest **tablicą w funkcji czasu**, więc uwolnienie może się
rozpocząć i zakończyć w trakcie obliczeń przejściowych.

**Brzegi bez warunku.** Ściany i wyloty bez warunku *Particle concentration*,
*Particle flux* lub *Particle mass transfer* zachowują naturalny warunek
**zerowego strumienia dyfuzyjnego**: zanieczyszczenie nie dyfunduje przez ścianę
i opuszcza wylot tylko razem z przepływem.

Tam, gdzie przepływ **wpływa** przez taki brzeg - zwykle przepływ wsteczny na
części wylotu z *Pressure (implicit)* - jest traktowany jako czysty płyn: solver
dodaje słaby człon napływu `max(-v . n, 0) * C` na tej części powierzchni,
rozłożony na ścianę tak, aby tłumił również węzły krawędziowe, w których wylot
styka się ze ścianą z warunkiem przylegania.
Bez tego brzeg zasilałby rozwiązanie, a stężenie na wylocie rosłoby bez
ograniczeń. Jeśli płyn wpływający z powrotem nie jest czysty, wydłuż obszar tak,
aby na wylocie występował tylko odpływ.

Warunki przepływu - ściany, wloty i wyloty - należą do zadania przepływu i są
opisane w podręczniku [Przepływ płynów](theory_fluid.md).

### 1.6 Warunki początkowe

| Warunek początkowy | Skutek |
|---|---|
| Particle concentration | początkowe pole stężenia |

Warunki początkowe są odczytywane **tylko przy pierwszym obliczeniu**; później
pole jest rozwijane od poprzedniego wyniku. Warunek brzegowy ma pierwszeństwo
przed warunkiem początkowym tam, gdzie oba się spotykają.

### 1.7 Stan ustalony czy przejściowy?

Dyspersja zanieczyszczeń jest przede wszystkim problemem **przejściowym**:
problemem transportu w czasie, bez sensownej odpowiedzi ustalonej dla zdarzenia
uwolnienia. Dyspersja ustalona odpowiada na inne pytanie - do jakiego pola
stężenia dąży ciągłe źródło - i jest pojedynczym rozwiązaniem liniowym na polu
przepływu (zob. punkt 2.6 o jej solverze macierzowym).

Dwuetapowy przebieg samouczka z punktu 3 jest metodą kanoniczną: doprowadź
przepływ ustalony do zbieżności, a następnie uruchom obliczenia ponownie z
włączonym solverem czasowym i zadaniem dyspersji.

### 1.8 Wyniki

| Wynik | Dotyczy | Znaczenie |
|---|---|---|
| Particle concentration `[kg/m^3]` | węzeł | rozwiązane pole stężenia |
| Relative saturation `[-]` | węzeł | `C / C_sat`, tylko gdy ustawiono maksymalne nasycenie |

Nasycenie względne jest domyślnie wyświetlane w zakresie od `0` do `1` i jest
usuwane po ponownym ustawieniu maksymalnego nasycenia na zero.

Dla tego typu problemu wymagana jest tylko **gęstość** materiału.

---

## 2. Interfejs graficzny

Wszystko, co istotne, znajduje się w panelu **Solver** (karty `Problem`,
`Warunki brzegowe`, `Warunki początkowe`, `Materiał`, `Wyniki`), menu
**Problem** i panelu **Model**.

### 2.1 Wybór problemu

`Problem` -> `Przebieg zadań problemów` (`Ctrl+P`) otwiera okno przebiegu zadań
problemu. Umieść zadanie **Contaminant dispersion** w tej samej grupie zadań co
zadanie **Incompressible viscous flow** - dyspersja na przepływie, klasyczne
połączenie przejściowe. Ten typ problemu wymaga obecności zadania przepływu.

Wartości grupy zadań - **Liczba iteracji** i **Zbieżność** - sterują iteracją
nieliniową przepływu i są opisane w podręczniku
[Przepływ płynów](theory_fluid.md). W obliczeniach przejściowych są to
iteracje *na krok czasowy*.

### 2.2 Karta Problem

**Solver czasowy** - pozostaw go wyłączonego dla dyspersji ustalonej, włącz dla
zdarzenia uwolnienia.

| Pole | Znaczenie |
|---|---|
| Włącz | włącza i wyłącza całkowanie przejściowe w czasie |
| Przybliżenie | theta schematu czasowego: wsteczny, centralny lub w przód |
| Czas początkowy | pierwsza wartość czasu w sekundach |
| Czas końcowy | `start time + time-step size * number of time-steps` (czas początkowy + rozmiar kroku czasowego × liczba kroków czasowych), tylko do odczytu |
| Rozmiar kroku czasowego | `dt` w sekundach |
| Liczba kroków czasowych | liczba kroków do obliczenia |
| Częstotliwość zapisu | zapisuje plik wyników co N kroków; `0` zapisuje tylko ostatni krok |

**Ustawienia dyspersji zanieczyszczeń** - wyświetlane, gdy w przebiegu zadań
znajduje się zadanie *Contaminant dispersion*. Zawierają dwa parametry z punktu
1.3:

| Pole | Znaczenie |
|---|---|
| Maksymalne nasycenie `[kg/m^3]` | największe możliwe stężenie; dodatnia szybkość emisji cząstek zatrzymuje się na nim i obliczane jest *Relative saturation*. `0` oznacza brak ograniczenia |
| Współczynnik dyfuzji `[m^2/s]` | efektywna dyfuzyjność zanieczyszczenia w płynie. `0` oznacza czystą adwekcję |

Typowe wartości obu podano w punkcie 1.3.

### 2.3 Karta Materiał

| Właściwość | Jednostki | Komentarz |
|---|---|---|
| Density | `kg/m^3` | `rho`, około `1000` dla wody, `1.2` dla powietrza |

Zadanie przepływu w tym samym problemie potrzebuje również lepkości
dynamicznej. Dostarczona baza materiałów zawiera **Water** (woda) i **Air**
(powietrze), co zwykle wyczerpuje ten krok.

### 2.4 Karta Warunki brzegowe

Warunki z punktu 1.5 są oferowane, gdy w przebiegu zadań znajduje się zadanie
*Contaminant dispersion*. Najpierw wybierz encję w drzewie `Model`; menedżer
zaoferuje wtedy warunki właściwe dla danego typu encji. Zaznacz warunek, aby go
przypisać, a następnie edytuj jego składowe w dolnym drzewie.

Każda składowa jest tablicą w funkcji czasu. Kliknij **Edytuj wartości zależne
od czasu**, aby otworzyć edytor składowych, w którym wartości wprowadza się w
funkcji czasu; między dwiema podanymi chwilami wartość jest interpolowana
liniowo, a po ostatniej chwili zachowywana jest ostatnia wartość. W ten sposób
modeluje się zdarzenie uwolnienia.

### 2.5 Warunki początkowe

Karta `Warunki początkowe` oferuje *Particle concentration*. Ustawia on pole
początkowe pierwszego obliczenia.

### 2.6 Konfiguracja solvera macierzowego

`Problem` -> `Konfiguruj solver macierzowy problemów` konfiguruje solvery
iteracyjne. Solver dyspersji używa pozycji **GMRES**, ponieważ jego macierz nie
jest symetryczna.

**Ustalona dyspersja zanieczyszczeń z dyfuzją** wymaga tu uwagi. Jest to
pojedyncze rozwiązanie liniowe, więc nie ma iteracji grupy zadań, na których
można by polegać, a układ zdominowany przez dyfuzję zbiega się powoli przy
domyślnym limicie GMRES wynoszącym 10 iteracji wewnętrznych razy 10
zewnętrznych. Solver macierzowy nie ostrzega o osiągnięciu limitu: odczytaj jego
tabelę iteracji w dzienniku solvera, a jeśli dochodzi do ostatniej iteracji
zewnętrznej, a residuum jest nadal wyraźnie powyżej wartości zbieżności solvera,
zwiększ liczbę iteracji zewnętrznych. Niezbieżne rozwiązanie daje błędną
odpowiedź bez żadnego innego sygnału.

### 2.7 Punkty monitorowania

`Problem` -> `Zdefiniuj punkty monitorowania` umieszcza sondy w podanych
współrzędnych i wybiera rejestrowaną zmienną - *Particle concentration*,
*Particle rate* lub *Relative saturation*. W obliczeniach przejściowych historia
jest wyświetlana za pomocą `Raport` -> `Punkty monitorowania`.

Punkt monitorowania w interesującym miejscu daje odpowiedź w postaci, w jakiej
zwykle jest potrzebna: stężenie w funkcji czasu w danym miejscu, odczytane z
wykresu, a nie z sekwencji obrazów.

### 2.8 Wyniki i rekordy

Karta `Wyniki` zawiera listę obliczonych zmiennych i steruje widokiem 3D.
Stężenie cząstek i nasycenie względne są skalarami w węzłach.

Karta `Rekordy` panelu `Model` zawiera listę rekordów wyników - po jednym dla
każdego zapisanego kroku czasowego obliczeń przejściowych. Animacja rekordów
odtwarza dyspersję, a czerwony przycisk nagrywania pod drzewem zapisuje animację
do pliku wideo.

---

## 3. Samouczek - przepływ przejściowy z dyspersją zanieczyszczeń

**Cel.** Uwolnić zanieczyszczenie do zbieżnego przepływu i śledzić smugę w
kanale w czasie.

Jest to dwuetapowy przebieg z punktu 1.7, odpowiadający dostarczonemu
samouczkowi *Dyspersja zanieczyszczeń w płynach*.

### Krok 1 - zacznij od zbieżnego przepływu

Najpierw doprowadź do zbieżności samouczek kanału w stanie ustalonym z
podręcznika [Przepływ płynów](theory_fluid.md). Obliczenia dyspersji
rozpoczęte ze stanu spoczynku marnują pierwsze kroki czasowe na wytworzenie pola
przepływu, zamiast cokolwiek transportować.

### Krok 2 - dodaj zadanie dyspersji

1. `Problem` -> `Przebieg zadań problemów` (`Ctrl+P`).
2. Zachowaj grupę zadań i zadanie **Incompressible viscous flow**.
3. Dodaj zadanie **Contaminant dispersion** w tej samej grupie.
4. Zmniejsz **Liczbę iteracji** grupy do wartości około `20` - są to teraz
   iteracje nieliniowe *na krok czasowy*, a nie iteracje rozwiązania ustalonego.
   Nie zmieniaj wartości **Zbieżność**; większość kroków ustabilizuje się przed
   dwudziestym przebiegiem i zakończy wcześniej, a te, które tego nie zrobią, są
   właśnie tymi, które potrzebują pełnych dwudziestu.

*Contaminant dispersion* wymaga obecności zadania przepływu; odczytuje pole
prędkości i nigdy go nie oblicza.

### Krok 3 - skonfiguruj solver czasowy

Otwórz kartę `Problem` i w grupie *Solver czasowy* ustaw:

| Pole | Wartość |
|---|---|
| Włącz | zaznaczone |
| Przybliżenie | `Backward difference (stable)` |
| Czas początkowy | `0` |
| Rozmiar kroku czasowego | według poniższej reguły Couranta |
| Liczba kroków czasowych | wystarczająca, aby smuga przeszła przez obszar |
| Częstotliwość zapisu | `1` dla płynnej animacji, więcej, aby plik był mały |

Dobierz krok czasowy na podstawie siatki i prędkości przepływu tak, aby płyn
przechodził w każdym kroku przez mniej więcej jeden element:

```
dt ~ h / |v|
```

Następnie dobierz liczbę kroków na podstawie czasu potrzebnego smudze na
przejście przez obszar, `L / |v|`, podzielonego przez ten krok.

W grupie *Ustawienia dyspersji zanieczyszczeń* na tej samej karcie pozostaw obie
wartości równe `0` dla czystej adwekcji lub wprowadź efektywny **Współczynnik
dyfuzji** z tabeli w punkcie 1.3, jeśli szerokość smugi ma znaczenie.
**Maksymalne nasycenie** ma sens tylko dla pary lub substancji rozpuszczonej;
nie może być mniejsze niż stężenie uwolnienia z kroku 4.

### Krok 4 - uwolnij zanieczyszczenie

1. Wybierz encję powierzchni wlotu.
2. Otwórz kartę `Warunki brzegowe` i zaznacz **Particle concentration**.
3. Zamiast wprowadzać pojedynczą wartość, kliknij **Edytuj wartości zależne od
   czasu**.
4. W edytorze składowych wprowadź profil uwolnienia - na przykład `0` w chwili
   `0`, stężenie uwolnienia w chwili rozpoczęcia uwolnienia i ponownie `0` w
   chwili jego zakończenia.

Między dwiema podanymi chwilami wartość jest interpolowana liniowo, a po
ostatniej chwili zachowywana jest ostatnia wartość, więc uwolnienie, które ma
rozpocząć się gwałtownie, wymaga dwóch blisko siebie położonych wpisów. To ten
zależny od czasu warunek zamienia ustalony wlot w zdarzenie uwolnienia i stanowi
istotę modelu dyspersji.

### Krok 5 - zachowaj warunki przepływu

Ściana **Wall**, warunek wlotu i wylot **Pressure (implicit)** pozostają bez
zmian. Przepływ jest nadal rozwiązywany w każdym kroku czasowym - dyspersja się
na nim opiera.

### Krok 6 - dodaj punkt monitorowania

`Problem` -> `Zdefiniuj punkty monitorowania`, umieść punkt poniżej wlotu w
kierunku przepływu i ustaw jego zmienną na **Particle concentration**. Czas
dotarcia i maksymalne stężenie w danym miejscu znacznie łatwiej odczytać z
historii niż z sekwencji kolorowych obrazów.

### Krok 7 - rozwiąż z ponownym uruchomieniem

1. `Rozwiązanie` -> `Uruchom solver` (`Ctrl+R`).
2. Zaznacz **Uruchom solver ponownie / kontynuuj**.

To ponowne uruchomienie sprawia, że zbieżne pole ustalone staje się punktem
wyjścia całkowania w czasie. Bez niego obliczenia zaczynają się od stanu
spoczynku, a pierwsze kroki są zmarnowane.

### Krok 8 - przejrzyj wyniki

- Panel `Model`, karta `Rekordy`: jeden rekord na zapisany krok. Przeglądaj je
  lub animuj, aby obserwować przemieszczanie się smugi.
- Karta `Wyniki`: wyświetl **Particle concentration**. Ustal zakres
  wyświetlania dla wszystkich rekordów, aby kolory oznaczały to samo w każdej
  klatce - w przeciwnym razie animacja sama zmienia skalę i smuga wydaje się nie
  zanikać.
- `Raport` -> `Punkty monitorowania` rysuje historię stężenia w sondzie.
  Odczytaj z niej czas dotarcia i porównaj go z `L / |v|`, czasem przejścia
  wynikającym z przepływu średniego - to najtańsza weryfikacja całego modelu.
- Przy zerowym współczynniku dyfuzji smuga rozszerza się tylko przez dyfuzję
  numeryczną, więc jej **szerokość** jest właściwością siatki i kroku
  czasowego, a nie fizyki. Przy efektywnym współczynniku dyfuzji szerokość jest
  wynikiem - przed zaufaniem jej sprawdź ją na gęstszej siatce.

### Rozwiązywanie problemów

| Objaw | Przyczyna |
|---|---|
| Stężenie nigdy nie opuszcza wlotu | pole przepływu jest zerowe - w grupie brakuje zadania przepływu albo obliczenia nie zostały ponownie uruchomione od zbieżnego przepływu |
| Smuga niemal natychmiast się rozmywa | liczba Couranta jest znacznie większa od 1 albo siatka jest zbyt gruba wzdłuż drogi |
| Stężenie staje się ujemne lub przeregulowuje | krok czasowy jest zbyt duży dla siatki, więc stabilizowana adwekcja oscyluje. Niewielkie przeregulowania w dół przy stromym czole są normalne; rozwiązanie nie jest obcinane na zerze, ponieważ obcinanie dodawałoby zanieczyszczenie |
| Zapisano tylko jeden rekord | częstotliwość zapisu wynosi `0`, co powoduje zapis tylko ostatniego kroku |
| Residuum każdego kroku pozostaje wysokie | zbyt mało iteracji na krok czasowy - przepływ w każdym kroku nie jest zbieżny |
| Smuga nigdy nie rozszerza się na boki | współczynnik dyfuzji wynosi zero, więc boczne rozprzestrzenianie pochodzi tylko z pola przepływu - wprowadź efektywny współczynnik dyfuzji, zob. punkt 1.3 |
| Smuga od razu rozprzestrzenia się na cały obszar | współczynnik dyfuzji jest zdecydowanie za duży - porównaj go z `U * L` |
| Dziennik ostrzega, że narzucone stężenie przekracza maksymalne nasycenie | warunek brzegowy lub początkowy przekracza maksymalne nasycenie; ograniczenie działa tylko na szybkość emisji cząstek, więc sprawdź obie wartości |
| Rozwiązanie ustalone z dyfuzją wygląda na błędne | solver macierzowy zatrzymał się na limicie iteracji - zwiększ liczbę iteracji zewnętrznych GMRES, zob. punkt 2.6 |
| Stężenie przekracza wartość na wlocie lub staje się ujemne przy ścianach, ale nie rośnie dalej | prawie czysta adwekcja - współczynnik dyfuzji jest bliski zera. Wprowadź efektywny współczynnik dyfuzji; na grubej siatce sprawdź też, czy pole przepływu jest zbieżne |
| Stężenie przekracza wartość na wlocie i narasta w postaci zaszumionego wzoru | przepływ nie jest zbieżny - `Residual ratio` pozostaje bliskie `1` w każdym kroku. Jego prędkość jest daleka od bezdywergencyjnej i żaden transport na niej nie jest wiarygodny; najpierw doprowadź przepływ do zbieżności (siatka, liczba Reynoldsa, iteracje na krok) |

---

## 4. Sprawdzanie modelu

**Czas przejścia.** Czas, w jakim znacznik pokonuje odległość `L` ze średnią
prędkością `V`, wynosi `L/V`. Odczytaj czas dotarcia z historii punktu
monitorowania i porównaj. Potwierdza to, że zadanie dyspersji opiera się na
oczekiwanym polu przepływu, i wykrywa obliczenia, które nie zostały ponownie
uruchomione od zbieżnego przepływu.

Samo pole przepływu najlepiej sprawdzić najpierw, za pomocą bilansu masy i
przepływów analitycznych opisanych w podręczniku
[Przepływ płynów](theory_fluid.md). Niezbieżny model przepływu nie przejdzie
żadnej kontroli z tego samego powodu i żaden transport na nim nie jest
wiarygodny.

---

## 5. Ograniczenia

- **Brak modelu turbulencji.** Przepływ jest laminarny (zob. podręcznik
  [Przepływ płynów](theory_fluid.md)), więc mieszanie w przepływie
  turbulentnym trzeba wprowadzić jako efektywny współczynnik dyfuzji.
- **Stała, izotropowa dyfuzja.** Współczynnik dyfuzji jest jedną wartością dla
  całego modelu; nie może zmieniać się w przestrzeni ani w czasie, podążać za
  kierunkiem przepływu ani pochodzić z modelu turbulencji. Trzeba go oszacować
  jako dyfuzyjność efektywną - zob. punkt 1.3.
- **Stałe maksymalne nasycenie.** Nasycenie nie podąża za temperaturą z wyniku
  *Heat transfer in fluids* i ogranicza tylko dodatnią szybkość emisji lub
  dodatni strumień cząstek. Stężenia narzucone warunkami nie są ograniczane, a
  pole nie jest obcinane.
- **Sprzężenie z przepływem jest jednokierunkowe.** Przepływ napędza transport;
  transport nigdy nie zmienia przepływu.
- **Brak monotoniczności przy prawie czystej adwekcji.** Galerkin z SUPG daje
  przeregulowania w górę i w dół, gdy współczynnik dyfuzji jest bliski zera,
  największe przy ścianach z warunkiem przylegania - zob. punkt 1.4.

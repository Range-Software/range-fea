# Analiza akustyczna - Podręcznik teorii i przewodnik użytkownika

Ten dokument opisuje solver akustyczny (`RSolverAcoustic`) programu Range FEA:
rozwiązywane równania, znaczenie wszystkich przyjmowanych danych wejściowych,
elementy interfejsu graficznego, które nim sterują, oraz dwa kompletne
samouczki.

**Spis treści**

1. [Podstawy teoretyczne](#1-podstawy-teoretyczne)
2. [Interfejs graficzny](#2-interfejs-graficzny)
3. [Samouczek - przejściowa analiza akustyczna](#3-samouczek---przejściowa-analiza-akustyczna)
4. [Samouczek - harmoniczna analiza akustyczna](#4-samouczek---harmoniczna-analiza-akustyczna)
5. [Weryfikacja](#5-weryfikacja)
6. [Ograniczenia](#6-ograniczenia)

---

## 1. Podstawy teoretyczne

### 1.1 Niewiadoma podstawowa - potencjał prędkości

Solver akustyczny nie rozwiązuje bezpośrednio ciśnienia. Jego niewiadomą
podstawową jest **potencjał prędkości** `phi`, pole skalarne o jednostce
`m^2/s`. Obie interesujące wielkości fizyczne wyznacza się z niego:

```
u = -grad(phi)                  particle velocity      [m/s]
p = rho * dphi/dt               acoustic pressure      [Pa]
```

(`particle velocity` = prędkość cząstki, `acoustic pressure` = ciśnienie
akustyczne.)

Użycie potencjału zamiast ciśnienia ma dwie praktyczne zalety. Problem
pozostaje skalarny, mimo że prędkość cząstki jest wektorem, a zadana prędkość
ściany - sposób, w jaki opisuje się większość źródeł akustycznych - wchodzi jako
naturalny warunek brzegowy, a nie jako więz.

Powyższa konwencja znaków (`u = -grad(phi)`, `p = +rho * dphi/dt`) jest
stosowana konsekwentnie w całym solverze i w zapisanych wynikach.

### 1.2 Równanie podstawowe

Dźwięk o małej amplitudzie w spoczywającym, jednorodnym płynie spełnia tłumione
równanie falowe dla potencjału prędkości:

```
laplace(phi) - (1/c^2) * d2phi/dt2 - (beta/c^2) * dphi/dt = 0
```

| Symbol | Znaczenie | Jednostki | Źródło |
|---|---|---|---|
| `phi` | potencjał prędkości | `m^2/s` | rozwiązywany |
| `c` | prędkość dźwięku | `m/s` | właściwość materiału |
| `rho` | gęstość płynu | `kg/m^3` | właściwość materiału |
| `beta` | współczynnik tłumienia objętościowego | `1/s` | właściwość materiału, domyślnie `0` |

Równanie jest liniowe. W konsekwencji jedno rozwiązanie na krok czasowy lub na
częstotliwość jest odpowiedzią końcową - nie wykonuje się zewnętrznej iteracji
nieliniowej, a `hasConverged()` zawsze zgłasza powodzenie po rozwiązaniu układu
liniowego.

**Prędkość dźwięku.** `c` można podać bezpośrednio przez właściwość materiału
*Speed of sound*. Jeśli jej brak lub wynosi zero, jest wyznaczana ze sztywności
objętościowej i gęstości:

```
c = sqrt(E / rho)
```

gdzie `E` jest właściwością materiału *Modulus of elasticity*. Dla gazu `E` jest
adiabatycznym modułem ściśliwości (`gamma * p0`, około `1.42e5 Pa` dla powietrza
pod ciśnieniem atmosferycznym), co razem z `rho = 1.2 kg/m^3` daje znane
`c = 344 m/s`. Bezpośrednie podanie `c` jest zwykle mniej podatne na błędy.

Elementy bez własnego materiału - na przykład powierzchnia brzegowa
ograniczająca siatkowaną objętość - dziedziczą średnie `c` i `rho` obszaru
obliczeniowego, aby człony impedancji i promieniowania były na nich poprawnie
obliczane.

### 1.3 Dyskretyzacja metodą elementów skończonych

Pomnożenie przez funkcję wagową `w`, całkowanie po obszarze, zastosowanie
twierdzenia o dywergencji i pomnożenie całości przez `c^2` daje postać słabą

```
c^2 * INT( grad(phi).grad(w) )  +  INT( d2phi/dt2 * w )  +  beta * INT( dphi/dt * w )
    +  SUM_boundaries( (c/zeta) * SURF( dphi/dt * w ) )
    =  c^2 * SURF( v_n * w )
```

która po podstawieniu funkcji kształtu elementów `N` staje się układem drugiego
rzędu

```
M * phi'' + C * phi' + K * phi = f
```

gdzie

| Macierz | Definicja | Komentarz |
|---|---|---|
| `M` | `INT( N_m * N_n )` | macierz mas |
| `K` | `c^2 * INT( grad(N_m).grad(N_n) )` | macierz sztywności |
| `C` | `beta * M + (c/zeta) * SURF( N_m * N_n )` | tłumienie objętościowe plus impedancja brzegowa |
| `f` | `c^2 * SURF( v_n * N_m )` | źródło zadanej prędkości normalnej |

`M` i `K` są dodatnio półokreślone; `C` jest dodatnio półokreślona, dopóki
stosunek impedancji `zeta` jest dodatni.

**Miary elementów.** Człony obszarowe (`M`, `K`, objętościowa część `C`) są
skalowane geometryczną miarą encji, do której należą:

| Encja | Miara | Gdzie jest ustawiana |
|---|---|---|
| Objętość | 1 | - |
| Powierzchnia | grubość powierzchni | właściwości geometryczne encji |
| Linia | pole przekroju linii | właściwości geometryczne encji |
| Punkt | objętość punktu | właściwości geometryczne encji |

Człony brzegowe (część impedancyjna `C` i źródło `f`) działają na zwykłej mierze
elementu - polu elementu powierzchni, długości elementu linii, jednostkowym polu
dla elementu punktowego. Powierzchnia ograniczająca siatkowaną objętość wnosi
więc tylko swoje człony brzegowe, ponieważ jej grubość domyślnie wynosi zero.

Elementy punktowe nie mają punktów całkowania. Ich wkłady są skupiane
bezpośrednio w ich jedynym węźle, co pozwala działać punktowym warunkom
brzegowym na końcach jednowymiarowych kanałów.

### 1.4 Warunki brzegowe

| Warunek brzegowy | Typ | Dotyczy | Składowa |
|---|---|---|---|
| Velocity potential | jawny (Dirichlet) | punkt, linia, powierzchnia | Potential `[m^2/s]` |
| Velocity | naturalny (źródło) | punkt, linia, powierzchnia | Velocity `[m/s]` |
| Absorbing boundary | naturalny (tłumienie) | punkt, linia, powierzchnia | Acoustic absorption coefficient `[-]` |
| Acoustic impedance | naturalny (tłumienie) | punkt, linia, powierzchnia | Acoustic impedance `[Pa*s/m]` |

**Velocity** (prędkość). Klasyczne źródło akustyczne: powierzchnia drgająca z
zadaną prędkością normalną, taka jak membrana głośnika lub tłok. Wartość
dodatnia napędza płyn, tzn. jest skierowana *do wnętrza* obszaru akustycznego.
Wchodzi do prawej strony jako `f = c^2 * SURF(v_n * N)`.

Węzeł bez żadnego warunku brzegowego jest idealną **sztywną ścianą** - warunkiem
naturalnym postaci słabej jest zerowa prędkość normalna. Aby otrzymać wnękę o
sztywnych ścianach, nie trzeba niczego przypisywać.

**Acoustic impedance** (impedancja akustyczna). Narzuca na brzegu stosunek
ciśnienia do zewnętrznej normalnej prędkości cząstki, `p = Z * u_n`. Wewnętrznie
wyraża się ją przez bezwymiarowy **stosunek impedancji właściwej**

```
zeta = Z / (rho * c)
```

i wnosi brzegową macierz tłumienia `(c/zeta) * SURF(N_m * N_n)`. `Z = rho*c`
(`zeta = 1`) jest impedancją charakterystyczną płynu i daje doskonale
nieodbijające zakończenie. Dla powietrza w warunkach pokojowych `rho*c` wynosi
blisko `415 Pa*s/m`, i taką wartość warunek brzegowy oferuje domyślnie.

**Absorbing boundary** (brzeg pochłaniający). Ten sam człon, ale
sparametryzowany **współczynnikiem pochłaniania** `alpha` dla padania
normalnego w zakresie `[0, 1]`, podawanym w kartach danych materiałów. Jest on
przeliczany na stosunek impedancji przez współczynnik odbicia fali płaskiej
`R = (zeta - 1) / (zeta + 1)` i `alpha = 1 - |R|^2`:

```
zeta = (1 + sqrt(1 - alpha)) / (1 - sqrt(1 - alpha))
```

| `alpha` | `zeta` | Zachowanie |
|---|---|---|
| `1.0` | `1` | pełne pochłanianie, zakończenie bezechowe, brak odbicia |
| `0.5` | `5.83` | częściowe pochłanianie |
| `0.1` | `38.0` | słabe pochłanianie |
| `0.0` | nieskończoność | sztywna ściana, brak jakiegokolwiek tłumienia |

Używaj `alpha = 1` na każdym sztucznym przecięciu nieograniczonego obszaru, aby
emulować swobodne promieniowanie, oraz zmierzonego `alpha` na ścianie pokrytej
okładziną. Jeśli do tej samej encji przypisano zarówno warunek *Acoustic
impedance*, jak i *Absorbing boundary*, pierwszeństwo ma impedancja.

### 1.5 Warunki początkowe

| Warunek początkowy | Ustawia | Komentarz |
|---|---|---|
| Velocity potential | `phi` w `t = 0` | rzadko niezerowy sam w sobie |
| Velocity potential / s | `dphi/dt` w `t = 0` | to on ustawia początkowe **ciśnienie**, `p = rho * dphi/dt` |

Oba są stosowane tylko w pierwszym kroku czasowym. Aby rozpocząć obliczenia
przejściowe od obszaru pod ciśnieniem, przypisz tej encji *Velocity potential /
s* `= p0 / rho`. Warunki początkowe nie mają wpływu na analizę harmoniczną.

### 1.6 Analiza przejściowa

Układ drugiego rzędu jest całkowany schematem **Newmarka** z `gamma = 1/2` oraz

```
beta_N = max( time-march approximation coefficient / 2 , 1/4 )
```

(`time-march approximation coefficient` = współczynnik przybliżenia schematu
czasowego.) Ograniczenie gwarantuje `beta_N >= 1/4`, czyli co najmniej
bezwarunkowo stabilny wariant średniego przyspieszenia, i zapobiega dzieleniu
przez zero dla przybliżenia w przód, którego współczynnik wynosi `0`.

W każdym kroku rozwiązywane jest

```
( K + a0*M + a1*C ) * phi_n+1 = f + M*(a0*phi_n + a2*phi'_n + a3*phi''_n)
                                  + C*(a1*phi_n + a4*phi'_n + a5*phi''_n)
```

ze zwykłymi współczynnikami

```
a0 = 1/(beta_N*dt^2)   a1 = gamma/(beta_N*dt)   a2 = 1/(beta_N*dt)
a3 = 1/(2*beta_N) - 1  a4 = gamma/beta_N - 1    a5 = dt/2*(gamma/beta_N - 2)
a6 = dt*(1 - gamma)    a7 = dt*gamma
```

po czym następuje aktualizacja stanu

```
phi''_n+1 = a0*(phi_n+1 - phi_n) - a2*phi'_n - a3*phi''_n
phi'_n+1  = phi'_n + a6*phi''_n + a7*phi''_n+1
```

Macierz układu jest symetryczna i dodatnio określona, więc rozwiązuje się ją
metodą **gradientów sprzężonych** z prekondycjonowaniem Jacobiego.

`phi'` i `phi''` są zapisywane w pliku wyników jako *Potential / s* i
*Potential / s^2*. Stanowią stan integratora czasowego, więc muszą być obecne,
aby ponowne uruchomienie lub następny krok czasowy przebiegły poprawnie.

**Analiza przejściowa wymaga włączonego solvera czasowego.** Bez niego człony
masy i tłumienia znikają, a problem degeneruje się do bezsensownego problemu
Laplace'a; solver zgłasza to jako błąd, zamiast zwracać bezsensowny wynik.

**Wybór kroku czasowego.** Schemat jest bezwarunkowo stabilny, więc `dt` jest
dyktowany dokładnością, a nie stabilnością. Przydatna reguła mówi, aby
najwyższą interesującą częstotliwość `f_max` rozdzielić co najmniej 20 krokami na
okres:

```
dt <= 1 / (20 * f_max)
```

**Wybór siatki.** Najkrótszą długość fali `lambda_min = c / f_max` rozdziel co
najmniej 10 elementami liniowymi:

```
h <= c / (10 * f_max)
```

### 1.7 Analiza harmoniczna

Dla odpowiedzi ustalonej przy pojedynczej częstości kołowej `omega = 2*pi*f`
zapisuje się `phi(x,t) = Re{ PHI(x) * exp(i*omega*t) }`. Wtedy
`dphi/dt -> i*omega*PHI` oraz `d2phi/dt2 -> -omega^2*PHI`, a układ staje się
zespolonym problemem **Helmholtza**

```
( K - omega^2*M + i*omega*C ) * PHI = F
```

Solvery macierzowe Range FEA są rzeczywiste, więc układ zespolony rozwiązuje się
jako równoważny rzeczywisty układ blokowy o podwójnym rozmiarze. Przy
`S = K - omega^2*M` i `D = omega*C`:

```
| S   -D |   | PHI_re |   | F_re |
|        | * |        | = |      |
| D    S |   | PHI_im |   | F_im |
```

Ta macierz blokowa jest niesymetryczna, a powyżej granicy statycznej
nieokreślona, więc rozwiązuje się ją metodą **GMRES** z prekondycjonowaniem
Jacobiego. GMRES z restartem utyka na układach nieokreślonych, gdy przestrzeń
Kryłowa jest znacznie mniejsza od układu, dlatego solver automatycznie zwiększa
restart GMRES do rozmiaru układu, w ramach stałego budżetu 256 MB na obie bazy
Kryłowa. Skonfigurowany restart jest tylko zwiększany, nigdy zmniejszany, a
zmiana jest zgłaszana w dzienniku solvera:

```
Increasing GMRES restart from 10 to 403 for the frequency domain system.
```

(„Zwiększanie restartu GMRES z 10 do 403 dla układu w dziedzinie
częstotliwości”.) Analiza harmoniczna przemiata listę częstotliwości

```
f_i = first frequency + i * frequency step,   i = 0 .. number of frequencies - 1
```

(`first frequency` = pierwsza częstotliwość, `frequency step` = krok
częstotliwości, `number of frequencies` = liczba częstotliwości.) Każda
częstotliwość jest niezależnym rozwiązaniem i tworzy własny rekord wyników,
podobnie jak analiza przejściowa tworzy jeden rekord na krok czasowy. Solver
czasowy nie odgrywa żadnej roli; jeśli jest włączony, jest pomijany z
ostrzeżeniem, a jego ustawienia pozostają nienaruszone, aby powrót do analizy
przejściowej ich nie utracił.

### 1.8 Wyniki pochodne

| Wynik | Dotyczy | Przejściowa | Harmoniczna |
|---|---|---|---|
| Potential `[m^2/s]` | węzeł | `phi(t)` | `Re{PHI}` |
| Potential (imaginary part) `[m^2/s]` | węzeł | - | `Im{PHI}` |
| Potential / s `[m^2/s^2]` | węzeł | `dphi/dt` | `omega * abs(PHI)` |
| Potential / s^2 `[m^2/s^3]` | węzeł | `d2phi/dt2` | `omega^2 * abs(PHI)` |
| Acoustic pressure `[Pa]` | węzeł | `rho * dphi/dt`, chwilowe | `omega * rho * abs(PHI)`, amplituda |
| Acoustic phase `[Deg]` | węzeł | - | `arg(P)` |
| Sound pressure level `[dB]` | węzeł | `20*log10(abs(p) / p_ref)` | `20*log10(abs(P)/sqrt(2) / p_ref)` |
| Acoustic particle velocity `[m/s]` | element | `-grad(phi)` | amplituda `-grad(PHI)`, dla każdej składowej |
| Acoustic intensity `[W/m^2]` | element | `p * u`, chwilowe | `0.5 * Re{P * conj(U)}`, uśrednione w czasie |

**Poziom ciśnienia akustycznego** (*Sound pressure level*) jest obliczany
względem **Ciśnienia odniesienia** z ustawień akustyki, domyślnie `2e-5 Pa` -
progu słyszalności w powietrzu. W analizie harmonicznej używana jest wartość
skuteczna sygnału harmonicznego, czyli amplituda podzielona przez `sqrt(2)`; w
analizie przejściowej używana jest wartość chwilowa, więc podawany poziom oscyluje
w obrębie okresu.

**Natężenie akustyczne** w analizie harmonicznej jest uśrednionym w czasie
natężeniem czynnym, czyli strumieniem netto mocy akustycznej. Jest tożsamościowo
równe zeru w czystej fali stojącej i wynosi `0.5 * |p| * |u|` w płaskiej fali
biegnącej.

### 1.9 Przejściowa czy harmoniczna?

| | Przejściowa | Harmoniczna |
|---|---|---|
| Rozwiązuje | `M*phi'' + C*phi' + K*phi = f` w czasie | `(K - w^2*M + i*w*C)*PHI = F` dla każdej częstotliwości |
| Odpowiada na | „co się dzieje, gdy ...” | „jaka jest odpowiedź ustalona przy ...” |
| Wymuszenie | dowolny przebieg czasowy, w tym pojedynczy impuls | jedna sinusoida naraz |
| Koszt | jedno rozwiązanie SPD na krok czasowy | jedno rozwiązanie nieokreślone podwójnego rozmiaru na częstotliwość |
| Rekordy | jeden na krok czasowy | jeden na częstotliwość |
| Informacja o fazie | ukryta w przebiegu czasowym | jawna, jako pole wynikowe |

**Używaj analizy przejściowej do**

- badania odpowiedzi impulsowej, echa i pogłosu;
- zdarzeń przejściowych - wybuchu, zamknięcia zaworu, trzaśnięcia drzwiami;
- wymuszenia szerokopasmowego, gdy jedno obliczenie z transformatą Fouriera
  punktu monitorowania obejmuje od razu całe widmo;
- każdego przypadku, w którym odpowiedzią jest sam przebieg czasowy.

**Używaj analizy harmonicznej do**

- funkcji odpowiedzi częstotliwościowej i krzywych tłumienia przenoszenia;
- rezonansów wnęk i kanałów oraz rozkładu ciśnienia przy rezonansie;
- skuteczności tłumików i absorberów w paśmie częstotliwości;
- map poziomu ciśnienia akustycznego dla pojedynczego tonu;
- każdego przypadku, w którym ważny jest tylko stan ustalony. Obliczenie
  harmoniczne jest zwykle znacznie tańsze niż prowadzenie obliczeń przejściowych
  wystarczająco długo, by stany przejściowe wygasły.

Praktycznym kompromisem jest zlokalizowanie rezonansów zgrubnym przemiataniem
harmonicznym, a następnie zagęszczenie tylko interesującego pasma.

---

## 2. Interfejs graficzny

Wszystko, co dotyczy akustyki, znajduje się w panelu **Solver** (karty
`Problem`, `Warunki brzegowe`, `Warunki początkowe`, `Materiał`, `Wyniki`), menu
**Problem** i panelu **Model**.

### 2.1 Wybór problemu akustycznego

`Problem` -> `Przebieg zadań problemów` (`Ctrl+P`) otwiera okno przebiegu zadań
problemu. Dodaj zadanie i wybierz **Acoustics** - „Customized wave equation for
solving acoustic problems” (dostosowane równanie falowe do rozwiązywania
problemów akustycznych).

Acoustics wyklucza się wzajemnie z **Wave - general** i z **Modal analysis**.
Można go łączyć z innymi fizykami, a wtedy kolejność wykonania określa przebieg
zadań.

### 2.2 Karta Problem

Po wybraniu akustyki karta `Problem` panelu Solver wyświetla dwie grupy.

**Solver czasowy** - wyświetlany, ponieważ akustyka jest typem problemu
zależnym od czasu.

| Pole | Znaczenie |
|---|---|
| Włącz | włącza i wyłącza przejściowe całkowanie w czasie |
| Przybliżenie | przybliżenie schematu czasowego; odpowiada `beta_N` Newmarka zgodnie z opisem w 1.6 |
| Czas początkowy | pierwsza wartość czasu w sekundach; ignorowana przy ponownym uruchomieniu solvera |
| Czas końcowy | `start time + time-step size * number of time-steps` (czas początkowy + rozmiar kroku czasowego × liczba kroków czasowych), tylko do odczytu |
| Rozmiar kroku czasowego | `dt` w sekundach |
| Liczba kroków czasowych | liczba kroków do obliczenia |
| Częstotliwość zapisu | zapisuje plik wyników co N kroków; `0` zapisuje tylko ostatni krok |

**Ustawienia analizy akustycznej** - grupa dodawana dla akustyki.

| Pole | Znaczenie |
|---|---|
| Typ analizy | *Transient* (przejściowa) lub *Harmonic* (harmoniczna) |
| Pierwsza częstotliwość `[Hz]` | pierwsza częstotliwość przemiatania harmonicznego |
| Krok częstotliwości `[Hz]` | przyrost między dwiema kolejnymi częstotliwościami |
| Liczba częstotliwości | liczba częstotliwości do rozwiązania; każda tworzy własny rekord |
| Przemiatanie | etykieta tylko do odczytu pokazująca wynikowy zakres częstotliwości |
| Ciśnienie odniesienia `[Pa]` | odniesienie dla poziomu ciśnienia akustycznego, domyślnie `2e-5 Pa` |

Trzy pola częstotliwości są wyszarzone, gdy wybrano *Transient*. Ciśnienie
odniesienia dotyczy obu typów analizy.

### 2.3 Karta Materiał

Karta `Materiał` zawiera materiał przypisany do wybranej encji i jego
właściwości. Dla akustyki istotne są:

| Właściwość | Jednostki | Wymagana | Komentarz |
|---|---|---|---|
| Density | `kg/m^3` | tak | `rho` |
| Speed of sound | `m/s` | jedna z dwóch | `c`, używana bezpośrednio, gdy niezerowa |
| Modulus of elasticity | `Pa` | jedna z dwóch | zastępczo, `c = sqrt(E/rho)` |
| Acoustic damping factor | `1/s` | nie | `beta`, tłumienie objętościowe, domyślnie `0` |

Encja jest rozwiązywana przez solver akustyczny, gdy ma gęstość **oraz**
prędkość dźwięku lub moduł sprężystości. Ponieważ te dwie właściwości są
alternatywne, kontrola konfiguracji nie zgłasza braku żadnej z nich; model,
który nie ma żadnej, jest odrzucany przez solver z jednoznacznym komunikatem.

Każda właściwość jest tablicą w funkcji temperatury, więc można wprowadzić
prędkość dźwięku zależną od temperatury, która będzie interpolowana, jeśli
równolegle rozwiązywany jest problem wymiany ciepła.

### 2.4 Karta Warunki brzegowe

Karta jest podzielona na listę menedżera dostępnych warunków brzegowych u góry i
edytor wybranego warunku u dołu. Najpierw wybierz encję w drzewie `Model`;
menedżer zaoferuje wtedy warunki brzegowe właściwe dla danego typu encji i
wybranego problemu. Zaznacz warunek, aby go przypisać, a następnie edytuj jego
składowe w dolnym drzewie.

Dla akustyki menedżer oferuje:

- **Velocity potential** - jawny, punkty, linie i powierzchnie. Składowa:
  *Potential* `[m^2/s]`.
- **Velocity** - punkty, linie i powierzchnie. Składowa: *Velocity* `[m/s]`.
- **Absorbing boundary** - punkty, linie i powierzchnie. Składowa: *Acoustic
  absorption coefficient*, bezwymiarowa, domyślnie `1.0`.
- **Acoustic impedance** - punkty, linie i powierzchnie. Składowa: *Acoustic
  impedance* `[Pa*s/m]`, domyślnie `415`.

Każda składowa jest tablicą w funkcji czasu, więc źródłu można nadać przebieg
czasowy: dodaj wiersze, aby stopniowo zwiększyć prędkość lub wyemitować krótki
impuls. W analizie harmonicznej używana jest wartość w bieżącej chwili solvera,
która dla tablicy z jedną wartością jest po prostu tą wartością.

Pozostaw ścianę bez żadnego warunku brzegowego, aby była akustycznie sztywna.

### 2.5 Karta Warunki początkowe

Taki sam układ jak karta warunków brzegowych. Dla akustyki oferuje
**Velocity potential** i **Velocity potential / s**. To drugi z nich ustawia
początkowe pole ciśnienia.

### 2.6 Konfiguracja solvera macierzowego

`Problem` -> `Konfiguruj solver macierzowy problemów` konfiguruje solvery
iteracyjne. Przejściowa analiza akustyczna używa pozycji **CG**, a analiza
harmoniczna pozycji **GMRES**.

Domyślny restart GMRES równy `10` jest znacznie za mały dla układu blokowego w
dziedzinie częstotliwości. Solver zwiększa go automatycznie, ale liczba iteracji
*zewnętrznych* i wartość zbieżności są przyjmowane zgodnie z konfiguracją, więc
zwiększ je tutaj, jeśli obliczenia harmoniczne nie są zbieżne.

### 2.7 Punkty monitorowania

`Problem` -> `Zdefiniuj punkty monitorowania` umieszcza sondy w podanych
współrzędnych i wybiera rejestrowaną zmienną. Punkt monitorowania dla *Acoustic
pressure* jest naturalnym mikrofonem: w obliczeniach przejściowych daje przebieg
ciśnienia w danym miejscu, który można obejrzeć za pomocą `Raport` ->
`Punkty monitorowania`.

### 2.8 Wyniki

Po obliczeniach karta `Wyniki` zawiera listę obliczonych zmiennych i steruje
tym, co pokazuje widok 3D. Zmienne akustyczne są opisane w punkcie 1.8.

Zmienne węzłowe (ciśnienie, poziom ciśnienia akustycznego, faza, potencjał) są
rysowane jako gładkie pola; zmienne elementowe (prędkość cząstki, natężenie) są
wektorami i najlepiej pokazywać je za pomocą pola wektorowego lub przekroju.

Karta `Rekordy` panelu `Model` zawiera listę poszczególnych rekordów wyników -
jeden na krok czasowy w obliczeniach przejściowych, jeden na częstotliwość w
przemiataniu harmonicznym - i pozwala je przeglądać lub animować.

`Raport` -> `Plik dziennika solvera` wyświetla pełne wyjście solvera, w tym blok
statystyk drukowany dla każdej zmiennej akustycznej po każdym rekordzie.

---

## 3. Samouczek - przejściowa analiza akustyczna

**Cel.** Obliczyć odpowiedź impulsową zamkniętej wnęki powietrznej i odczytać
przebieg ciśnienia w miejscu mikrofonu.

Ten samouczek zakłada zasiatkowany model objętościowy. Użyj `Plik` ->
`Nowy model`, narysuj prostopadłościan za pomocą `Geometria` ->
`Utwórz element` lub zaimportuj geometrię i podziel ją na czworościany -
samouczek *Rysowanie sześcianu* omawia te czynności.

### Krok 1 - wybierz problem

1. `Problem` -> `Przebieg zadań problemów` (`Ctrl+P`).
2. Dodaj zadanie i wybierz **Acoustics**.
3. Zatwierdź przyciskiem `OK`.

Karta `Problem` panelu Solver wyświetla teraz grupy *Solver czasowy* i
*Ustawienia analizy akustycznej*.

### Krok 2 - przypisz materiał

1. Wybierz encję objętości w drzewie `Model`.
2. Otwórz kartę `Materiał`.
3. Przypisz materiał lub utwórz nowy i nadaj mu:
   - **Density** (gęstość) = `1.2` `kg/m^3`
   - **Speed of sound** (prędkość dźwięku) = `344` `m/s`
4. Pozostaw **Acoustic damping factor** równe `0` dla wnęki bez tłumienia lub
   ustaw małą wartość, np. `1.0` `1/s`, aby odpowiedź zanikała.

### Krok 3 - wybierz typ analizy i krok czasowy

1. Otwórz kartę `Problem`.
2. W *Ustawieniach analizy akustycznej* ustaw **Typ analizy** = `Transient`.
3. W *Solverze czasowym*:
   - zaznacz **Włącz**;
   - **Czas początkowy** = `0`;
   - **Rozmiar kroku czasowego** = `2e-5` s. Zgodnie z `dt <= 1/(20*f_max)`
     rozdziela to częstotliwości do około `2.5 kHz`;
   - **Liczba kroków czasowych** = `500`, co daje `10 ms` symulowanego czasu;
   - **Częstotliwość zapisu** = `5`, co zapisuje 100 rekordów wyników.

Sprawdź siatkę z `h <= c/(10*f_max)`: dla `f_max = 2.5 kHz` i `c = 344 m/s`
rozmiar elementu nie powinien przekraczać około `14 mm`.

### Krok 4 - pobudź wnękę

Dwie możliwości, wybierz jedną.

**A - ruchoma ściana.** Wybierz powierzchnię reprezentującą źródło, otwórz kartę
`Warunki brzegowe` i zaznacz **Velocity**. Wprowadź przebieg czasowy w tablicy
składowej, na przykład krótki impuls:

| Czas `[s]` | Prędkość `[m/s]` |
|---|---|
| `0.0` | `0.0` |
| `0.0002` | `0.01` |
| `0.0004` | `0.0` |
| `0.01` | `0.0` |

**B - obszar początkowo pod ciśnieniem.** Wybierz encję, która ma na początku
być pod ciśnieniem, otwórz kartę `Warunki początkowe` i zaznacz **Velocity
potential / s**. Wprowadź `p0 / rho`; dla `p0 = 1 Pa` i `rho = 1.2 kg/m^3` daje to
`0.833`.

Pozostaw wszystkie inne powierzchnie bez warunku brzegowego, aby wnęka miała
sztywne ściany, lub zaznacz **Absorbing boundary** ze współczynnikiem
pochłaniania odpowiadającym okładzinie ściany, aby energia mogła uchodzić.

### Krok 5 - dodaj mikrofon

1. `Problem` -> `Zdefiniuj punkty monitorowania`.
2. Dodaj punkt w interesującym miejscu.
3. Wybierz **Acoustic pressure** jako jego zmienną.

### Krok 6 - rozwiąż

1. `Rozwiązanie` -> `Uruchom solver` (`Ctrl+R`).
2. Kontrola konfiguracji zgłasza brakujące materiały lub warunki brzegowe przed
   rozpoczęciem obliczeń.
3. Śledź postęp w `Raport` -> `Plik dziennika solvera`. Każdy krok czasowy
   wypisuje zbieżność CG i statystyki każdej zmiennej akustycznej.

### Krok 7 - przejrzyj wyniki

- Panel `Model`, karta `Rekordy`: przeglądaj lub animuj rekordy czasowe.
- Karta `Wyniki`: wyświetl **Acoustic pressure** lub **Sound pressure level** na
  modelu oraz **Acoustic particle velocity** jako pole wektorowe.
- `Raport` -> `Punkty monitorowania`: przebieg ciśnienia w mikrofonie. Jego
  szybkość zaniku daje czas pogłosu, a transformata Fouriera - rezonanse wnęki.

### Rozwiązywanie problemów

| Objaw | Przyczyna |
|---|---|
| *"Transient acoustic analysis requires an enabled time solver"* (przejściowa analiza akustyczna wymaga włączonego solvera czasowego) | w *Solverze czasowym* odznaczono *Włącz* albo typem analizy powinien być *Harmonic* |
| *"Acoustic solver requires a positive time step size"* (solver akustyczny wymaga dodatniego rozmiaru kroku czasowego) | rozmiar kroku czasowego wynosi zero |
| *"No element with a valid speed of sound was found"* (nie znaleziono żadnego elementu z prawidłową prędkością dźwięku) | żadna encja nie ma gęstości razem z prędkością dźwięku lub modułem sprężystości |
| Odpowiedź nigdy nie zanika | nigdzie nie ma tłumienia - jest to oczekiwane dla idealnie sztywnej wnęki bez strat; dodaj brzeg pochłaniający lub współczynnik tłumienia akustycznego |
| Oscylacje przed czołem fali | siatka lub krok czasowy są zbyt grube dla zawartości częstotliwościowej; zagęść jedno i drugie |

---

## 4. Samouczek - harmoniczna analiza akustyczna

**Cel.** Obliczyć odpowiedź częstotliwościową kanału z zakończeniem bezechowym
oraz mapę poziomu ciśnienia akustycznego dla każdej częstotliwości.

### Krok 1 - wybierz problem

`Problem` -> `Przebieg zadań problemów` (`Ctrl+P`), dodaj zadanie, wybierz
**Acoustics**.

### Krok 2 - przypisz materiał

Tak jak w samouczku przejściowym: **Density** `1.2 kg/m^3` i **Speed of sound**
`344 m/s` na encji płynu.

Dobierz rozmiar siatki do **najwyższej** częstotliwości przemiatania:
`h <= c / (10 * f_max)`.

### Krok 3 - skonfiguruj przemiatanie

Otwórz kartę `Problem` i w *Ustawieniach analizy akustycznej* ustaw:

| Pole | Wartość |
|---|---|
| Typ analizy | `Harmonic` |
| Pierwsza częstotliwość | `50` Hz |
| Krok częstotliwości | `50` Hz |
| Liczba częstotliwości | `20` |
| Ciśnienie odniesienia | `2e-5` Pa |

Etykieta *Przemiatanie* potwierdza `50 - 1000 [Hz]`, czyli 20 rozwiązań i 20
rekordów wyników.

Grupa *Solver czasowy* może pozostać włączona - analiza harmoniczna ją ignoruje
i zapisuje ostrzeżenie w dzienniku. Jej ustawienia są zachowywane.

### Krok 4 - pobudź kanał

Wybierz powierzchnię wlotu, otwórz kartę `Warunki brzegowe` i zaznacz
**Velocity**. Wprowadź jeden wiersz z prędkością tłoka, na przykład `0.01`
`m/s`. Wartość jest interpretowana jako amplituda wymuszenia harmonicznego przy
każdej częstotliwości przemiatania.

### Krok 5 - zakończ kanał

Wybierz powierzchnię wylotu i zaznacz:

- **Absorbing boundary** z **Acoustic absorption coefficient** = `1.0` dla
  doskonale nieodbijającego zakończenia; lub
- **Acoustic impedance** z **Acoustic impedance** = `rho * c` = `413` `Pa*s/m`,
  co jest tym samym wyrażonym inaczej.

Użyj mniejszego współczynnika pochłaniania, aby zamodelować zakończenie
częściowo odbijające, i pozostaw wylot bez żadnego warunku, aby zamodelować
zamknięty, sztywny koniec.

Ściany kanału nie wymagają warunku brzegowego - domyślnie są sztywne.

### Krok 6 - sprawdź solver macierzowy

`Problem` -> `Konfiguruj solver macierzowy problemów`, pozycja **GMRES**. Restart
jest zwiększany automatycznie, ale jeśli dziennik pokazuje stagnację residuum,
zwiększ liczbę iteracji zewnętrznych i w razie potrzeby złagodź lub zaostrz
wartość zbieżności.

### Krok 7 - rozwiąż

`Rozwiązanie` -> `Uruchom solver` (`Ctrl+R`). Dziennik pokazuje jeden blok na
częstotliwość:

```
Frequency:         1 of 20        | f =  5.000000e+01 [Hz]
{
    Increasing GMRES restart from 10 to ... for the frequency domain system.
    ...
    Acoustic pressure [Pa]
    Sound pressure level [dB]
    ...
}
```

(`Frequency: 1 of 20` = częstotliwość 1 z 20.)

### Krok 8 - przejrzyj wyniki

- Panel `Model`, karta `Rekordy`: jeden rekord na częstotliwość. Przeglądanie
  ich oznacza przechodzenie przez przemiatanie częstotliwości.
- Karta `Wyniki`: wyświetl **Sound pressure level** jako mapę w decybelach,
  **Acoustic pressure** jako amplitudę i **Acoustic phase**, aby zobaczyć czoła
  fal.
- **Acoustic intensity** pokazuje, dokąd płynie moc akustyczna. W dobrze
  zakończonym kanale jest równomierne i skierowane w stronę wylotu; we wnęce
  rezonansowej jest bliskie zeru.
- Punkt monitorowania dla **Acoustic pressure** rejestruje amplitudę przy każdej
  częstotliwości, czyli funkcję odpowiedzi częstotliwościowej w tym miejscu.

### Interpretacja wyniku

Dla prostego kanału pobudzanego prędkością `v` z zakończeniem bezechowym
analityczne rozwiązanie fali płaskiej ma postać

```
|p| = rho * c * v        constant along the duct
|u| = v
|PHI| = v / k            with k = omega / c
I = 0.5 * |p| * |u|
```

(`constant along the duct` = stałe wzdłuż kanału, `with` = przy.) Dla
`rho = 1.2`, `c = 344`, `v = 0.01` daje to `|p| = 4.13 Pa`, poziom ciśnienia
akustycznego `103.3 dB` i natężenie `0.0206 W/m^2`. Odchylenia od stałego `|p|`
wzdłuż kanału oznaczają, że zakończenie odbija - celowo lub dlatego, że
współczynnik pochłaniania nie jest równy `1`.

Przy sztywnym zakończeniu ten sam kanał staje się rezonansowy. Amplituda
ciśnienia osiąga maksima przy rezonansach ćwierćfalowych `f = (2n+1) * c / (4L)`,
a faza jest stała i wynosi `+/-90` stopni przy zerowym natężeniu netto.

### Rozwiązywanie problemów

| Objaw | Przyczyna |
|---|---|
| *"Harmonic acoustic analysis requires a positive frequency"* (harmoniczna analiza akustyczna wymaga dodatniej częstotliwości) | pierwsza częstotliwość wynosi zero i krok częstotliwości wynosi zero |
| Residuum stoi w miejscu, wyniki wyglądają na zaszumione | GMRES nie osiągnął zbieżności; zwiększ liczbę iteracji zewnętrznych w konfiguracji solvera macierzowego lub użyj rzadszej siatki |
| Amplituda gwałtownie rośnie przy jednej częstotliwości | przemiatanie trafiło na nietłumiony rezonans; dodaj brzeg pochłaniający lub współczynnik tłumienia akustycznego |
| Ciśnienie silnie zmienia się wzdłuż kanału, który miał być bezechowy | zakończenie odbija; sprawdź współczynnik pochłaniania lub wartość impedancji |

---

## 5. Weryfikacja

Test jednostkowy `tst_solver_acoustic` (w `range-solver-lib/tests`, budowany z
`-DRANGE_BUILD_TESTS=ON`) sprawdza solver względem rozwiązań analitycznych na
jednowymiarowym kanale o długości `1 m` zdyskretyzowanym 200 elementami
prętowymi, przy standardowych ustawieniach solvera macierzowego.

**Harmoniczna, zakończenie bezechowe** - `rho = 1.2 kg/m^3`, `c = 340 m/s`,
`v = 0.01 m/s`, `f = 500 Hz`:

| Wielkość | Analitycznie | Obliczona |
|---|---|---|
| Amplituda ciśnienia | `rho*c*v = 4.0800 Pa` | `4.0800 - 4.0804` w każdym węźle |
| Amplituda potencjału | `v/k = 1.0823e-3 m^2/s` | z dokładnością do 1 % |
| Prędkość cząstki | `0.01 m/s` | `0.009999 m/s` |
| Poziom ciśnienia akustycznego | `103.18 dB` | z dokładnością do `0.1 dB` |
| Natężenie | `0.0204 W/m^2` | z dokładnością do 2 % |
| Gradient fazy | `k * dx` | z dokładnością do 5 stopni |

**Harmoniczna, sztywne zakończenie** - fala stojąca daje fazę równą dokładnie
`+/-90` stopni i zerowe natężenie netto, a amplituda maksymalna przekracza
amplitudę fali biegnącej.

**Przejściowa** - pobudzany kanał z zakończeniem bezechowym pozostaje
ograniczony, stan Newmarka jest publikowany, aby całkowanie poprawnie
przebiegało między krokami, a pobudzone pole jest niezerowe i skończone
wszędzie.

---

## 6. Ograniczenia

- **Brak sprzężenia strukturalno-akustycznego.** Akustyki nie można łączyć z
  analizą modalną i nie ma interfejsu płyn-konstrukcja z `RSolverStress`.
  Drgającą konstrukcję trzeba odwzorować zadanym warunkiem brzegowym
  *Velocity*.
- **Tylko akustyka liniowa.** Mała amplituda, ośrodek w spoczynku. Brak
  przepływu średniego, efektów konwekcyjnych i propagacji nieliniowej.
- **Tylko impedancja rzeczywista.** Warunki brzegowe pochłaniający i
  impedancyjny przyjmują wartość rzeczywistą. Zespolona, zależna od
  częstotliwości impedancja powierzchniowa nie jest obsługiwana.
- **Dane materiałowe niezależne od częstotliwości.** Prędkość dźwięku i
  współczynnik tłumienia są tablicowane w funkcji temperatury, a nie
  częstotliwości, więc są stałe w całym przemiataniu.
- **Człony brzegowe używają zwykłej miary elementu.** W modelu jedno- lub
  dwuwymiarowym, w którym pole przekroju lub grubość nie wynosi `1`, człony
  źródła i impedancji nie są przez nie skalowane, w przeciwieństwie do członów
  obszarowych. Aby uniknąć tej niezgodności, użyj jednostkowego pola przekroju
  lub pełnego modelu trójwymiarowego.
- **Wymuszenie harmoniczne jest rzeczywiste.** Zadana prędkość wchodzi jako część
  rzeczywista obciążenia zespolonego; nie można zdefiniować źródeł o względnym
  przesunięciu fazowym.

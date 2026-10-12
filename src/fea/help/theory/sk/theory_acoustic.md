# Akustická analýza - Teoretický manuál a používateľská príručka

Tento dokument opisuje akustický riešič (`RSolverAcoustic`) v Range FEA:
rovnice, ktoré rieši, význam každého vstupu, ktorý prijíma, časti grafického
používateľského rozhrania, ktoré ho ovládajú, a dva podrobne rozpracované
tutoriály.

**Obsah**

1. [Teoretický základ](#1-teoretický-základ)
2. [Grafické používateľské rozhranie](#2-grafické-používateľské-rozhranie)
3. [Tutoriál - prechodová akustická analýza](#3-tutoriál---prechodová-akustická-analýza)
4. [Tutoriál - harmonická akustická analýza](#4-tutoriál---harmonická-akustická-analýza)
5. [Overenie](#5-overenie)
6. [Obmedzenia](#6-obmedzenia)

---

## 1. Teoretický základ

### 1.1 Primárna neznáma - rýchlostný potenciál

Akustický riešič nerieši priamo tlak. Jeho primárnou neznámou je **rýchlostný
potenciál** `phi`, skalárne pole s jednotkami `m^2/s`. Obe fyzikálne veličiny,
o ktoré ide, sa z neho získajú:

```
u = -grad(phi)                  particle velocity      [m/s]
p = rho * dphi/dt               acoustic pressure      [Pa]
```

(`u` je akustická rýchlosť častíc, `p` akustický tlak.) Použitie potenciálu
namiesto tlaku má dve praktické výhody. Úloha zostáva skalárna, hoci rýchlosť
častíc je vektor, a predpísaná rýchlosť steny - spôsob, akým sa opisuje väčšina
akustických zdrojov - vstupuje ako prirodzená okrajová podmienka, nie ako
väzba.

Vyššie uvedená konvencia znamienok (`u = -grad(phi)`, `p = +rho * dphi/dt`) sa
dôsledne používa v celom riešiči aj v uložených výsledkoch.

### 1.2 Riadiaca rovnica

Zvuk s malou amplitúdou v pokojnej homogénnej tekutine sa riadi tlmenou vlnovou
rovnicou pre rýchlostný potenciál:

```
laplace(phi) - (1/c^2) * d2phi/dt2 - (beta/c^2) * dphi/dt = 0
```

| Symbol | Význam | Jednotky | Zdroj |
|---|---|---|---|
| `phi` | rýchlostný potenciál | `m^2/s` | riešený |
| `c` | rýchlosť zvuku | `m/s` | vlastnosť materiálu |
| `rho` | hustota tekutiny | `kg/m^3` | vlastnosť materiálu |
| `beta` | objemový súčiniteľ tlmenia | `1/s` | vlastnosť materiálu, predvolene `0` |

Rovnica je lineárna. Preto je jedno riešenie na časový krok, alebo na
frekvenciu, konečnou odpoveďou - nevykonáva sa žiadna vonkajšia nelineárna
iterácia a `hasConverged()` po vyriešení lineárnej sústavy vždy hlási úspech.

**Rýchlosť zvuku.** `c` možno zadať priamo vlastnosťou materiálu *Rýchlosť
zvuku* (*Speed of sound*). Ak chýba alebo je nulová, odvodí sa z objemovej
tuhosti a hustoty:

```
c = sqrt(E / rho)
```

kde `E` je vlastnosť materiálu *Modul pružnosti* (*Modulus of elasticity*). Pre
plyn je `E` adiabatický modul objemovej pružnosti (`gamma * p0`, asi
`1.42e5 Pa` pre vzduch pri atmosférickom tlaku), čo spolu s `rho = 1.2 kg/m^3`
dáva známu hodnotu `c = 344 m/s`. Priame zadanie `c` je zvyčajne menej náchylné
na chyby.

Prvky, ktoré nemajú vlastný materiál - napríklad hraničná plocha ohraničujúca
objem so sieťou - zdedia priemerné `c` a `rho` počítanej oblasti, aby sa na nich
správne vyhodnotili členy impedancie a vyžarovania.

### 1.3 Diskretizácia metódou konečných prvkov

Vynásobenie váhovou funkciou `w`, integrácia cez oblasť, použitie Gaussovej
vety a vynásobenie `c^2` dáva slabú formuláciu

```
c^2 * INT( grad(phi).grad(w) )  +  INT( d2phi/dt2 * w )  +  beta * INT( dphi/dt * w )
    +  SUM_boundaries( (c/zeta) * SURF( dphi/dt * w ) )
    =  c^2 * SURF( v_n * w )
```

ktorá sa po dosadení tvarových funkcií prvku `N` stane sústavou druhého rádu

```
M * phi'' + C * phi' + K * phi = f
```

kde

| Matica | Definícia | Poznámka |
|---|---|---|
| `M` | `INT( N_m * N_n )` | matica hmotnosti |
| `K` | `c^2 * INT( grad(N_m).grad(N_n) )` | matica tuhosti |
| `C` | `beta * M + (c/zeta) * SURF( N_m * N_n )` | objemové tlmenie plus impedancia hranice |
| `f` | `c^2 * SURF( v_n * N_m )` | zdroj predpísanej normálovej rýchlosti |

`M` a `K` sú pozitívne semidefinitné; `C` je pozitívne semidefinitná, pokiaľ je
pomer impedancie `zeta` kladný.

**Miery prvkov.** Objemové členy (`M`, `K`, objemová časť `C`) sa škálujú
geometrickou mierou entity, ku ktorej patria:

| Entita | Miera | Kde sa nastavuje |
|---|---|---|
| Objem | 1 | - |
| Plocha | hrúbka plochy | geometrické vlastnosti entity |
| Čiara | prierez čiary | geometrické vlastnosti entity |
| Bod | objem bodu | geometrické vlastnosti entity |

Hraničné členy (impedančná časť `C` a zdroj `f`) pôsobia cez obyčajnú mieru
prvku - plochu plošného prvku, dĺžku čiarového prvku, jednotkovú plochu pre
bodový prvok. Plocha ohraničujúca objem so sieťou preto prispieva iba svojimi
hraničnými členmi, pretože jej hrúbka je predvolene nulová.

Bodové prvky nemajú integračné body. Ich príspevky sa sústreďujú priamo do ich
jediného uzla, vďaka čomu fungujú bodové okrajové podmienky na koncoch
jednorozmerných potrubí.

### 1.4 Okrajové podmienky

| Okrajová podmienka | Typ | Aplikuje sa na | Zložka |
|---|---|---|---|
| Rýchlostný potenciál (*Velocity potential*) | explicitná (Dirichletova) | bod, čiara, plocha | Potenciál `[m^2/s]` |
| Rýchlosť (*Velocity*) | prirodzená (zdroj) | bod, čiara, plocha | Rýchlosť `[m/s]` |
| Pohltivá hranica (*Absorbing boundary*) | prirodzená (tlmenie) | bod, čiara, plocha | Koeficient akustickej pohltivosti `[-]` |
| Akustická impedancia (*Acoustic impedance*) | prirodzená (tlmenie) | bod, čiara, plocha | Akustická impedancia `[Pa*s/m]` |

**Rýchlosť.** Klasický akustický zdroj: plocha, ktorá kmitá s predpísanou
normálovou rýchlosťou, ako membrána reproduktora alebo piest. Kladná hodnota
poháňa tekutinu, t. j. smeruje *do* akustickej oblasti. Na pravú stranu vstupuje
ako `f = c^2 * SURF(v_n * N)`.

Uzol bez akejkoľvek okrajovej podmienky je ideálna **tuhá stena** - prirodzenou
podmienkou slabej formulácie je nulová normálová rýchlosť. Na získanie dutiny s
tuhými stenami netreba nič priraďovať.

**Akustická impedancia.** Predpisuje na hranici pomer tlaku k vonkajšej
normálovej rýchlosti častíc, `p = Z * u_n`. Interne sa vyjadruje cez bezrozmerný
**pomer špecifickej impedancie**

```
zeta = Z / (rho * c)
```

a prispieva hraničnou maticou tlmenia `(c/zeta) * SURF(N_m * N_n)`.
`Z = rho*c` (`zeta = 1`) je charakteristická impedancia tekutiny a dáva dokonale
neodrážajúce zakončenie. Pre vzduch pri izbových podmienkach je `rho*c` blízko
`415 Pa*s/m`, čo je hodnota, ktorú okrajová podmienka ponúka predvolene.

**Pohltivá hranica.** Ten istý člen, ale parametrizovaný **koeficientom
pohltivosti** pri kolmom dopade `alpha` v intervale `[0, 1]`, ktorý uvádzajú
katalógové listy materiálov. Na pomer impedancie sa prevádza cez koeficient
odrazu rovinnej vlny `R = (zeta - 1) / (zeta + 1)` a `alpha = 1 - |R|^2`:

```
zeta = (1 + sqrt(1 - alpha)) / (1 - sqrt(1 - alpha))
```

| `alpha` | `zeta` | Správanie |
|---|---|---|
| `1.0` | `1` | úplne pohltivé, bezodrazové zakončenie, žiadny odraz |
| `0.5` | `5.83` | čiastočne pohltivé |
| `0.1` | `38.0` | slabo pohltivé |
| `0.0` | nekonečno | tuhá stena, vôbec žiadne tlmenie |

Na akomkoľvek umelom reze cez neohraničenú oblasť použite `alpha = 1` na
napodobnenie voľného vyžarovania a na obloženej stene nameranú hodnotu `alpha`.
Ak sú tej istej entite priradené podmienky *Akustická impedancia* aj *Pohltivá
hranica*, vyhráva impedancia.

### 1.5 Počiatočné podmienky

| Počiatočná podmienka | Nastavuje | Poznámka |
|---|---|---|
| Rýchlostný potenciál | `phi` v čase `t = 0` | samostatne zriedka nenulová |
| Rýchlostný potenciál / s | `dphi/dt` v čase `t = 0` | práve táto nastavuje počiatočný **tlak**, `p = rho * dphi/dt` |

Obe sa aplikujú iba v prvom časovom kroku. Ak chcete začať prechodový výpočet z
oblasti pod tlakom, priraďte tejto entite *Rýchlostný potenciál / s*
`= p0 / rho`. Počiatočné podmienky nemajú na harmonickú analýzu žiadny účinok.

### 1.6 Prechodová analýza

Sústava druhého rádu sa integruje **Newmarkovou** schémou s `gamma = 1/2` a

```
beta_N = max( time-march approximation coefficient / 2 , 1/4 )
```

(koeficient aproximácie časového pochodu delený dvomi, najmenej `1/4`). Toto
obmedzenie zaručuje `beta_N >= 1/4`, t. j. aspoň bezpodmienečne stabilný variant
priemerného zrýchlenia, a bráni deleniu nulou pri doprednej aproximácii, ktorej
koeficient je `0`.

Každý krok rieši

```
( K + a0*M + a1*C ) * phi_n+1 = f + M*(a0*phi_n + a2*phi'_n + a3*phi''_n)
                                  + C*(a1*phi_n + a4*phi'_n + a5*phi''_n)
```

s obvyklými koeficientmi

```
a0 = 1/(beta_N*dt^2)   a1 = gamma/(beta_N*dt)   a2 = 1/(beta_N*dt)
a3 = 1/(2*beta_N) - 1  a4 = gamma/beta_N - 1    a5 = dt/2*(gamma/beta_N - 2)
a6 = dt*(1 - gamma)    a7 = dt*gamma
```

po ktorom nasleduje aktualizácia stavu

```
phi''_n+1 = a0*(phi_n+1 - phi_n) - a2*phi'_n - a3*phi''_n
phi'_n+1  = phi'_n + a6*phi''_n + a7*phi''_n+1
```

Matica sústavy je symetrická pozitívne definitná, takže sa rieši **metódou
združených gradientov** s Jacobiho predpodmienením.

`phi'` a `phi''` sa zapisujú do súboru výsledkov ako *Potential / s* a
*Potential / s^2*. Sú stavom časového integrátora, takže musia byť prítomné, aby
reštart alebo nasledujúci časový krok mohol správne pokračovať.

**Prechodová analýza vyžaduje povolený časový riešič.** Bez neho členy hmotnosti
a tlmenia zaniknú a úloha zdegeneruje na nezmyselnú Laplaceovu úlohu; riešič to
ohlási ako chybu namiesto toho, aby vytvoril nezmysel.

**Voľba časového kroku.** Schéma je bezpodmienečne stabilná, takže `dt` určuje
presnosť, nie stabilita. Užitočným pravidlom je rozlíšiť najvyššiu frekvenciu,
o ktorú ide, `f_max` aspoň 20 krokmi na periódu:

```
dt <= 1 / (20 * f_max)
```

**Voľba siete.** Najkratšiu vlnovú dĺžku `lambda_min = c / f_max` rozlíšte
aspoň 10 lineárnymi prvkami:

```
h <= c / (10 * f_max)
```

### 1.7 Harmonická analýza

Pre ustálenú odozvu pri jednej uhlovej frekvencii `omega = 2*pi*f` napíšte
`phi(x,t) = Re{ PHI(x) * exp(i*omega*t) }`. Potom
`dphi/dt -> i*omega*PHI` a `d2phi/dt2 -> -omega^2*PHI` a sústava sa stane
komplexnou **Helmholtzovou** úlohou

```
( K - omega^2*M + i*omega*C ) * PHI = F
```

Maticové riešiče Range FEA sú reálne, takže komplexná sústava sa rieši ako
ekvivalentná reálna bloková sústava dvojnásobnej veľkosti. S `S = K - omega^2*M`
a `D = omega*C`:

```
| S   -D |   | PHI_re |   | F_re |
|        | * |        | = |      |
| D    S |   | PHI_im |   | F_im |
```

Táto bloková matica je nesymetrická a nad statickou limitou indefinitná, takže sa
rieši metódou **GMRES** s Jacobiho predpodmienením. Reštartovaný GMRES na
indefinitných sústavách stagnuje vždy, keď je Krylovov priestor oveľa menší ako
sústava, preto riešič automaticky rozšíri reštart GMRES na veľkosť sústavy,
obmedzený pevným rozpočtom 256 MB pre dve Krylovove bázy. Nastavený reštart sa
vždy iba zvyšuje, nikdy neznižuje, a zmena sa ohlási v logu riešiča:

```
Increasing GMRES restart from 10 to 403 for the frequency domain system.
```

Harmonická analýza rozmieta zoznam frekvencií

```
f_i = first frequency + i * frequency step,   i = 0 .. number of frequencies - 1
```

(prvá frekvencia plus `i`-násobok frekvenčného kroku, pre `i` od `0` po počet
frekvencií mínus jedna). Každá frekvencia je nezávislé riešenie a vytvára vlastný
záznam výsledkov, rovnako ako prechodová analýza vytvára jeden záznam na časový
krok. Časový riešič nehrá žiadnu úlohu; ak je náhodou povolený, preskočí sa s
varovaním a jeho nastavenia zostanú nedotknuté, aby sa pri prepnutí späť na
prechodovú analýzu nestratili.

### 1.8 Odvodené výsledky

| Výsledok | Aplikuje sa na | Prechodová | Harmonická |
|---|---|---|---|
| Potenciál `[m^2/s]` | uzol | `phi(t)` | `Re{PHI}` |
| Potenciál (imaginárna časť) `[m^2/s]` | uzol | - | `Im{PHI}` |
| Potenciál / s `[m^2/s^2]` | uzol | `dphi/dt` | `omega * abs(PHI)` |
| Potenciál / s^2 `[m^2/s^3]` | uzol | `d2phi/dt2` | `omega^2 * abs(PHI)` |
| Akustický tlak `[Pa]` | uzol | `rho * dphi/dt`, okamžitý | `omega * rho * abs(PHI)`, amplitúda |
| Akustická fáza `[Deg]` | uzol | - | `arg(P)` |
| Hladina akustického tlaku `[dB]` | uzol | `20*log10(abs(p) / p_ref)` | `20*log10(abs(P)/sqrt(2) / p_ref)` |
| Akustická rýchlosť častíc `[m/s]` | prvok | `-grad(phi)` | amplitúda `-grad(PHI)`, po zložkách |
| Akustická intenzita `[W/m^2]` | prvok | `p * u`, okamžitá | `0.5 * Re{P * conj(U)}`, časovo stredovaná |

**Hladina akustického tlaku** sa počíta voči *Referenčnému tlaku* nastavení
akustiky, predvolene `2e-5 Pa` - prah počutia vo vzduchu. V harmonickej analýze
sa používa efektívna hodnota harmonického signálu, teda amplitúda delená
`sqrt(2)`; v prechodovej analýze sa používa okamžitá veľkosť, takže uvádzaná
hladina v rámci periódy osciluje.

**Akustická intenzita** v harmonickej analýze je časovo stredovaná činná
intenzita, t. j. výsledný tok akustického výkonu. V čistom stojatom vlnení je
identicky nulová a v rovinnej postupnej vlne sa rovná `0.5 * |p| * |u|`.

### 1.9 Prechodová alebo harmonická?

| | Prechodová | Harmonická |
|---|---|---|
| Rieši | `M*phi'' + C*phi' + K*phi = f` v čase | `(K - w^2*M + i*w*C)*PHI = F` pre každú frekvenciu |
| Odpovedá na | "čo sa stane, keď ..." | "aká je ustálená odozva pri ..." |
| Budenie | ľubovoľný časový priebeh vrátane jediného impulzu | vždy jedna sínusoida |
| Náklady | jedno SPD riešenie na časový krok | jedno indefinitné riešenie dvojnásobnej veľkosti na frekvenciu |
| Záznamy | jeden na časový krok | jeden na frekvenciu |
| Informácia o fáze | implicitne v časovom priebehu | explicitne, ako pole výsledkov |

**Prechodovú analýzu použite na**

- štúdie impulzovej odozvy, ozveny a dozvuku;
- prechodové udalosti - rana, zatvorenie ventilu, buchnutie dverí;
- širokopásmové budenie, kde jeden výpočet, po ktorom nasleduje Fourierova
  transformácia monitorovacieho bodu, pokryje celé spektrum naraz;
- každý prípad, keď je odpoveďou samotný časový priebeh.

**Harmonickú analýzu použite na**

- frekvenčné prenosové funkcie a krivky prenosového útlmu;
- rezonancie dutín a potrubí a rozloženie tlaku pri rezonancii;
- účinnosť tlmičov a absorbérov vo frekvenčnom pásme;
- mapy hladiny akustického tlaku pri jednom tóne;
- každý prípad, keď záleží iba na ustálenom stave. Harmonický výpočet je zvyčajne
  oveľa lacnejší ako krokovanie prechodovej analýzy dostatočne dlho na to, aby
  prechodové javy odzneli.

Praktickým kompromisom je nájsť rezonancie hrubým harmonickým rozmietaním a
potom zjemniť iba zaujímavé pásmo.

---

## 2. Grafické používateľské rozhranie

Všetko, čo súvisí s akustikou, sa nachádza v paneli **Riešič** (záložky
`Úloha`, `Okrajové podmienky`, `Počiatočné podmienky`, `Materiál`, `Výsledky`),
v menu **Úloha** a v paneli **Model**.

### 2.1 Výber akustickej úlohy

`Úloha` -> `Poradie krokov úloh` (`Ctrl+P`) otvorí dialóg poradia krokov úloh.
Pridajte krok a vyberte **Akustika** (*Acoustics*) - "Upravená vlnová rovnica na
riešenie akustických úloh".

Akustika sa vzájomne vylučuje s úlohou **Vlna - všeobecná** (*Wave - general*) a
s **Modálnou analýzou**. Možno ju kombinovať s inými fyzikami, pričom poradie
vykonania určuje poradie krokov.

### 2.2 Záložka Úloha

Po výbere akustiky zobrazí záložka `Úloha` panela Riešič dve skupiny.

**Riešič v čase** - zobrazuje sa, pretože akustika je časovo závislý typ úlohy.

| Pole | Význam |
|---|---|
| Povoliť | zapína a vypína krokovanie v čase |
| Aproximácia | aproximácia časového pochodu; mapuje sa na Newmarkovo `beta_N` podľa 1.6 |
| Počiatočný čas | prvá hodnota času v sekundách; pri reštarte riešiča sa ignoruje |
| Konečný čas | `počiatočný čas + veľkosť časového kroku * počet časových krokov`, iba na čítanie |
| Veľkosť časového kroku | `dt` v sekundách |
| Počet časových krokov | koľko krokov sa má vypočítať |
| Frekvencia výstupu | zapíše súbor výsledkov každých N krokov; `0` zapíše iba posledný krok |

**Nastavenia akustickej analýzy** - skupina pridaná pre akustiku.

| Pole | Význam |
|---|---|
| Typ analýzy | `Transient` (prechodová) alebo `Harmonic` (harmonická) |
| Prvá frekvencia `[Hz]` | prvá frekvencia harmonického rozmietania |
| Frekvenčný krok `[Hz]` | prírastok medzi dvoma po sebe idúcimi frekvenciami |
| Počet frekvencií | koľko frekvencií sa má vyriešiť; každá vytvorí vlastný záznam |
| Rozmietanie | popis iba na čítanie, ktorý ukazuje výsledný rozsah frekvencií |
| Referenčný tlak `[Pa]` | referencia pre hladinu akustického tlaku, predvolene `2e-5 Pa` |

Tri frekvenčné polia sú sivé, kým je vybraná `Transient`. Referenčný tlak platí
pre oba typy analýzy.

### 2.3 Záložka Materiál

Záložka `Materiál` uvádza materiál priradený vybranej entite a vlastnosti
materiálu, ktoré obsahuje. Pre akustiku sú podstatné tieto vlastnosti:

| Vlastnosť | Jednotky | Požadovaná | Poznámka |
|---|---|---|---|
| Hustota | `kg/m^3` | áno | `rho` |
| Rýchlosť zvuku | `m/s` | jedna z dvoch | `c`, použije sa priamo, ak je nenulová |
| Modul pružnosti | `Pa` | jedna z dvoch | náhradná možnosť, `c = sqrt(E/rho)` |
| Súčiniteľ akustického tlmenia (*Acoustic damping factor*) | `1/s` | nie | `beta`, objemový útlm, predvolene `0` |

Akustický riešič rieši entitu, keď má hustotu **a** buď rýchlosť zvuku, alebo
modul pružnosti. Keďže tieto dve sú alternatívy, kontrola nastavenia ani jednu
nehlási ako chýbajúcu; model, ktorý nemá ani jednu, riešič odmietne s výslovným
hlásením.

Každá vlastnosť je tabuľkou v závislosti od teploty, takže možno zadať teplotne
závislú rýchlosť zvuku, ktorá sa bude interpolovať, ak súbežne beží úloha
prestupu tepla.

### 2.4 Záložka Okrajové podmienky

Záložka je rozdelená na zoznam dostupných okrajových podmienok hore a editor
vybranej podmienky dole. Najprv vyberte entitu v strome `Model`; zoznam potom
ponúkne okrajové podmienky platné pre daný typ entity a pre vybranú úlohu.
Zaškrtnutím podmienku priradíte a jej zložky potom upravíte v dolnom strome.

Pre akustiku zoznam ponúka:

- **Rýchlostný potenciál** - explicitná, body, čiary a plochy. Zložka:
  *Potenciál* `[m^2/s]`.
- **Rýchlosť** - body, čiary a plochy. Zložka: *Rýchlosť* `[m/s]`.
- **Pohltivá hranica** - body, čiary a plochy. Zložka: *Koeficient akustickej
  pohltivosti*, bezrozmerný, predvolene `1.0`.
- **Akustická impedancia** - body, čiary a plochy. Zložka: *Akustická
  impedancia* `[Pa*s/m]`, predvolene `415`.

Každá zložka je tabuľkou v čase, takže zdroju možno dať časový priebeh: pridajte
riadky na postupný nábeh rýchlosti alebo na vyslanie krátkeho impulzu. V
harmonickej analýze sa použije hodnota v aktuálnom čase riešiča, čo je pre
tabuľku s jedinou hodnotou jednoducho táto hodnota.

Ak má byť stena akusticky tuhá, nechajte ju bez akejkoľvek okrajovej podmienky.

### 2.5 Záložka Počiatočné podmienky

Rovnaké usporiadanie ako záložka okrajových podmienok. Pre akustiku ponúka
**Rýchlostný potenciál** a **Rýchlostný potenciál / s**. Počiatočné pole tlaku
nastavuje práve tá druhá.

### 2.6 Nastavenie maticového riešiča

`Úloha` -> `Nastaviť maticový riešič úloh` nastavuje iteračné riešiče.
Prechodová akustická analýza používa položku **CG**, harmonická analýza položku
**GMRES**.

Predvolený reštart GMRES `10` je pre blokovú sústavu vo frekvenčnej oblasti
príliš malý. Riešič ho zvýši automaticky, ale počet *vonkajších* iterácií a
hodnota konvergencie sa preberajú tak, ako sú nastavené, takže ak harmonický
výpočet nekonverguje, zvýšte ich tu.

### 2.7 Monitorovacie body

`Úloha` -> `Definovať monitorovacie body` umiestni sondy na zadané súradnice a
vyberie premennú, ktorá sa má zaznamenávať. Monitorovací bod na *Akustickom
tlaku* je prirodzeným mikrofónom: v prechodovom výpočte vytvorí časový priebeh
tlaku v danom mieste, ktorý si možno pozrieť cez `Výkaz` -> `Monitorovacie body`.

### 2.8 Výsledky

Po výpočte záložka `Výsledky` uvádza vypočítané premenné a ovláda, čo zobrazuje
3D pohľad. Akustické premenné sú tie z časti 1.8.

Uzlové premenné (tlak, hladina akustického tlaku, fáza, potenciál) sa kreslia ako
hladké polia; prvkové premenné (rýchlosť častíc, intenzita) sú vektory a
najlepšie sa zobrazujú ako vektorové pole alebo rez.

Záložka `Záznamy` panela `Model` uvádza jednotlivé záznamy výsledkov - jeden na
časový krok pri prechodovom výpočte, jeden na frekvenciu pri harmonickom
rozmietaní - a umožňuje nimi prechádzať alebo ich animovať.

`Výkaz` -> `Log súbor riešiča` zobrazí celý výstup riešiča vrátane bloku
štatistík vypisovaného pre každú akustickú premennú po každom zázname.

---

## 3. Tutoriál - prechodová akustická analýza

**Cieľ.** Vypočítať impulzovú odozvu uzavretej vzduchovej dutiny a odčítať
priebeh tlaku v mieste mikrofónu.

Tento tutoriál predpokladá objemový model so sieťou. Použite `Súbor` -> `Nový
model`, nakreslite kváder pomocou `Geometria` -> `Vytvoriť prvok` alebo
importujte geometriu a rozdeľte ju na tetrahedróny - tento postup pokrýva
tutoriál *Kresliť kocku*.

### Krok 1 - výber úlohy

1. `Úloha` -> `Poradie krokov úloh` (`Ctrl+P`).
2. Pridajte krok a vyberte **Akustika**.
3. Potvrďte tlačidlom `OK`.

Záložka `Úloha` panela Riešič teraz zobrazuje skupiny *Riešič v čase* a
*Nastavenia akustickej analýzy*.

### Krok 2 - priradenie materiálu

1. Vyberte objemovú entitu v strome `Model`.
2. Otvorte záložku `Materiál`.
3. Priraďte materiál, alebo ho vytvorte, a zadajte mu:
   - **Hustota** = `1.2` `kg/m^3`
   - **Rýchlosť zvuku** = `344` `m/s`
4. Pre netlmenú dutinu ponechajte **Súčiniteľ akustického tlmenia** na `0`,
   alebo ho nastavte na malú hodnotu, napríklad `1.0` `1/s`, aby odozva
   doznievala.

### Krok 3 - voľba typu analýzy a časového kroku

1. Otvorte záložku `Úloha`.
2. V skupine *Nastavenia akustickej analýzy* nastavte **Typ analýzy** =
   `Transient`.
3. V skupine *Riešič v čase*:
   - zaškrtnite **Povoliť**;
   - **Počiatočný čas** = `0`;
   - **Veľkosť časového kroku** = `2e-5` s. Podľa `dt <= 1/(20*f_max)` to
     rozlíši frekvencie približne do `2.5 kHz`;
   - **Počet časových krokov** = `500`, čo dáva `10 ms` simulovaného času;
   - **Frekvencia výstupu** = `5`, čo zapíše 100 záznamov výsledkov.

Sieť skontrolujte podľa `h <= c/(10*f_max)`: pre `f_max = 2.5 kHz` a
`c = 344 m/s` by veľkosť prvku nemala presiahnuť asi `14 mm`.

### Krok 4 - vybudenie dutiny

Dve možnosti, vyberte jednu.

**A - pohyblivá stena.** Vyberte plochu, ktorá predstavuje zdroj, otvorte
záložku `Okrajové podmienky` a zaškrtnite **Rýchlosť**. Do tabuľky zložky zadajte
časový priebeh, napríklad krátky impulz:

| Čas `[s]` | Rýchlosť `[m/s]` |
|---|---|
| `0.0` | `0.0` |
| `0.0002` | `0.01` |
| `0.0004` | `0.0` |
| `0.01` | `0.0` |

**B - oblasť na začiatku pod tlakom.** Vyberte entitu, ktorá má začínať pod
tlakom, otvorte záložku `Počiatočné podmienky` a zaškrtnite **Rýchlostný
potenciál / s**. Zadajte `p0 / rho`; pre `p0 = 1 Pa` a `rho = 1.2 kg/m^3` je to
`0.833`.

Všetky ostatné plochy nechajte bez okrajovej podmienky, aby mala dutina tuhé
steny, alebo zaškrtnite **Pohltivá hranica** s koeficientom pohltivosti
zodpovedajúcim obloženiu steny, aby energia mohla unikať.

### Krok 5 - pridanie mikrofónu

1. `Úloha` -> `Definovať monitorovacie body`.
2. Pridajte bod v mieste, ktoré vás zaujíma.
3. Ako jeho premennú vyberte **Akustický tlak**.

### Krok 6 - riešenie

1. `Riešenie` -> `Spustiť riešič` (`Ctrl+R`).
2. Kontrola nastavenia pred spustením výpočtu ohlási chýbajúce materiály alebo
   okrajové podmienky.
3. Priebeh sledujte v `Výkaz` -> `Log súbor riešiča`. Každý časový krok vypíše
   konvergenciu CG a štatistiky každej akustickej premennej.

### Krok 7 - kontrola výsledkov

- Panel `Model`, záložka `Záznamy`: prechádzajte časovými záznamami alebo ich
  animujte.
- Záložka `Výsledky`: zobrazte na modeli **Akustický tlak** alebo **Hladinu
  akustického tlaku** a **Akustickú rýchlosť častíc** ako vektorové pole.
- `Výkaz` -> `Monitorovacie body`: priebeh tlaku v mikrofóne. Rýchlosť jeho
  doznievania dáva dobu dozvuku; jeho Fourierova transformácia dáva rezonancie
  dutiny.

### Riešenie problémov

| Príznak | Príčina |
|---|---|
| *"Transient acoustic analysis requires an enabled time solver"* | v skupine *Riešič v čase* nie je zaškrtnuté *Povoliť*, alebo typ analýzy má byť `Harmonic` |
| *"Acoustic solver requires a positive time step size"* | veľkosť časového kroku je nulová |
| *"No element with a valid speed of sound was found"* | žiadna entita nemá hustotu spolu s rýchlosťou zvuku alebo modulom pružnosti |
| Odozva nikdy nedoznie | nikde nie je tlmenie - očakávané pre dokonale tuhú bezstratovú dutinu; pridajte pohltivú hranicu alebo súčiniteľ akustického tlmenia |
| Vlnky pred čelom vlny | sieť alebo časový krok je na frekvenčný obsah príliš hrubý; zjemnite oboje |

---

## 4. Tutoriál - harmonická akustická analýza

**Cieľ.** Vypočítať frekvenčnú odozvu potrubia s bezodrazovým zakončením a mapu
hladiny akustického tlaku pri každej frekvencii.

### Krok 1 - výber úlohy

`Úloha` -> `Poradie krokov úloh` (`Ctrl+P`), pridajte krok, vyberte **Akustika**.

### Krok 2 - priradenie materiálu

Ako v prechodovom tutoriáli: **Hustota** `1.2 kg/m^3` a **Rýchlosť zvuku**
`344 m/s` na entite tekutiny.

Sieť dimenzujte pre **najvyššiu** frekvenciu rozmietania:
`h <= c / (10 * f_max)`.

### Krok 3 - nastavenie rozmietania

Otvorte záložku `Úloha` a v skupine *Nastavenia akustickej analýzy* nastavte:

| Pole | Hodnota |
|---|---|
| Typ analýzy | `Harmonic` |
| Prvá frekvencia | `50` Hz |
| Frekvenčný krok | `50` Hz |
| Počet frekvencií | `20` |
| Referenčný tlak | `2e-5` Pa |

Popis *Rozmietanie* potvrdí `50 - 1000 [Hz]`, t. j. 20 riešení a 20 záznamov
výsledkov.

Skupina *Riešič v čase* môže zostať povolená - harmonická analýza ju ignoruje a
zapíše do logu varovanie. Jej nastavenia sa zachovajú.

### Krok 4 - budenie potrubia

Vyberte vstupnú plochu, otvorte záložku `Okrajové podmienky` a zaškrtnite
**Rýchlosť**. Zadajte jeden riadok s rýchlosťou piesta, napríklad `0.01` `m/s`.
Hodnota sa interpretuje ako amplitúda harmonického budenia pri každej frekvencii
rozmietania.

### Krok 5 - zakončenie potrubia

Vyberte výstupnú plochu a zaškrtnite buď:

- **Pohltivá hranica** s **Koeficientom akustickej pohltivosti** = `1.0` pre
  dokonale neodrážajúce zakončenie; alebo
- **Akustická impedancia** s **Akustickou impedanciou** = `rho * c` = `413`
  `Pa*s/m`, čo je to isté vyjadrené inak.

Na modelovanie čiastočne odrážajúceho zakončenia použite menší koeficient
pohltivosti a na modelovanie uzavretého tuhého konca nechajte výstup bez
akejkoľvek podmienky.

Steny potrubia nepotrebujú žiadnu okrajovú podmienku - predvolene sú tuhé.

### Krok 6 - kontrola maticového riešiča

`Úloha` -> `Nastaviť maticový riešič úloh`, položka **GMRES**. Reštart sa zvýši
automaticky, ale ak log ukazuje stagnujúce reziduum, zvýšte počet vonkajších
iterácií a podľa potreby uvoľnite alebo sprísnite hodnotu konvergencie.

### Krok 7 - riešenie

`Riešenie` -> `Spustiť riešič` (`Ctrl+R`). Log ukazuje jeden blok na
frekvenciu:

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

### Krok 8 - kontrola výsledkov

- Panel `Model`, záložka `Záznamy`: jeden záznam na frekvenciu. Prechádzanie nimi
  je frekvenčným rozmietaním.
- Záložka `Výsledky`: zobrazte **Hladinu akustického tlaku** pre mapu v
  decibeloch, **Akustický tlak** pre amplitúdu a **Akustickú fázu**, aby ste
  videli čelá vĺn.
- **Akustická intenzita** ukazuje, kam tečie akustický výkon. V dobre zakončenom
  potrubí je rovnomerná a smeruje k výstupu; v rezonujúcej dutine je takmer
  nulová.
- Monitorovací bod na **Akustickom tlaku** zaznamená amplitúdu pri každej
  frekvencii, t. j. frekvenčnú prenosovú funkciu v danom mieste.

### Interpretácia výsledku

Pre priame potrubie budené rýchlosťou `v` a bezodrazovo zakončené je analytické
riešenie rovinnej vlny

```
|p| = rho * c * v        constant along the duct
|u| = v
|PHI| = v / k            with k = omega / c
I = 0.5 * |p| * |u|
```

(`|p|` je pozdĺž potrubia konštantné, `k = omega / c`.) Pre `rho = 1.2`,
`c = 344`, `v = 0.01` je to `|p| = 4.13 Pa`, hladina akustického tlaku
`103.3 dB` a intenzita `0.0206 W/m^2`. Odchýlky od konštantného `|p|` pozdĺž
potrubia znamenajú, že zakončenie odráža - buď zámerne, alebo preto, že
koeficient pohltivosti nie je `1`.

S tuhým zakončením sa to isté potrubie stane rezonančným. Amplitúda tlaku má
maximá pri štvrťvlnových rezonanciách `f = (2n+1) * c / (4L)` a fáza je
konštantná na `+/-90` stupňoch, s nulovou výslednou intenzitou.

### Riešenie problémov

| Príznak | Príčina |
|---|---|
| *"Harmonic acoustic analysis requires a positive frequency"* | prvá frekvencia je nulová a frekvenčný krok je nulový |
| Reziduum stagnuje, výsledky vyzerajú zašumene | GMRES neskonvergoval; zvýšte vonkajšie iterácie v nastavení maticového riešiča alebo zhrubnite sieť |
| Amplitúda pri jednej frekvencii prudko narastie | rozmietanie trafilo netlmenú rezonanciu; pridajte pohltivú hranicu alebo súčiniteľ akustického tlmenia |
| Tlak sa pozdĺž údajne bezodrazového potrubia výrazne mení | zakončenie odráža; skontrolujte koeficient pohltivosti alebo hodnotu impedancie |

---

## 5. Overenie

Jednotkový test `tst_solver_acoustic` (v `range-solver-lib/tests`, zostavuje sa
s `-DRANGE_BUILD_TESTS=ON`) overuje riešič voči riešeniam v uzavretom tvare na
jednorozmernom potrubí dĺžky `1 m` diskretizovanom 200 prútovými prvkami, s
predvolenými nastaveniami maticového riešiča.

**Harmonická, bezodrazové zakončenie** - `rho = 1.2 kg/m^3`, `c = 340 m/s`,
`v = 0.01 m/s`, `f = 500 Hz`:

| Veličina | Analyticky | Vypočítané |
|---|---|---|
| Amplitúda tlaku | `rho*c*v = 4.0800 Pa` | `4.0800 - 4.0804` v každom uzle |
| Amplitúda potenciálu | `v/k = 1.0823e-3 m^2/s` | do 1 % |
| Rýchlosť častíc | `0.01 m/s` | `0.009999 m/s` |
| Hladina akustického tlaku | `103.18 dB` | do `0.1 dB` |
| Intenzita | `0.0204 W/m^2` | do 2 % |
| Gradient fázy | `k * dx` | do 5 stupňov |

**Harmonická, tuhé zakončenie** - stojaté vlnenie dáva fázu presne `+/-90`
stupňov a nulovú výslednú intenzitu a maximálna amplitúda prevyšuje amplitúdu
postupnej vlny.

**Prechodová** - budené, bezodrazovo zakončené potrubie zostáva ohraničené,
Newmarkov stav sa zverejňuje, aby integrácia správne pokračovala medzi krokmi, a
vybudené pole je všade nenulové a konečné.

---

## 6. Obmedzenia

- **Žiadna väzba konštrukcie a akustiky.** Akustiku nemožno kombinovať s modálnou
  analýzou a neexistuje rozhranie tekutina-konštrukcia s `RSolverStress`.
  Kmitajúcu konštrukciu treba reprezentovať predpísanou okrajovou podmienkou
  *Rýchlosť*.
- **Iba lineárna akustika.** Malá amplitúda, pokojné prostredie. Žiadne stredné
  prúdenie, žiadne konvekčné javy, žiadne nelineárne šírenie.
- **Iba reálna impedancia.** Pohltivá a impedančná okrajová podmienka preberajú
  reálnu hodnotu. Komplexná, frekvenčne závislá povrchová impedancia nie je
  podporovaná.
- **Frekvenčne nezávislé materiálové údaje.** Rýchlosť zvuku a súčiniteľ tlmenia
  sú tabelované v závislosti od teploty, nie od frekvencie, takže počas
  rozmietania sú konštantné.
- **Hraničné členy používajú obyčajnú mieru prvku.** Pre jedno- alebo
  dvojrozmerný model, ktorého prierez alebo hrúbka nie je `1`, sa nimi zdrojový a
  impedančný člen neškálujú, kým objemové členy áno. Nesúladu sa vyhnete
  jednotkovým prierezom alebo úplným trojrozmerným modelom.
- **Harmonické budenie je reálne.** Predpísaná rýchlosť vstupuje ako reálna časť
  komplexného zaťaženia; zdroje s relatívnym fázovým posunom nemožno definovať.

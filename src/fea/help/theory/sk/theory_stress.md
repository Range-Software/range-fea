# Analýza napätia - Teoretický manuál a používateľská príručka

Tento dokument opisuje štrukturálny riešič (`RSolverStress`) v Range FEA, ktorý
rieši typy úloh **Analýza napätia** (*Stress analysis*) aj **Modálna analýza**
(*Modal analysis*): rovnice, ktoré rieši, význam každého vstupu, ktorý prijíma,
časti grafického používateľského rozhrania, ktoré ho ovládajú, a dva podrobne
rozpracované tutoriály.

**Obsah**

1. [Teoretický základ](#1-teoretický-základ)
2. [Grafické používateľské rozhranie](#2-grafické-používateľské-rozhranie)
3. [Tutoriál - statická analýza napätia](#3-tutoriál---statická-analýza-napätia)
4. [Tutoriál - modálna analýza](#4-tutoriál---modálna-analýza)
5. [Kontrola modelu](#5-kontrola-modelu)
6. [Obmedzenia](#6-obmedzenia)

---

## 1. Teoretický základ

### 1.1 Primárna neznáma - pole posunutí

Primárnou neznámou je **vektor posunutia** `u = (ux, uy, uz)` v každom uzle, v
metroch. Každý uzol nesie tri stupne voľnosti, takže sieť s `N` uzlami dáva
sústavu `3N` rovníc. Pretvorenia, napätia a reakčné sily sa po riešení získajú z
poľa posunutí.

### 1.2 Riadiace rovnice

Riešič implementuje **lineárnu izotropnú pružnosť pri malých pretvoreniach**.
Definujú ju tri vzťahy.

**Rovnováha** telesa pod objemovým zaťažením `b` a povrchovým zaťažením `t`:

```
div(sigma) + b = 0
```

Vzťah medzi **pretvorením a posunutím**, malé pretvorenia:

```
eps_xx = dux/dx        gamma_xy = dux/dy + duy/dx
eps_yy = duy/dy        gamma_yz = duy/dz + duz/dy
eps_zz = duz/dz        gamma_xz = dux/dz + duz/dx
```

**Konštitutívny** vzťah s tepelným členom:

```
sigma = D * ( eps - alpha * dT )
```

| Symbol | Význam | Jednotky | Zdroj |
|---|---|---|---|
| `u` | posunutie | `m` | riešené |
| `sigma` | Cauchyho napätie | `Pa` | odvodené |
| `eps` | inžinierske pretvorenie | `-` | odvodené |
| `E` | modul pružnosti | `Pa` | vlastnosť materiálu |
| `v` | Poissonovo číslo | `-` | vlastnosť materiálu |
| `rho` | hustota | `kg/m^3` | vlastnosť materiálu |
| `alpha` | súčiniteľ teplotnej rozťažnosti | `1/K` | vlastnosť materiálu |
| `dT` | nárast teploty nad stav bez napätia | `K` | pozri 1.5 |

Všetky štyri vlastnosti materiálu sú **povinné** - entita, ktorej niektorá z
nich chýba, sa nerieši. Dva Rayleighove koeficienty tlmenia dynamickej analýzy
(pozri 1.7) sú voliteľné.

### 1.3 Matica pružnosti

**Objemové prvky** používajú úplnú trojrozmernú izotropnú maticu pružnosti vo
Voigtovom zápise, s vektorom napätia usporiadaným
`[sigma_xx, sigma_yy, sigma_zz, tau_yz, tau_xz, tau_xy]`:

```
        E                | 1-v   v    v      0        0        0     |
D = -----------------  * |  v   1-v   v      0        0        0     |
    (1+v)*(1-2v)         |  v    v   1-v     0        0        0     |
                         |  0    0    0   (1-2v)/2    0        0     |
                         |  0    0    0      0     (1-2v)/2    0     |
                         |  0    0    0      0        0     (1-2v)/2 |
```

**Plošné prvky** používajú dvojrozmerný tvar **rovinného pretvorenia** s
vektorom napätia `[sigma_xx, sigma_yy, tau_xy]`, vyhodnotený v lokálnej rovine
prvku a otočený späť do globálnych súradníc:

```
        E                | 1-v   v      0     |
D = -----------------  * |  v   1-v     0     |
    (1+v)*(1-2v)         |  0    0   (1-2v)/2 |
```

Toto je rovinné **pretvorenie**, nie rovinná napätosť. Tenká doska zaťažená vo
vlastnej rovine preto vyjde tuhšia, než by predpovedala idealizácia rovinnej
napätosti.

**Čiarové prvky** sa spracúvajú ako **prúty**: zostavuje sa iba osová tuhosť
`E * A` v smere prvku, kde `A` je prierez čiary. Neexistuje žiadna ohybová,
šmyková ani torzná tuhosť, a to ani pre typy prvkov s názvom "beam" (nosník).

### 1.4 Diskretizácia metódou konečných prvkov

Slabá formulácia rovnováhy dáva pre každý prvok

```
K = INT( B^T * D * B )        stiffness matrix
M = INT( rho * N^T * N )      consistent mass matrix
f = load vector, see 1.5
```

(maticu tuhosti, konzistentnú maticu hmotnosti a vektor zaťaženia, pozri 1.5),
kde `B` je matica vzťahu medzi pretvorením a posunutím zostavená z derivácií
tvarových funkcií. Každá matica prvku s `n` uzlami má rozmer `3n x 3n`, so
stupňami voľnosti usporiadanými striedavo ako
`[ux1, uy1, uz1, ux2, uy2, uz2, ...]`.

**Miery prvkov.** Ako inde v Range FEA, entity nižšej dimenzie dostávajú
geometrickú mieru, aby mohli reprezentovať skutočné telesá:

| Entita | Miera | Kde sa nastavuje |
|---|---|---|
| Objem | 1 | - |
| Plocha | hrúbka plochy | geometrické vlastnosti entity |
| Čiara | prierez čiary | geometrické vlastnosti entity |
| Bod | objem bodu | geometrické vlastnosti entity |

Plocha s nulovou hrúbkou alebo čiara s nulovým prierezom neprispieva žiadnou
tuhosťou ani hmotnosťou - stále však môže niesť zaťaženia, čo je presne to, čo
chceme pre zaťaženú stenu telesa so sieťou.

Matica hmotnosti sa zostavuje iba vtedy, keď je potrebná, teda pre dynamickú
prechodovú alebo modálnu analýzu.

### 1.5 Zaťaženia, okrajové a environmentálne podmienky

#### Okrajové podmienky

| Okrajová podmienka | Typ | Aplikuje sa na | Zložky |
|---|---|---|---|
| Posunutie (*Displacement*) | explicitná (Dirichletova) | bod, čiara, plocha, objem | Posunutie v X, Y, Z `[m]` |
| Normálové posunutie (*Normal displacement*) | explicitná, lokálny systém | plocha | Posunutie `[m]` |
| Valčekové posunutie (*Roller displacement*) | explicitná, lokálny systém | bod, čiara, plocha | Posunutie `[m]` |
| Sila (*Force*) | prirodzená | bod, čiara, plocha | Sila v X, Y, Z `[N]` |
| Sila (na jednotku plochy) (*Force (unit area)*) | prirodzená | plocha | Sila na jednotku plochy v X, Y, Z `[N/m^2]` |
| Tlak (*Pressure*) | prirodzená | plocha | Tlak `[Pa]` |
| Hmotnosť (*Weight*) | prirodzená | bod, čiara, plocha | Hmotnosť `[kg]` |

**Posunutie** je podmienka s **voliteľnými** zložkami: každú z jej troch zložiek
možno zapnúť alebo vypnúť samostatne. Zapnutá zložka obmedzí daný globálny smer
každého uzla entity na zadanú hodnotu; vypnutá zložka nechá tento smer úplne
voľný. Vypnutím *Posunutia v Y* a *Posunutia v Z* tak vznikne podpora, ktorá
drží X a nechá uzol kĺzať v rovine YZ.

Ak voľné smery nie sú globálne, ale ide o dotykovú rovinu zakrivenej plochy,
použite namiesto toho *Valčekové posunutie*.

**Normálové posunutie** a **Valčekové posunutie** pracujú v **lokálnom
systéme** zostavenom pre každý obmedzený uzol. Predvolene systém sleduje
geometriu:

- na **ploche** je lokálna os X spriemerovaná normála susedných plošných prvkov;
- na **čiare** sa lokálny systém zostaví zo smeru prvku;
- na **bode** nie je geometria, ktorú by bolo možné sledovať, takže sa vždy
  použije smer zadaný s okrajovou podmienkou.

Zaškrtnutie **Použiť zadaný lokálny smer** to pre plochu alebo čiaru prepíše a
lokálnu os X prevezme zo smeru zadaného pod ním. Použite ho, keď smer, ktorý
chcete obmedziť, nie je ten, ktorý náhodou dáva sieť - napríklad valček, ktorý
má kĺzať po fazetovej ploche pozdĺž globálnej osi.

Väzby z rôznych entít sa v uzle, ktorý zdieľajú, **kombinujú**. Riešič zozbiera
každú väzbu posunutia pôsobiacu na uzol, každú ako výrok `d . u = v` o jednom
smere, a Gramovou-Schmidtovou ortogonalizáciou zredukuje kolekciu najviac na tri
navzájom kolmé držané smery, pričom predpísané hodnoty prenáša cez tie isté
operácie. Tieto držané smery sa stanú vedúcimi osami systému uzla; čo zostane,
systém doplní a zostane voľné. Uzol tak môže byť v jednom smere držaný
globálnym *Posunutím*, v inom *Valčekovým posunutím* na naklonenej stene a v
treťom zostať voľný - tieto dve podmienky sa navzájom neprepisujú a globálne
formulovaná zložka sa nikdy potichu neprečíta v lokálnom systéme niekoho iného.

Dve entity, ktoré predpisujú **rôzne hodnoty v rovnakom smere**, nemožno splniť
obe. Riešič to pri ortogonalizácii zistí a zastaví sa s chybou, ktorá uvádza
uzol, namiesto toho, aby to rozhodol v prospech podmienky, ktorá bola náhodou
načítaná posledná.

Matice prvkov a vektory zaťaženia obmedzeného uzla sa otočia do tohto systému,
aplikuje sa väzba a výsledné posunutie sa otočí späť. V tomto systéme:

- **Normálové posunutie** zablokuje všetky tri lokálne zložky a predpísaný
  skalár sa aplikuje pozdĺž lokálnej normály. Uzol sa posunie presne o zadanú
  hodnotu pozdĺž normály a inak je fixovaný.
- **Valčekové posunutie** zablokuje na ploche alebo bode iba lokálnu normálovú
  zložku, takže uzol môže voľne kĺzať v dotykovej rovine. Na čiarovej entite
  namiesto toho zablokuje dve zložky kolmé na čiaru, takže uzol môže voľne kĺzať
  pozdĺž čiary.

**Sila** je celková sila v newtonoch rozložená na entitu, ktorej je priradená -
pre čiaru sa delí dĺžkou entity, pre plochu plochou entity a pre bodovú entitu
počtom bodov. Zjemnenie siete preto celkové zaťaženie nemení.

**Sila (na jednotku plochy)** je povrchové zaťaženie v `N/m^2` aplikované v
zadanom globálnom smere, zintegrované cez plochu prvku.

**Tlak** pôsobí v smere **normály** plochy. Riešič pre každú hraničnú plochu
určí, či jej normála smeruje do susedného počítaného objemového prvku, a podľa
toho otočí znamienko, takže kladný tlak vždy tlačí na teleso. Plocha, ktorá
neohraničuje žiadny objemový prvok, sa nikdy neklasifikuje ako smerujúca
dovnútra, takže tam zaťaženie pôsobí v smere **zápornej** normály plochy - v
takom prípade skontrolujte smer výsledného zaťaženia.

**Hmotnosť** je hmotnosť v kilogramoch pripojená k entite. Vynásobí sa
gravitačným zrýchlením environmentálnej podmienky a rozloží sa na entitu
rovnakým spôsobom ako Sila. Použite ju pre nemodelované pripojené hmoty.

#### Environmentálne podmienky

| Environmentálna podmienka | Zložky | Účinok |
|---|---|---|
| Gravitačné zrýchlenie (*Gravitational acceleration*) | Gravitačné zrýchlenie v X, Y, Z `[m/s^2]` | vlastná tiaž a okrajová podmienka Hmotnosť |
| Teplota (*Temperature*) | Teplota `[K]` | referenčná teplota stavu bez napätia |

**Vlastná tiaž** sa generuje automaticky pre každú entitu, ktorá má hustotu aj
geometrickú mieru, ako `rho * g` zintegrované cez prvok. Objaví sa, len čo je
priradená environmentálna podmienka gravitačného zrýchlenia; predvolená hodnota
je `-9.80665 m/s^2` v smere Z.

**Tepelné zaťaženie.** Rozdiel teplôt, ktorý poháňa člen teplotnej rozťažnosti,
je

```
dT = element temperature - environment temperature
```

(teplota prvku mínus teplota okolia). **Teplota okolia** pochádza z
environmentálnej podmienky *Teplota* a je referenčným stavom bez napätia.
**Teplota prvku** pochádza z riešiča prestupu tepla, ktorý beží skôr v tom istom
poradí krokov úloh, a odovzdáva sa cez zdieľané údaje riešičov. Spustenie
*Prestupu tepla* a potom *Analýzy napätia* v jednom poradí krokov preto dáva
analýzu tepelného napätia bez ďalšieho nastavovania. Bez tepelného riešiča
zostáva teplota prvku na predvolenej hodnote a tepelný člen je iba rozdielom voči
environmentálnej podmienke.

#### Počiatočné podmienky

| Počiatočná podmienka | Účinok |
|---|---|
| Posunutie | počiatočné pole posunutí dynamickej prechodovej analýzy |
| Rýchlosť konštrukcie | počiatočné pole rýchlostí dynamickej prechodovej analýzy |
| Tlak | počiatočné plošné tlakové zaťaženie |

Počiatočné podmienky sa čítajú iba pri prvom výpočte prechodovej analýzy.
Dynamická analýza bez nich začína z pokoja. Viazané smery nedostávajú vlastnú
počiatočnú rýchlosť, sledujú predpísané posunutie.

#### Väzba s riešičom prúdenia

Riešič získa výsledok **Tlak** z predchádzajúceho riešenia a použije ho ako
plošné zaťaženie všade, kde nie je priradená explicitná tlaková okrajová
podmienka. Krok prúdenia, po ktorom v tom istom poradí krokov nasleduje krok
napätia, preto automaticky prenesie tlak tekutiny na konštrukciu.

Opačným smerom každý neštrukturálny riešič pred spustením posunie sieť o uložený
výsledok **Posunutie** a potom ju vráti späť, takže riešenie prúdenia alebo
tepla, ktoré nasleduje po riešení napätia, vidí deformovaný tvar.

### 1.6 Statická analýza

Pri vypnutom časovom riešiči a type úlohy *Analýza napätia* riešič zostaví a
vyrieši

```
K * u = f
```

`K` je symetrická pozitívne definitná, len čo väzby posunutí odstránia módy
pohybu tuhého telesa, takže sa rieši **metódou združených gradientov** s
Jacobiho predpodmienením a veľkosťou bloku 3.

Ak je model nedostatočne uložený, matica je singulárna a riešenie neskonverguje.
Každý statický model potrebuje dosť väzieb na potlačenie všetkých šiestich módov
pohybu tuhého telesa - troch posunutí a troch natočení.

### 1.7 Prechodová analýza

Pri zapnutom časovom riešiči vyberá *Nastavenie prechodovej analýzy napätia*
jeden z dvoch typov analýzy.

#### Dynamická analýza

Predvolená. Riešič integruje rovnice lineárnej elastodynamiky

```
M * d2u/dt2 + C * du/dt + K * u = f(t)
```

takže sa reprodukuje zotrvačnosť, napäťové vlny, kmitanie, dynamické zosilnenie
aj rezonancia.

**Tlmenie.** `C` je Rayleighovo tlmenie, zostavené prvok po prvku z dvoch
voliteľných vlastností materiálu,

```
C = alpha_M * M + beta_K * K
```

*Rayleighov koeficient hmotnostného tlmenia* `alpha_M` `[1/s]` tlmí nízke
frekvencie, *Rayleighov koeficient tuhostného tlmenia* `beta_K` `[s]` vysoké.
Pomerné tlmenie módu s kruhovou frekvenciou `omega` je

```
zeta(omega) = alpha_M / (2*omega) + beta_K * omega / 2
```

takže cieľové pomerné tlmenie `zeta` pri dvoch frekvenciách `omega_1` a
`omega_2` sa dosiahne s

```
alpha_M = 2*zeta * omega_1*omega_2 / (omega_1 + omega_2)
beta_K  = 2*zeta / (omega_1 + omega_2)
```

Obe frekvencie vezmite z modálnej analýzy, okolo pásma, ktoré vás zaujíma.
Typické pomerné tlmenie konštrukcií je 0.01 až 0.05. Bez týchto dvoch
vlastností nie je žiadne fyzikálne tlmenie.

**Časová integrácia.** Newmarkova metóda s úpravou Hilber-Hughes-Taylor (HHT).
S posunutím `u`, rýchlosťou `v` a zrýchlením `a` na začiatku kroku každý krok
rieši

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

a potom aktualizuje

```
a_n+1 = a0*(u_n+1 - u_n) - a2*v_n - a3*a_n
v_n+1 = v_n + dt*( (1-gamma)*a_n + gamma*a_n+1 )
```

Efektívna matica je symetrická pozitívne definitná, takže sa rieši tou istou
metódou CG ako statická analýza.

- S **HHT alpha = 0** (predvolené) je schéma čistý Newmark s `gamma = 1/2` a
  `beta = max(theta/2, 1/4)`, kde `theta` je koeficient aproximácie časového
  pochodu (`1` spätná, `0.5` centrálna, `0` dopredná). Centrálna aj dopredná
  dávajú schému priemerného zrýchlenia - bezpodmienečne stabilnú, druhého rádu
  presnosti a bez numerického tlmenia. Spätná dáva `beta = 1/2`, ktorá je tiež
  bezpodmienečne stabilná, ale predlžuje periódy.
- S **HHT alpha** medzi `-1/3` a `0` je `gamma = 1/2 - alpha` a
  `beta = (1 - alpha)^2 / 4`. Schéma zostáva bezpodmienečne stabilná a druhého
  rádu presnosti a tlmí vysoké frekvencie, ktoré hrubá sieť zachytáva zle,
  pričom nízke módy takmer nemení. `-0.05` až `-0.1` je bežná voľba, keď je
  odozva znečistená vysokofrekvenčným šumom.

**Počiatočné zrýchlenie.** V prvom kroku sa zrýchlenie rieši z rovnováhy,
`M*a = f - K*u - C*v`, takže zaťaženie aplikované na začiatku vyvolá správne
počiatočné zrýchlenie. Smery bez hmotnosti dostanú nulu.

**Sústredená hmotnosť.** Voľba *Sústredená matica hmotnosti* nahradí
konzistentnú hmotnosť diagonálnou s rovnakou celkovou hmotnosťou, škálovaním
diagonály konzistentnej matice (HRZ sústredenie, ktoré zostáva kladné aj pre
kvadratické prvky). Mierne znižuje vlastné frekvencie a dáva hladšiu odozvu za
ostrými čelami vĺn.

**Voľba časového kroku.** Schéma je stabilná pre ľubovoľné `dt`, ale presnosť
vyžaduje približne 20 alebo viac krokov na periódu najvyššieho módu, ktorý
chcete zachytiť. Pre vlnu prechádzajúcu sieťou je dobrým začiatkom krok blízky
času, ktorý vlna potrebuje na prechod jedným prvkom (`dt ~ h/c`,
`c = sqrt(E/rho)`).

**Výsledky.** Okrem posunutia a napätia ukladá dynamická analýza uzlové vektory
*Rýchlosť konštrukcie* a *Zrýchlenie konštrukcie*. Tie zároveň prenášajú stav do
ďalšieho časového kroku a cez reštart riešiča.

#### Kvázistatická analýza

Každý časový krok je samostatné statické riešenie

```
K * u(t) = f(t)
```

so zanedbanou zotrvačnosťou a tlmením. Použite ju pre priebeh zaťaženia
aplikovaný pomaly v porovnaní s periódami konštrukcie, kde záleží iba na
postupnosti rovnovážnych stavov. Nepotrebuje hustotu ani obmedzenie časového
kroku.

### 1.8 Modálna analýza

Pri type úlohy *Modálna analýza* riešič zostaví `K` a `M` a rieši zovšeobecnenú
úlohu vlastných hodnôt

```
K * phi = lambda * M * phi        with  lambda = omega^2
```

(s `lambda = omega^2`). Každý vlastný pár je **mód**: `phi` je vlastný tvar,
ukladaný a zobrazovaný ako pole posunutí, a `lambda` je vlastná hodnota, z ktorej
vyplýva vlastná frekvencia ako

```
omega = sqrt(lambda)      [rad/s]
f     = sqrt(lambda) / (2*pi)      [Hz]
```

Tento prepočet robí riešič sám: nastavenia modálnej analýzy a 3D pohľad uvádzajú
vlastnú frekvenciu v Hz, kým log riešiča vypisuje popri nej aj príslušnú vlastnú
hodnotu. Mód, ktorý iterácia vlastných hodnôt nedokázala určiť, sa uvádza ako
`0` Hz s varovaním v logu.

K dispozícii sú dve metódy, vyberané nastavením modálnej metódy:

| Modálna metóda | Riešič vlastných hodnôt | Extrahuje |
|---|---|---|
| `Multiple modes` (viac módov) | podpriestorová iterácia s Rayleighovou-Ritzovou projekciou | požadovaný počet módov |
| `Only most dominant mode` (iba najdominantnejší mód) | inverzná mocninová iterácia s Rayleighovým podielom | jediný mód |

Obe metódy iterujú na `K^-1 * M`, takže konvergujú k **najnižším** frekvenciám -
čo sú tie, na ktorých záleží. Každý krok je lineárne riešenie s `K` s
nastaveniami metódy združených gradientov z nastavenia maticového riešiča, takže
náklady modálnej analýzy sú zhruba náklady statického riešenia vynásobené počtom
podpriestorových vektorov a iterácií.

Podpriestorová iterácia nesie blok vektorov, premietne `K` a `M` do podpriestoru,
ktorý tvoria, a výslednú malú hustú úlohu vlastných hodnôt vyrieši presne.
Niekoľko vektorov navyše nad požadovaný počet módov sa nesie so sebou, pretože
výrazne zrýchľujú konvergenciu požadovaných módov. Pre `p` požadovaných módov
obsahuje blok `min(2p, p+8)` vektorov, čo je obvyklý kompromis medzi rýchlosťou
konvergencie a nákladmi na iteráciu. Vlastné hodnoty vychádzajú priamo ako
`lambda` a sú zoradené vzostupne, takže **mód 0 je základný mód**.

**Práve sem ide čas výpočtu.** Jedna podpriestorová iterácia vykoná jedno úplné
lineárne riešenie na každý vektor bloku, takže požiadavka na 10 módov stojí 18
riešení na iteráciu a požiadavka na 100 stojí 108. Iterácia sa zastaví skôr iba
vtedy, keď sa **každá** požadovaná vlastná hodnota ustáli v rámci hodnoty
konvergencie, a najvyšší požadovaný mód sa ustáli vždy ako posledný - takže
veľký počet módov robí každú iteráciu drahou a zároveň skoré zastavenie
nepravdepodobným. Žiadajte iba módy, ktoré si naozaj chcete pozrieť.

Ak sa požadované módy do poslednej iterácie neustália, riešič to uvedie v logu
a ohlási zmenu, ktorú dosiahol, namiesto toho, aby neskonvergované hodnoty
predložil bez komentára. Najnižšie módy takého výpočtu sú zvyčajne stále v
poriadku; najvyššie nie.

Pre modálnu analýzu sa matica hmotnosti zostavuje vždy, takže hustota musí byť
prítomná na každej entite, ktorá sa jej zúčastňuje.

**Predpätá modálna analýza.** Pred zostavením matíc modálna analýza posunie každý
uzol o výsledok posunutia z predchádzajúceho riešenia a potom sieť obnoví.
Spustenie *Analýzy napätia* a potom *Modálnej analýzy* v jednom poradí krokov
preto dá módy **zaťaženej, deformovanej** konštrukcie namiesto nedeformovanej.
Uložený vlastný tvar je vlastný vektor pridaný k tomuto počiatočnému posunutiu.

Aktualizuje sa iba geometria - matica tuhosti je stále lineárna pružná matica
vyhodnotená na posunutej sieti. Zachytí to zmenu tvaru, nie geometrické
vystuženie napnutého prvku.

Modálna analýza je **výlučná**: nemožno ju kombinovať so žiadnou inou fyzikou
okrem predchádzajúcej analýzy napätia.

### 1.9 Odvodené výsledky

Po riešení sa napätia prvkov získajú z poľa posunutí ako priemer cez integračné
body výrazu

```
sigma = D * ( B * u  -  eps_th )
```

s teplotným pretvorením `eps_th = alpha * dT * [1, 1, 1, 0, 0, 0]` pre objemové
prvky. Plošné prvky sú v rovinnom pretvorení: rozťažnosť kolmo na ich rovinu je
potlačená, čo zvyšuje teplotné pretvorenie v rovine na
`eps_th = (1+v) * alpha * dT * [1, 1, 0]`. To isté teplotné pretvorenie vstupuje
do tepelného zaťaženia `f_th = INT( B^T * D * eps_th )`. Čiarové prvky uvádzajú
`E * (eps - alpha * dT)`.

Zo šiestich zložiek riešič vytvorí dva invarianty a kvadraticky ich spojí do
napätia podľa von Misesa:

```
normal part  QN = sqrt( sx^2 + sy^2 + sz^2 - (sx*sy + sy*sz + sz*sx) )
shear part   QS = sqrt( 3 * (tyz^2 + txz^2 + txy^2) )
von Mises    QVM = sqrt( QN^2 + QS^2 )
```

(normálová časť `QN`, šmyková časť `QS`, napätie podľa von Misesa `QVM`), čo je
klasická definícia

```
QVM = sqrt( sx^2 + sy^2 + sz^2 - (sx*sy + sy*sz + sz*sx) + 3*(tyz^2 + txz^2 + txy^2) )
```

Pre plošné prvky sa používajú dvojrozmerné ekvivalenty, `QN` z dvoch normálových
napätí v rovine a `QS = sqrt(3 * tau_xy^2)`. Pre čiarové prvky sa uvádza iba
osová hodnota a šmyková časť je nulová.

**Uzlové sily** sa získavajú prvok po prvku ako vnútorná pružná sila

```
f = K * u - f_th
```

a sčítavajú sa v uzloch. V obmedzenom uzle je to reakčná sila; v nezaťaženom
voľnom uzle by mala pri skonvergovanom statickom riešení vyjsť takmer nulová,
vďaka čomu je výsledok Sila užitočnou kontrolou rezidua. Teleso, ktoré sa môže
voľne tepelne rozťahovať, preto nevykazuje sily ani napätie.

| Výsledok | Aplikuje sa na | Význam |
|---|---|---|
| Posunutie `[m]` | uzol | vektor posunutia, alebo vlastný tvar v modálnej analýze |
| Napätie podľa von Misesa `[Pa]` | prvok | `sqrt(QN^2 + QS^2)` ako vyššie |
| Normálové napätie `[Pa]` | prvok | `QN` |
| Šmykové napätie `[Pa]` | prvok | `QS` |
| Normálové napätie v X, Y, Z `[Pa]` | prvok | `sigma_xx`, `sigma_yy`, `sigma_zz` |
| Šmykové napätie v YZ, XZ, XY `[Pa]` | prvok | `tau_yz`, `tau_xz`, `tau_xy` |
| Sila `[N]` | uzol | získaná uzlová sila, reakcia v obmedzených uzloch |

Šesť zložiek napätia sa pre objemové prvky uvádza v **globálnych** súradniciach.
Plošné a čiarové prvky vyhodnocujú svoje napätie vo vlastnom lokálnom systéme
prvku, takže ich zložky sa uvádzajú v tomto systéme: plocha vyplní zložky v
rovine `sigma_xx`, `sigma_yy` a `tau_xy` a čiara iba osovú `sigma_xx`.

### 1.10 Statická, prechodová alebo modálna?

| | Statická | Dynamická prechodová | Kvázistatická prechodová | Modálna |
|---|---|---|---|---|
| Rieši | `K*u = f` | `M*u'' + C*u' + K*u = f(t)` v čase | `K*u = f(t)` v každom čase | `K*phi = omega^2*M*phi` |
| Odpovedá na | "o koľko sa prehne a ako vysoko je namáhaná" | "ako reaguje na rýchlo sa meniace zaťaženie" | "ako sleduje pomaly sa meniace zaťaženie" | "pri akých frekvenciách chce kmitať a v akom tvare" |
| Potrebuje hmotnosť | nie | áno | nie | áno |
| Potrebuje väzby proti pohybu tuhého telesa | áno | áno | áno | áno |
| Záznamy | jeden | jeden na časový krok | jeden na časový krok | jeden na extrahovaný mód |

**Statickú analýzu použite na** kontroly pevnosti a tuhosti, limity priehybu,
reakčné sily, vlastnú tiaž, tlakové nádoby, tepelné napätie. Je predvolená a
pokrýva väčšinu práce.

**Dynamickú prechodovú analýzu použite na** nárazy, náhle aplikované alebo
uvoľnené zaťaženia, priebehy rázov a vibrácií, šírenie vĺn a odozvu na
harmonické budenie v blízkosti rezonancie - všade, kde záleží na zotrvačnosti.

**Kvázistatickú prechodovú analýzu použite na** zaťaženie, ktoré sa mení v čase
dostatočne pomaly na to, aby ho konštrukcia sledovala v rovnováhe.

**Modálnu analýzu použite na** vlastné frekvencie a vlastné tvary, vyhnutie sa
rezonancii so známym budením, posúdenie, či je konštrukcia dynamicky dostatočne
tuhá, a na kontrolu neúmyselných mechanizmov - takmer nulová vlastná hodnota
zvyčajne znamená chýbajúcu väzbu alebo odpojenú časť.

---

## 2. Grafické používateľské rozhranie

Všetko podstatné sa nachádza v paneli **Riešič** (záložky `Úloha`,
`Okrajové podmienky`, `Počiatočné podmienky`, `Environmentálne podmienky`,
`Materiál`, `Výsledky`), v menu **Úloha** a v paneli **Model**.

### 2.1 Výber úlohy

`Úloha` -> `Poradie krokov úloh` (`Ctrl+P`) otvorí dialóg poradia krokov úloh.
Pridajte krok a vyberte buď:

- **Analýza napätia** - "Analýza napätia a lineárnej elastodynamiky vrátane
  modálnej analýzy";
- **Modálna analýza** - "Modálna analýza napätia".

Analýza napätia sa voľne kombinuje s inými fyzikami. Dva užitočné reťazce sú
*Prestup tepla* a potom *Analýza napätia* pre tepelné napätie a *Nestlačiteľné
viskózne prúdenie* a potom *Analýza napätia* pre tlakové zaťaženie.

Modálna analýza vylučuje každý iný typ úlohy okrem predchádzajúcej analýzy
napätia, ktorá z nej urobí predpätú modálnu analýzu.

### 2.2 Záložka Úloha

**Riešič v čase** - zobrazuje sa pre analýzu napätia, keďže je to časovo
závislý typ úlohy. Pre statickú analýzu ho nechajte vypnutý.

| Pole | Význam |
|---|---|
| Povoliť | zapína a vypína prechodový pochod |
| Aproximácia | nastavuje Newmarkovo beta dynamickej analýzy, pozri 1.7; odporúčaná je centrálna |
| Počiatočný čas | prvá hodnota času v sekundách; pri reštarte riešiča sa ignoruje |
| Konečný čas | `počiatočný čas + veľkosť časového kroku * počet časových krokov`, iba na čítanie |
| Veľkosť časového kroku | `dt` v sekundách |
| Počet časových krokov | koľko krokov sa má vypočítať |
| Frekvencia výstupu | zapíše súbor výsledkov každých N krokov; `0` zapíše iba posledný krok |

**Nastavenie prechodovej analýzy napätia** - zobrazuje sa pre analýzu napätia a je aktívne iba vtedy,
keď je časový riešič zapnutý.

| Pole | Význam | Predvolené |
|---|---|---|
| Typ analýzy | *Dynamická* (so zotrvačnosťou) alebo *Kvázistatická* (statické riešenie v každom časovom kroku) | Dynamická |
| Numerické tlmenie (HHT alpha) | od `-1/3` do `0`; záporné hodnoty tlmia vysokofrekvenčný šum | 0 |
| Sústredená matica hmotnosti | použije diagonálnu maticu hmotnosti namiesto konzistentnej | vypnuté |

Posledné dve platia iba pre dynamickú analýzu, inak sú neaktívne.

**Nastavenia modálnej analýzy** - zobrazuje sa, keď je typ úlohy *Modálna
analýza*.

| Pole | Význam | Predvolené |
|---|---|---|
| Modálna metóda | `Only most dominant mode` alebo `Multiple modes` | `Multiple modes` |
| Iterácie | iterácie riešiča vlastných hodnôt; pre metódu dominantného módu nedostupné | 100 |
| Extrahovať módy | koľko módov sa má extrahovať; pre metódu dominantného módu nedostupné | 10 |
| Konvergenčná hodnota | prah konvergencie riešiča vlastných hodnôt | 1.0e-9 |

Oba počty sú nezávislé: iterácie spresňujú celú sadu módov spoločne, nevytvárajú
ich po jednom.

**Extrahovať módy je nastavenie, ktoré rozhoduje o tom, ako dlho výpočet trvá.**
Každý mód pridá lineárne riešenie do každej iterácie a výpočet sa môže zastaviť
skôr až vtedy, keď sa ustália všetky požadované módy, takže náklady rastú
rýchlejšie ako počet. Desať je rozumný začiatok a je predvolenou hodnotou; sto je
výpočet iného rádu a na veľkej sieti môže trvať hodiny. Zvyšujte ho, iba keď
viete, že vyššie módy potrebujete.

Ak výpočet trvá dlhšie, než očakávate, log riešiča uvádza podpriestorový vektor,
na ktorom práve pracuje, takže priebeh v rámci iterácie je viditeľný, a po každej
iterácii vypíše dosiahnutú mieru konvergencie vedľa požadovanej hodnoty.

### 2.3 Záložka Materiál

Záložka `Materiál` uvádza materiál priradený vybranej entite. Analýza napätia
vyžaduje **všetky štyri** tieto vlastnosti:

| Vlastnosť | Jednotky | Poznámka |
|---|---|---|
| Modul pružnosti | `Pa` | `E`, asi `2.1e11` pre oceľ |
| Poissonovo číslo | `-` | `v`, asi `0.3` pre oceľ; musí zostať pod `0.5` |
| Hustota | `kg/m^3` | potrebná pre vlastnú tiaž, dynamickú a modálnu analýzu |
| Súčiniteľ teplotnej rozťažnosti | `1/K` | ak nie je tepelné zaťaženie, nastavte `0` |

Entita, ktorej niektorá z nich chýba, sa nerieši a kontrola nastavenia na to
upozorní pred spustením výpočtu.

Ďalšie dve vlastnosti sú voliteľné a číta ich iba dynamická prechodová analýza:

| Vlastnosť | Jednotky | Poznámka |
|---|---|---|
| Rayleighov koeficient hmotnostného tlmenia | `1/s` | `alpha_M`, pozri 1.7; ak chýba, nula |
| Rayleighov koeficient tuhostného tlmenia | `s` | `beta_K`, pozri 1.7; ak chýba, nula | Každá vlastnosť je tabuľkou v závislosti od
teploty, takže teplotne závislý modul sa rešpektuje, keď ako prvý beží tepelný
riešič.

### 2.4 Záložka Okrajové podmienky

Záložka je rozdelená na zoznam dostupných okrajových podmienok hore a editor
vybranej podmienky dole. Najprv vyberte entitu v strome `Model`; zoznam potom
ponúkne podmienky platné pre daný typ entity a pre vybranú úlohu. Zaškrtnutím
podmienku priradíte a jej zložky potom upravíte v dolnom strome.

Ponúkajú sa podmienky z časti 1.5. Poznámky k ich použitiu:

- **Posunutie** zobrazuje vedľa každej z troch zložiek zaškrtávacie pole.
  Obmedzené sú iba zaškrtnuté zložky, takže stenu možno držať v X a nechať voľnú
  v Y a Z. Pole hodnoty nezaškrtnutej zložky je sivé.
- **Normálové posunutie** a **Valčekové posunutie** preberajú lokálny systém
  predvolene z geometrie - zo spriemerovaných normál prvkov plochy, zo smeru
  prvku čiary. Na **bodovej** entite nie je geometria, ktorú by bolo možné
  sledovať, takže editor *Lokálny smer* sa vždy zobrazuje a vždy používa.
- Na ploche alebo čiare strom podmienky zobrazuje zaškrtávacie pole **Použiť
  zadaný lokálny smer**. Zaškrtnite ho, aby sa zobrazil editor *Lokálny smer* a
  systém sa prevzal zo smeru, ktorý zadáte, namiesto zo siete. Šípky smeru
  kreslené v 3D pohľade sledujú to z dvojice, čo práve platí. Ak sa plošná väzba
  s nezaškrtnutým poľom správa zvláštne, overte, že normály plochy sú konzistentné,
  a zosynchronizujte ich nástrojmi geometrie.
- Väzby z viacerých entít, ktoré sa stretávajú v jednom uzle, sa kombinujú, takže
  stena môže byť zároveň držaná v globálnom smere *Posunutím* a valčekovo uložená
  na naklonenej rovine. Ak dve z nich žiadajú rôzne hodnoty v rovnakom smere,
  výpočet sa zastaví s chybou, ktorá uvádza uzol.
- **Tlak** a **Sila (na jednotku plochy)** sa aplikujú iba na plochy.
- Každá zložka je tabuľkou v čase, takže zaťaženiu možno pre prechodový výpočet
  dať časový priebeh.

### 2.5 Počiatočné a environmentálne podmienky

Na záložke `Environmentálne podmienky` sa zapína **gravitácia**. Priraďte
*Gravitačné zrýchlenie* entite, ktorá má niesť svoju vlastnú tiaž - zvyčajne
celému modelu - a nastavte vektor; predvolená hodnota je `-9.80665` v Z. Priraďte
*Teplotu*, aby ste nastavili referenčnú teplotu stavu bez napätia pre analýzu
tepelného napätia.

Záložka `Počiatočné podmienky` ponúka *Posunutie*, *Rýchlosť konštrukcie* a
*Tlak*, aplikované pri prvom výpočte prechodovej analýzy.

### 2.6 Nastavenie maticového riešiča

`Úloha` -> `Nastaviť maticový riešič úloh` nastavuje iteračné riešiče.
Štrukturálny riešič používa položku **CG**, a to pre statické aj prechodové
riešenie, ako aj pre vnútorné riešenia iterácie vlastných hodnôt.

Ak statické riešenie nekonverguje, obvyklou príčinou je nedostatočne uložený
model, nie nastavenie riešiča. Skôr než zvýšite počet iterácií, skontrolujte
väzby.

### 2.7 Monitorovacie body

`Úloha` -> `Definovať monitorovacie body` umiestni sondy na zadané súradnice a
vyberie premennú, ktorá sa má zaznamenávať - typicky *Posunutie* alebo *Napätie
podľa von Misesa*. V prechodovom výpočte priebeh zobrazí `Výkaz` ->
`Monitorovacie body`.

### 2.8 Výsledky a záznamy

Záložka `Výsledky` uvádza vypočítané premenné a ovláda 3D pohľad. Posunutie je
uzlový vektor; tri miery napätia sú prvkové skaláry. Dynamická prechodová
analýza pridáva uzlové vektory *Rýchlosť konštrukcie* a *Zrýchlenie
konštrukcie*.

Pre zobrazenie deformovaného tvaru zobrazte *Posunutie* a v nastaveniach pohľadu
zapnite škálovanie deformácie - skutočné posunutia sú zvyčajne v skutočnej
mierke príliš malé na to, aby boli viditeľné.

Záložka `Záznamy` panela `Model` uvádza záznamy výsledkov:

- **statická analýza** - jediný záznam;
- **prechodová analýza** - jeden záznam na každý zapísaný časový krok;
- **modálna analýza** - jeden záznam na extrahovaný mód, číslovaný od
  základného. Prechádzanie záznamami prechádza módmi a ich animácia animuje
  vlastné tvary.

Pre modálny záznam 3D pohľad označí záznam číslom módu a vlastnou frekvenciou
tohto módu v Hz.

`Výkaz` -> `Log súbor riešiča` zobrazí celý výstup riešiča vrátane vlastnej
hodnoty vypísanej pre každý extrahovaný mód a štatistík posunutia, napätia podľa
von Misesa a sily po každom zázname.

---

## 3. Tutoriál - statická analýza napätia

**Cieľ.** Vypočítať priehyb a napätie konzoly zaťaženej vlastnou tiažou a
koncovým zaťažením a odčítať reakciu v podpore.

Tento tutoriál predpokladá objemový model so sieťou - dobre poslúži dlhý kváder.
Použite `Súbor` -> `Nový model`, nakreslite kváder pomocou `Geometria` ->
`Vytvoriť prvok` alebo importujte geometriu a rozdeľte ju na tetrahedróny; tento
postup pokrýva tutoriál *Kresliť kocku*. Votknutý koniec a zaťažený koniec
označte ako samostatné plošné entity, aby im bolo možné priradiť okrajové
podmienky.

### Krok 1 - výber úlohy

1. `Úloha` -> `Poradie krokov úloh` (`Ctrl+P`).
2. Pridajte krok a vyberte **Analýza napätia**.
3. Potvrďte tlačidlom `OK`.

### Krok 2 - priradenie materiálu

1. Vyberte objemovú entitu v strome `Model`.
2. Otvorte záložku `Materiál` a priraďte materiál so všetkými štyrmi
   vlastnosťami, pre konštrukčnú oceľ:
   - **Modul pružnosti** = `2.1e11` `Pa`
   - **Poissonovo číslo** = `0.3`
   - **Hustota** = `7850` `kg/m^3`
   - **Súčiniteľ teplotnej rozťažnosti** = `1.2e-5` `1/K`

### Krok 3 - ponechanie statickej analýzy

Otvorte záložku `Úloha` a v skupine *Riešič v čase* nechajte **Povoliť**
nezaškrtnuté. Riešič potom raz zostaví a vyrieši `K*u = f`.

### Krok 4 - uloženie modelu

1. Vyberte plošnú entitu na votknutom konci.
2. Otvorte záložku `Okrajové podmienky` a zaškrtnite **Posunutie**.
3. Nechajte všetky tri zložky zaškrtnuté a nastavené na `0`.

Tým sa stena úplne fixuje a odstráni sa všetkých šesť módov pohybu tuhého telesa.
Overte, že nič iné nezostalo voľne plávať - odpojená časť bez väzby robí sústavu
singulárnou a riešenie neskonverguje.

Pre podporu, ktorá má kĺzať namiesto úplného votknutia, použite na ploche
namiesto toho **Valčekové posunutie**. Zablokuje iba smer normály plochy a uzol
nechá voľne sa pohybovať v dotykovej rovine.

### Krok 5 - aplikácia zaťažení

**Vlastná tiaž.** Vyberte objemovú entitu, otvorte záložku `Environmentálne
podmienky` a zaškrtnite **Gravitačné zrýchlenie**. Ponechajte predvolenú hodnotu
`-9.80665` v Z.

**Koncové zaťaženie.** Vyberte plošnú entitu na voľnom konci, otvorte záložku
`Okrajové podmienky` a zaškrtnite **Sila**. Zadajte celkovú silu v newtonoch,
napríklad `-1000` v Z. Hodnota je celková cez entitu, takže sa pri zjemnení
siete nemení.

Alternatívy podľa toho, čím zaťaženie fyzikálne je:

- **Tlak** pre rovnomerný normálový tlak na stenu, v `Pa`;
- **Sila (na jednotku plochy)** pre smerované povrchové zaťaženie v `N/m^2`;
- **Hmotnosť** pre pripojenú hmotu v `kg`, ktorá nie je modelovaná ako geometria.

### Krok 6 - riešenie

1. `Riešenie` -> `Spustiť riešič` (`Ctrl+R`).
2. Kontrola nastavenia pred spustením výpočtu ohlási chýbajúce materiály alebo
   chýbajúce okrajové podmienky.
3. Priebeh sledujte v `Výkaz` -> `Log súbor riešiča`.

### Krok 7 - kontrola výsledkov

- Záložka `Výsledky`: zobrazte **Posunutie**, aby ste videli prehnutý tvar, a
  **Napätie podľa von Misesa** pre rozloženie napätia. V nastaveniach pohľadu
  zapnite škálovanie deformácie, aby bol priehyb viditeľný.
- **Normálové napätie** a **Šmykové napätie** oddeľujú dva príspevky, z ktorých
  sa skladá uvádzaná hodnota podľa von Misesa.
- **Sila** na votknutej stene je reakcia. Jej súčet by mal vyrovnávať aplikované
  zaťaženie plus vlastnú tiaž - rýchla a účinná kontrola, že model je zaťažený
  tak, ako ste zamýšľali.
- Monitorovací bod na *Posunutí* na konci dá priehyb ako číslo, nie ako farbu.

### Krok 8 - voliteľne, tepelné napätie

Pridanie tepelného zaťaženia:

1. Do poradia krokov úloh pridajte krok **Prestup tepla** *pred* krok napätia a
   nastavte jeho okrajové podmienky a vlastnosti materiálu.
2. Štrukturálnej entite priraďte environmentálnu podmienku **Teplota** s
   referenčnou teplotou stavu bez napätia, napríklad `293.15` K.

Riešič napätia prevezme vypočítané teploty prvkov z tepelného riešiča a člen
teplotnej rozťažnosti pridá automaticky.

### Riešenie problémov

| Príznak | Príčina |
|---|---|
| Riešenie nekonverguje | model je nedostatočne uložený; časť voľne pláva alebo podpory neodstraňujú všetkých šesť módov pohybu tuhého telesa |
| *Entity has material assigned which is missing required properties* | chýba jedna zo štyroch požadovaných vlastností |
| Posunutia sú všade nulové | do modelu sa nedostalo žiadne zaťaženie - skontrolujte, či je priradená gravitácia, alebo či sú zložky Sily nenulové |
| Posunutia sú obrovské | modul pružnosti je o rády nesprávny, alebo plošná entita má nulovú hrúbku, a teda žiadnu tuhosť |
| Zaťaženie sa mení pri zjemnení siete | *Sila (na jednotku plochy)* alebo *Tlak* bol mienený ako celková sila, alebo naopak |
| Tlak tlačí nesprávnym smerom | normály plochy nie sú konzistentné alebo plocha neohraničuje objem |

---

## 4. Tutoriál - modálna analýza

**Cieľ.** Extrahovať niekoľko prvých vlastných módov tej istej konzoly.

### Krok 1 - výber úlohy

`Úloha` -> `Poradie krokov úloh` (`Ctrl+P`), pridajte krok, vyberte **Modálna
analýza**.

Modálnu analýzu nemožno kombinovať s inými fyzikami, s jedinou výnimkou analýzy
napätia zaradenej pred ňou, ktorá z nej urobí predpätú modálnu analýzu.

### Krok 2 - priradenie materiálu

Tie isté štyri vlastnosti ako v statickom tutoriáli. **Hustota** tu záleží viac
ako kdekoľvek inde: je celou maticou hmotnosti a frekvencie sa škálujú ako
`1/sqrt(rho)`.

### Krok 3 - uloženie modelu

Na votknutom konci priraďte tú istú okrajovú podmienku **Posunutie**.

Práve väzby odlišujú módy konzoly od módov voľne plávajúcej tyče. Ak model
zostane neuložený, prvých šesť vlastných hodnôt bude takmer nulových - módy
pohybu tuhého telesa - a zaujímavé módy sa posunú ďalej v zozname.

### Krok 4 - nastavenie modálnej analýzy

Otvorte záložku `Úloha` a v skupine *Nastavenia modálnej analýzy* nastavte:

| Pole | Hodnota |
|---|---|
| Modálna metóda | `Multiple modes` |
| Iterácie | `100` |
| Extrahovať módy | `6` |
| Konvergenčná hodnota | `1.0e-9` |

Šesť módov je rozumný začiatok a menej ako predvolených desať. Každý mód navyše
pridá lineárne riešenie do každej iterácie, takže veľký počet módov na veľkej
sieti je pomalý.

Ak potrebujete iba jediný mód, zvoľte `Only most dominant mode`; používa
inverznú mocninovú iteráciu na jedinom vektore a je oveľa lacnejšia.

### Krok 5 - zaťaženia nie sú potrebné

Modálna analýza nemá vektor zaťaženia - úloha vlastných hodnôt je homogénna.
Zaťaženia priradené modelu sa ignorujú, s jednou výnimkou: ak predtým prebehla
analýza napätia, jej výsledok posunutia deformuje sieť pre modálne zostavenie.

### Krok 6 - riešenie

`Riešenie` -> `Spustiť riešič` (`Ctrl+R`). Log uvádza podpriestorové iterácie a
mieru ich konvergencie a potom jeden blok na extrahovaný mód:

```
Extracting mode 6
{
    Eigen-value = ...
    Storing results
    ...
}
```

Módy sa spracúvajú od najvyššieho extrahovaného indexu smerom k základnému, takže
základný mód sa počíta ako posledný a ukladá sa ako záznam 1.

### Krok 7 - kontrola módov

- Panel `Model`, záložka `Záznamy`: jeden záznam na mód, číslovaný od
  základného. Prechádzajte nimi, aby ste videli každý vlastný tvar.
- Záložka `Výsledky`: zobrazte **Posunutie**. Vlastný tvar je definovaný iba až
  na mierku, takže veľkosti sú ľubovoľné - záleží na tvare. Zapnite škálovanie
  deformácie a záznam animujte, aby ste mód zviditeľnili.
- Vlastná hodnota každého módu sa vypisuje v logu riešiča a zobrazuje ako popis
  záznamu v 3D pohľade.

### Krok 8 - odčítanie vlastných frekvencií

3D pohľad označí každý modálny záznam číslom módu a jeho vlastnou frekvenciou v
Hz. Log riešiča uvádza tú istú frekvenciu pre každý extrahovaný mód spolu s
príslušnou vlastnou hodnotou:

```
Eigen-value = 1e+06, frequency = 159.155 [Hz]
```

Mód, ktorý iterácia vlastných hodnôt nedokázala určiť, sa uvádza ako `0` Hz a
nesie varovanie v logu.

Ako pri každom výsledku sa oplatí pri prvom nastavení modelu overiť základnú
frekvenciu ručným výpočtom - časť 5 uvádza vzorec pre konzolu.

### Krok 9 - voliteľne, predpäté módy

1. V poradí krokov úloh umiestnite krok **Analýza napätia** *pred* krok
   **Modálna analýza**.
2. Statické zaťaženia a väzby nastavte ako v tutoriáli 3.

Modálna analýza potom beží na deformovanej sieti. Takto možno vidieť, ako
statické zaťaženie mení tvar, v ktorom konštrukcia kmitá.

### Riešenie problémov

| Príznak | Príčina |
|---|---|
| Prvé vlastné hodnoty sú takmer nulové | model je nedostatočne uložený alebo je časť odpojená - skutočný mechanizmus |
| Riešič vlastných hodnôt nekonverguje | príliš málo iterácií alebo zle podmienená matica hmotnosti, pretože entita nemá hustotu alebo geometrickú mieru |
| Extrahovanie módov je veľmi pomalé | požadovaných je príliš veľa módov; každý mód stojí úplné lineárne riešenie |
| Výpočet trvá hodiny alebo sa zdá, že nikdy neskončí | požadovaných je príliš veľa módov - každý stojí lineárne riešenie v každej iterácii a výpočet sa nemôže zastaviť skôr, kým sa všetky neustália. Priebeh v rámci iterácie sledujte podľa počtu podpriestorových vektorov v logu |
| Frekvencia vyzerá absurdne | podpriestorová iterácia neskonvergovala - log to na konci neskonvergovaného výpočtu uvedie; extrahujte menej módov, zvýšte počet iterácií alebo skontrolujte hustotu a väzby |
| Vlastné tvary vyzerajú rovnako | blízko seba ležiace módy symetrickej konštrukcie alebo príliš málo podpriestorových iterácií na ich oddelenie |
| Menej záznamov, ako bolo požadovaných módov | podpriestor stratil nezávislé smery; log uvádza, koľko módov bolo extrahovaných, a ukladá iba tie |

---

## 5. Kontrola modelu

Riešič nemá zabudovanú sadu overovacích testov, takže nový model sa oplatí pred
tým, než mu začnete dôverovať, overiť voči niečomu, čo viete vypočítať ručne.
Tri lacné kontroly, v poradí podľa užitočnosti:

**Bilancia reakcií.** Sčítajte výsledok *Sila* cez obmedzené uzly. Musí
vyrovnávať aplikované zaťaženia plus vlastnú tiaž `rho * V * g`. To zachytí
nesprávne veľkosti zaťaženia, nesprávne jednotky aj nesprávne smery tlaku.

**Priehyb konca konzoly.** Pre nosník dĺžky `L` s kvadratickým momentom prierezu
`I` a koncovým zaťažením `F` dáva Eulerova-Bernoulliho teória

```
delta = F * L^3 / (3 * E * I)
```

Nosník vymodelujte objemovými prvkami a porovnajte. Očakávajte, že výsledok
metódy konečných prvkov bude na hrubej sieti o niečo tuhší a pri zjemňovaní siete
bude konvergovať zdola. Na túto kontrolu nepoužívajte čiarové prvky - nemajú
ohybovú tuhosť.

**Základná frekvencia konzoly.** Pre ten istý nosník s plochou prierezu `A`,

```
f1 = (1.875^2 / (2*pi)) * sqrt( E*I / (rho * A * L^4) )
```

Porovnajte so `sqrt(lambda_0) / (2*pi)` z modálnej analýzy. Tým sa súčasne overí
matica hmotnosti, matica tuhosti aj váš prepočet vlastných hodnôt.

**Náhle zaťažená tyč.** Tyč votknutá na jednom konci a na druhom zaťažená
osovou silou `F` zapnutou v čase `t = 0` je kontrolou dynamickej analýzy.
Napäťová vlna beží k votknutému koncu a späť rýchlosťou `c = sqrt(E/rho)`, takže
voľný koniec sa lineárne posunie na dvojnásobok statického posunutia,
`2 * F * L / (E * A)`, v čase `t = 2*L/c` a späť na nulu v čase `t = 4*L/c`. Bez
tlmenia sa cyklus opakuje. S Rayleighovým tlmením zanikne a tyč sa ustáli na
statickom posunutí. Použite nulové Poissonovo číslo, aby bola tyč skutočne
jednorozmerná.

Pri ktorejkoľvek z nich sieť aspoň raz zjemnite a overte, že odpoveď konverguje,
a nie sa posúva.

---

## 6. Obmedzenia

### Obmedzenia modelovania

- **Iba lineárna pružnosť.** Malé pretvorenia, malé posunutia, izotropný
  materiál, žiadna plasticita, žiadny kontakt, žiadne veľké deformácie a žiadne
  vybočenie. Výsledky majú zmysel iba dovtedy, kým konštrukcia zostáva vo svojom
  lineárnom rozsahu.
- **Tlmenie je iba Rayleighovo.** Matica tlmenia je `alpha_M*M + beta_K*K` pre
  každý materiál, takže pomerné tlmenie sleduje krivku z 1.7 a nedá sa nastaviť
  mód po móde. Nie sú k dispozícii diskrétne tlmiče, frekvenčne nezávislé
  (hysterézne) tlmenie ani okrajová podmienka tlmenia.
- **Iba implicitná časová integrácia.** Každý krok je lineárne riešenie. Nie je
  k dispozícii explicitná schéma centrálnych diferencií, takže veľmi krátke deje,
  ako balistický náraz, potrebujú veľa nákladných krokov.
- **Plošné prvky sú v rovinnom pretvorení.** Tenká doska zaťažená vo vlastnej
  rovine bude tuhšia ako idealizácia rovinnej napätosti. Ak na rozdiele záleží,
  tenké konštrukcie modelujte objemovými prvkami.
- **Čiarové prvky sú prúty.** Zostavuje sa iba osová tuhosť `E*A`. Neexistuje
  ohyb, šmyk ani krútenie, nech sa typ prvku volá akokoľvek, takže rám
  modelovaný čiarovými prvkami sa bude správať ako kĺbovo spojená priehradová
  konštrukcia.
- **Predpätie je iba geometrické.** Predpätá modálna analýza znova vyhodnotí
  lineárnu tuhosť na posunutej sieti. Nepridáva člen geometrickej tuhosti, takže
  vystuženie napnutého lana ani zmäkčenie stlačenej vzpery sa nezachytí.
- **Žiadne modálne participačné faktory ani efektívne hmotnosti.** Riešič uvádza
  iba vlastné hodnoty a vlastné tvary, takže neexistuje priamy údaj o tom, ktoré
  módy záležia pre daný smer budenia.

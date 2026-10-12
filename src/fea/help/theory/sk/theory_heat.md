# Prestup tepla - Teoretický manuál a používateľská príručka

Tento dokument opisuje tepelný riešič (`RSolverHeat`) v Range FEA, ktorý rieši
typ úlohy **Prestup tepla** (*Heat transfer*): rovnice, ktoré rieši, význam
každého vstupu, ktorý prijíma, časti grafického používateľského rozhrania, ktoré
ho ovládajú, a dva podrobne rozpracované tutoriály.

**Obsah**

1. [Teoretický základ](#1-teoretický-základ)
2. [Grafické používateľské rozhranie](#2-grafické-používateľské-rozhranie)
3. [Tutoriál - ustálený prestup tepla](#3-tutoriál---ustálený-prestup-tepla)
4. [Tutoriál - prechodový prestup tepla](#4-tutoriál---prechodový-prestup-tepla)
5. [Kontrola modelu](#5-kontrola-modelu)
6. [Obmedzenia](#6-obmedzenia)

---

## 1. Teoretický základ

### 1.1 Primárna neznáma - teplotné pole

Primárnou neznámou je **teplota** `T` v každom uzle, v kelvinoch. Každý uzol
nesie jediný stupeň voľnosti, takže sieť s `N` uzlami dáva sústavu `N` rovníc.
Tepelný tok sa po riešení získa z teplotného poľa.

Teplota je skalár, vďaka čomu je tepelný model výrazne lacnejší ako štrukturálny
model na tej istej sieti: jedna neznáma na uzol namiesto troch a matica s
deviatinou prvkov.

Predvolená teplota uzla, ktorého sa nedotýka žiadna podmienka ani predchádzajúci
výsledok, je `293.15 K`.

### 1.2 Riadiaca rovnica

Riešič implementuje **lineárne vedenie tepla** s konvekčnými hranicami.
Energetická bilancia telesa s tepelnou vodivosťou `k`, hustotou `rho` a mernou
tepelnou kapacitou `c` je

```
rho * c * dT/dt = div( k * grad(T) ) + q
```

a pri vypnutom časovom riešiči prechodový člen odpadne a zostane ustálená
bilancia

```
div( k * grad(T) ) + q = 0
```

Teplo opúšťa povrch konvekciou podľa Newtonovho zákona ochladzovania,

```
qs = h * ( T - Tf )
```

| Symbol | Význam | Jednotky | Zdroj |
|---|---|---|---|
| `T` | teplota | `K` | riešená |
| `q` | hustota tepelného zdroja | `W/m^3` | podmienka Teplo, Joulovo teplo, sálanie |
| `k` | tepelná vodivosť | `W/(m*K)` | vlastnosť materiálu |
| `c` | tepelná kapacita | `J/(kg*K)` | vlastnosť materiálu |
| `rho` | hustota | `kg/m^3` | vlastnosť materiálu |
| `h` | súčiniteľ konvekcie | `W/(m^2*K)` | konvekčná podmienka, pozri 1.5 |
| `Tf` | teplota tekutiny | `K` | konvekčná podmienka |
| `Q` | tepelný tok | `W/m^2` | odvodený, pozri 1.9 |

Neexistuje advekčný člen. Úloha prestupu tepla prenáša energiu vedením vnútri
telesa a konvekciou cez jeho hranicu; neprenáša energiu s prúdiacim prostredím.
Na to použite *Prestup tepla v tekutinách*.

**Požadované vlastnosti materiálu.** Tepelná úloha vyžaduje **štyri**:

| Vlastnosť | Jednotky | Používa sa na |
|---|---|---|
| Tepelná vodivosť | `W/(m*K)` | maticu vedenia |
| Tepelná kapacita | `J/(kg*K)` | maticu kapacity, iba prechodová analýza |
| Hustota | `kg/m^3` | maticu kapacity, iba prechodová analýza |
| Emisivita | `-` | iba sálanie tepla |

Entita, ktorej materiálu niektorá zo štyroch chýba, sa **nerieši**, a to ani pri
čisto ustálenom výpočte vedenia, ktorý sa na hustotu ani emisivitu nikdy
nepozrie. Kontrola nastavenia na to upozorní pred spustením výpočtu. Jedinou
výnimkou je entita s tepelnou okrajovou podmienkou: tá sa do sústavy zahrnie bez
ohľadu na materiál a jej vodivosť a kapacita sú potom jednoducho nulové.

Každá vlastnosť je tabuľkou v závislosti od teploty. Hodnota použitá pre prvok sa
z tabuľky číta pri **teplote prvku z predchádzajúceho výpočtu**, takže teplotne
závislá vodivosť sa oneskoruje o jednu iteráciu. Časť 1.6 vysvetľuje, ako ju
dokonvergovať.

### 1.3 Diskretizácia metódou konečných prvkov

Slabá formulácia energetickej bilancie dáva pre každý prvok

```
K = INT( B^T * k * B )  +  INT( N^T * h * N )      conduction + convection
M = INT( rho * c * N^T * N )                       capacity matrix
f = INT( N^T * q )  +  h * Tf * A / n              sources + convection
```

(vedenie plus konvekcia, matica kapacity, zdroje plus konvekcia), kde `B` sa
zostavuje z derivácií tvarových funkcií a `N` sú samotné tvarové funkcie. Každá
matica prvku s `n` uzlami má rozmer `n x n` - jeden riadok na uzol, nie tri.

Konvekčné členy sa zostavujú **iba na plošných prvkoch**, a to iba tam, kde je
priradená konvekčná okrajová podmienka. Konvekčná tuhosť má konzistentný tvar
`INT( N^T * h * N )`, kým konvekčné zaťaženie sa rozdelí rovnomerne do uzlov
prvku. Na rozumnej sieti sú tieto dva tvary dostatočne konzistentné a rozdiel so
zjemňovaním siete zaniká.

**Miery prvkov.** Ako inde v Range FEA, entity nižšej dimenzie dostávajú
geometrickú mieru, aby mohli reprezentovať skutočné telesá:

| Entita | Miera | Kde sa nastavuje |
|---|---|---|
| Objem | 1 | - |
| Plocha | hrúbka plochy | geometrické vlastnosti entity |
| Čiara | prierez čiary | geometrické vlastnosti entity |
| Bod | objem bodu | geometrické vlastnosti entity |

**Plocha** vedie teplo vo vlastnej rovine, cez prierez rovný jej hrúbke, a
akumuluje energiu úmerne tejto hrúbke. Neexistuje teplotný gradient naprieč
hrúbkou: plošný prvok je hrubý jeden uzol, takže je z princípu konštrukcie
naprieč hrúbkou izotermický. Plocha s **nulovou hrúbkou** preto nič nevedie a
nič neakumuluje, ale stále nesie konvekciu - čo je presne to, čo chceme pre
hraničnú stenu objemu so sieťou, kde stena existuje iba na to, aby niesla
konvekčnú podmienku.

**Čiara** vedie pozdĺž vlastného smeru cez svoj prierez. **Bod** nemá smer,
pozdĺž ktorého by viedol, takže neprispieva vôbec žiadnym vedením: nesie iba
kapacitu, úmernú svojmu objemu bodu, a tepelný zdroj. Bod je sústredená tepelná
hmota, nie vodič.

Matica kapacity sa zostavuje iba vtedy, keď je potrebná, teda pre prechodovú
analýzu.

### 1.4 Zdroje a okrajové podmienky

| Okrajová podmienka | Typ | Aplikuje sa na | Zložky |
|---|---|---|---|
| Teplota (*Temperature*) | explicitná (Dirichletova) | bod, čiara, plocha, objem | Teplota `[K]` |
| Teplo (*Heat*) | prirodzená, zdroj | bod, čiara, plocha, objem | Teplo `[W]`, pozri nižšie |
| Jednoduchá konvekcia (*Simple convection*) | prirodzená (Robinova) | plocha | Súčiniteľ konvekcie `[W/(m^2*K)]`, Teplota tekutiny `[K]` |
| Nútená konvekcia (*Forced convection*) | prirodzená, korelácia | plocha | Hustota, Dynamická viskozita, Teplota tekutiny, Tepelná kapacita, Hydraulický priemer, Tepelná vodivosť, Rýchlosť |
| Prirodzená konvekcia (*Natural convection*) | prirodzená, korelácia | plocha | Hustota, Dynamická viskozita, Teplota tekutiny, Tepelná kapacita, Hydraulický priemer, Tepelná vodivosť, Súčiniteľ teplotnej rozťažnosti |
| Tepelný výkon (na jednotku plochy) (*Heat rate (unit area)*) | prirodzená, zdroj | plocha | Teplo na jednotku plochy `[W/m^2]` |
| Tepelný výkon (na jednotku objemu) (*Heat rate (unit volume)*) | prirodzená, zdroj | objem | Teplo na jednotku objemu `[W/m^3]` |

**Teplota** je jedinou explicitnou podmienkou tepelnej úlohy. Každý uzol entity,
ku ktorej je priradená, sa odoberie zo sústavy rovníc a drží sa na zadanej
hodnote; jeho príspevok sa prenesie na pravú stranu susedných rovníc. Ustálený
model potrebuje aspoň jednu takú podmienku alebo aspoň jednu konvekčnú plochu -
pozri 1.7.

Kde sa entity prekrývajú, **vyhráva entita nižšej dimenzie**: teplota predpísaná
na ploche prepíše teplotu predpísanú na objeme za ňou v uzloch, ktoré zdieľajú,
čiara prepíše plochu a bod prepíše všetko. Na rozdiel od väzieb posunutia
štrukturálneho riešiča sa teploty nekombinujú - v uzle je iba jeden stupeň
voľnosti, ktorý treba držať, takže naposledy načítaná podmienka jednoducho
prevládne.

**Teplo** je tepelný zdroj a jeho hodnota je **celkový výkon vo `W`** dodaný do
entity, ku ktorej je priradený, bez ohľadu na dimenziu tejto entity. Riešič
potrebuje hustotu zdroja, takže celková hodnota sa pred zostavením rozloží na
mieru entity:

| Entita | Hustota, ktorú riešič aplikuje |
|---|---|
| Objem | hodnota / objem entity, `W/m^3` |
| Plocha | hodnota / plocha entity, `W/m^2` |
| Čiara | hodnota / dĺžka entity, `W/m` |
| Bod | hodnota / počet bodových prvkov, `W` na každý |

Dodaný výkon je preto presne zadané číslo - nemení sa s veľkosťou entity, so
zjemnením siete ani s mierkou modelu. Podiel nesú iba prvky, ktoré riešič dokáže
počítať, takže entita bez vlastností materiálu sa preskočí a jej podmienka sa
namiesto toho ohlási. Plošný tepelný zdroj sa hrúbkou plochy neškáluje.

**Tepelný výkon (na jednotku plochy)** a **Tepelný výkon (na jednotku objemu)**
predpisujú ten istý zdrojový člen ako hustotu namiesto celkovej hodnoty: `W/m^2`
cez plochu plošnej entity a `W/m^3` cez objem objemovej entity. Siahnite po nich,
keď je zdroj prirodzene hustotou - ohrievač dimenzovaný na meter štvorcový - a po
*Teple*, keď je prirodzene celkovou hodnotou, napríklad teleso so známym
príkonom. Na entite, ktorá ich nesie viac, sa všetky sčítajú a všetky prijímajú
tabuľku v čase, takže ohrievač možno počas prechodového výpočtu zapínať a
vypínať.

**Konvekčné** podmienky sa aplikujú na plochy a pridávajú člen tuhosti aj
zaťaženie, takže pôsobia ako Robinova okrajová podmienka, nie ako pevný tok:
zvýšenie teploty steny zväčší teplo, ktoré stena stráca. Všetky tri varianty
končia ako tá istá dvojica `(h, Tf)`; líšia sa iba tým, odkiaľ sa berie `h`:

- **Jednoduchá konvekcia** berie `h` a `Tf` priamo z dvoch hodnôt, ktoré
  zadáte. Použite ju vždy, keď je súčiniteľ konvekcie známy z príručky, merania
  alebo predchádzajúceho výpočtu. Je jediná z troch, ktorú máte úplne pod
  kontrolou.
- **Nútená konvekcia** má dva režimy. Na stene medzi pevnou látkou so sieťou a
  tekutinou so sieťou, len čo krok *Prestup tepla v tekutinách* vytvoril výsledok,
  spája obe riešenia priamo - nepoužíva sa žiadna korelácia, pretože rozlíšené
  prúdenie už konvekciu nesie. Riešič tepla v tekutinách odovzdá pre každý prvok
  steny dvojicu `(h, Tf)`, ktorá reprodukuje tepelný tok, ktorý jeho vlastné
  riešenie vedie cez stenu; pozri *Združený prestup tepla* v 1.6. Všade inde - na
  ploche, ktorá nehraničí so žiadnou tekutinou so sieťou, alebo pri prvom prechode
  spriahnutého výpočtu, kým krok tepla v tekutinách ešte neprebehol - sa `h`
  vypočíta z korelácie pre vonkajšie obtekanie rovinnej dosky z vlastností
  tekutiny, hydraulického priemeru, *Teploty tekutiny* a *Rýchlosti*, ktoré
  zadáte. Log uvádza, ktorý z režimov sa používa.
- **Prirodzená konvekcia** počíta `h` z korelácie pre vodorovné dosky z
  vlastností tekutiny, hydraulického priemeru a rozdielu teplôt medzi stenou a
  tekutinou.

Ak je tej istej ploche priradených viac konvekčných podmienok, nesčítajú sa:
jednoduchá sa načíta prvá, nútená ju prepíše a prirodzená prepíše obe.

### 1.5 Konvekčné korelácie

Oba vypočítavané tvary zostavia z vlastností tekutiny zadaných s okrajovou
podmienkou obvyklé bezrozmerné čísla, s hydraulickým priemerom `d` ako
charakteristickou dĺžkou:

```
Pr = c * mu / k                                        Prandtl
Re = rho * v * d / mu                                  Reynolds
Gr = | rho^2 * g * beta * (Ts - Tf) * d^3 / mu^2 |     Grashof,  g = 9.81
Ra = Gr * Pr                                           Rayleigh
```

(Prandtlovo, Reynoldsovo, Grashofovo a Rayleighovo číslo) a Nusseltovo číslo
prevedú na súčiniteľ konvekcie ako

```
h = Nu * k / d
```

| Tvar | Korelácia | Nusseltovo číslo |
|---|---|---|
| Nútená | vonkajšie obtekanie, rovinná doska | `Nu = 0.036 * Re^(4/5) * Pr^(1/3)` |
| Prirodzená | vodorovná doska, `Ra < 2e7` | `Nu = 0.54 * Ra^(1/4)` |
| Prirodzená | vodorovná doska, `Ra >= 2e7` | `Nu = 0.14 * Ra^(1/3)` |

`k`, `c`, `rho`, `mu` a `beta` v týchto výrazoch sú vlastnosti **tekutiny**
zadané s okrajovou podmienkou, nie vlastnosti materiálu pevnej látky.

Obe podmienky sa vytvárajú s vlastnosťami **suchého vzduchu pri 20 stupňoch
Celzia a 101.325 kPa**, takže tekutina je opísaná za vás a zadať treba už iba
prúdenie a geometriu:

| Zložka | Predvolená hodnota |
|---|---|
| Hustota | `1.2041 kg/m^3` |
| Dynamická viskozita | `1.8205e-5 kg/(m*s)` |
| Tepelná kapacita | `1005 J/(kg*K)` |
| Tepelná vodivosť | `0.02514 W/(m*K)` |
| Súčiniteľ teplotnej rozťažnosti | `3.4112e-3 1/K`, hodnota ideálneho plynu `1/T` |
| Teplota tekutiny | `293.15 K` |
| Hydraulický priemer | `1 m` |

Pre akúkoľvek inú tekutinu ich nahraďte - sú to vlastnosti tekutiny, nie
materiálu steny. *Prirodzená konvekcia* je preto po priradení pripravená na
riešenie; *Nútená konvekcia* ešte potrebuje **rýchlosť**, ktorú žiadna predvolená
hodnota neuhádne - pokiaľ plocha nehraničí s tekutinou so sieťou, kde sa žiadna
z týchto hodnôt nečíta.

Každé z týchto čísel delí `mu`, `k` alebo `d` a `Re` a `Pr` klesnú na nulu, ak
klesne `rho`, `c` alebo `v`, čo by zanechalo `h = 0` a plochu, ktorá sa nikdy
neochladí. Nulová hodnota sa preto považuje za chybu, nie za niečo, čo treba
zamaskovať: riešič sa zastaví a uvedie zložku a entitu, napríklad

```
Value of 'Velocity' configured in 'Forced convection' boundary condition on
entity 'Surface' is zero - the convection correlation can not be evaluated.
```

Rýchlosť vypočítaná riešičom prúdenia sa do korelácie nikdy neprivádza. Stena s
podmienkou nulového sklzu drží tekutinu v pokoji, takže rýchlosť vedľa nej je
nulová, a korelácia by potrebovala rýchlosť voľného prúdu a strednú teplotu, ktoré
všeobecné trojrozmerné prúdenie nedefinuje. Stena hraničiaca s tekutinou so
sieťou je namiesto toho spriahnutá s riešením tekutiny.

Tri dôsledky stojí za to mať na pamäti:

- Korelácia je **pevná**. Riešič sa nepozerá na orientáciu plochy, na to, či je
  prúdenie vnútorné alebo vonkajšie, ani na to, či je medzná vrstva laminárna
  alebo turbulentná. Všetko vstupuje cez hydraulický priemer, ktorý je
  charakteristickou dĺžkou korelácie - je vaším jediným nástrojom na zohľadnenie
  geometrie.
- Nulový hydraulický priemer by dal `h = 0`, teda izolovanú stenu, takže riešič
  sa radšej zastaví, než by ho prijal.
- Prirodzená konvekcia závisí od **teploty steny**, ktorá sa berie z teploty
  prvku z **predchádzajúceho** riešenia. Pri prvom výpočte ustálenej analýzy je to
  počiatočná teplota, takže súčiniteľ sa vyhodnotí pri nesprávnom rozdiele
  teplôt. Na jeho dokonvergovanie zabaľte krok do skupiny krokov úloh s
  niekoľkými iteráciami, alebo použite *Jednoduchú konvekciu* so súčiniteľom,
  ktorý ste vypočítali sami. Nulový rozdiel teplôt dáva `Nu = 1`.

### 1.6 Environmentálne, počiatočné a väzobné podmienky

#### Environmentálne podmienky

| Environmentálna podmienka | Zložky | Účinok |
|---|---|---|
| Teplota (*Temperature*) | Teplota `[K]` | teplota pozadia entity, iba prvý výpočet |

#### Počiatočné podmienky

| Počiatočná podmienka | Účinok |
|---|---|
| Teplota | počiatočné teplotné pole, aplikované pri prvom výpočte |

Environmentálne podmienky, počiatočné podmienky a okrajové podmienky sa čítajú v
tomto poradí a každá prepíše predchádzajúcu, takže **okrajová podmienka vyhráva
nad počiatočnou, ktorá vyhráva nad environmentálnou**. Environmentálne a
počiatočné hodnoty sa čítajú **iba pri prvom výpočte**; potom pole pokračuje z
predchádzajúceho výsledku.

#### Väzba s inými riešičmi

Tepelný riešič výsledky zdieľané so zvyškom poradia krokov spotrebúva aj vytvára.

| Smer | Premenná | Účinok |
|---|---|---|
| Sálanie tepla -> teplo | Teplo - časť od sálania | pripočíta sa k zdrojovému členu **plošných** prvkov |
| Elektrostatika -> teplo | Joulovo teplo | pripočíta sa k zdrojovému členu každého prvku |
| Napätie -> teplo | Posunutie | sieť sa pre tepelné riešenie zdeformuje a potom obnoví |
| Teplo -> napätie | Teplota | poháňa člen teplotnej rozťažnosti štrukturálneho riešiča |
| Teplo -> akýkoľvek riešič | Teplota | vyberá riadok každej teplotne závislej tabuľky materiálu |
| Teplo v tekutinách -> teplo | `(h, Tf)` steny | poháňa steny s *Nútenou konvekciou* hraničiace s tekutinou so sieťou |
| Teplo -> teplo v tekutinách | Teplota pevnej látky | drží tie isté steny pri riešení tekutiny na teplote pevnej látky |

**Sálanie tepla** (*Radiative heat transfer*) je samostatný typ úlohy s
vlastným riešičom. Vyžaduje krok *Prestup tepla*, rieši sústavu radiozity
uzavretého priestoru cez plošné záplaty pomocou faktorov vzájomného ožiarenia a
emisivity materiálu a výsledok vracia ako plošný tepelný zdroj. Oba kroky
pridajte do jednej skupiny krokov úloh a nechajte skupinu iterovať: riešič
sálania hlási konvergenciu a skupinu ukončí, keď sa teplo jeho záplat prestane
meniť.

**Joulovo teplo** prichádza rovnako z kroku *Elektrostatika* zaradeného pred
krok prestupu tepla, takže odporový ohrev nepotrebuje žiadne ďalšie nastavenie.

Pokiaľ nie je spriahnutý s tekutinou, tepelný riešič hlási konvergenciu
bezpodmienečne, takže skupina krokov obsahujúca iba krok prestupu tepla sa vždy
zastaví po jednej iterácii. Iterovať sa oplatí, keď je niečo, od čoho tepelné
riešenie závisí, samo oneskorené - teplo zo sálania, súčiniteľ prirodzenej
konvekcie alebo silne teplotne závislá vodivosť.

#### Združený prestup tepla

Tepelný riešič rieši **iba pevné látky**. Objem, ktorého materiál je tekutina -
plyn alebo kvapalina, alebo materiál s neurčeným skupenstvom, ktorý má
dynamickú viskozitu, čo pokrýva každú tekutinu z databázy materiálov - nikdy nie
je súčasťou tepelného riešenia, nech nesie akékoľvek vlastnosti alebo podmienky.
Rovnako ani bodová, čiarová alebo plošná entita ležiaca vnútri tekutiny, napríklad
vstup; podmienka na nej, ktorú číta iba tepelný riešič, sa ohlási v logu. Oblasť
tekutiny patrí typu úlohy *Prestup tepla v tekutinách*, ktorý teplo unáša s
prúdením.

Tieto dve sa stretávajú na stenách s podmienkou **Nútená konvekcia**. S oboma
krokmi v jednej skupine krokov úloh - krok tepla v tekutinách ako prvý - každý
prechod skupiny urobí nasledovné:

1. Riešič tepla v tekutinách drží uzly steny na teplote, ktorú tepelný riešič
   naposledy vypočítal v pevnej látke (pri úplne prvom prechode, keď takú teplotu
   ešte nemá, je stena izolovaná), vyrieši tekutinu a v každom uzle steny zistí
   tepelný tok vstupujúci do tekutiny. Tento tok je reziduom rovníc tekutiny v
   uzle, tým istým, ktoré by videlo jediné riešenie oboch oblastí, takže tenká
   tepelná medzná vrstva nestojí nič na presnosti.
2. Pre každý prvok steny odovzdá dvojicu `(h, Tf)`. `h = k * G` je vodivosť
   prvého prvku tekutiny za stenou - `k` delené výškou prvku nad stenou, pre
   tetrahedrón - a `Tf` je nastavená tak, aby `q = h * (Tw - Tf)` bol tok z
   kroku 1.
3. Tepelný riešič ich aplikuje na stenu tak, ako by aplikoval podmienku
   *Jednoduchá konvekcia*, vyrieši pevnú látku a vráti svoju teplotu steny.

Dvojica `(h, Tf)` je numerický prostriedok, vďaka ktorému sa obe riešenia
zhodnú - `h` nie je súčiniteľ prestupu tepla z príručky a mení sa so sieťou.
Konvergujú teplota a tok cez stenu, ktoré sú teplotou a tokom spriahnutej úlohy.
Nepotrebuje sa nič o rýchlosti tekutiny vedľa steny, a preto stena s podmienkou
nulového sklzu - kde je prúdenie v pokoji - prenáša teplo presne tak, ako má.

Teplota steny odovzdávaná tekutine sa relaxuje **Aitkenovým faktorom**
vypočítaným z posledných dvoch prechodov. Bez neho môže zle vodivá pevná látka
vedľa dobre rozlíšenej tekutiny potrebovať stovky prechodov; s ním sa skupina
zvyčajne ustáli za niekoľko. Kým sú spriahnuté, oba riešiče hlásia ako svoju
konvergenciu relatívnu zmenu svojho teplotného poľa, takže nastavte hodnotu
konvergencie skupiny krokov - `1e-6` je rozumný začiatok - a dajte jej dostatok
iterácií. Nulová hodnota konvergencie spustí všetky.

### 1.7 Ustálená analýza

Pri vypnutom časovom riešiči riešič zostaví a vyrieši

```
K * T = f
```

`K` je symetrická pozitívne definitná, len čo je predpísaná aspoň jedna teplota,
takže sa rieši **metódou združených gradientov** s Jacobiho predpodmienením.

Ustálený model **bez okrajovej podmienky Teplota a bez konvekčnej plochy** je
tepelným ekvivalentom neupevnenej konštrukcie: energiu doň možno privádzať, ale
nemá kam odísť, matica je singulárna a riešenie neskonverguje. Každý ustálený
model potrebuje aspoň jednu cestu k známej teplote.

### 1.8 Prechodová analýza

Pri zapnutom časovom riešiči riešič krokuje

```
M * dT/dt + K * T = f
```

theta schémou, kde `alpha` je koeficient aproximácie časového pochodu:

```
( M + alpha*dt*K ) * T_n+1 = dt*f + ( M - (1-alpha)*dt*K ) * T_n
```

Nastavenie *Aproximácia* časového riešiča vyberá `alpha`:

| Aproximácia | `alpha` | Výsledný pochod | Stabilita |
|---|---|---|---|
| Spätná diferencia (*Backward difference (stable)*) | `1` | `(M + dt*K)*T_n+1 = dt*f + M*T_n`, implicitný krok | bezpodmienečná |
| Centrálna diferencia (*Central difference (accurate)*) | `0.5` | Crank-Nicolson | bezpodmienečná, môže oscilovať |
| Dopredná diferencia (*Forward difference (fast)*) | `0` | `M*T_n+1 = dt*f + (M - dt*K)*T_n`, explicitný krok | podmienečná |

**Spätná diferencia** je pre tepelný model bezpečnou predvolenou voľbou. Stojí
jedno maticové riešenie na krok, ale krok je obmedzený presnosťou, ktorú chcete,
nie veľkosťou najmenšieho prvku. **Centrálna diferencia** má presnosť druhého
rádu a je lepšou voľbou, len čo je časový krok dostatočne malý na rozlíšenie
prechodového deja, hoci pri prudkom zapnutí môže zakmitať. **Dopredná diferencia**
dáva celú maticu vedenia na starý časový krok, vďaka čomu je každý krok lacný a
výpočet krehký.

Dopredná vetva je stabilná iba dovtedy, kým časový krok zostáva zhruba pod

```
dt < rho * c * h^2 / ( 2 * k )
```

kde `h` je najmenšia veľkosť prvku. Keďže teplotná vodivosť `k/(rho*c)` kovu je
rádovo `1e-5 m^2/s`, prvok veľkosti milimetra už vynúti časový krok hlboko pod
desatinou sekundy. Pre čokoľvek okrem veľmi jemného časového rozlíšenia použite
implicitný pochod.

Nestabilný prechodový výpočet sa ľahko rozpozná: teplotné pole osciluje z kroku
na krok s rastúcou amplitúdou, často medzi fyzikálne nemožnými hodnotami,
namiesto toho, aby sa ustálilo. Liekom je menší časový krok alebo implicitný
pochod.

### 1.9 Odvodené výsledky

Po riešení sa prvkový tepelný tok získa z teplotného poľa ako priemer cez
integračné body výrazu

```
Q = -k * grad(T)
```

vyhodnoteného v prvku a pre čiarové a plošné prvky otočeného do globálnych
súradníc. Na ploche s konvekciou sa konvekčný tok

```
Qh = h * ( Tf - T )
```

pripočíta v smere normály plochy, takže vektor toku steny s konvekciou smeruje
tam, kam teplo skutočne odchádza.

Tok je **hustota** vo `W/m^2`, nezávislá od veľkosti prvku, v ktorom bola
vypočítaná, takže ju možno porovnávať medzi prvkami aj s ručným výpočtom a so
zjemňovaním siete konverguje.

Ukladá sa aj **súčiniteľ prestupu tepla**. Je to súčiniteľ, ktorý riešič na
každom plošnom prvku skutočne použil, čo je jediný spôsob, ako zistiť, čo
vytvorili korelácie *Nútenej konvekcie* a *Prirodzenej konvekcie* - a na ploche
s prirodzenou konvekciou sa mení od prvku k prvku, pretože závisí od miestnej
teploty steny. Na stene spriahnutej s tekutinou je to vodivosť prvého prvku
tekutiny, ktorá závisí od siete - pozri *Združený prestup tepla* v 1.6. Na každom
prvku bez konvekčnej podmienky je nulový.

| Výsledok | Aplikuje sa na | Význam |
|---|---|---|
| Teplota `[K]` | uzol | vyriešené teplotné pole |
| Tepelný tok `[W/m^2]` | prvok | vektor vodivého toku, plus konvekčná časť na plochách s konvekciou |
| Súčiniteľ prestupu tepla `[W/(m^2*K)]` | prvok | použitý súčiniteľ konvekcie, nulový tam, kde nie je konvekcia |

Po každom zázname log riešiča vypíše štatistiky všetkých troch a hodnoty vo
všetkých monitorovacích bodoch.

### 1.10 Ustálená alebo prechodová?

| | Ustálená | Prechodová |
|---|---|---|
| Rieši | `K*T = f` | `M*T' + K*T = f` v čase |
| Odpovedá na | "ako veľmi sa nakoniec zohreje" | "ako dlho trvá, kým sa tam dostane" |
| Používa hustotu a tepelnú kapacitu | nie, ale aj tak musia byť prítomné | áno |
| Potrebuje predpísanú teplotu alebo plochu s konvekciou | áno | nie, stačí kapacita |
| Záznamy | jeden | jeden na zapísaný časový krok |

**Ustálenú analýzu použite na** konečné rozloženie teploty nepretržite
pracujúcej súčiastky, dimenzovanie chladiča, tepelný most v stene alebo teplotné
pole, ktoré napája analýzu tepelného napätia. Je predvolená a pokrýva väčšinu
práce.

**Prechodovú analýzu použite na** zohrievanie a chladnutie, pracovný cyklus,
tepelný šok, kalenie alebo čokoľvek, kde vás zaujíma časová konštanta súčiastky.

---

## 2. Grafické používateľské rozhranie

Všetko podstatné sa nachádza v paneli **Riešič** (záložky `Úloha`,
`Okrajové podmienky`, `Počiatočné podmienky`, `Environmentálne podmienky`,
`Materiál`, `Výsledky`), v menu **Úloha** a v paneli **Model**.

### 2.1 Výber úlohy

`Úloha` -> `Poradie krokov úloh` (`Ctrl+P`) otvorí dialóg poradia krokov úloh.
Pridajte krok a vyberte **Prestup tepla** - "Ustálený a prechodový prestup
tepla".

Prestup tepla sa voľne kombinuje s každou inou fyzikou okrem *Modálnej analýzy*.
Reťazce, ktoré stojí za to poznať:

- *Prestup tepla* a potom *Analýza napätia* - tepelné napätie;
- *Prestup tepla* a *Sálanie tepla* v jednej iterujúcej skupine krokov - vedenie
  plus sálanie v uzavretom priestore;
- *Elektrostatika* a potom *Prestup tepla* - odporový ohrev.

Ak chcete iterovať spriahnutú dvojicu, pridajte skupinu krokov a nastavte jej
počet iterácií; skupina sa zastaví skôr, keď každý krok v nej hlási
konvergenciu.

### 2.2 Záložka Úloha

**Riešič v čase** - zobrazuje sa pre prestup tepla, keďže je to časovo závislý
typ úlohy. Pre ustálenú analýzu ho nechajte vypnutý.

| Pole | Význam |
|---|---|
| Povoliť | zapína a vypína prechodový pochod |
| Aproximácia | theta časového pochodu: spätná, centrálna alebo dopredná, pozri 1.8 |
| Počiatočný čas | prvá hodnota času v sekundách; pri reštarte riešiča sa ignoruje |
| Konečný čas | `počiatočný čas + veľkosť časového kroku * počet časových krokov`, iba na čítanie |
| Veľkosť časového kroku | `dt` v sekundách |
| Počet časových krokov | koľko krokov sa má vypočítať |
| Frekvencia výstupu | zapíše súbor výsledkov každých N krokov; `0` zapíše iba posledný krok |

Tepelná úloha nemá vlastnú skupinu nastavení. Všetko ostatné sa priraďuje
entitám na záložkách podmienok.

### 2.3 Záložka Materiál

Záložka `Materiál` uvádza materiál priradený vybranej entite. Prestup tepla
vyžaduje **všetky štyri** tieto vlastnosti:

| Vlastnosť | Jednotky | Poznámka |
|---|---|---|
| Tepelná vodivosť | `W/(m*K)` | `k`, asi `50` pre konštrukčnú oceľ, `15` pre nehrdzavejúcu, `200` pre hliník |
| Tepelná kapacita | `J/(kg*K)` | `c`, asi `460` pre oceľ |
| Hustota | `kg/m^3` | `rho`, asi `7850` pre oceľ |
| Emisivita | `-` | používa ju iba sálanie tepla, ale aj tak je povinná |

Entita, ktorej niektorá z nich chýba, sa nerieši a kontrola nastavenia na to
upozorní pred spustením výpočtu. Ak nerobíte sálanie tepla, na splnenie
požiadavky stačí emisivita `0`.

Materiál musí byť aj **pevnou látkou**. Objem, ktorého materiál je plyn alebo
kvapalina - alebo nemá nastavené skupenstvo a má dynamickú viskozitu - sa
prenechá typu úlohy *Prestup tepla v tekutinách*, aj keď má všetky štyri
vlastnosti.

Každá vlastnosť je tabuľkou v závislosti od teploty, takže teplotne závislá
vodivosť sa rešpektuje - vyhodnotená pri teplote prvku z predchádzajúceho
výpočtu, vďaka čomu sa oplatí krok iterovať, keď je závislosť silná.

### 2.4 Záložka Okrajové podmienky

Záložka je rozdelená na zoznam dostupných okrajových podmienok hore a editor
vybranej podmienky dole. Najprv vyberte entitu v strome `Model`; zoznam potom
ponúkne podmienky platné pre daný typ entity a pre vybranú úlohu. Zaškrtnutím
podmienku priradíte a jej zložky potom upravíte v dolnom strome.

Ponúkajú sa podmienky z časti 1.4. Poznámky k ich použitiu:

- **Teplota** je jediná podmienka, ktorá hodnotu drží, nie poháňa. Priraďte ju
  stene, hrane alebo bodu, ktorých teplotu poznáte.
- **Teplo** sa ponúka na každom type entity a preberá celkový výkon vo `W`;
  riešič ho na entitu rozloží za vás. Je to podmienka, po ktorej treba siahnuť,
  keď poznáte príkon toho, čo teplo vytvára.
- **Tepelný výkon (na jednotku objemu)** na objeme a **Tepelný výkon (na
  jednotku plochy)** na ploche predpisujú ten istý zdroj ako hustotu, pre zdroj,
  ktorý je prirodzene dimenzovaný na jednotku objemu alebo plochy.
- Tri **konvekčné** podmienky sa aplikujú iba na plochy. Priraďte *Jednoduchú
  konvekciu*, pokiaľ nemáte konkrétny dôvod nechať riešič súčiniteľ
  korelovať za vás; je predvídateľná. *Nútená konvekcia* na stene hraničiacej s
  tekutinou so sieťou spája pevnú látku s riešením tepla v tekutinách a nečíta
  žiadnu zo svojich zložiek; inde z nich súčiniteľ koreluje.
- Každá zložka je tabuľkou v čase, takže predpísanej teplote alebo tepelnému
  zdroju možno pre prechodový výpočet dať časový priebeh. Takto sa modeluje
  pracovný cyklus alebo postupne nabiehajúci ohrievač.

### 2.5 Počiatočné a environmentálne podmienky

Záložka `Počiatočné podmienky` ponúka *Teplotu*, ktorá nastavuje počiatočné pole
prechodového výpočtu. Priraďte ju celému modelu a dajte jej teplotu, pri ktorej
súčiastka začína.

Záložka `Environmentálne podmienky` tiež ponúka *Teplotu*, ako hodnotu pozadia
entity. Obe sa čítajú iba pri prvom výpočte a okrajová podmienka prepíše obe.

### 2.6 Nastavenie maticového riešiča

`Úloha` -> `Nastaviť maticový riešič úloh` nastavuje iteračné riešiče. Tepelný
riešič používa položku **CG**.

Ak ustálené riešenie nekonverguje, obvyklou príčinou je model bez predpísanej
teploty a bez plochy s konvekciou, nie nastavenie riešiča. Skôr než zvýšite počet
iterácií, skontrolujte okrajové podmienky.

### 2.7 Monitorovacie body

`Úloha` -> `Definovať monitorovacie body` umiestni sondy na zadané súradnice a
vyberie premennú, ktorá sa má zaznamenávať - pre tepelnú úlohu *Teplotu*. V
prechodovom výpočte priebeh zobrazí `Výkaz` -> `Monitorovacie body`, čo je
najrýchlejší spôsob, ako z modelu odčítať krivku zohrievania alebo časovú
konštantu.

### 2.8 Výsledky a záznamy

Záložka `Výsledky` uvádza vypočítané premenné a ovláda 3D pohľad. Teplota je
uzlový skalár, tepelný tok prvkový vektor a súčiniteľ prestupu tepla prvkový
skalár, ktorý je mimo plôch s konvekciou nulový.

Záložka `Záznamy` panela `Model` uvádza záznamy výsledkov:

- **ustálená analýza** - jediný záznam;
- **prechodová analýza** - jeden záznam na každý zapísaný časový krok. Animácia
  záznamov prehrá zohrievanie.

`Výkaz` -> `Log súbor riešiča` zobrazí celý výstup riešiča vrátane štatistík
teploty, tepelného toku a súčiniteľa prestupu tepla po každom zázname.

---

## 3. Tutoriál - ustálený prestup tepla

**Cieľ.** Vypočítať rozloženie teploty v tyči, ktorá je na jednom konci držaná
horúca a chladená okolitým vzduchom, a odčítať teplotu na studenom konci.

Tento tutoriál predpokladá objemový model so sieťou - dobre poslúži dlhý kváder.
Použite `Súbor` -> `Nový model`, nakreslite kváder pomocou `Geometria` ->
`Vytvoriť prvok` alebo importujte geometriu a rozdeľte ju na tetrahedróny; tento
postup pokrýva tutoriál *Kresliť kocku*. Horúcu koncovú stenu a ostatné steny
označte ako samostatné plošné entity, aby im bolo možné priradiť podmienky.

### Krok 1 - výber úlohy

1. `Úloha` -> `Poradie krokov úloh` (`Ctrl+P`).
2. Pridajte krok a vyberte **Prestup tepla**.
3. Potvrďte tlačidlom `OK`.

### Krok 2 - priradenie materiálu

1. Vyberte objemovú entitu v strome `Model`.
2. Otvorte záložku `Materiál` a priraďte materiál so všetkými štyrmi
   vlastnosťami, pre konštrukčnú oceľ:
   - **Tepelná vodivosť** = `50` `W/(m*K)`
   - **Tepelná kapacita** = `460` `J/(kg*K)`
   - **Hustota** = `7850` `kg/m^3`
   - **Emisivita** = `0`

Tepelnú kapacitu a hustotu ustálené riešenie nepoužíva, ale musia byť prítomné,
aby sa entita vôbec riešila.

### Krok 3 - ponechanie ustálenej analýzy

Otvorte záložku `Úloha` a v skupine *Riešič v čase* nechajte **Povoliť**
nezaškrtnuté. Riešič potom raz zostaví a vyrieši `K*T = f`.

### Krok 4 - predpísanie horúceho konca

1. Vyberte plošnú entitu na horúcom konci.
2. Otvorte záložku `Okrajové podmienky` a zaškrtnite **Teplota**.
3. Nastavte hodnotu na `353.15` K, teda 80 stupňov Celzia.

Pamätajte, že všetky teploty v Range FEA sú absolútne, v kelvinoch.

### Krok 5 - chladenie zvyšku

1. Vyberte plošnú entitu pokrývajúcu ostatné steny.
2. Otvorte záložku `Okrajové podmienky` a zaškrtnite **Jednoduchá konvekcia**.
3. Nastavte **Súčiniteľ konvekcie** = `10` `W/(m^2*K)` a **Teplota tekutiny** =
   `293.15` K.

Súčiniteľ asi `10` je reprezentatívny pre povrch v pokojnom vzduchu; pri nútenom
prúdení vzduchu je bližšie k `50` a pri vode sa pohybuje v stovkách až tisícoch.

Toto je krok, ktorý robí model riešiteľným. Bez neho má tyč tepelný vstup, ale
žiadny odvod, a ustálený stav neexistuje.

### Krok 6 - riešenie

1. `Riešenie` -> `Spustiť riešič` (`Ctrl+R`).
2. Kontrola nastavenia pred spustením výpočtu ohlási chýbajúce materiály alebo
   chýbajúce okrajové podmienky.
3. Priebeh sledujte v `Výkaz` -> `Log súbor riešiča`.

### Krok 7 - kontrola výsledkov

- Záložka `Výsledky`: zobrazte **Teplotu**. Pole by malo plynulo klesať od
  predpísaného horúceho konca k teplote tekutiny.
- Najstudenejší bod by mal zostať nad teplotou tekutiny. Teplota kdekoľvek pod
  `Tf` alebo nad predpísanými `353.15` K znamená, že niečo privádza energiu
  nesprávnym smerom - skontrolujte znamienko každého tepelného zdroja.
- **Tepelný tok** ukazuje, kade energia putuje, vo `W/m^2`. Hodnotu na stenách s
  konvekciou vynásobte ich plochou a porovnajte s výkonom vstupujúcim do tyče.
- **Súčiniteľ prestupu tepla** by mal na stenách s konvekciou ukazovať `10` a
  všade inde `0`. Je to najrýchlejší spôsob, ako overiť, že sa konvekčná podmienka
  dostala do prvkov, ktoré ste mysleli, a jediný spôsob, ako zistiť, čo vypočítala
  korelovaná podmienka.
- Monitorovací bod na *Teplote* na studenom konci dá odpoveď ako číslo, nie ako
  farbu.

### Krok 8 - voliteľne, pridanie tepelného zdroja

Ak chcete modelovať ohrievač zabudovaný v tyči, vyberte objemovú entitu, otvorte
záložku `Okrajové podmienky` a zaškrtnite **Teplo**. Ak chcete do tyče dodať
celkovo `P` wattov, zadajte `P` - riešič ho rozloží na objem entity.

Keď je zdroj dimenzovaný na jednotku objemu, použite namiesto toho **Tepelný
výkon (na jednotku objemu)**; na dodanie rovnakého celkového výkonu zadajte
`P / V` `W/m^3` - pozri 1.4.

Ustálená teplota potom musí stúpať, kým plocha s konvekciou nedokáže odovzdať
presne tento výkon - čo je kontrola energetickej bilancie z časti 5.

### Krok 9 - voliteľne, tepelné napätie

Prenesenie výsledku do štrukturálnej analýzy:

1. Do poradia krokov úloh pridajte krok **Analýza napätia** *za* krok prestupu
   tepla a dajte entite štrukturálne vlastnosti, ktoré potrebuje.
2. Štrukturálnej entite priraďte environmentálnu podmienku **Teplota** s
   referenčnou teplotou stavu bez napätia, napríklad `293.15` K.

Riešič napätia prevezme vypočítané teploty prvkov a člen teplotnej rozťažnosti
pridá automaticky.

### Riešenie problémov

| Príznak | Príčina |
|---|---|
| Riešenie nekonverguje | nie je podmienka Teplota ani plocha s konvekciou, takže ustálený stav nie je definovaný |
| *Entity has material assigned which is missing required properties* | chýba jedna zo štyroch požadovaných vlastností - často emisivita |
| Celý model je na jednej teplote | nevstupuje žiadne teplo alebo nič nevytvára gradient od predpísanej hodnoty |
| Teploty sú absurdne vysoké | do podmienky *Teplo*, ktorá preberá celkovú hodnotu vo wattoch, bola zadaná hustota vo `W/m^3`, alebo je súčiniteľ konvekcie príliš malý |
| Stena medzi pevnou látkou a tekutinou sa neochladzuje | krok tepla v tekutinách beží po kroku prestupu tepla alebo skupina vykoná jedinú iteráciu - krok tepla v tekutinách dajte na prvé miesto a nechajte skupinu iterovať |
| Tepelné riešenie ignoruje tekutinu | zámerné - materiál tekutiny rieši iba *Prestup tepla v tekutinách* |
| Teploty vychádzajú všade blízko `293.15` K | entita sa neriešila - v logu skontrolujte chýbajúcu vlastnosť materiálu |

---

## 4. Tutoriál - prechodový prestup tepla

**Cieľ.** Začať s tou istou tyčou pri izbovej teplote, zapnúť horúci koniec a
zistiť, ako dlho trvá, kým sa zohreje.

### Krok 1 - výber úlohy

Ten istý krok **Prestup tepla** ako v tutoriáli 3. Ak je model stále nastavený,
treba pridať iba nastavenia časového riešiča a počiatočnú podmienku.

### Krok 2 - priradenie materiálu

Tie isté štyri vlastnosti. Tu záleží na **tepelnej kapacite** a **hustote**:
spolu s vodivosťou určujú teplotnú vodivosť `k/(rho*c)`, ktorá riadi, ako rýchlo
sa šíri teplotné čelo.

### Krok 3 - nastavenie počiatočnej teploty

1. Vyberte objemovú entitu.
2. Otvorte záložku `Počiatočné podmienky` a zaškrtnite **Teplota**.
3. Nastavte ju na `293.15` K.

Bez nej model aj tak začne z predvolených `293.15` K, ale jej výslovné uvedenie
robí model samodokumentujúcim a umožňuje začať z akéhokoľvek iného stavu.

### Krok 4 - nastavenie časového riešiča

Otvorte záložku `Úloha` a v skupine *Riešič v čase* nastavte:

| Pole | Hodnota |
|---|---|
| Povoliť | zaškrtnuté |
| Aproximácia | `Backward difference (stable)` |
| Počiatočný čas | `0` |
| Veľkosť časového kroku | `1` |
| Počet časových krokov | `300` |
| Frekvencia výstupu | `10` |

*Spätná diferencia* je implicitný pochod - pozri tabuľku v 1.8 - a tu je tou
správnou voľbou. Zostáva stabilná pri časovom kroku zvolenom podľa fyziky, nie
podľa siete.

Časový krok zvoľte podľa časovej konštanty, ktorú očakávate, s cieľom približne
päťdesiatich krokov cez zaujímavú časť krivky. Časť 5 uvádza vzorec na prvý
odhad. Zapisovanie každého desiateho kroku udrží súbor výsledkov malý bez straty
tvaru krivky.

### Krok 5 - ponechanie okrajových podmienok

Predpísaná **Teplota** na horúcom konci a **Jednoduchá konvekcia** na ostatných
stenách sa prenesú bez zmeny. Každá zložka je tabuľkou v čase, takže horúcemu
koncu možno namiesto toho dať nábeh: otvorte editor zložky a pridajte časové
body, aby teplota stúpala počas prvých niekoľkých sekúnd namiesto okamžitého
skoku.

### Krok 6 - pridanie monitorovacieho bodu

`Úloha` -> `Definovať monitorovacie body`, umiestnite bod na studený koniec a
nastavte jeho premennú na **Teplota**. Krivka zohrievania sa oveľa ľahšie číta
ako časový priebeh než ako sled farebných obrázkov.

### Krok 7 - riešenie

`Riešenie` -> `Spustiť riešič` (`Ctrl+R`). Log vypíše jeden blok na časový krok:

```
time step:         1 of 300       | time =  0.000000e+00 [sec] | dt =  1.000000e+00 [sec]
```

po ktorom nasledujú iterácie maticového riešiča a štatistiky teploty v danom
kroku.

### Krok 8 - kontrola výsledkov

- Panel `Model`, záložka `Záznamy`: jeden záznam na zapísaný krok. Prechádzajte
  nimi alebo ich animujte, aby ste videli, ako tepelné čelo postupuje do tyče.
- Záložka `Výsledky`: zobrazte **Teplotu**. Rozsah zobrazenia zafixujte pre
  všetky záznamy, aby farby znamenali v každom snímku to isté, inak sa animácia
  sama preškáluje a postup je neviditeľný.
- `Výkaz` -> `Monitorovacie body` vykreslí krivku zohrievania na studenom konci.
  Odčítajte z nej časovú konštantu: čas potrebný na pokrytie približne 63
  percent celkového nárastu.
- Posledný záznam by sa mal priblížiť ustálenému výsledku z tutoriálu 3. Ak nie,
  výpočet bol príliš krátky.

### Riešenie problémov

| Príznak | Príčina |
|---|---|
| Teplota osciluje a z kroku na krok rastie | explicitný pochod s príliš veľkým časovým krokom - prepnite aproximáciu na *Backward difference (stable)* alebo zmenšite `dt` |
| Výsledok sa počas celého výpočtu takmer nemení | časový krok je hlboko pod časovou konštantou alebo je výpočet príliš krátky |
| Prvý krok skočí na konečnú odpoveď | časový krok je vysoko nad časovou konštantou, takže pochod je fakticky ustálený |
| Zapísal sa iba jeden záznam | frekvencia výstupu je `0`, čo zapíše iba posledný krok |
| Prechodový dej nikdy nedosiahne ustálený stav | to je často správne - skôr než budete podozrievať riešič, porovnajte s ustáleným výpočtom |

---

## 5. Kontrola modelu

Riešič nemá zabudovanú sadu overovacích testov, takže nový model sa oplatí pred
tým, než mu začnete dôverovať, overiť voči niečomu, čo viete vypočítať ručne.
Štyri lacné kontroly, v poradí podľa užitočnosti:

**Rovinná stena.** Pre dosku hrúbky `L` s vodivosťou `k` držanú na `T1` a `T2` na
jej dvoch stenách je teplota naprieč doskou lineárna a tok je

```
q = k * ( T1 - T2 ) / L          [W/m^2]
```

Dosku vymodelujte objemovými prvkami, predpíšte obe steny a overte, že stredná
rovina je presne v polovici a že vypočítaný **Tepelný tok** zodpovedá `q`. To
zachytí nesprávne jednotky, nesprávnu vodivosť aj sieť, ktorá nie je prepojená
tak, ako si myslíte. Sieť raz zjemnite a overte, že obe čísla zostanú na mieste.

**Stena s konvekciou.** Pre tú istú dosku držanú na jednej stene na `T1` a
chladenú na druhej tekutinou s teplotou `Tf` so súčiniteľom `h` sa chladená
plocha ustáli na

```
Ts = ( (k/L)*T1 + h*Tf ) / ( k/L + h )
```

Porovnajte s vypočítanou teplotou povrchu. Toto overí konvekčnú podmienku, ktorá
je časťou tepelného modelu, v ktorej sa najľahšie urobí chyba.

**Sústredená časová konštanta.** Na prechodovú kontrolu vezmite teleso s objemom
`V` a povrchom `A` bez predpísanej teploty, začínajúce na `T0` a chladené iba
konvekciou. Kým Biotovo číslo `h*V/(A*k)` zostáva pod približne `0.1`, je teleso
takmer izotermické a

```
tau  = rho * c * V / ( h * A )
T(t) = Tf + ( T0 - Tf ) * exp( -t / tau )
```

Odčítajte `tau` z priebehu monitorovacieho bodu a porovnajte. Toto súčasne overí
maticu kapacity, konvekčnú podmienku aj váš časový krok - a je to najrýchlejší
spôsob, ako zvoliť časový krok pre skutočný model, pretože `tau/50` je dobrý prvý
odhad.

**Energetická bilancia.** Pre ustálený model so známym vstupným výkonom `P` musí
plocha s konvekciou pri konečnej teplote odovzdať presne `P`:

```
P = h * A * ( Ts_mean - Tf )
```

Strednú teplotu povrchu odčítajte zo štatistík v logu riešiča a overte
bilanciu. Je to jediná globálna kontrola, ktorá zachytí hodnotu Tepla zadanú ako
celková namiesto hustoty.

Pri ktorejkoľvek z nich sieť aspoň raz zjemnite a overte, že odpoveď konverguje,
a nie sa posúva.

---

## 6. Obmedzenia

### Obmedzenia modelovania

- **Iba lineárne vedenie.** Vodivosť, kapacita a hustota prvku sú počas riešenia
  konštantné, čítané z tabuľky materiálu pri teplote predchádzajúceho výpočtu.
  Silne teplotne závislá vlastnosť potrebuje na dokonvergovanie iterujúcu
  skupinu krokov.
- **Žiadna zmena skupenstva.** Neexistuje latentné teplo ani entalpická
  formulácia, takže tavenie, tuhnutie, var a kondenzácia sa nemodelujú. Tabuľka
  materiálu nedokáže reprezentovať latentnú plošinu.
- **Žiadna advekcia.** Energia sa pohybuje vedením vnútri telesa a konvekciou cez
  jeho hranicu. Pohybujúce sa prostredie so sebou energiu nenesie; na to použite
  *Prestup tepla v tekutinách*.
- **Konvekcia je hraničný model.** Korelované tvary používajú každý jednu pevnú
  koreláciu - vonkajšia rovinná doska pre nútené prúdenie, vodorovné dosky pre
  prirodzenú konvekciu - s hydraulickým priemerom ako charakteristickou dĺžkou.
  Orientácia plochy, režim prúdenia ani to, či je prúdenie vnútorné alebo
  vonkajšie, sa nerozpoznávajú. Keď na súčiniteli záleží, vypočítajte ho sami a
  zadajte ho cez *Jednoduchú konvekciu*.
- **Súčiniteľ prirodzenej konvekcie je oneskorený.** Vyhodnocuje sa pri teplote
  steny z predchádzajúceho riešenia, takže ustálený výpočet s jediným prechodom
  fakticky používa počiatočný rozdiel teplôt.
- **Sálanie iba cez samostatný typ úlohy.** V samotnom tepelnom riešiči nie je
  člen povrchovej emisivity; sálanie v uzavretom priestore vyžaduje krok *Sálanie
  tepla*, jeho maticu faktorov vzájomného ožiarenia a iteráciu medzi nimi.
- **Plochy sú naprieč hrúbkou izotermické.** Plošná entita vedie iba vo vlastnej
  rovine. Stenu, pri ktorej záleží na gradiente naprieč hrúbkou, modelujte
  objemovými prvkami.
- **Bodové prvky nevedú.** Bod je sústredená kapacita a tepelný zdroj, nič viac.
- **Ustálený model potrebuje odvod.** Bez predpísanej teploty aj bez plochy s
  konvekciou je matica singulárna a riešenie neskonverguje.

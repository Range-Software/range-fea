# Prestup tepla v tekutinách - Teoretický manuál a používateľská príručka

Tento dokument opisuje riešič prestupu tepla v tekutinách v Range FEA -
`RSolverFluidHeat`, ktorý rieši typ úlohy **Prestup tepla v tekutinách** (*Heat
transfer in fluids*): rovnicu, ktorú rieši, význam každého vstupu, ktorý
prijíma, jeho väzbu na tepelný riešič pevnej látky a časti grafického
používateľského rozhrania, ktoré ho ovládajú.

Prestup tepla v tekutinách sa rieši na prúdovom poli kroku **Nestlačiteľné
viskózne prúdenie**. Riešič prúdenia, jeho okrajové podmienky, nelineárnu
iteráciu a schému v čase opisuje manuál [Prúdenie tekutín](theory_fluid.md) a
tepelný riešič pevných látok manuál [Prestup tepla](theory_heat.md).

**Obsah**

1. [Teoretický základ](#1-teoretický-základ)
2. [Grafické používateľské rozhranie](#2-grafické-používateľské-rozhranie)
3. [Obmedzenia](#3-obmedzenia)

---

## 1. Teoretický základ

### 1.1 Primárna neznáma - teplota

Primárnou neznámou je **teplota** `T` v `K`, jeden stupeň voľnosti na uzol.

| Typ úlohy | Riešič | Primárna neznáma | Jednotky | Stupne voľnosti na uzol |
|---|---|---|---|---|
| Prestup tepla v tekutinách | `RSolverFluidHeat` | teplota | `K` | 1 |

Tento typ úlohy **vyžaduje** v tej istej úlohe krok *Nestlačiteľné viskózne
prúdenie*: pole rýchlosti číta z jeho výsledku a sám ho nikdy nepočíta.

### 1.2 Riadiaca rovnica

**Prestup tepla v tekutinách** rieši advekciu a difúziu teploty na prúdovom poli,

```
rho * c * ( dT/dt + v . grad(T) ) = div( k * grad(T) ) + q
```

s rovnakou stabilizáciou SUPG konvekčného člena ako riešič prúdenia a s rovnakou
theta schémou v čase. Rýchlosť `v` sa **preberá z výsledku prúdenia**, nerieši sa
tu - preto tento typ úlohy vyžaduje krok *Nestlačiteľné viskózne prúdenie*.

| Symbol | Význam | Jednotky | Zdroj |
|---|---|---|---|
| `T` | teplota | `K` | riešená |
| `v` | rýchlosť | `m/s` | výsledok prúdenia |
| `k` | tepelná vodivosť | `W/(m*K)` | vlastnosť materiálu |
| `c` | tepelná kapacita | `J/(kg*K)` | vlastnosť materiálu |
| `rho` | hustota | `kg/m^3` | vlastnosť materiálu |
| `q` | hustota tepelného zdroja | `W/m^3` | podmienka Teplo, Joulovo teplo, sálanie |

Rozdiel oproti obyčajnému typu úlohy **Prestup tepla** je práve člen
`v . grad(T)`: energia je unášaná pohybujúcou sa tekutinou, nielen vedená. To
tiež znamená, že tento riešič nemá konvekčné okrajové podmienky - nie je čo
korelovať, pretože teplo odchádzajúce zo steny do tekutiny je rozlíšené sieťou, a
nie modelované.

### 1.3 Okrajové podmienky

| Okrajová podmienka | Typ | Aplikuje sa na | Zložky |
|---|---|---|---|
| Teplota (*Temperature*) | explicitná (Dirichletova) | bod, čiara, plocha, objem | Teplota `[K]` |
| Teplo (*Heat*) | prirodzená, zdroj | bod, čiara, plocha, objem | Teplo `[W]` |

**Teplota** drží uzly svojej entity na zadanej hodnote. Priraďte ju vstupnej
ploche na nastavenie teploty tekutiny vstupujúcej do oblasti a stene na
modelovanie ohrievanej alebo chladenej hranice.

**Teplo** je zdroj a zadáva sa ako **celkový výkon vo `W`** privedený do entity,
ktorý sa pred zostavením rozloží na mieru entity - rovnaká konvencia ako v
tepelnom riešiči. *Tepelný výkon (na jednotku plochy)* a *Tepelný výkon (na
jednotku objemu)* rozhranie pre tento typ úlohy ponúka, ale riešič prestupu tepla
v tekutinách ich **nečíta**; ako zdroj tu použite *Teplo*.

Každá zložka je **tabuľkou v čase**.

Podmienky prúdenia - steny, vstupy a výstupy - patria kroku prúdenia a opisuje
ich manuál [Prúdenie tekutín](theory_fluid.md).

### 1.4 Združený prestup tepla

Stena medzi tekutinou so sieťou a pevnou látkou so sieťou, ktorá nesie podmienku
**Nútená konvekcia** (*Forced convection*) obyčajného riešiča *Prestupu tepla*,
tieto dve oblasti prepája. Po prebehnutí tepelného riešiča drží riešič prestupu
tepla v tekutinách uzly steny na teplote pevnej látky - dovtedy je stena
izolovaná - a pre každý prvok steny vráti dvojicu `(h, Tf)`, ktorá reprodukuje
tepelný tok, ktorý jeho riešenie cez stenu prenáša. Tepelný riešič túto dvojicu
aplikuje ako konvekčnú podmienku. Tok je reziduom rovníc tekutiny v uzloch steny,
konzistentný s diskretizáciou, takže zostáva presný aj v tenkej tepelnej hraničnej
vrstve. Teplota steny sa z prechodu na prechod relaxuje Aitkenovým faktorom, čo
výmenu ustáli v niekoľkých prechodoch. Manuál [Prestup tepla](theory_heat.md)
opisuje túto schému v časti *Združený prestup tepla*. Sálavé a Joulovo teplo
prichádzajúce z iných riešičov sa pripočítavajú k zdrojovému členu rovnako ako v
tepelnom riešiči.

### 1.5 Konvergencia

Bez väzby riešič prestupu tepla v tekutinách hlási konvergenciu bezpodmienečne -
po fixovaní prúdového poľa je lineárny, takže jedno riešenie je konečnou
odpoveďou. Skupina obsahujúca krok prúdenia aj krok prestupu tepla v tekutinách
preto skončí, keď skonverguje krok prúdenia. So spriahnutými stenami namiesto
toho hlási relatívnu zmenu svojho teplotného poľa, takže skupina iteruje, kým sa
teplota steny neustáli.

### 1.6 Spätná väzba na prúdenie - vztlak

Teplota, ktorú tento riešič vypočíta, sa odovzdáva späť prúdeniu. Pri každom
prechode riešič prúdenia vyhodnotí hustotu a dynamickú viskozitu každého prvku
pri najnovšej teplote, takže pri materiáli s hustotou závislou od teploty poháňa
objemová sila `rho(T)*g` **prirodzenú konvekciu** - teplá tekutina stúpa,
studená klesá. Dodávané materiály *Water* a *Air* majú hustotu závislú od
teploty.

V ustálenom výpočte vidí prvý prechod prúdenia iba počiatočnú teplotu, takže
vztlak sa objaví až od druhej iterácie skupiny; dajte skupine niekoľko iterácií
a hodnotu konvergencie. V prechodovom výpočte s jednou iteráciou na časový krok
vidí prúdenie teplotu z predchádzajúceho kroku. Schému opisuje manuál
[Prúdenie tekutín](theory_fluid.md) v časti 1.9.

### 1.7 Počiatočné podmienky

| Počiatočná podmienka | Účinok |
|---|---|
| Teplota | počiatočné teplotné pole |

Počiatočné podmienky sa čítajú **iba pri prvom výpočte**; potom pole pokračuje z
predchádzajúceho výsledku. Kde sa stretne okrajová a počiatočná podmienka,
okrajová má prednosť.

### 1.8 Výsledky

| Výsledok | Aplikuje sa na | Význam |
|---|---|---|
| Teplota `[K]` | uzol | vyriešené teplotné pole |
| Tepelný tok `[W/m^2]` | prvok | vodivý tok `-k*grad(T)` |

---

## 2. Grafické používateľské rozhranie

Všetko podstatné sa nachádza v paneli **Riešič** (záložky `Úloha`,
`Okrajové podmienky`, `Počiatočné podmienky`, `Materiál`, `Výsledky`), v menu
**Úloha** a v paneli **Model**.

### 2.1 Výber úlohy

`Úloha` -> `Poradie krokov úloh` (`Ctrl+P`) otvorí dialóg poradia krokov úloh.
Kombinácie, ktoré sa oplatí poznať:

- *Nestlačiteľné viskózne prúdenie* a *Prestup tepla v tekutinách* v jednej
  skupine - ohrievané alebo chladené prúdenie. Keď prúdenie poháňa alebo
  citeľne mení vztlak, dajte skupine niekoľko iterácií, aby sa prúdenie a
  teplota ustálili spolu - pozri 1.6;
- *Prestup tepla v tekutinách* a samostatný krok *Prestup tepla* na pevnej látke
  so sieťou, krok prestupu tepla v tekutinách ako prvý, v jednej skupine s
  niekoľkými iteráciami - združený prestup tepla cez steny s podmienkou *Nútená
  konvekcia*.

Tento typ úlohy vyžaduje prítomnosť kroku prúdenia. Hodnoty skupiny krokov -
**počet iterácií** a **Konvergencia** - opisuje manuál
[Prúdenie tekutín](theory_fluid.md).

### 2.2 Záložka Úloha

**Riešič v čase** - zobrazuje sa, pretože tento typ úlohy je časovo závislý. Pre
ustálený výpočet ho nechajte vypnutý. Jeho polia opisuje manuál
[Prúdenie tekutín](theory_fluid.md).

Tento typ úlohy vlastnú skupinu nastavení nemá. Všetko ostatné sa priraďuje
entitám na záložkách podmienok.

### 2.3 Záložka Materiál

| Vlastnosť | Jednotky | Poznámka |
|---|---|---|
| Hustota | `kg/m^3` | `rho`, približne `1000` pre vodu, `1.2` pre vzduch |
| Dynamická viskozita | `kg/(m*s)` | `mu`, približne `1.0e-3` pre vodu, `1.8e-5` pre vzduch |
| Tepelná vodivosť | `W/(m*K)` | `k`, približne `0.6` pre vodu, `0.025` pre vzduch |
| Tepelná kapacita | `J/(kg*K)` | `c`, približne `4180` pre vodu, `1005` pre vzduch |

Entita, ktorej chýba niektorá vyžadovaná vlastnosť, sa nerieši a kontrola
nastavení na to pred spustením výpočtu upozorní. Dodaná databáza materiálov
obsahuje **Vodu** a **Vzduch**, čo zvyčajne tento krok celý vybaví. Každá
vlastnosť je tabuľkou v závislosti od teploty, vyhodnocovanou pri teplote prvku z
predchádzajúceho výpočtu.

### 2.4 Záložka Okrajové podmienky

**Teplota** a **Teplo** sa objavia, keď je v poradí krokov úloh krok *Prestup
tepla v tekutinách* - pozri časť 1.3. Každá zložka je tabuľkou v čase; kliknutím
na **Upraviť časovo závislé hodnoty** zadáte hodnoty v závislosti od času.

### 2.5 Počiatočné podmienky

Záložka `Počiatočné podmienky` ponúka *Teplotu*. Nastavuje počiatočné pole
prvého výpočtu.

### 2.6 Nastavenie maticového riešiča

`Úloha` -> `Nastaviť maticový riešič úloh` konfiguruje iteračné riešiče. Riešič
prestupu tepla v tekutinách používa položku **GMRES**, pretože jeho matica nie je
symetrická.

### 2.7 Monitorovacie body a výsledky

`Úloha` -> `Definovať monitorovacie body` môže zaznamenávať *Teplotu*. Záložka
`Výsledky` zobrazuje teplotu ako uzlový skalár a tepelný tok ako prvkový vektor.

---

## 3. Obmedzenia

- **Vztlak bez rozťažnosti tekutiny.** Teplota mení prúdenie cez `rho(T)` a
  `mu(T)` (pozri 1.6), tepelná rozťažnosť tekutiny však nikdy nevstupuje do
  bilancie hmotnosti `div(v) = 0`. To je presné pre malé rozdiely hustoty
  prirodzenej konvekcie, nie pre plyn ohriaty o stovky kelvinov. Vztlakové
  prúdenie je laminárne ako každé prúdenie v Range FEA.
- **Väzba je postupná, nie monolitická.** Prúdenie a teplota sa riešia jedno po
  druhom a údaje si vymieňajú medzi prechodmi. Ustálené vztlakové prúdenie
  potrebuje skupinu s niekoľkými iteráciami; pri jednej iterácii na časový krok
  prechodové prúdenie zaostáva za teplotou o jeden krok.
- **Podmienky `Heat rate` sa ignorujú.** *Tepelný výkon (na jednotku plochy)* a
  *Tepelný výkon (na jednotku objemu)* rozhranie pre *Prestup tepla v tekutinách*
  ponúka, ale `RSolverFluidHeat` ich nečíta; dostane sa k nemu iba podmienka
  *Teplo*. Obyčajný typ úlohy *Prestup tepla* číta všetky tri.
- **Žiadne konvekčné okrajové podmienky.** Teplo odchádzajúce zo steny do
  tekutiny so sieťou sa rozlišuje, nie koreluje, takže podmienky *Jednoduchá*,
  *Nútená* a *Prirodzená konvekcia* patria obyčajnému tepelnému riešiču. Združený
  prestup tepla sa nastavuje ako krok *Prestup tepla* na pevnej látke, ktorej
  steny s *Nútenou konvekciou* čítajú vypočítaný stav tekutiny.
- **Žiadny model turbulencie.** Prúdenie je laminárne, pozri manuál
  [Prúdenie tekutín](theory_fluid.md), takže turbulentný prestup tepla nie je
  zachytený.

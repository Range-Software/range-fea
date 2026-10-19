# Prúdenie poréznym prostredím - Teoretický manuál a používateľská príručka

Tento dokument opisuje riešič prúdenia poréznym prostredím (`RSolverPotential`)
v Range FEA, ktorý rieši typ úlohy **Prúdenie poréznym prostredím** (*Flow
through porous media*): rovnice, ktoré rieši, význam každého vstupu, ktorý
prijíma, časti grafického používateľského rozhrania, ktoré ho ovládajú, a dva
podrobne rozpracované tutoriály.

**Obsah**

1. [Teoretický základ](#1-teoretický-základ)
2. [Grafické používateľské rozhranie](#2-grafické-používateľské-rozhranie)
3. [Tutoriál - prúdenie pieskovým filtrom poháňané tlakom](#3-tutoriál---prúdenie-pieskovým-filtrom-poháňané-tlakom)
4. [Tutoriál - vzostupné presakovanie zvislým stĺpcom](#4-tutoriál---vzostupné-presakovanie-zvislým-stĺpcom)
5. [Kontrola modelu](#5-kontrola-modelu)
6. [Obmedzenia](#6-obmedzenia)

---

## 1. Teoretický základ

### 1.1 Primárna neznáma - pole tlaku

Primárnou neznámou je **tlak** `p` pórovej tekutiny v každom uzle, v pascaloch.
Každý uzol nesie jediný stupeň voľnosti, takže sieť s `N` uzlami dáva sústavu `N`
rovníc - rovnako lacnú ako tepelný alebo elektrostatický model na tej istej sieti.
Rýchlosť tekutiny sa po riešení získa z poľa tlaku.

Pole tlaku riadené touto rovnicou je určené len **až na aditívnu konštantu**:
pripočítanie rovnakej hodnoty k tlaku všade nezmení žiadnu rýchlosť. Každý model
preto potrebuje aspoň jednu okrajovú podmienku *Pressure (explicit)*, pozri 1.7.
Či sa tlak chápe ako absolútny alebo pretlak, je na používateľovi - riešič
pracuje výlučne s jeho gradientom.

### 1.2 Riadiace rovnice

Riešič implementuje **Darcyho zákon** pre pomalé prúdenie jedinej nestlačiteľnej
tekutiny, ktorá vypĺňa póry tuhého porézneho telesa:

```
v = -( k / mu ) * ( grad(p) - rho * g )
```

spolu so zákonom zachovania hmotnosti nestlačiteľnej tekutiny

```
div( v ) = 0
```

čo spolu dáva potenciálovú rovnicu (Laplaceovho typu) pre tlak:

```
div( ( k / mu ) * ( grad(p) - rho * g ) ) = 0
```

| Symbol | Význam | Jednotky | Zdroj |
|---|---|---|---|
| `p` | tlak pórovej tekutiny | `Pa` | riešená veličina |
| `v` | Darcyho (filtračná) rýchlosť | `m/s` | dopočítaná, pozri 1.8 |
| `k` | vnútorná priepustnosť porézneho telesa | `m^2` | vlastnosť materiálu *Permeability to fluids* |
| `mu` | dynamická viskozita pórovej tekutiny | `kg/(m*s)` | vlastnosť materiálu *Dynamic viscosity* |
| `rho` | hustota pórovej tekutiny | `kg/m^3` | vlastnosť materiálu *Density* |
| `g` | gravitačné zrýchlenie | `m/s^2` | environmentálna podmienka, pozri 1.6 |

Pomer `k/mu` hrá úlohu, akú v tepelnom modeli hrá vodivosť. Bez gravitácie je
rovnica Laplaceovou rovnicou pre tlak a prúdenie smeruje priamo proti gradientu
tlaku. S gravitáciou člen `rho*g` spôsobí, že tekutina v pokoji nesie
hydrostatický gradient tlaku: `grad(p) = rho*g` dáva `v = 0`.

**Rýchlosť je Darcyho rýchlosť.** Je to objemový prietok na jednotku **celkového**
prierezu, pevnej fázy aj pórov spolu - nie rýchlosť tekutiny vo vnútri pórov.
Stredná rýchlosť v póroch je väčšia o prevrátenú hodnotu pórovitosti `n`:

```
v_pore = v / n
```

Pórovitosť do ustáleného prúdenia nevstupuje, takže ju riešič nepotrebuje ani
neprijíma. Použite ju ručne na odhad doby prechodu.

**Priepustnosť a hydraulická vodivosť.** Mechanika zemín a hydrogeológia často
uvádzajú namiesto vnútornej priepustnosti *hydraulickú vodivosť* (koeficient
filtrácie) `K` v `m/s` - koeficient Darcyho zákona zapísaného pre hydraulickú
výšku. Obe veličiny súvisia cez vlastnosti tekutiny:

```
k = K * mu / ( rho * |g| )
```

Pre vodu pri 20 °C zodpovedá `K = 1e-4 m/s` hodnote `k = 1.02e-11 m^2`. Ropné
inžinierstvo uvádza priepustnosť v darcy: `1 D = 9.869e-13 m^2`.

**Povinné vlastnosti materiálu.** Úloha prúdenia poréznym prostredím vyžaduje
**tri** z nich:

| Vlastnosť | Jednotky | Použitie |
|---|---|---|
| Permeability to fluids | `m^2` | matica tuhosti - priepustnosť pevnej fázy |
| Dynamic viscosity | `kg/(m*s)` | matica tuhosti - viskozita **tekutiny** |
| Density | `kg/m^3` | gravitačný člen - hustota **tekutiny** |

**Hustota a viskozita patria pórovej tekutine, nie poréznemu telesu.** Materiál
pre tento typ úlohy preto opisuje *nasýtené* prostredie: priepustnosť pevného
skeletu a hustotu a viskozitu tekutiny, ktorá ho vypĺňa. Takto sú zostavené
dodávané materiály s názvom *... (water saturated)* (pozri 2.3). Materiál, ktorý
uvádza objemovú hmotnosť suchej zeminy, skreslí hydrostatický tlak v pomere oboch
hustôt.

Entita, ktorej materiálu chýba niektorá z troch vlastností, sa **nerieši**.
Kontrola nastavenia na to upozorní pred spustením výpočtu. Jedinou výnimkou je
entita s okrajovou podmienkou tohto typu úlohy: tá sa do sústavy zahrnie bez
ohľadu na materiál a jej priepustnosť je potom jednoducho nulová. Takto dostane
prítok aj okrajová plocha bez materiálu.

Každá vlastnosť je tabuľka v závislosti od teploty. Úloha prúdenia poréznym
prostredím sama osebe nemá pole teploty, takže každá tabuľka sa vyhodnotí pri
`293.15 K`. Tabuľky sa uplatnia len vtedy, keď úloha *Heat transfer* zaradená
skôr v tom istom poradí krokov vypočítala teploty prvkov - tak prúdi teplejšia
tekutina s nižšou viskozitou rýchlejšie.

### 1.3 Diskretizácia metódou konečných prvkov

Slabá formulácia rovnice pre tlak dáva pre každý prvok

```
K = INT( B^T * ( k / mu ) * B )                  tuhosť
f = INT( B^T * ( k / mu ) * rho * g )            gravitačné zaťaženie
  + INT_S( N^T * v_in )                          prítok cez hranicu
```

kde `B` sa zostavuje z derivácií tvarových funkcií, `N` sú tvarové funkcie a
`v_in` je rýchlosť prítoku predpísaná na okrajovej ploche. Každá matica prvku má
rozmer `n x n` pre prvok s `n` uzlami. Matica hmotnosti ani akumulácie neexistuje:
prúdenie je ustálené, pozri 1.10.

**Miery prvkov.** Ako inde v Range FEA, entity nižšej dimenzie dostávajú
geometrickú mieru, aby mohli reprezentovať skutočné telesá:

| Entita | Miera | Kde sa nastavuje |
|---|---|---|
| Objem | 1 | - |
| Plocha | hrúbka plochy | geometrické vlastnosti entity |
| Čiara | prierez čiary | geometrické vlastnosti entity |
| Bod | - | body nevedú prúdenie |

**Plocha** s hrúbkou vedie prúdenie vo svojej rovine - takto sa zostavuje
dvojrozmerný model (rez hrádzou alebo zvodnenou vrstvou). Jej gravitačné
zaťaženie používa zložku `g` ležiacu v rovine plochy. Plocha s **nulovou
hrúbkou** - bežná okrajová plocha objemového modelu - nemá tuhosť a len prijíma
prítok.

**Čiara** s prierezom vedie prúdenie pozdĺž svojho smeru, pre jednorozmerný model
stĺpca alebo potrubia s náplňou. **Bod** neprispieva ničím. Bodový prvok, ktorého
uzol nepatrí žiadnemu inému počítanému prvku, zanechá v matici prázdny riadok -
bodovým entitám porézny materiál nepriraďujte.

### 1.4 Okrajové podmienky

| Okrajová podmienka | Typ | Použiteľná na | Zložky |
|---|---|---|---|
| Pressure (explicit) | explicitná (Dirichletova) | plocha | Pressure `[Pa]` |
| Velocity (inflow) | prirodzená (predpísaný tok) | plocha | Velocity `[m/s]` |
| Volumetric flow rate (inflow) | prirodzená (predpísaný tok) | plocha | Volumetric flow rate `[m^3/s]` |

Okrajová plocha bez akejkoľvek z nich je **nepriepustná**: neprechádza ňou žiadna
tekutina. To je prirodzená podmienka rovnice, takže steny, dno zvodnenej vrstvy a
roviny symetrie nepotrebujú žiadnu podmienku.

**Pressure (explicit)** je jediná explicitná podmienka. Každý uzol plochy sa
odstráni zo sústavy rovníc a drží sa na zadanom tlaku. Je to miesto, kde tekutina
model opúšťa (výtok, drén, voľná voda nádrže) alebo doň vstupuje pod známym
tlakom. Každý model potrebuje aspoň jednu. Novo priradená podmienka začína na
`1000 Pa`.

**Velocity (inflow)** predpisuje normálovú Darcyho rýchlosť cez plochu.
**Kladná** hodnota je prítok do porézneho telesa, **záporná** odtok (odber).
Hodnota je tok na jednotku plochy, takže celkový prietok rastie s plochou.

**Volumetric flow rate (inflow)** predpisuje **celkový** prietok plochou v
`m^3/s`. Riešič ho vydelí plochou celej entity a výsledok použije ako rovnomernú
rýchlosť prítoku, takže prietok sa nezmení pri zmene siete ani zväčšení plochy.
Kladná hodnota je opäť prítok.

Žiadna z podmienok prítoku tlak nepredpisuje: tlak na vstupe je súčasťou
výsledku. Kde majú plocha s prítokom a plocha s *Pressure (explicit)* spoločné
uzly, v týchto uzloch vyhráva tlak.

Plocha s prítokom **nepotrebuje materiál ani hrúbku**: do sústavy ju zaradí samotná
podmienka. Ak plocha porézny materiál a hrúbku má, vedie navyše prúdenie vo svojej
rovine.

Podmienka *Pressure (implicit)* riešiča nestlačiteľného prúdenia sa pre tento typ
úlohy neponúka.

Všetky zložky prijímajú tabuľku v závislosti od času, ktorá sa uplatní pri
zapnutom riešiči v čase, pozri 1.10.

### 1.5 Gravitácia

Gravitácia vstupuje cez environmentálnu podmienku *Gravitational acceleration*.
**Každá entita ju má predvolene s `g = (0, 0, -9.80665) m/s^2`**, takže gravitácia
smeruje pozdĺž zápornej osi Z, pokiaľ sa nezmení. Dva dôsledky stojí za to
poznať:

- V modeli, ktorého os Z smeruje nahor, dostane tekutina v pokoji automaticky
  hydrostatický tlak rastúci o `rho*|g|` na meter hĺbky. To je obvykle žiaduce.
- Model postavený s inou osou smerujúcou nahor musí mať podmienku upravenú na
  každej poréznej entite, inak tekutina „padá" do strany. Nastavením všetkých
  troch zložiek na nulu sa gravitácia vypne - napríklad pre vodorovný model, v
  ktorom zaujíma len rozdiel tlakov.

Gravitácia v kombinácii s plochou *Pressure (explicit)*, ktorá nie je vodorovná,
si vyžaduje pozornosť: rovnomerný tlak na zvislej ploche **nie je** hydrostatický,
takže pri tejto ploche tekutina cirkuluje. Buď zadajte tlak v závislosti od výšky
(rozdeľte plochu na viac entít), alebo gravitáciu vypnite, ak nie je súčasťou
otázky.

### 1.6 Environmentálne, počiatočné a väzbové podmienky

#### Environmentálne podmienky

| Environmentálna podmienka | Zložky | Účinok |
|---|---|---|
| Gravitational acceleration | g v X, Y, Z `[m/s^2]` | gravitačný člen Darcyho zákona, pozri 1.5 |
| Temperature | Temperature `[K]` | ponúka sa, ale nečíta - pozri nižšie |

Environmentálna podmienka *Temperature* sa ponúka, pretože vlastnosti materiálu sú
tabuľky v závislosti od teploty. Riešič ju **nečíta**: teploty prvkov preberá len
zo zdieľaných dát, ktoré zanechá úloha *Heat transfer*, a inak vyhodnocuje každú
tabuľku pri `293.15 K`.

#### Počiatočné podmienky

| Počiatočná podmienka | Účinok |
|---|---|
| Pressure | ponúka sa, nečíta sa |
| Velocity | ponúka sa, nečíta sa |

Prúdenie je ustálené, takže výsledok nezávisí od počiatočného poľa a riešič
nečíta ani jednu počiatočnú podmienku. Obe sa ponúkajú, pretože ich typ úlohy
zdieľa s úlohou nestlačiteľného prúdenia.

#### Väzba s inými riešičmi

| Smer | Veličina | Účinok |
|---|---|---|
| Teplo -> prúdenie poréznym prostredím | Temperature | vyberá riadok každej teplotne závislej tabuľky materiálu |

Tlak ani rýchlosť nepreberá žiadny iný riešič. Prúdenie poréznym prostredím sa dá
kombinovať s každou inou fyzikou okrem *Incompressible viscous flow* a *Modal
analysis*.

Riešič hlási konvergenciu bezpodmienečne, takže skupina krokov obsahujúca len
úlohu prúdenia poréznym prostredím sa vždy zastaví po jednej iterácii.

### 1.7 Riešenie sústavy

Riešič zostaví a vyrieši

```
K * p = f
```

Matica `K` je symetrická a kladne definitná, len čo je predpísaný aspoň jeden
tlak, preto sa rieši metódou **združených gradientov** s Jacobiho
predpodmienením. Zostavenie je paralelizované cez prvky s vyrovnávacími pamäťami
pre každé vlákno, ktoré sa na konci raz zlúčia, takže výsledok nezávisí od počtu
vlákien.

Priepustnosti skutočných materiálov siahajú od približne `1e-9 m^2` (štrk) až po
`1e-20 m^2` (neporušený íl), čím by sa prvky matice dostali blízko k hraniciam
zaokrúhľovania iteračného riešiča. Pred riešením sa preto každá rovnica vydelí
najväčším `k/mu` v modeli. Vydelenie všetkých rovníc rovnakým číslom riešenie
nezmení; len udrží iteráciu dobre podmienenú.

Model **bez okrajovej podmienky Pressure (explicit)** nemá jednoznačné riešenie.
Riešič to zistí a zastaví sa s chybou *Flow through porous media requires at
least one 'Pressure (explicit)' boundary condition*.

Ďalšie spôsoby, ako dospieť k singulárnej alebo prázdnej sústave, všetky tiché:

- entita, ktorej materiálu chýba jedna z troch povinných vlastností, sa nerieši a
  jej uzly zo sústavy vypadnú;
- entita s nulovou priepustnosťou alebo nulovou viskozitou nemá tuhosť, takže jej
  uzly v sústave zostanú, ale nie sú s ničím spojené;
- plocha s nulovou hrúbkou alebo čiara s nulovým prierezom nevedie prúdenie.

### 1.8 Odvodené výsledky

Po riešení sa rýchlosť prvku dopočíta z uzlového poľa tlaku. Gradient tlaku sa
spriemeruje cez integračné body prvku a skombinuje s gravitačným členom; pre
čiarové a plošné prvky sa otočí do globálnych súradníc:

```
v = -( k / mu ) * ( grad(p) - rho * g )
```

| Výsledok | Aplikovaný na | Význam |
|---|---|---|
| Pressure `[Pa]` | uzol | vyriešené pole pórového tlaku |
| Velocity `[m/s]` | prvok | vektor Darcyho rýchlosti |

Rýchlosť je **hustota** v tom istom zmysle ako výsledky polí ostatných riešičov:
nezávisí od veľkosti prvku, v lineárnom prvku je konštantná a pri zjemňovaní
siete konverguje. Okrajové plochy s nulovou hrúbkou ukazujú nulovú rýchlosť -
výsledok zobrazujte na objeme.

Celkový prietok sa ako výsledok neukladá. Vypočítajte ho ručne z rýchlosti a
plochy prierezu alebo z podmienok prítoku, pozri 5.

Po zapísaní záznamu vypíše log riešiča štatistiky oboch veličín a hodnoty v
monitorovacích bodoch.

### 1.9 Platnosť Darcyho zákona

Darcyho zákon platí pre pomalé laminárne prúdenie, v ktorom viskózne sily
prevládajú nad zotrvačnosťou tekutiny. Bežné kritérium používa Reynoldsovo číslo
vytvorené zo stredného priemeru zrna `d`:

```
Re = rho * |v| * d / mu  <  1 ... 10
```

Prúdenie podzemnej vody v zeminách, presakovanie hrádzami a násypmi, prúdenie
filtrami, sypanými vrstvami a väčšinou poréznych technických materiálov ho spĺňa
s veľkou rezervou. Prúdenie hrubým štrkom, kamennou nasypaninou alebo veľmi
otvorenými penami pri vysokých rýchlostiach nie; Darcyho zákon potom podhodnocuje
tlakovú stratu (Forchheimerov režim) a tento riešič vykáže príliš veľkú rýchlosť.

### 1.10 Ustálené prúdenie a riešič v čase

Typ úlohy povoľuje riešič v čase, ale samotné prúdenie je **ustálené**: tekutina
aj porézne teleso sú nestlačiteľné a neexistuje člen akumulácie. Pri zapnutom
riešiči v čase rieši každý časový krok samostatný ustálený stav s hodnotami
podmienok v danom čase. To je správny model, keď sa okrajové podmienky menia
pomaly v porovnaní s časom, za ktorý sa tlak ustáli - hladina nádrže nad
pieskovým filtrom meniaca sa v priebehu hodín - a umožňuje úlohe prúdenia
poréznym prostredím zdieľať poradie krokov s prechodovou tepelnou úlohou.

Konsolidácia, odvodnenie nenasýtenej zeminy, prúdenie stlačiteľného plynu a
akýkoľvek iný jav, pri ktorom sa množstvo tekutiny uloženej v póroch mení v čase,
sú mimo modelu, pozri kapitolu 6.

---

## 2. Grafické používateľské rozhranie

Všetko podstatné sa nachádza v doku **Riešič** (karty `Úloha`,
`Okrajové podmienky`, `Počiatočné podmienky`, `Environmentálne podmienky`,
`Materiál`, `Výsledky`), v ponuke **Úloha** a v doku **Model**.

### 2.1 Výber úlohy

`Úloha` -> `Poradie krokov úloh` (`Ctrl+P`) otvorí dialóg poradia krokov úloh.
Pridajte krok a vyberte **Flow through porous media**.

Typ úlohy sa nedá kombinovať s *Incompressible viscous flow* ani s *Modal
analysis*. Krok *Heat transfer* zaradený pred ním zabezpečí, že tabuľky materiálu
budú sledovať vypočítanú teplotu.

### 2.2 Karta Úloha

Karta zobrazuje skupinu *Riešič v čase*, pretože typ úlohy riešič v čase
podporuje - čo prechodový výpočet znamená tu, vysvetľuje 1.10. Prúdenie
poréznym prostredím nemá vlastnú skupinu nastavení. Všetko sa priraďuje entitám
na kartách podmienok.

### 2.3 Karta Materiál

Karta `Materiál` zobrazuje materiál priradený vybranej entite. Prúdenie
poréznym prostredím vyžaduje **všetky tri** z týchto vlastností:

| Vlastnosť | Jednotky | Poznámka |
|---|---|---|
| Permeability to fluids | `m^2` | vnútorná priepustnosť pevnej fázy, pozri tabuľku nižšie |
| Dynamic viscosity | `kg/(m*s)` | pórovej tekutiny, `1.003e-3` pre vodu pri 20 °C |
| Density | `kg/m^3` | pórovej tekutiny, `998.2` pre vodu pri 20 °C |

Dodávaná knižnica obsahuje vodou nasýtené porézne materiály pripravené na
použitie. Každý nesie teplotne závislú hustotu a viskozitu vody a typickú
priepustnosť:

| Materiál | Priepustnosť `[m^2]` | Hydraulická vodivosť pri 20 °C `[m/s]` |
|---|---|---|
| Gravel (water saturated) | `1e-9` | `1e-2` |
| Coarse Sand (water saturated) | `1e-10` | `1e-3` |
| Fine Sand (water saturated) | `1e-12` | `1e-5` |
| Sandstone (water saturated) | `1e-13` | `1e-6` |
| Silt (water saturated) | `1e-14` | `1e-7` |
| Clay (water saturated) | `1e-17` | `1e-10` |

Priepustnosť skutočnej zeminy sa môže od týchto hodnôt líšiť o rád aj viac - sú
to východiskové hodnoty, nie údaje o konkrétnej lokalite. Na modelovanie inej
tekutiny niektorý z nich skopírujte a nahraďte hustotu a viskozitu. Obyčajný
materiál *Sand* je suchý sypký materiál bez viskozity, preto je pre tento typ
úlohy označený ako neplatný.

### 2.4 Karta Okrajové podmienky

Karta je rozdelená na zoznam dostupných okrajových podmienok hore a editor
vybranej podmienky dole. Najprv vyberte plošnú entitu v strome `Model`; zoznam
potom ponúkne podmienky z kapitoly 1.4. Zaškrtnutím ju priradíte a jej zložky
upravíte v dolnom strome.

- **Pressure (explicit)** drží tlak na výtoku. Model bez nej sa nevyrieši.
- **Velocity (inflow)** a **Volumetric flow rate (inflow)** privádzajú do modelu
  známy prietok. Kladné hodnoty tečú dnu, záporné von.
- Plochy bez podmienky sú nepriepustné.

### 2.5 Počiatočné a environmentálne podmienky

Karta `Environmentálne podmienky` ponúka *Gravitational acceleration*, ktorú už
každá entita má s hodnotou `(0, 0, -9.80665) m/s^2` (pozri 1.5), a
*Temperature*, ktorú tento riešič ignoruje.

Karta `Počiatočné podmienky` ponúka *Pressure* a *Velocity*, ktoré tento riešič
nečíta.

### 2.6 Nastavenie maticového riešiča

`Úloha` -> `Nastaviť maticový riešič úloh` nastavuje iteračné riešiče. Riešič
prúdenia poréznym prostredím používa položku **CG**. Ak riešenie nekonverguje,
hľadajte najskôr entitu s nulovou priepustnosťou alebo izolovaný bodový prvok,
nie nastavenie riešiča.

### 2.7 Monitorovacie body

`Úloha` -> `Definovať monitorovacie body` umiestni sondy na zadané súradnice a
vyberie zaznamenávanú veličinu - *Pressure* alebo *Velocity*. `Výkaz` ->
`Monitorovacie body` zobrazí hodnoty.

### 2.8 Výsledky a záznamy

Karta `Výsledky` zobrazuje vypočítané veličiny a ovláda 3D zobrazenie. Tlak je
uzlový skalár, rýchlosť vektor na prvkoch. Zobrazte rýchlosť ako vektorové pole
na objeme, aby ste videli dráhy prúdenia.

Karta `Záznamy` doku `Model` obsahuje záznamy výsledkov: jeden pri ustálenom
výpočte, jeden na každý výstupný časový krok pri zapnutom riešiči v čase.

`Výkaz` -> `Log súbor riešiča` zobrazí úplný výstup riešiča vrátane štatistík
oboch veličín.

---

## 3. Tutoriál - prúdenie pieskovým filtrom poháňané tlakom

**Cieľ.** Vypočítať prúdenie vody vodorovným pieskovým filtrom poháňané známym
rozdielom tlakov a overiť prietok podľa Darcyho zákona.

Tutoriál predpokladá objemový model kvádra so sieťou - použite `Súbor` ->
`Nový model`, nakreslite kváder cez `Geometria` -> `Vytvoriť prvok` alebo
importujte geometriu a vytvorte tetraédrovú sieť; tutoriál *Draw cube* tento
postup opisuje. Obe koncové plochy označte ako samostatné plošné entity, aby sa
im dali priradiť podmienky.

Filter nech je `1 m` dlhý pozdĺž osi X s prierezom `0.1 m` x `0.1 m`.

### Krok 1 - výber úlohy

1. `Úloha` -> `Poradie krokov úloh` (`Ctrl+P`).
2. Pridajte krok a vyberte **Flow through porous media**.
3. Potvrďte tlačidlom `OK`.

*Riešič v čase* nechajte vypnutý - výsledkom je jediný ustálený stav.

### Krok 2 - priradenie materiálu

1. Vyberte objemovú entitu v strome `Model`.
2. Otvorte kartu `Materiál` a priraďte **Coarse Sand (water saturated)**.

Materiál nesie priepustnosť piesku `1e-10 m^2` a hustotu a viskozitu vody.

### Krok 3 - vypnutie gravitácie

Filter je vodorovný a zaujíma len rozdiel tlakov, preto predvolenú gravitáciu
odstráňte:

1. Pri vybranej objemovej entite otvorte kartu `Environmentálne podmienky`.
2. Vyberte **Gravitational acceleration** a všetky tri zložky nastavte na `0`.

S ponechanou gravitáciou by voda niesla aj hydrostatický tlak cez výšku filtra
`0.1 m` a rovnomerné tlaky na zvislých koncových plochách by spôsobili jej
cirkuláciu pri nich - pozri 1.5.

### Krok 4 - predpísanie oboch koncov

1. Vyberte plošnú entitu v `x = 0`.
2. Otvorte kartu `Okrajové podmienky`, zaškrtnite **Pressure (explicit)** a
   nastavte `10000` Pa - približne jeden meter vodného stĺpca.
3. Vyberte plošnú entitu v `x = 1 m`, zaškrtnite **Pressure (explicit)** a
   nastavte `0` Pa.

Štyri bočné plochy nedostanú žiadnu podmienku: sú to nepriepustné steny.

### Krok 5 - riešenie

1. `Riešenie` -> `Spustiť riešič` (`Ctrl+R`).
2. Priebeh sledujte vo `Výkaz` -> `Log súbor riešiča`.

Výpočet je jediné riešenie metódou združených gradientov.

### Krok 6 - prezretie výsledkov

- Karta `Výsledky`: zobrazte **Pressure**. Klesá lineárne z `10000` Pa na `0` Pa
  pozdĺž filtra, s rovinnými izobarami kolmými na jeho os.
- Zobrazte **Velocity**. Smeruje pozdĺž +X a je rovnaká v každom prvku.
- Monitorovací bod na *Pressure* v polovici filtra ukáže `5000` Pa.

### Krok 7 - ručný výpočet

S `k = 1e-10 m^2` a `mu = 1.003e-3 kg/(m*s)` pre vodu pri 20 °C:

```
v = ( k / mu ) * dp / L   = 9.97e-8 * 10000 / 1     = 9.97e-4  m/s
Q = v * A                 = 9.97e-4 * 0.01          = 9.97e-6  m^3/s  (0.6 l/min)
```

Výsledok **Velocity** musí ukázať `9.97e-4 m/s`. Reynoldsovo číslo pri veľkosti
zrna `1 mm` je `998 * 9.97e-4 * 1e-3 / 1.003e-3 = 1`, na hranici Darcyho režimu
(1.9). Pri pórovitosti `0.35` sa samotná voda pohybuje rýchlosťou
`9.97e-4 / 0.35 = 2.8e-3 m/s` a filtrom prejde asi za šesť minút.

### Riešenie problémov

| Príznak | Príčina |
|---|---|
| *Flow through porous media requires at least one 'Pressure (explicit)' boundary condition* | nikde nie je predpísaný tlak na výtoku |
| *Entity has material assigned which is missing required properties* | materiálu chýba viskozita - často suchý materiál zeminy ako *Sand* |
| Tlak sa mení s výškou a rýchlosť má zložku Z | predvolená gravitácia je stále zapnutá, pozri krok 3 |
| Rýchlosť je všade nulová | oba konce majú rovnaký tlak alebo je priepustnosť nulová |

---

## 4. Tutoriál - vzostupné presakovanie zvislým stĺpcom

**Cieľ.** Pretlačiť vodu nahor zvislým pieskovým stĺpcom známym prietokom a
ukázať, ako sa tlak na vstupe delí na hydrostatickú a trecú časť.

Použite kváder široký `0.1 m` x `0.1 m` a vysoký `1 m` pozdĺž osi **Z**, so
spodnou plochou (`z = 0`) a hornou plochou (`z = 1 m`) ako samostatnými plošnými
entitami.

### Krok 1 - úloha a materiál

1. Vyberte **Flow through porous media** ako v tutoriáli 3.
2. Objemu priraďte **Coarse Sand (water saturated)**.

### Krok 2 - ponechanie gravitácie

Stĺpec stojí pozdĺž osi Z, takže predvolená gravitácia `(0, 0, -9.80665) m/s^2`
smeruje správne. Podmienku *Gravitational acceleration* nechajte tak.

### Krok 3 - okrajové podmienky

1. Vyberte hornú plochu, zaškrtnite **Pressure (explicit)** a nastavte `0` Pa:
   voľná voda nad stĺpcom.
2. Vyberte spodnú plochu, zaškrtnite **Volumetric flow rate (inflow)** a
   nastavte `1e-6` m^3/s. Na ploche `0.01 m^2` to zodpovedá rýchlosti prítoku
   `1e-4 m/s`.

Horná plocha je vodorovná, takže rovnomerný tlak na nej je v súlade s
gravitáciou.

### Krok 4 - riešenie a prezretie

`Riešenie` -> `Spustiť riešič` (`Ctrl+R`), potom zobrazte **Pressure** a
**Velocity**.

- Rýchlosť je `1e-4 m/s` smerom nahor (+Z) v každom prvku.
- Tlak rastie lineárne z `0` Pa na vrchu po maximum na dne.

### Krok 5 - rozdelenie tlaku na vstupe

Tlak na dne musí zdvihnúť vodný stĺpec a pretlačiť ho pieskom:

```
p_hydrostatic = rho * g * L            = 998.2 * 9.80665 * 1       = 9789 Pa
p_friction    = mu * v * L / k         = 1.003e-3 * 1e-4 * 1 / 1e-10 = 1003 Pa
p_bottom      = p_hydrostatic + p_friction                         = 10792 Pa
```

Monitorovací bod na spodnej ploche súčet potvrdí. Nastavte prietok na nulu a
riešte znova: rýchlosť zmizne a tlak na dne klesne na čisto hydrostatických
`9789 Pa` - riešič presne reprodukuje tekutinu v pokoji.

Zvyšujte prietok, kým trecia časť nedosiahne vztlakom zníženú tiaž piesku,
približne `(rho_s - rho) * g * (1 - n) * L`: to je začiatok fluidizácie
(tekutý piesok), ktorú tuhý skelet tohto modelu nedokáže vystihnúť - riešič
jednoducho počíta stále väčší tlak.

### Riešenie problémov

| Príznak | Príčina |
|---|---|
| Tlak na dne je len trecia časť | gravitácia je na objeme vypnutá |
| Tlak na dne je len hydrostatická časť | prítok je na nesprávnej ploche alebo má nulovú hodnotu |
| Rýchlosť smeruje do strany | stĺpec bol postavený pozdĺž inej osi ako Z; upravte podmienku gravitácie |

---

## 5. Kontrola modelu

**Rovnomerný stĺpec.** Pre teleso dĺžky `L` držané na `p1` a `p2` na dvoch
protiľahlých plochách, z jedného materiálu a s vypnutou gravitáciou, je tlak
pozdĺž telesa lineárny a

```
p(x) = p1 + ( p2 - p1 ) * x / L
v    = ( k / mu ) * ( p1 - p2 ) / L
```

Výsledok rýchlosti musí súhlasiť na toleranciu riešiča a tlak v strednej rovine
musí byť priemerom oboch predpísaných hodnôt.

**Hydrostatika.** Teleso s gravitáciou, s podmienkou *Pressure (explicit)* len na
vodorovnej hornej ploche a inak nepriepustnými stenami musí ukázať nulovú
rýchlosť a tlak rastúci o `rho*|g|` na meter hĺbky. Tým sa naraz overí hustota aj
podmienka gravitácie.

**Bilancia prietoku.** V ustálenom modeli musí všetka tekutina, ktorá vstúpi, aj
vystúpiť. Pri prítoku predpísanom cez *Volumetric flow rate (inflow)* musí
rýchlosť v priereze vynásobená jeho plochou vrátiť predpísaný prietok. Rýchlosť v
stĺpci odčítajte monitorovacím bodom.

**Sériové vrstvy.** Pre dve vrstvy dĺžky `L1`, `L2` a priepustnosti `k1`, `k2`
zapojené za sebou medzi dvoma predpísanými tlakmi kladie prúdenie odpor ako dva
odpory v sérii:

```
v  = ( p1 - p2 ) / ( mu * ( L1 / k1 + L2 / k2 ) )
pi = p1 - v * mu * L1 / k1          tlak na rozhraní
```

Na rozdiel od elektrostatického riešiča tento delí tlak v pomere
**priepustností**, ako by to robila sieť odporov.

**Radiálne prúdenie k studni.** Pre napätú zvodnenú vrstvu hrúbky `H` medzi
studňou s polomerom `rw` držanou na `pw` a vonkajším polomerom `R` držaným na
`pR` je tlak logaritmický a výdatnosť studne je

```
p(r) = pw + ( pR - pw ) * ln( r / rw ) / ln( R / rw )
Q    = 2 * pi * H * ( k / mu ) * ( pR - pw ) / ln( R / rw )
```

Tým sa riešič overí na nerovnomernom poli a ukáže sa, či je sieť pri studni dosť
jemná.

Sieť aspoň raz zjemnite a overte, že tlak konverguje a neposúva sa.

---

## 6. Obmedzenia

- **Ustálené, nestlačiteľné, jednofázové prúdenie.** Neexistuje člen akumulácie,
  takže konsolidácia, prúdenie stlačiteľného plynu a prechodová odozva napätej
  zvodnenej vrstvy sa nemodelujú. Prechodový výpočet je postupnosť ustálených
  stavov, pozri 1.10.
- **Len plne nasýtené prostredie.** Neexistuje voľná hladina (depresná krivka) ani
  nenasýtená zóna. Presakovanie násypom, ktorého hladinu podzemnej vody treba
  nájsť, je mimo modelu, pokiaľ sa nasýtená oblasť nevytvorí explicitne sieťou.
- **Tuhé porézne teleso.** Skelet sa nedeformuje, takže neexistuje väzba na
  analýzu napätia, efektívne napätie, konsolidácia ani fluidizácia.
- **Izotropná priepustnosť.** Priepustnosť je skalár. Vrstevnaté zeminy s rôznou
  vodorovnou a zvislou priepustnosťou treba modelovať ako samostatné vrstvy.
- **Len Darcyho režim.** Zotrvačné (Forchheimerove) a turbulentné prúdenie vo
  veľmi hrubých materiáloch sa nemodeluje, pozri 1.9.
- **Okrajové podmienky len na plochách.** Tlaky a prítoky sa predpisujú na
  plošných entitách; neexistuje výronová plocha, studňa ani čiarový zdroj, ani
  predpísaný tlak na čiare alebo v bode.
- **Žiadny výsledok celkového prietoku.** Prietoky prierezom treba vypočítať z
  rýchlosti ručne.
- **Počiatočné podmienky sa nečítajú** a environmentálna podmienka *Temperature*
  nemá účinok; tabuľky materiálu sledujú teplotu len cez tepelnú úlohu zaradenú
  skôr v poradí krokov.
- **Model potrebuje predpísaný tlak.** Bez neho sa riešič zastaví s chybou.

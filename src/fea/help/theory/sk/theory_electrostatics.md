# Elektrostatika - Teoretický manuál a používateľská príručka

Tento dokument opisuje riešič elektrostatiky (`RSolverElectrostatics`) v Range
FEA, ktorý rieši typ úlohy **Elektrostatika** (*Electro-statics*): rovnice, ktoré
rieši, význam každého vstupu, ktorý prijíma, časti grafického používateľského
rozhrania, ktoré ho ovládajú, a dva podrobne rozpracované tutoriály.

**Obsah**

1. [Teoretický základ](#1-teoretický-základ)
2. [Grafické používateľské rozhranie](#2-grafické-používateľské-rozhranie)
3. [Tutoriál - potenciál a prúd vo vodiči](#3-tutoriál---potenciál-a-prúd-vo-vodiči)
4. [Tutoriál - odporový ohrev](#4-tutoriál---odporový-ohrev)
5. [Kontrola modelu](#5-kontrola-modelu)
6. [Obmedzenia](#6-obmedzenia)

---

## 1. Teoretický základ

### 1.1 Primárna neznáma - pole elektrického potenciálu

Primárnou neznámou je **elektrický potenciál** `V` v každom uzle, vo voltoch.
Každý uzol nesie jediný stupeň voľnosti, takže sieť s `N` uzlami dáva sústavu `N`
rovníc. Elektrické pole, prúdová hustota a všetko, čo sa z nich odvodzuje, sa po
riešení získa z poľa potenciálu.

Potenciál je skalár, vďaka čomu je elektrostatický model rovnako lacný ako
tepelný model na tej istej sieti: jedna neznáma na uzol namiesto troch a matica
s deviatinou prvkov štrukturálneho modelu.

Predvolený potenciál uzla, ktorého sa nedotýka žiadna podmienka ani
predchádzajúci výsledok, je `0 V`.

Pole potenciálu je definované iba **až na aditívnu konštantu**. Nič v rovniciach
neurčuje nulovú úroveň, takže každý model potrebuje aspoň jednu okrajovú
podmienku *Elektrický potenciál* - pozri 1.7. Je to elektrostatický ekvivalent
neupevnenej konštrukcie alebo tepelného modelu bez predpísanej teploty.

### 1.2 Riadiaca rovnica

Riešič implementuje **rovnicu elektrostatického poľa** - Poissonovu rovnicu pre
potenciál lineárneho izotropného dielektrika:

```
div( e0 * er * grad(V) ) = -rho
```

a bez hustoty náboja kdekoľvek sa redukuje na Laplaceovu rovnicu

```
div( e0 * er * grad(V) ) = 0
```

Elektrické pole je záporný gradient potenciálu a prúdová hustota vyplýva z
Ohmovho zákona:

```
E = -grad(V)
J = sigma * E
```

| Symbol | Význam | Jednotky | Zdroj |
|---|---|---|---|
| `V` | elektrický potenciál | `V` | riešený |
| `rho` | hustota náboja | `C/m^3`, pozri 1.5 | podmienka Hustota náboja |
| `e0` | permitivita vákua | `F/m` | zabudovaná, `8.854187817e-12` |
| `er` | relatívna permitivita | `-` | vlastnosť materiálu |
| `sigma` | elektrická vodivosť | `S/m` | vlastnosť materiálu |
| `E` | elektrické pole | `V/m` | odvodené, pozri 1.8 |
| `J` | prúdová hustota | `A/m^2` | odvodená, pozri 1.8 |

Permitivita vákua je konštantou riešiča, nie vlastnosťou materiálu ani
nastavením - s materiálom sa zadáva iba **relatívna** permitivita.

**Pole poháňa permitivita, nie vodivosť.** Toto je najdôležitejšia vec, ktorú
treba o tomto riešiči vedieť. Matica tuhosti sa zostavuje iba z `e0*er`;
vodivosť do sústavy rovníc nikdy nevstupuje. Používa sa až potom, na prevod
vypočítaného poľa na prúdovú hustotu, rezistivitu a Joulovo teplo. Dôsledok
rozoberá 1.9: v telese z jedného materiálu to nemá žiadny vplyv, ale v telese z
**dvoch** sa potenciál rozdelí v pomere permitivít, nie v pomere odporov.

**Požadované vlastnosti materiálu.** Elektrostatická úloha vyžaduje **tri**:

| Vlastnosť | Jednotky | Používa sa na |
|---|---|---|
| Relatívna permitivita | `-` | maticu tuhosti |
| Elektrická vodivosť | `S/m` | prúdovú hustotu, rezistivitu, Joulovo teplo |
| Hustota | `kg/m^3` | nič - vyžadovaná, nikdy nečítaná |

Entita, ktorej materiálu niektorá z troch chýba, sa **nerieši**, hoci hustotu
tento riešič vôbec nečíta. Kontrola nastavenia na to upozorní pred spustením
výpočtu. Jedinou výnimkou je entita s elektrostatickou okrajovou podmienkou: tá
sa do sústavy zahrnie bez ohľadu na materiál a jej permitivita a vodivosť sú
potom jednoducho nulové.

Nulová permitivita nie je neškodná predvolená hodnota - prvok s `er = 0` nemá
žiadnu tuhosť, takže do sústavy neprispieva ničím a jeho uzly nechá voľne
plávať. Ak sa má entita riešiť, dajte jej skutočnú permitivitu: `1` pre vákuum
alebo, dostatočne presne, pre vzduch.

Každá vlastnosť je tabuľkou v závislosti od teploty, ale samotný krok
elektrostatiky nemá teplotné pole, pri ktorom by ju čítal, takže každá tabuľka sa
vyhodnotí pri `293.15 K`. Tabuľky sa rešpektujú iba vtedy, keď krok *Prestup
tepla* skôr v tom istom poradí krokov vypočítal teploty prvkov, ktoré sa zdieľajú
s krokmi nasledujúcimi po ňom.

Relatívna permitivita je **bezrozmerná** - v používateľskom rozhraní sa zobrazuje
ako `N/A`, rovnako ako emisivita a Poissonovo číslo - a jej hodnota sa používa
ako obyčajný násobiteľ `e0`. Zadajte `1` pre vákuum, `1.00059` pre vzduch, `2.1`
pre PTFE, asi `4` pre sklo.

### 1.3 Diskretizácia metódou konečných prvkov

Slabá formulácia rovnice poľa dáva pre každý prvok

```
K = INT( B^T * e0 * er * B )        stiffness
f = INT( N^T * rho )                charge source
```

(maticu tuhosti a zdroj náboja), kde `B` sa zostavuje z derivácií tvarových
funkcií a `N` sú samotné tvarové funkcie. Každá matica prvku s `n` uzlami má
rozmer `n x n` - jeden riadok na uzol, nie tri. Neexistuje matica hmotnosti ani
kapacity: úloha je statická a 1.10 vysvetľuje, čo to znamená pre poradie krokov.

**Miery prvkov.** Ako inde v Range FEA, entity nižšej dimenzie dostávajú
geometrickú mieru, aby mohli reprezentovať skutočné telesá:

| Entita | Miera | Kde sa nastavuje |
|---|---|---|
| Objem | 1 | - |
| Plocha | hrúbka plochy | geometrické vlastnosti entity |
| Čiara | prierez čiary | geometrické vlastnosti entity |
| Bod | objem bodu | geometrické vlastnosti entity |

**Plocha** vedie pole vo vlastnej rovine, cez prierez rovný jej hrúbke.
Neexistuje gradient potenciálu naprieč hrúbkou: plošný prvok je hrubý jeden
uzol, takže je z princípu konštrukcie naprieč hrúbkou ekvipotenciálny. Plocha s
**nulovou hrúbkou** preto nemá vôbec žiadnu tuhosť a nič nespája - na rozdiel od
tepelného riešiča, kde plocha s nulovou hrúbkou stále nesie konvekciu, je tu
plocha s nulovou hrúbkou jednoducho inertná. Plošnej entite dajte hrúbku vždy,
keď má viesť, a pamätajte, že stena s nulovou hrúbkou nedostane ani **žiadne
odvodené výsledky** (1.8).

**Čiara** vedie pole pozdĺž vlastného smeru, cez svoj prierez. **Bod** nemá smer,
pozdĺž ktorého by pole viedol, takže neprispieva **vôbec žiadnou tuhosťou**: nesie
iba zdroj náboja. Bodový prvok, ktorého uzol nezdieľa žiadny iný počítaný prvok,
preto vytvorí v matici prázdny riadok, s ktorým si metóda združených gradientov
neporadí - bod je miesto na vloženie náboja alebo na predpísanie potenciálu, nie
vodič.

### 1.4 Zdroje a okrajové podmienky

| Okrajová podmienka | Typ | Aplikuje sa na | Zložky |
|---|---|---|---|
| Elektrický potenciál (*Electric potential*) | explicitná (Dirichletova) | bod, čiara, plocha, objem | Elektrický potenciál `[V]` |
| Hustota náboja (*Charge density*) | prirodzená, zdroj | bod, čiara, plocha, objem | Hustota náboja, pozri 1.5 |

Tieto dve tvoria celý slovník tohto typu úlohy. Neexistuje predpísaný tok,
povrchový náboj, podmienka vzdialeného poľa alebo otvorenej hranice ani
podmienka symetrie - pozri časť 6.

**Elektrický potenciál** je jedinou explicitnou podmienkou elektrostatickej
úlohy. Každý uzol entity, ku ktorej je priradená, sa odoberie zo sústavy rovníc a
drží sa na zadanej hodnote; jeho príspevok sa prenesie na pravú stranu
susedných rovníc. Každý model potrebuje aspoň jednu takú podmienku. Novo
priradená podmienka začína na `1 V`.

Kde sa entity prekrývajú, **vyhráva entita nižšej dimenzie**: potenciál
predpísaný na ploche prepíše potenciál predpísaný na objeme za ňou v uzloch,
ktoré zdieľajú, čiara prepíše plochu a bod prepíše všetko. Potenciály sa
nekombinujú - v uzle je iba jeden stupeň voľnosti, ktorý treba držať, takže
naposledy načítaná podmienka jednoducho prevládne.

**Hustota náboja** je zdrojový člen Poissonovej rovnice. Na rozdiel od podmienky
*Teplo* tepelného riešiča sa jej hodnota **nerozkladá** na mieru entity: zadané
číslo sa aplikuje na každý prvok entity tak, ako je, ako hustota. Celkový náboj
umiestnený v objemovej entite je preto hodnota vynásobená objemom tejto entity a
pri zväčšení entity rastie - nezostáva na mieste tak ako hodnota *Tepla* vo
wattoch.

| Entita | Čo hodnota znamená |
|---|---|
| Objem | náboj na jednotku objemu |
| Plocha | náboj na jednotku objemu prierezu váženého hrúbkou |
| Čiara | náboj na jednotku objemu prierezu |
| Bod | uzlový zdroj náboja |

Obe zložky prijímajú tabuľku v čase. Keďže samotná úloha je statická, časový
priebeh má účinok iba vtedy, keď poradie krokov zdieľa iný, časovo závislý krok
- a ako vysvetľuje 1.10, aj táto kombinácia je obmedzená.

### 1.5 Hodnota hustoty náboja

Zložka *Hustota náboja* je **objemová hustota náboja** v `C/m^3`, ako hovorí jej
popis v používateľskom rozhraní. Je to hustota, nie celková hodnota, vďaka čomu
nezávisí od siete - a ako uvádza 1.4, práve to ju odlišuje od podmienky *Teplo*
tepelného riešiča, ktorá preberá celkovú hodnotu vo wattoch a rozloží ju za vás.

Znamienko sleduje obvyklú konvenciu. Integrácia `div(e0*er*grad(V)) = -rho` per
partes ponechá náboj na pravej strane s kladným znamienkom,

```
every entity type      f = +INT( N^T * rho )
```

(pre každý typ entity), takže **kladná** hustota náboja zvyšuje potenciál okolo
seba a záporná ho znižuje, rovnako na objemových, plošných, čiarových aj bodových
entitách.

Pamätajte, že samotný zdroj náboja z modelu riešiteľný model nerobí. Potenciál je
stále definovaný iba až na konštantu, kým ho niečo neurčí, takže model poháňaný
nábojom potrebuje aj aspoň jednu podmienku *Elektrický potenciál* - na vonkajšej
hranici oblasti so sieťou, ak nie inde. Pozri 1.7.

### 1.6 Environmentálne, počiatočné a väzobné podmienky

#### Environmentálne podmienky

| Environmentálna podmienka | Zložky | Účinok |
|---|---|---|
| Teplota (*Temperature*) | Teplota `[K]` | ponúkaná, ale nečítaná - pozri nižšie |

Environmentálna podmienka *Teplota* sa pre elektrostatickú úlohu ponúka, pretože
vlastnosti materiálu sú tabuľkami v závislosti od teploty. Riešič elektrostatiky
ju **nečíta**: teploty prvkov preberá iba zo zdieľaných údajov, ktoré zanechal
krok *Prestup tepla*, a inak vyhodnocuje každú tabuľku pri `293.15 K`. Jej
priradenie modelu iba s elektrostatikou nič nezmení.

#### Počiatočné podmienky

| Počiatočná podmienka | Účinok |
|---|---|
| Elektrický potenciál | počiatočné pole potenciálu, aplikované pri prvom výpočte |

Počiatočný potenciál sa číta **iba pri prvom výpočte**. Keďže riešenie je priame,
nie časový pochod, skonvergovanú odpoveď nemení - určuje iba počiatočný bod
iterácie metódy združených gradientov a hodnotu každého uzla, ktorý riešič zo
sústavy vynechá.

Environmentálne podmienky, počiatočné podmienky a okrajové podmienky sa čítajú v
tomto poradí a každá prepíše predchádzajúcu, takže **okrajová podmienka vyhráva
nad počiatočnou, ktorá vyhráva nad environmentálnou**.

#### Väzba s inými riešičmi

Riešič elektrostatiky vytvára dva výsledky, ktoré spotrebúvajú iné riešiče.

| Smer | Premenná | Účinok |
|---|---|---|
| Elektrostatika -> teplo | Joulovo teplo | pripočíta sa k zdrojovému členu každého prvku |
| Elektrostatika -> teplo v tekutinách | Joulovo teplo | pripočíta sa k zdrojovému členu každého prvku |
| Elektrostatika -> magnetostatika | Prúdová hustota | zdroj magnetického poľa |
| Teplo -> elektrostatika | Teplota | vyberá riadok každej teplotne závislej tabuľky materiálu |

**Joulovo teplo** preberie krok *Prestup tepla* zaradený v poradí krokov úloh
**za** krokom elektrostatiky, kde sa pripočíta k zdrojovému členu prvku presne
tak, ako by to urobila podmienka *Tepelný výkon (na jednotku objemu)*. Odporový
ohrev preto nepotrebuje žiadne nastavenie okrem zoradenia oboch krokov -
prechádza ním tutoriál 4.

**Magnetostatika** je samostatný typ úlohy, ktorý *vyžaduje* krok elektrostatiky:
číta prvkovú prúdovú hustotu, ktorú tento riešič ukladá - rovnako na objemových,
plošných aj čiarových entitách - a vyhodnocuje magnetické pole tohto prúdu
Biotovým-Savartovým zákonom. Krok elektrostatiky pridajte ako prvý.

Keďže riešič hlási konvergenciu bezpodmienečne, skupina krokov obsahujúca iba
krok elektrostatiky sa vždy zastaví po jednej iterácii. Iterovať sa oplatí iba
vtedy, keď je niečo, od čoho riešenie závisí, samo oneskorené - napríklad silne
teplotne závislá permitivita napájaná krokom prestupu tepla.

### 1.7 Riešenie sústavy

Riešič zostaví a vyrieši

```
K * V = f
```

`K` je symetrická pozitívne definitná, len čo je predpísaný aspoň jeden
potenciál, takže sa rieši **metódou združených gradientov** s Jacobiho
predpodmienením. Zostavovanie je paralelizované cez prvky s vyrovnávacími
pamäťami pre každé vlákno, ktoré sa na konci raz zlúčia, takže výsledok nezávisí
od počtu vlákien.

Model **bez okrajovej podmienky Elektrický potenciál** je singulárny: potenciál
je definovaný iba až na konštantu, matica má netriviálne jadro a riešenie
neskonverguje. Každý model potrebuje aspoň jeden predpísaný potenciál a model,
ktorý má viesť prúd, potrebuje dva s rôznymi hodnotami.

Treba poznať ešte tri ďalšie spôsoby, ako dospieť k singulárnej alebo prázdnej
sústave, pretože všetky tri sú tiché:

- entita, ktorej materiálu chýba jedna z troch požadovaných vlastností, sa
  nerieši a jej uzly zo sústavy vypadnú;
- entita s `er = 0` nemá žiadnu tuhosť, takže jej uzly v sústave zostanú, ale
  nie sú s ničím spojené;
- plocha s nulovou hrúbkou alebo čiara s nulovým prierezom tiež nemá žiadnu
  tuhosť.

### 1.8 Odvodené výsledky

Po riešení sa prvkové výsledky získajú z uzlového poľa potenciálu. Elektrické
pole je záporný gradient potenciálu spriemerovaný cez integračné body a pre
čiarové a plošné prvky otočený do globálnych súradníc:

```
E = -grad(V)
J = sigma * E
w = e0 * er * |E|^2 / 2          electric energy
r = |E| / |J|                    electrical resistivity
q = sigma * |E|^2                Joule heat
```

(`w` je elektrická energia, `r` elektrická rezistivita, `q` Joulovo teplo.)

| Výsledok | Aplikuje sa na | Význam |
|---|---|---|
| Elektrický potenciál `[V]` | uzol | vyriešené pole potenciálu |
| Elektrické pole `[V/m]` | prvok | vektor poľa |
| Prúdová hustota `[A/m^2]` | prvok | `sigma*E`, nulová tam, kde je vodivosť nulová |
| Elektrická energia `[J/m^3]` | prvok | hustota energie poľa |
| Elektrická rezistivita `[V*m/A]` | prvok | `1/sigma` všade, kde tečie prúd, inde `0` |
| Joulovo teplo `[W/m^3]` | prvok | odporová disipácia, odovzdávaná tepelnému riešiču |

Všetky tieto výsledky sú **hustoty**: derivácie tvarových funkcií sa priemerujú
cez integračné body prvku a nie sú vážené determinantom jeho Jacobiho matice a
hrúbka plochy a prierez čiary zostávajú v tuhosti, kam patria, namiesto toho,
aby vstúpili do odvodeného poľa. Pole, prúdová hustota a elektrická energia preto
nezávisia od veľkosti prvku, v ktorom boli vypočítané, takže ich možno porovnávať
medzi prvkami aj s ručným výpočtom a so zjemňovaním siete konvergujú.

**Elektrická rezistivita** je výsledok, po ktorom treba siahnuť pri kontrole
modelu, pretože oba činitele, z ktorých sa skladá, sa navzájom vykrátia:
`|E|/|sigma*E|` je presne `1/sigma`, nech pole robí čokoľvek. Je to najrýchlejšie
potvrdenie, že vodivosť určená pre entitu sa skutočne dostala do jej prvkov. Na
každom prvku, kde prúdová hustota klesne pod numerickú toleranciu, čo zahŕňa
každý prvok s nulovou vodivosťou, sa ukladá ako `0`.

**Joulovo teplo** je hustota disipácie `sigma*|E|^2` vo `W/m^3`. Presne to
očakáva krok *Prestup tepla*, keď hodnotu pripočíta k svojmu zdrojovému členu a
zintegruje ju cez prvok, takže výkon prechádzajúci do spriahnutého tepelného
výpočtu je výkon, ktorý model skutočne disipuje. Ako ostatné je to hustota, takže
nezávisí od veľkosti prvku a pri zjemňovaní konverguje.

Ešte dve miesta, kde odvodené výsledky jednoducho zostanú nulové: **bodové
prvky** sa nikdy nespracúvajú a **plocha s nulovou hrúbkou** alebo **čiara s
nulovým prierezom** sa preskočí, takže obe si ponechajú nulové pole bez ohľadu na
to, aký veľký je gradient potenciálu naprieč nimi.

Po zapísaní záznamu log riešiča vypíše štatistiky všetkých šiestich premenných a
hodnoty vo všetkých monitorovacích bodoch.

### 1.9 Čo riešič je a čo nie je

Názov je presný: je to **elektrostatický** riešič. Počíta pole rozloženia
nábojov a potenciálov v dielektriku. Prúdová hustota, ktorú uvádza, je to, čo pre
toto pole dáva Ohmov zákon - odvodená veličina, vypočítaná dodatočne, ktorá samotné
pole nikdy neovplyvní.

Úlohu **ustáleného prúdenia elektrického prúdu** naproti tomu riadi

```
div( sigma * grad(V) ) = 0
```

s vodivosťou na mieste, kam tento riešič dáva permitivitu. Obe dávajú v
homogénnom telese rovnaké pole potenciálu, pretože obe sa redukujú na Laplaceovu
rovnicu a konštanta sa vykráti. Rozchádzajú sa, len čo je teleso z viac ako
jedného materiálu:

| | Tento riešič | Riešič prúdového poľa |
|---|---|---|
| Potenciál sa delí v pomere | `1/er` každej oblasti | `1/sigma` každej oblasti |
| Vhodný pre | kondenzátor, dielektrikum, izolant | sieť odporov, prípojnicu, vodivú dráhu |

Vodič z jedného materiálu s predpísanými potenciálmi na koncoch je teda
modelovaný správne: potenciál je lineárny a vydelením dĺžkou dostanete pole. Dva
vodiče s rôznou vodivosťou v sérii nie - rozdelenie sleduje permitivity, čo asi
nie je to, čo ste zamýšľali. Taký prípad modelujte ako jeden materiál alebo
rozdelenie vypočítajte ručne.

### 1.10 Statická analýza a časový riešič

Elektrostatika je deklarovaná ako typ úlohy **bez** časového riešiča. To má dva
viditeľné dôsledky.

V modeli iba s elektrostatikou sa skupina *Riešič v čase* na záložke `Úloha`
vôbec nezobrazí, výpočet vytvorí jediný záznam a nie je čo nastavovať.

Prekvapivejšie je, že pridanie kroku elektrostatiky do poradia krokov **vypne
časový riešič pre celé poradie**. Časový riešič sa ponúka iba vtedy, keď ho
podporuje *každý* typ úlohy v poradí, takže prechodová tepelná analýza, ktorej
pridáte krok elektrostatiky, sa stane ustálenou, bez akéhokoľvek varovania okrem
zmiznutia skupiny. Ak potrebujete prechodovú tepelnú odpoveď pre odporový
ohrievač, spustite krok elektrostatiky samostatne a jeho disipáciu prenesite do
prechodového tepelného modelu ako podmienku *Teplo*, namiesto toho, aby ste oba
kroky dali do jedného poradia.

---

## 2. Grafické používateľské rozhranie

Všetko podstatné sa nachádza v paneli **Riešič** (záložky `Úloha`,
`Okrajové podmienky`, `Počiatočné podmienky`, `Environmentálne podmienky`,
`Materiál`, `Výsledky`), v menu **Úloha** a v paneli **Model**.

### 2.1 Výber úlohy

`Úloha` -> `Poradie krokov úloh` (`Ctrl+P`) otvorí dialóg poradia krokov úloh.
Pridajte krok a vyberte **Elektrostatika** - "Výpočet elektrického poľa,
prúdovej hustoty a vzniknutého Joulovho tepla".

Elektrostatika sa voľne kombinuje s každou inou fyzikou okrem *Modálnej
analýzy*. Reťazce, ktoré stojí za to poznať:

- *Elektrostatika* a potom *Prestup tepla* - odporový ohrev;
- *Elektrostatika* a potom *Magnetostatika* - magnetické pole vypočítaného prúdu,
  čo je vôbec jediný spôsob, ako spustiť magnetostatickú analýzu;
- *Elektrostatika*, potom *Prestup tepla* a potom *Analýza napätia* - tepelné
  napätie od odporového ohrevu.

Pamätajte na 1.10: každé poradie krokov, ktoré obsahuje krok elektrostatiky, sa
rieši ako ustálené.

### 2.2 Záložka Úloha

Pre model iba s elektrostatikou je táto záložka **prázdna**. Neexistuje časový
riešič, pretože ho typ úlohy nepodporuje, a elektrostatika nemá vlastnú skupinu
nastavení. Všetko sa priraďuje entitám na záložkách podmienok.

Ak záložka zobrazuje skupinu *Riešič v čase*, umiestnil ju tam iný krok v poradí
- a podľa 1.10 opäť zmizne, len čo sa pridá krok elektrostatiky.

### 2.3 Záložka Materiál

Záložka `Materiál` uvádza materiál priradený vybranej entite. Elektrostatika
vyžaduje **všetky tri** tieto vlastnosti:

| Vlastnosť | Jednotky | Poznámka |
|---|---|---|
| Relatívna permitivita | `N/A`, bezrozmerná | `er`, `1` pre vákuum, `1.00059` pre vzduch, asi `4` pre sklo |
| Elektrická vodivosť | `S/m` | `sigma`, `5.882e7` pre meď, `5e-15` pre vzduch |
| Hustota | `kg/m^3` | potrebná na to, aby sa entita riešila, nikdy nečítaná |

Entita, ktorej niektorá z nich chýba, sa nerieši a kontrola nastavenia pred
spustením výpočtu upozorní hlásením *Entity ... has material assigned which is
missing required properties*. Keďže tento riešič hustotu nikdy nepoužíva,
požiadavku splní akákoľvek kladná hodnota.

Dodávaná knižnica materiálov má všetky tri už pre kovy, vzduch a sklá, takže na
začiatok stačí priradiť *Meď* (*Copper*) alebo *Vzduch* (*Air*).

Každá vlastnosť je tabuľkou v závislosti od teploty, ale teplotná závislosť sa
rešpektuje iba vtedy, keď krok prestupu tepla skôr v poradí vytvoril teploty
prvkov - pozri 1.6.

### 2.4 Záložka Okrajové podmienky

Záložka je rozdelená na zoznam dostupných okrajových podmienok hore a editor
vybranej podmienky dole. Najprv vyberte entitu v strome `Model`; zoznam potom
ponúkne podmienky platné pre daný typ entity a pre vybranú úlohu. Zaškrtnutím
podmienku priradíte a jej zložky potom upravíte v dolnom strome.

Ponúkajú sa iba dve podmienky z časti 1.4. Poznámky k ich použitiu:

- **Elektrický potenciál** je podmienka, ktorá hodnotu drží, nie poháňa, a
  ponúka sa na každom type entity. Priraďte ju stene, hrane alebo bodu, ktorých
  potenciál je známy. Model bez žiadnej takej podmienky sa nevyrieši.
- **Hustota náboja** je zdrojový člen a kladná hodnota zvyšuje potenciál okolo
  seba. Pred použitím si prečítajte 1.5: hodnota je hustota a na rozdiel od
  podmienky *Teplo* tepelného riešiča sa na entitu nerozkladá.
- Obe zložky sú tabuľkami v čase, na čom záleží iba v poradí krokov poháňanom
  iným, časovo závislým krokom.

### 2.5 Počiatočné a environmentálne podmienky

Záložka `Počiatočné podmienky` ponúka *Elektrický potenciál*, ktorý nastavuje
počiatočné pole iteračného riešenia. Skonvergovanú odpoveď nemení.

Záložka `Environmentálne podmienky` ponúka *Teplotu*, ktorú tento riešič
ignoruje - pozri 1.6.

### 2.6 Nastavenie maticového riešiča

`Úloha` -> `Nastaviť maticový riešič úloh` nastavuje iteračné riešiče. Riešič
elektrostatiky používa položku **CG**.

Ak riešenie nekonverguje, obvyklou príčinou je model bez predpísaného potenciálu
alebo entita s nulovou permitivitou, nie nastavenie riešiča. Skôr než zvýšite
počet iterácií, skontrolujte okrajové podmienky a materiál.

### 2.7 Monitorovacie body

`Úloha` -> `Definovať monitorovacie body` umiestni sondy na zadané súradnice a
vyberie premennú, ktorá sa má zaznamenávať - pre elektrostatickú úlohu je
obvyklou voľbou *Elektrický potenciál* a je to najrýchlejší spôsob, ako z modelu
odčítať hodnotu ako číslo, nie ako farbu. `Výkaz` -> `Monitorovacie body` hodnoty
zobrazí.

### 2.8 Výsledky a záznamy

Záložka `Výsledky` uvádza vypočítané premenné a ovláda 3D pohľad. Elektrický
potenciál je uzlový skalár; elektrické pole a prúdová hustota sú prvkové vektory;
elektrická energia, elektrická rezistivita a Joulovo teplo sú prvkové skaláry.

Záložka `Záznamy` panela `Model` uvádza záznamy výsledkov. Elektrostatická
analýza je statická, takže je presne **jeden**.

`Výkaz` -> `Log súbor riešiča` zobrazí celý výstup riešiča vrátane štatistík
všetkých šiestich premenných po zapísaní záznamu.

---

## 3. Tutoriál - potenciál a prúd vo vodiči

**Cieľ.** Vypočítať rozloženie potenciálu pozdĺž medenej tyče držanej na dvoch
rôznych potenciáloch a odčítať pole, prúd a odpor.

Tento tutoriál predpokladá objemový model so sieťou - dobre poslúži dlhý kváder.
Použite `Súbor` -> `Nový model`, nakreslite kváder pomocou `Geometria` ->
`Vytvoriť prvok` alebo importujte geometriu a rozdeľte ju na tetrahedróny; tento
postup pokrýva tutoriál *Kresliť kocku*. Obe koncové steny označte ako samostatné
plošné entity, aby im bolo možné priradiť podmienky.

Uvažujte tyč dlhú `0.1 m` s prierezom `0.01 m` krát `0.01 m`, vďaka čomu sa ručné
výpočty na konci ľahko sledujú.

### Krok 1 - výber úlohy

1. `Úloha` -> `Poradie krokov úloh` (`Ctrl+P`).
2. Pridajte krok a vyberte **Elektrostatika**.
3. Potvrďte tlačidlom `OK`.

Záložka `Úloha` zostane prázdna - nie je čo nastavovať.

### Krok 2 - priradenie materiálu

1. Vyberte objemovú entitu v strome `Model`.
2. Otvorte záložku `Materiál` a priraďte **Meď**, alebo materiál so všetkými
   tromi vlastnosťami:
   - **Relatívna permitivita** = `2.8`
   - **Elektrická vodivosť** = `5.882e7` `S/m`
   - **Hustota** = `8960` `kg/m^3`

Hustotu tento riešič nepoužíva, ale musí byť prítomná, aby sa entita vôbec
riešila. Pole poháňa permitivita; vodivosť iba dodatočne prevedie pole na prúd.

### Krok 3 - predpísanie oboch koncov

1. Vyberte plošnú entitu na jednom konci.
2. Otvorte záložku `Okrajové podmienky` a zaškrtnite **Elektrický potenciál**.
3. Nastavte hodnotu na `0` V - toto je zem.
4. Vyberte plošnú entitu na druhom konci, znova zaškrtnite **Elektrický
   potenciál** a nastavte ho na `10` V.

Toto je krok, ktorý robí model riešiteľným. Jeden predpísaný potenciál je
minimum; dva s rôznymi hodnotami vytvoria pole.

Nepridávajte podmienku *Hustota náboja*. Vodič s predpísanými koncovými
potenciálmi nenesie žiadny priestorový náboj, takže zdrojový člen patrí na nulu a
potenciál je priamka medzi oboma koncami - vďaka čomu je ručná kontrola v kroku 6
presná.

### Krok 4 - riešenie

1. `Riešenie` -> `Spustiť riešič` (`Ctrl+R`).
2. Kontrola nastavenia pred spustením výpočtu ohlási chýbajúce materiály.
3. Priebeh sledujte v `Výkaz` -> `Log súbor riešiča`.

Výpočet je jediné riešenie metódou združených gradientov a na sieti tejto
veľkosti skončí za niekoľko sekúnd.

### Krok 5 - kontrola výsledkov

- Záložka `Výsledky`: zobrazte **Elektrický potenciál**. Mal by lineárne klesať z
  `10` V na `0` V pozdĺž tyče, s rovinnými ekvipotenciálnymi plochami kolmými na
  jej os. Akékoľvek zakrivenie v homogénnej tyči znamená, že sieť nie je
  prepojená tak, ako sa predpokladá.
- **Elektrická rezistivita** by mala na každom prvku tyče ukazovať
  `1/5.882e7 = 1.7e-8` `V*m/A`. Je to najlepšia samostatná kontrola, že sa
  materiál dostal do prvkov: je presná, nezávislá od siete a nesprávna v
  okamihu, keď je nesprávna vodivosť.
- **Elektrické pole** smeruje pozdĺž tyče a malo by rovnomerne ukazovať
  `10/0.1 = 100` `V/m` a **Prúdová hustota** `sigma*E = 5.88e9` `A/m^2`. Obe sú
  hustoty, takže čísla platia bez ohľadu na veľkosť prvku - odvodzuje ich krok 6.
- Monitorovací bod na *Elektrickom potenciáli* v polovici tyče by mal ukazovať
  `5` V.

### Krok 6 - ručný výpočet čísel

Prvé dva riadky nižšie by už mali ukazovať výsledky **Elektrické pole** a
**Prúdová hustota**, takže sú priamou kontrolou riešenia. Posledné tri sú celkové
hodnoty, nie hustoty, ktoré nedáva žiadny prvkový výsledok, a treba ich
vypočítať. Pre homogénnu tyč dĺžky `L` a prierezu `A` držanú na rozdiele
potenciálov `U`:

```
E = U / L                       = 10 / 0.1            = 100      V/m
J = sigma * E                   = 5.882e7 * 100       = 5.88e9   A/m^2
I = J * A                       = 5.88e9 * 1e-4       = 5.88e5   A
R = L / ( sigma * A )           = 0.1 / (5.882e7*1e-4) = 1.7e-5  ohm
P = U^2 / R                     = 100 / 1.7e-5        = 5.88e6   W
```

Prúd je obrovský, pretože masívna medená tyč s takým prierezom je veľmi dobrý
vodič a `10` V na nej je skrat - užitočná pripomienka, že elektrostatický model
ochotne vypočíta situáciu, ktorá by skutočnú súčiastku odparila.

### Krok 7 - voliteľne, zjemnenie siete

Sieť raz zjemnite a výpočet zopakujte. Každý výsledok - **potenciál**,
**elektrické pole**, **prúdová hustota**, **elektrická energia**,
**rezistivita** a **Joulovo teplo** - si musí udržať svoju hodnotu. Všetky sú
hustoty, takže hodnota, ktorá sa pri zjemňovaní posúva, je znakom problému modelu,
nie jemnejšej siete, a oplatí sa ho vypátrať skôr, než budete pokračovať.

### Riešenie problémov

| Príznak | Príčina |
|---|---|
| Riešenie nekonverguje | nikde nie je podmienka *Elektrický potenciál*, takže potenciál je definovaný iba až na konštantu |
| *Entity has material assigned which is missing required properties* | chýba jedna z troch požadovaných vlastností - často hustota, ktorú riešič nikdy nepoužíva |
| Celý model je na jednom potenciáli | predpísaný bol iba jeden potenciál alebo druhý má rovnakú hodnotu |
| Potenciál je všade nulový okrem predpísaných stien | entita má `er = 0`, takže nemá žiadnu tuhosť a nič nespája |
| Rezistivita je všade nulová | elektrická vodivosť je nulová, takže netečie žiadny prúd a rezistivita sa ukladá ako nula |
| Plošná entita neukazuje vôbec žiadne pole | jej hrúbka je nulová, takže sa nezostavuje ani nespracúva |

---

## 4. Tutoriál - odporový ohrev

**Cieľ.** Vziať Joulovo teplo vypočítané z elektrického poľa a ohriať ním tyč
zreťazením kroku elektrostatiky a kroku prestupu tepla.

### Krok 1 - zostavenie poradia krokov

1. `Úloha` -> `Poradie krokov úloh` (`Ctrl+P`).
2. Pridajte krok **Elektrostatika**.
3. **Za** neho pridajte krok **Prestup tepla**.
4. Potvrďte tlačidlom `OK`.

Na poradí záleží: tepelný riešič číta Joulovo teplo, ktoré zanechal riešič
elektrostatiky, takže krok elektrostatiky musí prebehnúť ako prvý. Na ich
prepojenie nie je potrebná žiadna podmienka ani nastavenie.

Všimnite si, čo sa stane so záložkou `Úloha`: skupina *Riešič v čase*, ktorú by
zobrazil samotný krok prestupu tepla, je preč, pretože elektrostatika nemá časový
riešič a poradie preberá najreštriktívnejšie nastavenie - pozri 1.10. Tento
reťazec je iba ustálený.

### Krok 2 - doplnenie materiálu

Krok prestupu tepla potrebuje okrem troch vlastností kroku elektrostatiky ďalšie
štyri vlastné, takže materiál entity musí mať všetky z nich:

| Vlastnosť | Hodnota pre meď | Potrebuje |
|---|---|---|
| Relatívna permitivita | `2.8` | elektrostatika |
| Elektrická vodivosť | `5.882e7` `S/m` | elektrostatika |
| Hustota | `8960` `kg/m^3` | obe |
| Tepelná vodivosť | `401` `W/(m*K)` | teplo |
| Tepelná kapacita | `385` `J/(kg*K)` | teplo |
| Emisivita | `0` | teplo |

Dodávaný materiál *Meď* ich už má. Entitu, ktorej chýba vlastnosť požadovaná
**ktorýmkoľvek** krokom, tento krok preskočí.

### Krok 3 - ponechanie elektrických okrajových podmienok

Dve podmienky **Elektrický potenciál** z tutoriálu 3 sa prenesú bez zmeny.
Znížte budiaci potenciál na niečo menej prudké - `0.01` V na tyči stále vytvorí
veľký prúd a udrží tepelný zdroj v rozsahu, v ktorom je tepelná odpoveď
čitateľná.

### Krok 4 - pridanie tepelných okrajových podmienok

Tepelná polovica modelu potrebuje vlastný odvod, presne ako by ho potrebovala
samostatne:

1. Vyberte plošnú entitu pokrývajúcu boky tyče.
2. Otvorte záložku `Okrajové podmienky` a zaškrtnite **Jednoduchá konvekcia**
   (*Simple convection*).
3. Nastavte **Súčiniteľ konvekcie** = `10` `W/(m^2*K)` a **Teplota tekutiny** =
   `293.15` K.

Bez plochy s konvekciou alebo predpísanej teploty má tepelný model zdroj, ale
žiadny odvod, a ustálený stav neexistuje. Všimnite si, že stena použitá na
konvekciu nepotrebuje pre tepelný riešič žiadnu hrúbku, ale stena s nulovou
hrúbkou nevedie ani elektrické pole - elektrické podmienky ponechajte na
koncových stenách a konvekciu na bokoch a navzájom sa neovplyvnia.

### Krok 5 - riešenie

`Riešenie` -> `Spustiť riešič` (`Ctrl+R`). Log ukazuje najprv krok
elektrostatiky so štatistikami jeho šiestich premenných a potom krok prestupu
tepla so štatistikami teploty, tepelného toku a súčiniteľa prestupu tepla.

### Krok 6 - kontrola výsledkov

- Záložka `Výsledky`: zobrazte **Joulovo teplo**. Je mostom medzi oboma fyzikami
  - všade, kde je nenulové, tepelný riešič videl zdroj.
- Zobrazte **Teplotu**. Tyč by mala byť najteplejšia tam, kde je Joulovo teplo
  najväčšie, a smerom k plochám s konvekciou klesať k teplote tekutiny.
- Zobrazte **Elektrický potenciál** a overte, že elektrická polovica modelu je
  stále lineárnym poľom z tutoriálu 3.

### Krok 7 - kontrola energetickej bilancie

Joulovo teplo je hustota disipácie vo `W/m^3`, takže celkový výkon, ktorý
elektrická polovica modelu dodá tepelnej, je táto hustota krát objem, na ktorý
pôsobí. Pre homogénnu tyč by mal vyjsť `P = U^2/R` z tutoriálu 3, s
`R = L/(sigma*A)`.

V ustálenom stave musí plocha s konvekciou odovzdať presne tento výkon, čo dáva tú
istú globálnu kontrolu, akú opisuje manuál prestupu tepla:

```
P = h * A_s * ( Ts_mean - Tf )
```

Strednú teplotu povrchu odčítajte zo štatistík v logu riešiča a overte, že
bilancia sedí. Je to jediná kontrola, ktorá pokrýva obe fyziky naraz: zachytí
nesprávnu vodivosť, nesprávny súčiniteľ konvekcie aj entitu, ktorá v
ktorejkoľvek polovici potichu zostala nevyriešená.

### Riešenie problémov

| Príznak | Príčina |
|---|---|
| Teplota je rovnomerne `293.15` K | Joulovo teplo je nulové - skontrolujte elektrickú vodivosť a to, že krok elektrostatiky beží ako prvý |
| Tepelné riešenie nekonverguje | tepelná polovica nemá predpísanú teplotu ani plochu s konvekciou |
| Skupina *Riešič v čase* zmizla | očakávané - krok elektrostatiky robí celé poradie ustáleným, pozri 1.10 |
| Nárast teploty sa mení so zjemnením siete | neočakávané - každý elektrostatický výsledok je hustota, takže skontrolujte tepelnú polovicu modelu a kvalitu siete |
| Zobrazujú sa iba výsledky elektrostatiky | kroku prestupu tepla chýba jedna z jeho štyroch požadovaných vlastností materiálu |

---

## 5. Kontrola modelu

Riešič nemá zabudovanú sadu overovacích testov, takže nový model sa oplatí pred
tým, než mu začnete dôverovať, overiť voči niečomu, čo sa dá vypočítať ručne.
Kontroly nižšie sú napísané pre **potenciál**, ktorý je primárnou neznámou aj
veličinou, v ktorej je vyjadrené každé riešenie v uzavretom tvare; výsledky
elektrického poľa a prúdovej hustoty by mali súhlasiť s gradientmi, ktoré z neho
vyplývajú.

**Homogénna tyč.** Pre teleso dĺžky `L` držané na `V1` a `V2` na dvoch protiľahlých
stenách, s jedným materiálom a bez náboja, je potenciál pozdĺž tyče lineárny a

```
V(x) = V1 + ( V2 - V1 ) * x / L
E    = ( V1 - V2 ) / L          [V/m]
```

Umiestnite monitorovací bod do strednej roviny a overte, že ukazuje priemer
oboch predpísaných hodnôt, a ďalší do štvrtiny dĺžky pre štvrtinovú hodnotu. To
zachytí neprepojenú sieť, podmienku priradenú nesprávnej entite aj nulovú
permitivitu. Sieť raz zjemnite a overte, že čísla zostanú na mieste.

**Rovnomerný priestorový náboj.** Tyč vyššie preveruje iba tuhosť. Ak chcete
skontrolovať aj zdrojový člen, vezmite to isté teleso, držte **obe** koncové
steny na `0` V a objemu priraďte rovnomernú *Hustotu náboja* `rho`. Potenciál je
potom parabola, s `eps = e0*er`:

```
V(x)  = rho * x * ( L - x ) / ( 2 * eps )
Vmax  = rho * L^2 / ( 8 * eps )          at the mid-plane
```

(`Vmax` v strednej rovine.) Monitorovací bod v strednej rovine dá `Vmax` priamo.
Kontrolujú sa dve veci naraz: veľkosť, ktorá potvrdzuje, že permitivita a hustota
náboja sa čítajú podľa očakávania, a **znamienko** - kladná hustota náboja musí
medzi dvoma uzemnenými stenami vytvoriť kladný hrb potenciálu. Zopakujte to s
nábojom na plošnej alebo čiarovej entite a na bode a overte, že polarita sa s
typom entity nemení.

**Rezistivita.** Každý prvok, ktorým tečie prúd, musí ukladať elektrickú
rezistivitu presne `1/sigma`. Odčítajte ju zo štatistík v logu riešiča a
porovnajte s vodivosťou v materiáli. Je presná, nie približná, nezávislá od siete
a je najrýchlejším potvrdením, že sa materiál dostal do prvkov, do ktorých mal.

**Koaxiálne pole.** Pre medzikružie medzi polomermi `a` a `b` s vnútornou plochou
na `V1` a vonkajšou na `V2` je potenciál logaritmický:

```
V(r) = V2 + ( V1 - V2 ) * ln( b / r ) / ln( b / a )
```

Vymodelujte medzikružie, predpíšte obe plochy a porovnajte monitorovacie body na
niekoľkých polomeroch. Toto overí riešič na poli, ktoré nie je homogénne, čo tyč
nedokáže, a ukáže, či je sieť pri vnútornom vodiči dostatočne jemná.

**Dielektriká v sérii.** Pre dve vrstvy hrúbky `d1`, `d2` s relatívnou
permitivitou `er1`, `er2` uložené medzi dvoma predpísanými potenciálmi je
potenciál na rozhraní

```
Vi = ( V1*er1/d1 + V2*er2/d2 ) / ( er1/d1 + er2/d2 )
```

Porovnajte s vypočítaným potenciálom na rozhraní. Toto priamo potvrdzuje to, čo
opisuje časť 1.9: rozdelenie sleduje **permitivity**. Spustenie toho istého
modelu s dvoma rôznymi vodivosťami a rovnakou permitivitou, pri ktorom sa
potenciál rozhrania nepohne, je ukážkou, že ide o elektrostatický riešič, nie o
riešič prúdového poľa.

**Odpor a disipácia.** Prvkové výsledky sú hustoty, takže celková hodnota - prúd,
odpor, výkon - musí vychádzať z potenciálu. Pre homogénnu tyč:

```
R = L / ( sigma * A )      [ohm]
I = U / R                  [A]
P = U * I = U^2 / R        [W]
```

Použite ich vždy, keď musí celková hodnota opustiť model - pri dimenzovaní vodiča
alebo pri kontrole voči meraniu. `P` je tiež hodnota, na ktorú sa musí výsledok
**Joulovo teplo** zintegrovať cez objem tyče, čo je kontrola, že elektrická a
tepelná polovica spriahnutého výpočtu súhlasia.

Pri ktorejkoľvek z nich sieť aspoň raz zjemnite a overte, že potenciál
konverguje, a nie sa posúva.

---

## 6. Obmedzenia

### Obmedzenia modelovania

- **Elektrostatika, nie prúdové pole.** Tuhosť sa zostavuje z permitivity;
  vodivosť do sústavy rovníc nikdy nevstupuje. Potenciál v telese z dvoch
  materiálov sa preto delí v pomere permitivít, nie odporov. Vodič z jedného
  materiálu je modelovaný správne; sieť odporov nie - pozri 1.9.
- **Iba statická analýza.** Neexistuje časový riešič, kapacita ani indukčnosť,
  takže nabíjacie prechodové deje, striedavé polia, posuvný prúd, vírivé prúdy a
  povrchový jav sú mimo modelu. Pridanie kroku elektrostatiky do poradia krokov
  tiež odstráni časový riešič z poradia ako celku, takže krok prestupu tepla
  zaradený za ním môže byť iba ustálený - pozri 1.10.
- **Iba lineárne izotropné dielektriká.** Permitivita a vodivosť prvku sú skaláry
  a počas riešenia sú konštantné. Anizotropné, feroelektrické a od poľa závislé
  materiály sa nemodelujú a neexistuje elektrický prieraz dielektrika.
- **Iba dve okrajové podmienky.** Predpísaný potenciál a hustota náboja tvoria
  celý slovník. Neexistuje predpísaný povrchový náboj, predpísaný normálový tok,
  podmienka symetrie ani spracovanie otvorenej hranice alebo nekonečných prvkov -
  neohraničenú úlohu poľa treba orezať tak, že sieťou pokryjete dostatok
  okolitého vzduchu a na jeho vonkajšej hranici predpíšete potenciál.
- **Žiadne prerozdelenie voľného náboja.** Náboj sa nepresúva na povrch vodiča;
  riešič nemá vôbec žiadnu predstavu o vodiči nad rámec vlastností materiálu,
  ktoré dostal. Vodič modelujte predpísaním jeho potenciálu, nie tým, že mu
  dáte veľkú vodivosť.
- **Žiadna spätná väzba od prúdu.** Prúdová hustota sa dopočítava z poľa, takže
  vodivosť, ktorá sa mení s poľom alebo s teplotou, ktorú disipácia vytvára, nemá
  v rámci jedného riešenia žiadny účinok.
- **Plochy sú naprieč hrúbkou ekvipotenciálne.** Plošná entita vedie pole iba vo
  vlastnej rovine. Plocha s nulovou hrúbkou alebo čiara s nulovým prierezom nemá
  vôbec žiadnu tuhosť a preskočí sa pri zostavovaní aj pri spracovaní výsledkov.
- **Bodové prvky nevedú.** Bod nesie zdroj náboja a môže držať predpísaný
  potenciál, nič viac, a nikdy sa nespracúva. Bod, ktorého uzol nepatrí žiadnemu
  inému počítanému prvku, zanechá v matici prázdny riadok.
- **Teplotná závislosť materiálu sa nečíta.** Tabuľky sa vyhodnocujú pri
  `293.15 K`, pokiaľ krok prestupu tepla skôr v poradí nevytvoril teploty prvkov;
  environmentálna podmienka *Teplota* nemá na tento riešič žiadny účinok.
- **Model potrebuje predpísaný potenciál.** Bez neho má matica netriviálne jadro
  a riešenie neskonverguje.

# Proudění tekutin - Teoretická příručka a uživatelská příručka

Tento dokument popisuje řešič proudění tekutin v Range FEA - `RSolverFluid`,
který obsluhuje typ úlohy **Nestlačitelné viskózní proudění**
(*Incompressible viscous flow*): rovnice, které řeší, význam každého vstupu,
který přijímá, části grafického uživatelského rozhraní, které ho ovládají, a
podrobný návod.

Dva typy úloh řešené na proudovém poli mají vlastní příručky:
[Přenos tepla v tekutinách](theory_fluid_heat.md) (`RSolverFluidHeat`) a
[Rozptyl kontaminantu](theory_fluid_particle.md) (`RSolverFluidParticle`).

**Obsah**

1. [Teoretický základ](#1-teoretický-základ)
2. [Grafické uživatelské rozhraní](#2-grafické-uživatelské-rozhraní)
3. [Návod - ustálené proudění kanálem](#3-návod---ustálené-proudění-kanálem)
4. [Kontrola modelu](#4-kontrola-modelu)
5. [Omezení](#5-omezení)

---

## 1. Teoretický základ

### 1.1 Primární neznámé - pole rychlosti a tlaku

Primárními neznámými **nestlačitelného viskózního proudění** jsou vektor
**rychlosti** `v` v `m/s` a **tlak** `p` v `Pa`, obojí v každém uzlu. Každý uzel
nese **čtyři** stupně volnosti - `vx`, `vy`, `vz` a `p` - takže síť s `N` uzly
dává soustavu `4N` rovnic.

Proto je model proudění fyzikálně nejdražším typem v Range FEA: čtyři neznámé
na uzel proti třem u strukturálního modelu a jedné u tepelného modelu, matice,
která není symetrická ani pozitivně definitní, a nelineární iterace z části 1.6
obalující každé lineární řešení.

Dva doprovodné typy úloh přidávají každý jeden skalár, řešený na proudovém poli,
které vytvořil řešič tekutiny:

| Typ úlohy | Řešič | Primární neznámá | Jednotky | Stupně volnosti na uzel |
|---|---|---|---|---|
| Nestlačitelné viskózní proudění | `RSolverFluid` | rychlost a tlak | `m/s`, `Pa` | 4 |
| Přenos tepla v tekutinách | `RSolverFluidHeat` | teplota | `K` | 1 |
| Rozptyl kontaminantu | `RSolverFluidParticle` | koncentrace částic | `kg/m^3` | 1 |

Oba doprovodné typy **vyžadují** ve stejné úloze krok *Nestlačitelné viskózní
proudění*: pole rychlosti čtou z jeho výsledku a samy ho nikdy nepočítají.
Každý popisuje vlastní příručka -
[Přenos tepla v tekutinách](theory_fluid_heat.md) a
[Rozptyl kontaminantu](theory_fluid_particle.md).

Výchozím stavem uzlu, kterého se nedotkne žádná podmínka ani předchozí
výsledek, je nulová rychlost a nulový tlak.

### 1.2 Řídicí rovnice

Řešič implementuje **Navierovy-Stokesovy rovnice nestlačitelného proudění** pro
newtonovskou tekutinu s hustotou `rho` a dynamickou viskozitou `mu`. Obě jsou
materiálovými vlastnostmi vyhodnocovanými pro každý prvek při jeho teplotě -
odkud tato teplota pochází a jak činí proudění vztlakovým, viz 1.9. Bilance
hybnosti a hmoty zní

```
rho * ( dv/dt + (v . grad) v ) = -grad(p) + div( mu * ( grad(v) + grad(v)^T ) ) + rho * g

div(v) = 0
```

a při vypnutém řešiči v čase přechodový člen vypadne a zůstane ustálená
bilance

```
rho * (v . grad) v = -grad(p) + div( mu * ( grad(v) + grad(v)^T ) ) + rho * g

div(v) = 0
```

| Symbol | Význam | Jednotky | Zdroj |
|---|---|---|---|
| `v` | rychlost | `m/s` | řešená veličina |
| `p` | tlak | `Pa` | řešená veličina |
| `rho` | hustota | `kg/m^3` | materiálová vlastnost |
| `mu` | dynamická viskozita | `kg/(m*s)`, tedy `Pa*s` | materiálová vlastnost |
| `g` | gravitační zrychlení | `m/s^2` | environmentální podmínka Gravitational acceleration |

Tlak zde není termodynamickou veličinou. Nestlačitelnost z něj činí Lagrangeův
multiplikátor, který vynucuje `div(v) = 0`, což má dva praktické důsledky, které
stojí za to si osvojit, než budete číst jakýkoli výsledek:

- Smysl mají jen **rozdíly tlaku**. Model, ve kterém žádná podmínka nikde
  neurčuje tlak, určuje tlakové pole jen až na aditivní konstantu a matice je v
  této konstantě singulární - viz 1.7.
- Tlak reaguje okamžitě v celé oblasti. V této formulaci neexistuje akustická
  vlna ani rychlost zvuku, takže tlak zadaný na výstupu se projeví na vstupu v
  rámci téhož řešení.

**Reynoldsovo číslo** sestavené z charakteristického rozměru `L` a rychlosti
`V`,

```
Re = rho * V * L / mu
```

je tím jediným číslem, které rozhoduje, zda se model proudění bude chovat
rozumně. Zhruba pod `Re = 1` je konvektivní člen zanedbatelný a úloha je téměř
lineární; do několika tisíc je proudění laminární a řešič je ve svém živlu;
daleko nad tím je skutečné proudění turbulentní a tento řešič turbulenci
nemodeluje - viz část 5.

### 1.3 Diskretizace metodou konečných prvků a stabilizace

Rychlost a tlak se na každém prvku interpolují **stejnými** lineárními bázovými
funkcemi - interpolace stejného řádu `P1/P1` a `Q1/Q1`. Taková kombinace sama o
sobě nesplňuje podmínku inf-sup (Babuškovu-Brezziho) a vedla by k
šachovnicovému tlakovému poli, proto je formulace **stabilizovaná**. Přidávají
se tři stabilizační členy, každý s vlastním parametrem na úrovni prvku:

| Člen | Účel |
|---|---|
| **SUPG** - streamline upwind Petrov-Galerkin | stabilizuje konvektivní člen, který by jinak v proudění s převládající konvekcí vytvářel oscilace mezi uzly |
| **PSPG** - pressure stabilising Petrov-Galerkin | činí rychlost a tlak stejného řádu přípustnými a odstraňuje šachovnicový tlakový mód |
| **LSIC** - least-squares on the incompressibility constraint | tlumí chybu divergence a zlepšuje zachování hmoty prvek po prvku |

Parametry SUPG a PSPG se sestavují z Reynoldsova čísla prvku

```
h   = element length along the flow direction
Re  = rho * |v| * h / ( 2 * mu )            element Reynolds number

tau = h * Re / ( 6 * |v| )      for  0 < Re <= 3
tau = h / ( 2 * |v| )           for  Re > 3
```

(`element length along the flow direction` - délka prvku ve směru proudění,
`element Reynolds number` - Reynoldsovo číslo prvku, `for` - pro), takže
upwinding je silný tam, kde v prvku převládá konvekce, a slábne tam, kde
převládá difúze. Parametr LSIC je `|v| * h / 2`. Tyto parametry patří řešiči
proudění; řešič rozptylu používá vlastní parametr SUPG omezený časovým krokem,
viz příručka [Rozptyl kontaminantu](theory_fluid_particle.md).

Parametr SUPG používá **místní** rychlost prvku a délku prvku, takže se
přizpůsobuje prvek po prvku. Parametr PSPG místo toho používá **globální
rychlost proudu** a globální měřítko délky prvku, což udržuje tlakovou
stabilizaci v celé síti rovnoměrnou. Rychlost proudu se bere z podmínek přítoku
v prvním průchodu každého řešení - v každém časovém kroku přechodového výpočtu -
nikoli z řešeného pole,

```
V_stream = sum( |Q| over inflow surfaces ) / sum( area of inflow surfaces )
```

(součet `|Q|` přes přítokové povrchy dělený součtem jejich ploch), tedy plochou
vážená střední rychlost přítoku, s náhradní hodnotou `1 m/s`, pokud model nemá
vůbec žádnou podmínku přítoku. Měřítko délky prvku se odvozuje z objemu prvku:
`cbrt(6*V/pi)` pro čtyřstěn, `cbrt(V)` pro šestistěn.

**Podporované prvky.** Rovnice proudění se sestavují **pouze na objemových
prvcích** a pouze na **lineárních čtyřstěnech** (`TETRA1`) a **lineárních
šestistěnech** (`HEXA1`). Jakýkoli jiný typ objemového prvku zastaví výpočet s
chybou, která typ uvede. Povrchové prvky se účastní jen tam, kde nesou podmínku
*Pressure (implicit)*, kde přispívají tlakovým zatížením a ničím jiným. Bodové a
čárové entity nepřispívají vůbec ničím - na rozdíl od tepelného modelu nejsou
vodiči nižší dimenze, prostě chybějí.

To je nejvýraznější praktický rozdíl oproti ostatním fyzikám v Range FEA: model
tekutiny musí být **vysíťovaný objem**. Než budete od řešení proudění cokoli
očekávat, vygenerujte čtyřstěnnou síť pomocí `Geometrie` -> `Objem` ->
`Generovat čtyřstěnnou síť`.

### 1.4 Okrajové podmínky

| Okrajová podmínka | Typ | Použití na | Složky |
|---|---|---|---|
| Wall | explicitní | bod, čára, povrch | žádné |
| Wall (frictionless) | explicitní | bod, čára, povrch | žádné |
| Velocity (inflow) | explicitní | povrch | Velocity `[m/s]` |
| Volumetric flow rate (inflow) | explicitní | povrch | Volumetric flow rate `[m^3/s]` |
| Pressure (explicit) | explicitní | povrch | Pressure `[Pa]` |
| Pressure (implicit) | přirozená, zatížení | povrch | Pressure `[Pa]` |

**Wall** (stěna) je podmínka nulového skluzu. Každému uzlu entity se ze
soustavy odstraní všechny tři složky rychlosti a drží se na nule. Nemá žádné
složky - není co zadávat, zaškrtnutí je celým nastavením. Je to podmínka pro
každou pevnou hranici, které se tekutina dotýká.

**Wall (frictionless)** (stěna bez tření) je podmínka skluzu neboli symetrie:
tekutina se může pohybovat podél povrchu, ale ne skrz něj. Uplatňuje se tak, že
se pro normálu každého povrchového prvku najde **dominantní globální osa** a na
nule se drží jen tato jediná složka rychlosti, zbylé dvě zůstanou volné.
Uvědomte si, co to znamená: podmínka je přesná jen pro povrch, jehož normála
leží podél `x`, `y` nebo `z`. Na povrchu nakloněném vůči globálním osám váže
nesprávný směr a čím blíže je normála 45 stupňům mezi dvěma osami, tím horší je
aproximace. Používejte ji pro rovinné roviny symetrie modelu zarovnaného s
globálními osami a jinde používejte *Wall*.

**Velocity (inflow)** (rychlost přítoku) předepisuje rychlost zadané velikosti
směřující podél **průměrné normály** povrchové entity do tekutiny. Znaménko se
určuje z orientace prvků entity vůči objemu za nimi, takže kladná hodnota
znamená proudění vstupující do oblasti a záporná hodnota proudění, které ji
opouští. Zadáváte rychlost, nikoli vektor - směr vychází z geometrie.

**Volumetric flow rate (inflow)** (objemový průtok přítoku) předepisuje totéž z
celkového průtoku: řešič vydělí průtok plochou entity a výslednou střední
rychlost uplatní podél stejné průměrné normály.

```
v = Q / A          A = area of the entity
```

(`area of the entity` - plocha entity.) Sáhněte po ní vždy, když skutečně
známou veličinou je průtok - výkon čerpadla, potrubí v `m^3/s` - a po
*Velocity (inflow)*, když znáte rychlost. Obě drží všechny tři složky rychlosti
v každém uzlu entity, takže obě činí ze vstupu rovnoměrný pístový tok; ani jedna
nedokáže na vstupní stěně vytvořit vyvinutý profil. Záleží-li na profilu, dejte
modelu náběhovou délku.

**Pressure (explicit)** (explicitní tlak) odstraní tlakový stupeň volnosti z
každého uzlu entity a drží ho na zadané hodnotě. Je tlakovou obdobou předepsané
rychlosti a jedním ze způsobů, jak určit aditivní konstantu z části 1.2.

**Pressure (implicit)** (implicitní tlak) je přirozeným protějškem: místo
fixace uzlového tlaku uplatní na povrchové prvky **tlakové zatížení** a
rychlost tam ponechá volnou, takže tekutina sama rozhodne, kolik a s jakým
profilem vytéká. Zatížení zahrnuje hydrostatický člen,

```
t = -( p + rho * g * h ) * n
```

kde `h` je výška uzlu měřená **podél směru gravitace od nejnižšího bodu**
modelu a `n` je normála povrchu. Směr gravitace je průměrem environmentálních
podmínek *Gravitational acceleration* přiřazených povrchům nesoucím tuto
podmínku. Bez gravitace hydrostatický člen zmizí a zatížení tvoří jen zadaný
tlak.

To je výstupní podmínka, po které sáhnout téměř v každém modelu: nechá vyvinout
profil odtoku, místo aby ho vynutila plochý, a zadáním `0` se výstup stane
referencí při atmosférickém tlaku. *Pressure (explicit)* je tužší, víc
předepisující volba.

Každá složka každé z těchto podmínek je **tabulkou v závislosti na čase**, takže
vstup lze během prvních sekund přechodového výpočtu postupně rozjet nebo
průtoku zadat pracovní cyklus.

**Přednost.** Rychlostní a tlakové podmínky jsou nezávislé - vážou různé stupně
volnosti, takže vstupní povrch může nést rychlostní podmínku a výstupní tlakovou
bez vzájemného ovlivnění. Mezi podmínkami stejného druhu na stejné entitě
převáží poslední načtená a *Wall* vynuluje rychlost svých uzlů až po uplatnění
všech ostatních rychlostních podmínek. Entita, která je vstupem i stěnou, je
proto stěnou.

### 1.5 Environmentální a počáteční podmínky

#### Environmentální podmínky

| Environmentální podmínka | Složky | Účinek |
|---|---|---|
| Gravitational acceleration | `gx`, `gy`, `gz` `[m/s^2]` | objemová síla `rho*g` na entitu a hydrostatický člen *Pressure (implicit)* |
| Temperature | Temperature `[K]` | teplota pozadí kroku *Heat transfer in fluids*; samotný řešič proudění ji nečte - viz 1.9 |

**Gravitational acceleration** má výchozí hodnotu `(0, 0, -9.80665) m/s^2`,
tedy gravitaci ve směru záporné osy `Z`. Přiřaďte ji objemové entitě kvůli
objemové síle a výstupnímu povrchu, aby se hydrostatický člen
*Pressure (implicit)* měřil ve správném směru.

Gravitace vstupuje do rovnice hybnosti jako objemová síla `rho*g`. V uzavřené
oblasti se stejnoměrnou hustotou vytváří stejnoměrné gravitační pole
hydrostatický tlakový gradient a žádný pohyb - což je správná odpověď. Tam, kde
se hustota mění s teplotou, mění se s ní i objemová síla a rozdíl pohání
**vztlakové proudění** - přirozenou konvekci, viz 1.9. Výsledek tlaku vždy
obsahuje hydrostatickou část `rho*(g . x)`; nic se neodečítá.

#### Počáteční podmínky

| Počáteční podmínka | Účinek |
|---|---|
| Velocity | počáteční pole rychlosti, použije se při prvním výpočtu |
| Pressure | počáteční pole tlaku, použije se při prvním výpočtu |

Počáteční pole teploty a koncentrace doprovodných typů úloh popisují jejich
vlastní příručky.

Počáteční podmínky se čtou **pouze při prvním výpočtu**; poté pole pokračují od
předchozího výsledku. Díky tomu funguje postup s restartem z části 1.7:
přechodový výpočet spuštěný s volbou *Restartovat řešič / pokračovat* převezme
zkonvergované ustálené pole, místo aby začal znovu z klidu.

Tam, kde se setkají, má okrajová podmínka přednost před počáteční podmínkou.

### 1.6 Nelineární iterace

Konvektivní člen `(v . grad) v` je v neznámé kvadratický, takže rovnice proudění
jsou **nelineární** a nelze je vyřešit jedním průchodem. Řešič je linearizuje a
iteruje: každý průchod sestaví reziduum bilance hybnosti a hmoty při aktuálním
poli, vyřeší **přírůstek** a přičte ho,

```
J * dx = -R(x)
x <- x + dx
```

kde `x` obsahuje uzlové rychlosti a tlaky. Levá strana je přibližným, nikoli
přesným Jacobiánem, což ze schématu činí modifikovanou Newtonovu iteraci:
poblíž odpovědi konverguje pomaleji než úplná Newtonova metoda, ale odpověď, ke
které konverguje, je určena **reziduem** `R`, které tvoří samotné diskretizované
Navierovy-Stokesovy rovnice.

**Krok je tlumený.** Protože matice není přesnou derivací rezidua, může úplný
krok `dx` přestřelit a zanechat pole dál od řešení, než začalo - reziduum
dalšího průchodu pak vyjde vyšší než předchozí. Řešič hlídá právě tuto situaci a
uplatní jen část vypočteného kroku,

```
x <- x + omega * dx
```

počínaje `omega = 1`. Průchod, který reziduum zhoršil, ho zmenší na polovinu, až
na spodní mez `0.1`; průchod, který ho zlepšil, nechá `omega` opět růst vždy o
čtvrtinu, takže ústup je rychlý a zotavení uvážlivé. Platná hodnota se v logu
vypisuje jako `Relaxation` a u výpočtu, který nikdy nepřestřelí, zůstává na `1`
- dobře se chovající model proudění si mechanismu ani nevšimne. `omega` se
nastaví zpět na `1` na začátku každého řešení, což v přechodovém výpočtu znamená
v každém časovém kroku.

Plynou z toho dva důsledky, které řídí celý postup práce s modelem proudění:

- **Jeden průchod nikdy nestačí.** Jediná iterace kroku proudění vrátí to, co
  náhodou dal první přírůstek, což není řešením ničeho. Krok proudění musí být
  zabalen do **skupiny kroků úlohy** s počtem iterací - stovky pro ustálený
  výpočet, desítky na časový krok pro přechodový.
- **Počet je horní mezí, nikoli cílem.** Skupina nese také **hodnotu
  konvergence** a své iterace ukončí, jakmile krok proudění ohlásí konvergenci
  pod ní. Nastavte počet dost vysoko pro nejtěžší krok a nechte hodnotu
  konvergence každý z nich zastavit, jakmile se ustálí.

Po každé iteraci zapíše řešič do souboru konvergence a do logu čtyři čísla:

| Veličina | Význam |
|---|---|
| `Residual` | norma vektoru rezidua, vzdálenost od řešení |
| `Convergence-R` | změna rezidua od předchozí iterace |
| `Convergence-V` | relativní velikost přírůstku rychlosti, `\|\|dv\|\| / \|\|v\|\|` |
| `Convergence-P` | relativní velikost přírůstku tlaku, `\|\|dp\|\| / \|\|p\|\|` |

S hodnotou konvergence se porovnávají `Convergence-V` a `Convergence-P`. Obě
jsou bezrozměrné a obě klesají, jak se iterace ustaluje: měří, jak velký krok
pole ještě dělá ve srovnání s velikostí pole samotného, takže hodnota `1e-5`
znamená, že se řešení v daném průchodu posunulo o stotisícinu sebe sama.

Malý krok však sám o sobě není řešením. Téměř singulární soustava - model bez
tlakové reference nebo síť, která neunese požadované Reynoldsovo číslo - dělá
nepatrné kroky, protože se **nemůže pohnout**, nikoli proto, že dorazila, a její
reziduum stojí na místě nebo roste. Test má proto dvě části a skupina se zastaví
jen tehdy, když platí obě:

| Část | Podmínka |
|---|---|
| Pole se přestalo pohybovat | `Convergence-V` i `Convergence-P` jsou pod hodnotou konvergence |
| Pole skutečně kleslo | reziduum kleslo na **desetinu** hodnoty v prvním průchodu tohoto řešení |

Druhá část je pevná a nelze ji nastavit - je kontrolou správnosti první, nikoli
druhým knoflíkem k ladění. Log ji vypisuje jako `Residual ratio` vedle jejího
cíle, takže je vidět, jak blízko k zastavení byl výpočet, který se dříve
nezastavil. V přechodovém výpočtu je každý časový krok samostatným řešením,
takže se poměr měří znovu od začátku každého kroku.

`Zprávy` -> `Konvergence řešiče` vykreslí všechna čtyři čísla. Zkonvergovaný
ustálený výpočet ukazuje reziduum klesající o několik řádů a pak se
zplošťující, přičemž `Convergence-V` a `Convergence-P` monotónně klesají k
nule. Reziduum, které se zploští vysoko, osciluje nebo roste, znamená, že výpočet
nezkonvergoval - a skupina se u takového výpočtu dříve nezastaví, ať přírůstky
dělají cokoli.

Hodnota konvergence `0` vypne předčasné ukončení a spustí plný počet, což
odpovídá chování řešiče předtím, než tato hodnota existovala. Sáhněte po ní,
když chcete pevné množství práce na krok bez ohledu na to, co dělá reziduum.

### 1.7 Ustálená analýza

Při vypnutém řešiči v čase řešič iteruje ustálenou bilanci hybnosti a hmoty k
pevnému bodu. Lineární soustava každé iterace není symetrická, takže se řeší
metodou **GMRES** s Jacobiho předpodmíněním, nikoli metodou sdružených
gradientů, kterou používají symetrické fyziky.

Ustálený model proudění potřebuje přinejmenším:

- vysíťovaný **objem** s přiřazenou hustotou a dynamickou viskozitou;
- **Wall** na každé pevné hranici;
- podmínku **přítoku** nebo rozdíl tlaků, který proudění pohání;
- **tlakovou referenci** - povrch s *Pressure (explicit)* nebo
  *Pressure (implicit)*. Bez ní je tlak určen jen až na konstantu a iterace se
  neustálí.

Model, jehož hranice tvoří výhradně stěny a předepsané rychlosti, skrývá ještě
jednu past: předepsaný přítok a odtok se musí **vyrovnávat**, protože
nestlačitelná tekutina se nemůže hromadit. Předepsání 5 m^3/s dovnitř a
4 m^3/s ven požaduje nemožné a řešení nezkonverguje. Ponecháním výstupu jako
*Pressure (implicit)* se otázce zcela vyhnete, což je hlavní důvod jej
upřednostnit.

Obvyklým postupem je nejprve nechat zkonvergovat ustálený výpočet a použít ho
jako výchozí pole přechodového výpočtu, protože přechodový výpočet spuštěný z
klidu stráví první časové kroky řešením rozběhového přechodu místo fyziky,
která vás zajímá.

### 1.8 Přechodová analýza

Při zapnutém řešiči v čase řešič kroku nestacionární rovnice schématem theta,
kde `alpha` je koeficient aproximace časového kroku:

| Aproximace | `alpha` | Výsledné krokování | Stabilita |
|---|---|---|---|
| Backward difference (stable) | `1` | plně implicitní krok | bezpodmínečná |
| Central difference (accurate) | `0.5` | Crankovo-Nicolsonové schéma | bezpodmínečná, může oscilovat |
| Forward difference (fast) | `0` | explicitní krok | podmíněná |

**Backward difference** je nastavení, které pro model proudění používat.
Čas stojí nelineární iterace v každém časovém kroku a implicitní krokování
umožňuje volit časový krok podle fyziky, nikoli podle sítě.

I při implicitním krokování je časový krok modelu proudění omezen přesností,
prostřednictvím **Courantova čísla**

```
C = |v| * dt / h
```

kde `h` je velikost prvku ve směru proudění. Courantovo číslo blízké `1`
znamená, že tekutina za krok projde přibližně jedním prvkem, což je správný řád
pro rozlišení přechodového děje; o mnoho více a řešič rozmazává přenos, který
má rozlišit, ať je stabilita krokování jakákoli. Zvolte `dt` podle sítě a
očekávané rychlosti:

```
dt ~ h / |v|
```

Každý časový krok provede úplnou nelineární iteraci z části 1.6, takže cena
přechodového výpočtu je počet časových kroků krát iterace na krok. Deset až
padesát iterací na krok je typický rozsah, jakmile je proudové pole už
zkonvergované z ustáleného výpočtu. Krok, jehož reziduum do konce jeho iterací
neklesne, je krokem s nezkonvergovanou odpovědí a chyba se přenese do každého
následujícího kroku.

### 1.9 Přenos tepla a rozptyl kontaminantu

Na proudovém poli, které tento řešič vytváří, se řeší dva typy úloh, každý s
vlastní příručkou:

- **Přenos tepla v tekutinách** (`RSolverFluidHeat`) přenáší teplotu s prouděním
  a je svázán s tepelným řešičem vysíťované pevné látky - viz
  [Přenos tepla v tekutinách](theory_fluid_heat.md);
- **Rozptyl kontaminantu** (`RSolverFluidParticle`) přenáší koncentraci
  kontaminantu s prouděním - viz
  [Rozptyl kontaminantu](theory_fluid_particle.md).

Oba čtou rychlost z výsledku proudění. Rozptyl kontaminantu proudění nikdy
nemění. Přenos tepla v tekutinách ano, prostřednictvím materiálových
vlastností.

#### Teplotně závislé vlastnosti a vztlak

Při **každém průchodu** řešič proudění vyhodnotí hustotu a dynamickou viskozitu
každého prvku z materiálových tabulek při teplotě prvku, kterou zanechal
poslední průchod přenosu tepla. Bez kroku přenosu tepla v úloze je tato teplota
všude výchozích `293.15 K`. Environmentální podmínku *Temperature* řešič
proudění přímo nečte; k proudění se dostane jen přes krok přenosu tepla.

Hustota v objemové síle `rho(T)*g` činí proudění vztlakovým: teplá tekutina je
lehčí než tekutina kolem ní a stoupá, studená klesá. Přirozená konvekce, tepelné
vlečky a stratifikace proto vycházejí ze skupiny kroků *Incompressible viscous
flow* a *Heat transfer in fluids*, pokud má materiál **tabulku hustoty závislou
na teplotě**. Dodávané materiály *Water* a *Air* ji mají. Materiál, jehož
hustota je jedinou hodnotou, nedává vztlak při žádném rozdílu teplot.

Jak si oba kroky vyměňují data:

- **Ustálený stav.** Dejte oba kroky do jedné skupiny, krok proudění jako první,
  s několika iteracemi a hodnotou konvergence. První průchod proudění vidí
  počáteční teplotu - počáteční podmínku *Temperature*, nebo `293.15 K` - takže
  ještě nemá žádný vztlak. Od druhé iterace vidí každý průchod proudění teplotu
  z předchozího tepelného průchodu. Model poháněný výhradně vztlakem potřebuje
  dost iterací, aby se oba ustálily společně; skupina s jedinou iterací je nikdy
  nesváže.
- **Přechodový výpočet.** Každý časový krok skupinu spustí znovu a každý průchod
  proudění vidí nejnovější teplotu. S jednou iterací na časový krok používá
  proudění teplotu předchozího kroku - zpoždění o jeden krok, které je
  přijatelné, dokud časový krok rozlišuje tepelný přechodový děj. Několik
  iterací na krok ho odstraní.

Skupina skončí, jakmile zkonverguje krok proudění, protože nesvázaný krok
*Heat transfer in fluids* se hlásí jako zkonvergovaný v každém průchodu. Změna
teploty posune reziduum proudění přes `rho(T)` a `mu(T)`, takže zkonvergované
proudění je zároveň prouděním, které teplota už dál netlačí.

Formulace ponechává `rho(T)` v každém členu bilance hybnosti a `div(v) = 0` pro
hmotu. Pro malé rozdíly hustoty u přirozené konvekce v kapalině nebo v plynu v
rozsahu několika desítek kelvinů to dává stejnou odpověď jako Boussinesqova
aproximace. Nejde o stlačitelnou formulaci: tepelná roztažnost tekutiny
nevstupuje do bilance hmoty, takže plyn ohřátý o stovky kelvinů je mimo její
rozsah.

### 1.10 Odvozené výsledky

| Výsledek | Použití na | Vytváří | Význam |
|---|---|---|---|
| Velocity `[m/s]` | uzel | proudění | vyřešené pole rychlosti, tři složky |
| Pressure `[Pa]` | uzel | proudění | vyřešené pole tlaku |

Výsledky doprovodných typů úloh uvádějí jejich vlastní příručky.

Rychlost se ukládá jako uzlový **vektor**, takže záložka `Výsledky` může
zobrazit její velikost jako barvu, její jednotlivé složky nebo šipky. Pro
výsledek proudění stojí za to znát dva zobrazovací nástroje:

- **rovinu řezu** objemem, protože zajímavá struktura proudění je uvnitř oblasti,
  nikoli na jejím povrchu;
- **proudnice**, vytvářené přes `Geometrie` -> `Proudnice`, které sledují pole
  rychlosti a jsou nejrychlejším způsobem, jak uvidět recirkulaci nebo mrtvou
  zónu.

Po každém záznamu vypíše log řešiče statistiku rychlosti a tlaku a hodnoty ve
všech monitorovacích bodech.

### 1.11 Ustálená, nebo přechodová?

| | Ustálená | Přechodová |
|---|---|---|
| Řeší | ustálenou bilanci, iterovaně | nestacionární rovnice, krokované v čase |
| Odpovídá na | „jak vypadá vyvinuté proudění“ | „jak se proudění vyvíjí a co s sebou nese“ |
| Iterace | stovky, v jedné skupině kroků | desítky na časový krok |
| Záznamy | jeden | jeden na zapsaný časový krok |
| Potřebuje tlakovou referenci | ano | ano |

**Ustálenou analýzu použijte pro** vyvinuté proudové pole potrubí, rozdělovače
nebo kanálu výměníku tepla, tlakovou ztrátu na součásti a jako výchozí pole pro
jakýkoli přechodový výpočet. Právě tím začíná téměř každý model proudění.

**Přechodovou analýzu použijte pro** rozběh a odstavení, pulzující nebo
cyklický přítok, odtrhávání vírů a - především - pro **rozptyl kontaminantu**,
který je úlohou přenosu v čase a pro jednorázový únik nemá smysluplnou
ustálenou odpověď.

Dvoukrokový postup návodu *Rozptyl kontaminantu v tekutinách* je ten
kanonický: nechat ustálené proudění zkonvergovat a pak výpočet restartovat se
zapnutým řešičem v čase a krokem rozptylu - viz příručka
[Rozptyl kontaminantu](theory_fluid_particle.md).

---

## 2. Grafické uživatelské rozhraní

Vše podstatné se nachází v panelu **Řešič** (záložky `Úloha`,
`Okrajové podmínky`, `Počáteční podmínky`, `Environmentální podmínky`,
`Materiál`, `Výsledky`), v nabídce **Úloha** a v panelu **Model**.

### 2.1 Výběr úlohy

`Úloha` -> `Pořadí kroků úloh` (`Ctrl+P`) otevře dialog pořadí kroků úlohy.
Přidejte krok a vyberte **Incompressible viscous flow** - „Steady-state and
transient flow of newtonian fluids“ (ustálené a přechodové proudění
newtonovských tekutin).

Krok proudění patří do **skupiny kroků**, která nese obě hodnoty řídící
nelineární iteraci z 1.6. Kteroukoli z nich upravíte dvojitým kliknutím:

| Hodnota | Význam |
|---|---|
| **počet iterací** | nejvyšší počet průchodů, které smí jedno řešení - nebo jeden časový krok - provést |
| **Konvergence** | skupina ukončí průchody, jakmile se krok proudění ustálí pod touto hodnotou **a** jeho reziduum klesne; `0` vždy spustí plný počet |

Výchozí hodnota konvergence je `1e-5`, což je pro model proudění bezpečná
relativní velikost kroku. Umístění kroku proudění mimo skupinu, kde proběhne
právě jednou, není užitečné - viz 1.6.

Kombinace, které stojí za to znát:

- samotné *Incompressible viscous flow* ve skupině s několika sty iteracemi -
  ustálené proudové pole;
- *Incompressible viscous flow* a *Contaminant dispersion* v jedné skupině -
  rozptyl na proudění, klasická přechodová dvojice, viz
  [Rozptyl kontaminantu](theory_fluid_particle.md);
- *Incompressible viscous flow* a *Heat transfer in fluids* v jedné skupině -
  ohřívané nebo chlazené proudění, viz
  [Přenos tepla v tekutinách](theory_fluid_heat.md).

Proudění vylučuje *Flow through porous media* a *Modal analysis*; oba doprovodné
typy úloh vyžadují přítomnost kroku proudění.

### 2.2 Záložka Úloha

**Řešič v čase** - zobrazuje se pro všechny tři typy úloh tekutin, protože
všechny jsou časově závislé. Pro ustálený výpočet proudění ho nechte vypnutý.

| Pole | Význam |
|---|---|
| Zapnutí | zapíná a vypíná přechodové krokování |
| Aproximace | theta časového krokování: zpětná, centrální nebo dopředná, viz 1.8 |
| Počáteční čas | první hodnota času v sekundách; restart z ustáleného výsledku umístí tento výsledek do počátečního času, restart přerušeného přechodového výpočtu pokračuje od jeho posledního kroku |
| Koncový čas | `start time + time-step size * number of time-steps`, pouze pro čtení |
| Velikost časového kroku | `dt` v sekundách, viz Courantovo doporučení v 1.8 |
| Počet časových kroků | kolik kroků se má vypočítat |
| Frekvence výstupu | zapsat soubor výsledků každých N kroků; `0` zapíše pouze poslední krok |

Typ úlohy proudění nemá vlastní skupinu nastavení; skupinu *Nastavení rozptylu
kontaminantu* popisuje příručka [Rozptyl kontaminantu](theory_fluid_particle.md).
Vše ostatní se přiřazuje pro každou entitu v záložkách podmínek a počet
iterací - na kterém zde záleží víc než kdekoli jinde v Range FEA - se nastavuje v
dialogu pořadí kroků úlohy, nikoli v této záložce.

### 2.3 Záložka Materiál

Záložka `Materiál` uvádí materiál přiřazený vybrané entitě. Typ úlohy
proudění vyžaduje:

| Vlastnost | Jednotky | Poznámka |
|---|---|---|
| Density | `kg/m^3` | `rho`, asi `1000` pro vodu, `1.2` pro vzduch |
| Dynamic viscosity | `kg/(m*s)` | `mu`, asi `1.0e-3` pro vodu, `1.8e-5` pro vzduch |

Doprovodné typy úloh uvádějí své vlastnosti ve svých příručkách. Entita, které
chybí některá vlastnost požadovaná typem úlohy v pořadí kroků, se neřeší a
kontrola nastavení na to upozorní před spuštěním výpočtu. Dodávaná databáze
materiálů obsahuje **Water** a **Air**, což je obvykle celý tento krok.

Všimněte si, že požadovaná viskozita je **dynamická** viskozita `mu` v `Pa*s`,
nikoli kinematická viskozita `nu = mu/rho` v `m^2/s`. Zadání `1.0e-6` pro vodu,
protože je to známé číslo, je nejčastější materiálovou chybou v modelu proudění
a činí tekutinu tisíckrát méně viskózní, než by měla být - což se obvykle
projeví výpočtem, který nezkonverguje.

Každá vlastnost je tabulkou v závislosti na teplotě, vyhodnocovanou při teplotě
prvku z předchozího výpočtu.

### 2.4 Záložka Okrajové podmínky

Záložka je rozdělena na seznam dostupných okrajových podmínek nahoře a editor
vybrané podmínky dole. Nejprve vyberte entitu ve stromu `Model`; správce pak
nabídne podmínky platné pro daný typ entity a vybrané typy úloh. Zaškrtnutím ji
přiřadíte a poté upravte její složky ve spodním stromu.

Nabízejí se podmínky z části 1.4. Poznámky k jejich použití:

- **Wall** patří na každou pevnou hranici a nepřijímá vůbec žádnou hodnotu.
  Povrchová entita vysíťovaného objemu, která nenese žádnou podmínku, *není*
  stěnou - je to nevázaná hranice, což je zřídka zamýšleno. Stěny jsou první
  věcí, kterou je třeba zkontrolovat u modelu, který se chová podivně.
- **Wall (frictionless)** je pro roviny symetrie zarovnané s globálními osami.
  Na nakloněném povrchu váže nesprávný směr - viz 1.4.
- **Velocity (inflow)** a **Volumetric flow rate (inflow)** obě pohánějí
  proudění dovnitř povrchem podél jeho průměrné normály. Vyberte tu, která
  odpovídá číslu, které skutečně máte. Záporná hodnota pohání proudění opačným
  směrem, což je jeden ze způsobů, jak zadat výstup.
- **Pressure (implicit)** je výstupní podmínka, kterou upřednostnit: nechá
  vyvinout profil odtoku a se zadanou hodnotou `0` činí z výstupu tlakovou
  referenci. **Pressure (explicit)** místo toho fixuje uzlový tlak.
- Podmínky *Heat transfer in fluids* a *Contaminant dispersion* se objeví,
  jakmile je jejich krok v pořadí kroků, a popisují je jejich vlastní příručky.
- Každá složka je tabulkou v závislosti na čase. Kliknutím na **Upravit časově
  závislé hodnoty** otevřete editor složek, kde se hodnoty zadávají v
  závislosti na čase; mezi dvěma zadanými časy se hodnota lineárně interpoluje a
  po posledním čase zůstává poslední hodnota. Takto se modeluje jednorázový
  únik, postupně se rozbíhající přítok nebo pracovní cyklus.

### 2.5 Počáteční a environmentální podmínky

Záložka `Počáteční podmínky` nabízí pro úlohu proudění *Velocity* a
*Pressure*. Nastavují výchozí pole prvního výpočtu a díky nim může přechodový
výpočet začít z něčeho jiného než z klidu.

Záložka `Environmentální podmínky` nabízí *Gravitational acceleration* s
výchozí hodnotou `(0, 0, -9.80665) m/s^2`. Přiřaďte ji objemu kvůli objemové
síle a každému povrchu s *Pressure (implicit)*, aby se hydrostatický člen této
podmínky měřil ve správném směru.

### 2.6 Nastavení maticového řešiče

`Úloha` -> `Nastavit maticový řešič úloh` nastavuje iterační řešiče. Řešič
proudění používá položku **GMRES**, protože jeho matice není symetrická.

Než zde cokoli změníte, rozlišujte dvě vnořené iterace: iterace **maticového
řešiče** řeší jednu linearizovanou soustavu, kdežto iterace **skupiny kroků**
řídí nelinearitu z části 1.6. Model proudění, který nekonverguje, téměř vždy
potřebuje více iterací skupiny kroků, lepší síť nebo opravenou sadu okrajových
podmínek - nikoli vyšší počet iterací GMRES.

### 2.7 Monitorovací body

`Úloha` -> `Definovat monitorovací body` umístí sondy na zadané souřadnice a
vybere proměnnou, která se má zaznamenávat - pro proudění *Velocity* nebo
*Pressure*. V přechodovém výpočtu zobrazí historii
`Zprávy` -> `Monitorovací body`.

### 2.8 Výsledky a záznamy

Záložka `Výsledky` uvádí vypočtené proměnné a ovládá 3D zobrazení. Rychlost je
uzlový vektor a tlak uzlový skalár.

Záložka `Záznamy` panelu `Model` uvádí záznamy výsledků:

- **ustálená analýza** - jediný záznam;
- **přechodová analýza** - jeden záznam na zapsaný časový krok. Animace záznamů
  přehraje proudění a tlačítko **Záznam** v dolní části stromu zapíše animaci do
  souboru videa.

`Zprávy` -> `Konvergence řešiče` vykreslí historii konvergence popsanou v 1.6 a
je to zpráva, kterou po každém výpočtu proudění otevřít jako první. `Zprávy` ->
`Log soubor řešiče` ukazuje celý výstup řešiče včetně statistiky rychlosti a
tlaku po každém záznamu.

---

## 3. Návod - ustálené proudění kanálem

**Cíl.** Vypočítat vyvinuté proudové pole a tlakovou ztrátu tekutiny hnané
kanálem a ověřit, že je výsledek zkonvergovaný.

Návod předpokládá vysíťovaný objemový model - dodávaný model **Channel.tmsh** je
přesně tento případ a používá ho i návod *Rozptyl kontaminantu v tekutinách*.
Pokud si sestavíte vlastní, nakreslete kvádr, tetraedrizujte ho a označte
vstupní stěnu, výstupní stěnu a zbývající stěny jako tři samostatné povrchové
entity, aby jim bylo možné přiřadit podmínky.

### Krok 1 - vyberte úlohu

1. `Úloha` -> `Pořadí kroků úloh` (`Ctrl+P`).
2. Přidejte **skupinu kroků** a dvojitým kliknutím na hodnotu nastavte její
   **počet iterací** na `2000`.
3. Hodnotu **Konvergence** ponechte na výchozím `1e-5`.
4. Do skupiny přidejte krok **Incompressible viscous flow**.
5. Potvrďte tlačítkem `OK`.

Obě hodnoty jsou celým smyslem tohoto kroku. Výpočet provede až 2000 průchodů a
zastaví se, jakmile se pole ustálí pod `1e-5` - viz 1.6. Počet nastavte
velkoryse: je to strop a výpočet obvykle ukončí hodnota konvergence.

### Krok 2 - vygenerujte síť

`Geometrie` -> `Objem` -> `Generovat čtyřstěnnou síť`.

Model proudění se řeší pouze na objemových prvcích. Bez objemové sítě nemá
řešič co sestavit.

### Krok 3 - přiřaďte materiál

1. Ve stromu `Model` vyberte objemovou entitu.
2. Otevřete záložku `Materiál` a přiřaďte **Water**, nebo zadejte:
   - **Density** = `1000` `kg/m^3`
   - **Dynamic viscosity** = `1.0e-3` `kg/(m*s)`

Zkontrolujte, že viskozita je dynamická v `Pa*s`, a ne kinematická
- viz 2.3.

### Krok 4 - ponechte analýzu ustálenou

Otevřete záložku `Úloha` a ve skupině *Řešič v čase* nechte **Zapnutí**
nezaškrtnuté.

### Krok 5 - přiřaďte stěny

1. Vyberte povrchovou entitu pokrývající pevné hranice kanálu.
2. Otevřete záložku `Okrajové podmínky` a zaškrtněte **Wall**.

Není co zadávat. Jde o podmínku nulového skluzu a právě ta způsobí, že proudění
vytvoří profil, místo aby proklouzlo jako píst.

### Krok 6 - poháněte proudění

1. Vyberte vstupní povrchovou entitu.
2. Zaškrtněte **Volumetric flow rate (inflow)** a zadejte průtok v `m^3/s` -
   nebo místo toho zaškrtněte **Velocity (inflow)** a zadejte rychlost v `m/s`.

Než budete pokračovat, odhadněte Reynoldsovo číslo ze střední rychlosti a šířky
kanálu:

```
Re = rho * V * L / mu
```

Pod několika tisíci je model v rozsahu, který tento řešič zvládá. Daleko nad
ním počítejte s tím, že výpočet nezkonverguje, a než mu budete věnovat čas,
přečtěte si část 5.

### Krok 7 - otevřete výstup

1. Vyberte výstupní povrchovou entitu.
2. Zaškrtněte **Pressure (implicit)** a nastavte **Pressure** = `0` `Pa`.

Tento krok činí model řešitelným. Dává tlakovému poli referenci a nechá vyvinout
profil odtoku, místo aby ho vynutil plochý. Model se vstupem a stěnami, ale bez
jakékoli tlakové podmínky, má singulární tlak a nezkonverguje.

### Krok 8 - řešte

1. `Řešení` -> `Spustit řešič` (`Ctrl+R`).
2. Kontrola nastavení ohlásí chybějící materiály nebo okrajové podmínky ještě
   před spuštěním výpočtu.
3. Průběh sledujte v `Zprávy` -> `Log soubor řešiče`.

Může to chvíli trvat - model proudění jakékoli velikosti je skutečná práce a
log vypisuje jeden blok na iteraci. V logu sledujte `Convergence target`:
průchod, který ho dosáhne, je označen a skupina pak ohlásí *All sub-tasks have
converged* a zastaví se.

### Krok 9 - zkontrolujte konvergenci

`Zprávy` -> `Konvergence řešiče`.

Udělejte to dřív, než se podíváte na jakýkoli obrázek. Reziduum by mělo klesnout
o několik řádů a pak se zploštit a `Convergence-V` a `Convergence-P` by se měly
blížit nule. Pokud reziduum v poslední iteraci stále strmě klesá, byl výpočet
příliš krátký - zvyšte počet iterací a restartujte. Pokud osciluje nebo roste,
odpověď není řešením a hledat je třeba v části 5.

### Krok 10 - prohlédněte si výsledky

- Záložka `Výsledky`: zobrazte **Velocity**. Profil napříč kanálem by měl být
  hladký, na stěnách nulový a nejrychlejší blízko středu.
- Veďte objemem rovinu řezu - zajímavá struktura proudění je uvnitř oblasti,
  nikoli na jejím povrchu.
- Zobrazte **Pressure**. Měl by monotónně klesat od vstupu k výstupu a na
  výstupu, kde jste ho nastavili, ukazovat `0`.
- Vytvořte **proudnice** pomocí `Geometrie` -> `Proudnice`, abyste okamžitě
  uviděli recirkulaci nebo mrtvé zóny.
- Zkontrolujte bilanci hmoty: střední rychlost na výstupu krát plocha výstupu
  by se měla rovnat objemovému průtoku předepsanému na vstupu. To je
  nejrychlejší globální kontrola, že model dělá to, co jste požadovali - viz
  část 4.

### Řešení problémů

| Příznak | Příčina |
|---|---|
| Reziduum nikdy neklesá | nikde není tlaková podmínka, takže tlak je neurčený |
| Reziduum osciluje nebo roste | Reynoldsovo číslo je pro síť příliš vysoké, nebo byla viskozita zadána jako kinematická |
| Proudění proklouzne bez profilu | na pevných hranicích chybí podmínka *Wall* |
| Výsledek vypadá jako první iterace | krok nebyl umístěn do skupiny kroků, takže proběhl jednou |
| Výpočet se zastavil daleko před počtem iterací | zkonvergoval - přírůstky i reziduum splnily test z 1.6 |
| Výpočet se nikdy nezastaví dříve, ať je hodnota konvergence jakkoli volná | reziduum neklesá - přečtěte v logu `Residual ratio` a považujte proudové pole za nezkonvergované |
| *Failed to calculate element scales. Unsupported element type* | síť obsahuje jiné objemové prvky než lineární čtyřstěny nebo šestistěny |
| Nic se nevyřešilo | nebyla vygenerována objemová síť, nebo materiálu chybí hustota nebo viskozita |
| Předepsaný přítok a odtok se nevyrovnávají | nestlačitelná tekutina se nemůže hromadit - na výstupu použijte *Pressure (implicit)* |

---

## 4. Kontrola modelu

Řešič nemá vestavěnou sadu ověřovacích testů, proto se vyplatí nový model před
použitím ověřit vůči něčemu, co lze spočítat ručně. Čtyři levné kontroly,
seřazené podle užitečnosti:

**Bilance hmoty.** U každého zkonvergovaného nestlačitelného modelu musí to, co
vteče dovnitř, vytéct ven:

```
sum( v . n * A ) over the inlets  =  sum( v . n * A ) over the outlets
```

(součet přes vstupy se rovná součtu přes výstupy.) Odečtěte střední rychlost na
výstupu ze statistiky v logu řešiče, vynásobte ji plochou výstupu a porovnejte s
předepsaným přítokem. Tato jediná globální kontrola odhalí nezkonvergovaný
výpočet, chybějící stěnu i okrajovou podmínku na nesprávné entitě a nic
nestojí.

**Hydrostatika.** Vypněte všechny přítoky, ponechte oblast uzavřenou stěnami,
přiřaďte environmentální podmínku *Gravitational acceleration* a řešte.
Tekutina se musí uklidnit a tlak se musí měnit lineárně s hloubkou:

```
p(h) = p_ref + rho * g * h
```

Jakýkoli zbytkový pohyb je numerický. Tím se v jednom výpočtu ověří objemová
síla gravitace, tlaková reference i jednotky materiálu a je to nejrychlejší
první model, který sestavit v novém nastavení.

**Rovinné Poiseuilleovo proudění.** Pro ustálené laminární proudění mezi dvěma
rovnoběžnými deskami ve vzdálenosti `H` je profil parabolický, rychlost v ose je
`1.5` násobkem střední a tlakový gradient je

```
dp/dx = 12 * mu * V_mean / H^2
```

Vymodelujte přímý kanál se stěnami nahoře a dole, předepsaným přítokem a
výstupem *Pressure (implicit)* a porovnejte s tím vypočtenou rychlost v ose a
tlakovou ztrátu. To je nejostřejší dostupná kontrola: najednou ověří viskózní
člen, stěnu bez skluzu i jednotky viskozity. Než budete měřit, dejte modelu
dostatečnou náběhovou délku, aby se profil vyvinul, protože vstupní podmínka
vnucuje plochý profil.

**Hagenovo-Poiseuilleovo proudění.** Stejná kontrola v kruhové trubce o průměru
`D` a délce `L`, kde je rychlost v ose `2` násobkem střední a

```
dp = 128 * mu * Q * L / ( pi * D^4 )
```

platí, dokud je proudění laminární, tedy zhruba pod `Re = 2300`.

U kterékoli z těchto kontrol zjemněte síť alespoň jednou a potvrďte, že odpověď
konverguje, a ne že se posouvá - a před jakýmkoli porovnáním potvrďte, že se
historie konvergence zploštila. Nezkonvergovaný model proudění selže v každé z
těchto kontrol ze stejného důvodu.

---

## 5. Omezení

### Omezení modelování

- **Žádný model turbulence.** Řešič integruje laminární Navierovy-Stokesovy
  rovnice. Členy SUPG, PSPG a LSIC jsou numerickou stabilizací, nikoli
  uzávěrem turbulence - nereprezentují míchání ani dodatečnou disipaci
  turbulentního proudění. Zhruba nad `Re = 2300` v trubce je skutečné proudění
  turbulentní a tento řešič ho nemodeluje: výpočet může stále zkonvergovat, ale
  odpověď popisuje laminární proudění, které neexistuje. Neexistuje model
  `k`-`e`, stěnová funkce ani formulace velkých vírů.
- **Pouze nestlačitelné a newtonovské.** Bilance hmoty je `div(v) = 0` a
  viskózní napětí je lineární v rychlosti deformace. Hustota a viskozita mohou
  záviset na teplotě, ale ne na tlaku. Stlačitelné proudění, vlivy Machova čísla,
  volné hladiny, více fází a nenewtonovské chování jsou mimo formulaci.
- **Vztlak bez roztažnosti tekutiny.** Teplotně závislá hustota pohání
  přirozenou konvekci prostřednictvím `rho(T)*g` (viz 1.9), ale tepelná
  roztažnost nikdy nevstupuje do bilance hmoty. To je přesné pro malé rozdíly
  hustoty u přirozené konvekce, nikoli pro plyn ohřátý o stovky kelvinů.
  Vztlakové proudění je laminární jako každé jiné proudění zde - vlečka nebo
  dutina nad Rayleighovým číslem zhruba `1e8` až `1e9` je ve skutečnosti
  turbulentní.
- **Teplota se k proudění dostane jen z kroku přenosu tepla.** Bez něj se
  materiálové vlastnosti vyhodnocují při `293.15 K`; samotná environmentální
  podmínka *Temperature* je nemění.
- **Rozptyl kontaminantu je jednosměrný.** Proudění nese kontaminant;
  koncentrace proudění nikdy nemění. Omezení doprovodných typů úloh uvádějí
  jejich vlastní příručky.
- **Pouze objemové prvky, pouze lineární čtyřstěny a šestistěny.** Model
  proudění musí být vysíťovaným objemem z prvků `TETRA1` nebo `HEXA1`; cokoli
  jiného výpočet zastaví. Povrchy se účastní jen k nesení zatížení
  *Pressure (implicit)* a bodové a čárové entity nepřispívají ničím.
- **Stěna bez tření je zarovnaná s osami.** Váže jedinou dominantní globální
  složku normály každého prvku, takže je přesná jen na povrchu, jehož normála
  leží podél `x`, `y` nebo `z`.
- **Vstupy jsou rovnoměrné.** *Velocity (inflow)* i *Volumetric flow rate
  (inflow)* vnucují plochý profil podél průměrné normály povrchu. Vyvinutý
  vstupní profil je nutné vytvořit náběhovou délkou v síti.

### Omezení řešiče

- **Iterace je modifikovaným Newtonovým schématem**, takže konvergence poblíž
  odpovědi je lineární, nikoli kvadratická. U ustáleného výpočtu počítejte se
  stovkami iterací a nastavte počet iterací skupiny kroků dost vysoko, aby
  výpočet ukončila hodnota konvergence, a ne počet. Tlumení z 1.6 udrží takový
  výpočet v sestupu, ale kvadratickým ho udělat nedokáže.
- **Centrální diference řešiče proudění není Crankovo-Nicolsonové schéma.** Váha
  theta se uplatňuje na matici, ale ne na reziduum, takže
  *Central difference (accurate)* řeší reziduum zpětné Eulerovy metody s maticí
  poloviční tuhosti. Odpovědí je odpověď zpětné diference, dosažená pomaleji.
  Dokud to nebude opraveno, používejte pro proudění
  *Backward difference (stable)*. Řešiče rozptylu kontaminantu se to netýká -
  je lineární a jeho krokování je skutečným Crankovým-Nicolsonovým schématem.
- **Stabilizační parametry se nederivují.** `Tsupg`, `Tpspg`, `Tlsic` a délka
  prvku všechny závisí na rychlosti a všechny vstupují do matice jako konstanty
  vyhodnocené při aktuálním poli. To je u stabilizované formulace obvyklá praxe
  a jeden z důvodů, proč Jacobián není přesný.
- **Část testu konvergence týkající se rezidua je pevná.** Řešení se považuje za
  zkonvergované, až když jeho reziduum klesne na desetinu hodnoty na začátku
  daného řešení, a tuto desetinu nelze změnit. Je záměrně konzervativní: pomalu
  konvergující model ji možná nikdy nesplní a pak proběhne s plným počtem
  iterací. Jak blízko se takový výpočet dostal, uvádí
  `Residual ratio` v logu.
- **Je vyžadována tlaková reference.** Bez povrchu s *Pressure (explicit)* ani
  *Pressure (implicit)* je tlak určen jen až na konstantu a iterace se neustálí.
- **Předepsané průtoky se musí vyrovnávat.** Model, jehož hranice tvoří výhradně
  stěny a předepsané rychlosti, musí mít přítok přesně rovný odtoku;
  nestlačitelná tekutina rozdíl nepojme.
- **Cena.** Čtyři neznámé na uzel, nesymetrická matice a nelineární iterace
  kolem každého lineárního řešení činí z modelu proudění s velkým odstupem
  nejdražší typ úlohy v Range FEA. Podle toho dimenzujte síť a před spuštěním
  jakéhokoli přechodového výpočtu nechte zkonvergovat ustálené pole.

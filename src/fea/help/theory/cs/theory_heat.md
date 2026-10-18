# Přenos tepla - Teoretická příručka a uživatelská příručka

Tento dokument popisuje tepelný řešič (`RSolverHeat`) v Range FEA, který
obsluhuje typ úlohy **Přenos tepla** (*Heat transfer*): rovnice, které řeší,
význam každého vstupu, který přijímá, části grafického uživatelského rozhraní,
které ho ovládají, a dva podrobné návody.

**Obsah**

1. [Teoretický základ](#1-teoretický-základ)
2. [Grafické uživatelské rozhraní](#2-grafické-uživatelské-rozhraní)
3. [Návod - ustálený přenos tepla](#3-návod---ustálený-přenos-tepla)
4. [Návod - přechodový přenos tepla](#4-návod---přechodový-přenos-tepla)
5. [Kontrola modelu](#5-kontrola-modelu)
6. [Omezení](#6-omezení)

---

## 1. Teoretický základ

### 1.1 Primární neznámá - teplotní pole

Primární neznámou je **teplota** `T` v každém uzlu, v kelvinech. Každý uzel
nese jediný stupeň volnosti, takže síť s `N` uzly dává soustavu `N` rovnic.
Tepelný tok se po vyřešení získá z teplotního pole.

Teplota je skalár, díky čemuž je tepelný model výrazně levnější než
strukturální model na stejné síti: jedna neznámá na uzel místo tří a matice s
devítinou prvků.

Výchozí teplota uzlu, kterého se nedotkne žádná podmínka ani předchozí
výsledek, je `293.15 K`.

### 1.2 Řídicí rovnice

Řešič implementuje **lineární vedení tepla** s konvektivními hranicemi.
Energetická bilance tělesa s tepelnou vodivostí `k`, hustotou `rho` a měrnou
tepelnou kapacitou `c` je

```
rho * c * dT/dt = div( k * grad(T) ) + q
```

a při vypnutém řešiči v čase přechodový člen vypadne a zůstane ustálená
bilance

```
div( k * grad(T) ) + q = 0
```

Teplo opouští povrch konvekcí podle Newtonova ochlazovacího zákona,

```
qs = h * ( T - Tf )
```

| Symbol | Význam | Jednotky | Zdroj |
|---|---|---|---|
| `T` | teplota | `K` | řešená veličina |
| `q` | hustota zdroje tepla | `W/m^3` | podmínka Heat, Jouleovo teplo, záření |
| `k` | tepelná vodivost | `W/(m*K)` | materiálová vlastnost |
| `c` | tepelná kapacita | `J/(kg*K)` | materiálová vlastnost |
| `rho` | hustota | `kg/m^3` | materiálová vlastnost |
| `h` | součinitel přestupu tepla | `W/(m^2*K)` | podmínka konvekce, viz 1.5 |
| `Tf` | teplota tekutiny | `K` | podmínka konvekce |
| `Q` | tepelný tok | `W/m^2` | odvozený, viz 1.9 |

Neexistuje žádný advekční člen. Úloha přenosu tepla přenáší energii vedením
uvnitř tělesa a konvekcí přes jeho hranici; nepřenáší energii proudícím
médiem. K tomu použijte *Heat transfer in fluids*.

**Požadované materiálové vlastnosti.** Tepelná úloha vyžaduje **čtyři**:

| Vlastnost | Jednotky | Použití |
|---|---|---|
| Thermal conductivity | `W/(m*K)` | matice vedení |
| Heat capacity | `J/(kg*K)` | matice kapacity, pouze přechodový výpočet |
| Density | `kg/m^3` | matice kapacity, pouze přechodový výpočet |
| Emissivity | `-` | pouze přenos tepla zářením |

Entita, jejímuž materiálu některá ze čtyř vlastností chybí, se **neřeší**, a to
ani při čistě ustáleném výpočtu vedení, který se na hustotu ani emisivitu nikdy
nepodívá. Kontrola nastavení na to upozorní před spuštěním výpočtu. Jedinou
výjimkou je entita, která nese tepelnou okrajovou podmínku: ta je do soustavy
zahrnuta bez ohledu na materiál a její vodivost a kapacita jsou pak prostě
nulové.

Každá vlastnost je tabulkou v závislosti na teplotě. Hodnota pro prvek se čte z
tabulky při **teplotě prvku z předchozího výpočtu**, takže teplotně závislá
vodivost se zpožďuje o jednu iteraci. Jak ji nechat zkonvergovat, vysvětluje
část 1.6.

### 1.3 Diskretizace metodou konečných prvků

Slabá formulace energetické bilance dává pro každý prvek

```
K = INT( B^T * k * B )  +  INT( N^T * h * N )      conduction + convection
M = INT( rho * c * N^T * N )                       capacity matrix
f = INT( N^T * q )  +  h * Tf * A / n              sources + convection
```

(`conduction + convection` - vedení + konvekce, `capacity matrix` - matice
kapacity, `sources + convection` - zdroje + konvekce), kde `B` je sestaveno z
derivací bázových funkcí a `N` jsou samotné bázové funkce. Každá matice prvku
má rozměr `n x n` pro prvek s `n` uzly - jeden řádek na uzel, ne tři.

Konvekční členy se sestavují **pouze na povrchových prvcích** a jen tam, kde je
přiřazena okrajová podmínka konvekce. Konvekční tuhost má konzistentní tvar
`INT( N^T * h * N )`, kdežto konvekční zatížení se rovnoměrně soustředí do uzlů
prvku. Na rozumné síti jsou oba dostatečně konzistentní a rozdíl se se
zjemňováním sítě ztrácí.

**Míry prvků.** Stejně jako jinde v Range FEA dostávají entity nižší dimenze
geometrickou míru, aby mohly reprezentovat skutečná tělesa:

| Entita | Míra | Kde se nastavuje |
|---|---|---|
| Objem | 1 | - |
| Povrch | tloušťka povrchu | geometrické vlastnosti entity |
| Čára | plocha průřezu čáry | geometrické vlastnosti entity |
| Bod | objem bodu | geometrické vlastnosti entity |

**Povrch** vede teplo ve své vlastní rovině průřezem rovným své tloušťce a
akumuluje energii úměrně této tloušťce. Neexistuje teplotní gradient napříč
tloušťkou: povrchový prvek je silný jeden uzel, takže je napříč tloušťkou z
konstrukce izotermický. Povrch s **nulovou tloušťkou** proto nic nevede a nic
neakumuluje, ale stále nese konvekci - což je přesně to, co je potřeba u
hraniční stěny vysíťovaného objemu, kde stěna existuje jen proto, aby nesla
podmínku konvekce.

**Čára** vede teplo ve svém vlastním směru průřezem rovným své ploše průřezu.
**Bod** nemá směr, kterým by vedl, takže nepřispívá žádným vedením: nese pouze
kapacitu úměrnou svému objemu bodu a zdroj tepla. Bod je soustředěná tepelná
hmota, nikoli vodič.

Matice kapacity se sestavuje jen tehdy, když je potřeba, tedy pro přechodovou
analýzu.

### 1.4 Zdroje a okrajové podmínky

| Okrajová podmínka | Typ | Použití na | Složky |
|---|---|---|---|
| Temperature | explicitní (Dirichletova) | bod, čára, povrch, objem | Temperature `[K]` |
| Heat | přirozená, zdroj | bod, čára, povrch, objem | Heat `[W]`, viz níže |
| Simple convection | přirozená (Robinova) | povrch | Convection coefficient `[W/(m^2*K)]`, Fluid temperature `[K]` |
| Forced convection | přirozená, korelace | povrch | Density, Dynamic viscosity, Fluid temperature, Heat capacity, Hydraulic diameter, Thermal conductivity, Velocity |
| Natural convection | přirozená, korelace | povrch | Density, Dynamic viscosity, Fluid temperature, Heat capacity, Hydraulic diameter, Thermal conductivity, Thermal expansion coefficient |
| Heat rate (unit area) | přirozená, zdroj | povrch | Heat per unit area `[W/m^2]` |
| Heat rate (unit volume) | přirozená, zdroj | objem | Heat per unit volume `[W/m^3]` |

**Temperature** (teplota) je jedinou explicitní podmínkou tepelné úlohy. Každý
uzel entity, které je přiřazena, se odstraní ze soustavy rovnic a drží se na
zadané hodnotě; jeho příspěvek se přenese na pravou stranu sousedních rovnic.
Ustálený model potřebuje alespoň jednu z nich nebo alespoň jeden povrch s
konvekcí - viz 1.7.

Tam, kde se entity překrývají, **vyhrává entita nižší dimenze**: teplota
předepsaná na povrchu přepíše ve sdílených uzlech teplotu předepsanou na objemu
za ním, čára přepíše povrch a bod přepíše vše. Na rozdíl od vazeb posunutí
strukturálního řešiče se teploty nekombinují - na uzel připadá jen jeden stupeň
volnosti, takže prostě převáží poslední načtená podmínka.

**Heat** (teplo) je zdroj tepla a jeho hodnotou je **celkový výkon ve `W`**
dodaný do entity, které je přiřazen, bez ohledu na dimenzi této entity. Řešič
potřebuje hustotu zdroje, proto se celková hodnota před sestavením rozloží na
míru entity:

| Entita | Hustota, kterou řešič použije |
|---|---|
| Objem | hodnota / objem entity, `W/m^3` |
| Povrch | hodnota / plocha entity, `W/m^2` |
| Čára | hodnota / délka entity, `W/m` |
| Bod | hodnota / počet bodových prvků, `W` na každý |

Dodaný výkon je proto přesně zadané číslo - nemění se s velikostí entity, se
zjemněním sítě ani s měřítkem modelu. Podíl dostanou jen prvky, které řešič umí
spočítat, takže entita bez materiálových vlastností se přeskočí a její podmínka
se místo toho ohlásí. Povrchový zdroj tepla se tloušťkou povrchu neškáluje.

**Heat rate (unit area)** a **Heat rate (unit volume)** předepisují tentýž
zdrojový člen jako hustotu místo celkové hodnoty: `W/m^2` na plochu povrchové
entity a `W/m^3` na objem objemové entity. Sáhněte po nich, když je zdroj
přirozeně hustotou - topné těleso dimenzované na metr čtvereční - a po *Heat*,
když je přirozeně celkovou hodnotou, například topné těleso známého příkonu.
Na entitě, která jich nese více, se všechny sčítají a všechny přijímají tabulku
v závislosti na čase, takže topné těleso lze během přechodového výpočtu zapínat
a vypínat.

Podmínky **konvekce** platí pro povrchy a přidávají člen tuhosti i zatížení,
takže působí jako Robinova okrajová podmínka, nikoli jako pevný tok: zvýšení
teploty stěny zvýší teplo, které stěna ztrácí. Všechny tři varianty končí
stejnou dvojicí `(h, Tf)`; liší se jen tím, odkud se `h` bere:

- **Simple convection** (jednoduchá konvekce) bere `h` a `Tf` přímo ze dvou
  zadaných hodnot. Použijte ji vždy, když je součinitel přestupu tepla znám z
  příručky, měření nebo předchozího výpočtu. Je to jediná ze tří, kterou máte
  plně pod kontrolou.
- **Forced convection** (nucená konvekce) má dva režimy. Na stěně mezi
  vysíťovanou pevnou látkou a vysíťovanou tekutinou, jakmile krok
  *Heat transfer in fluids* vytvoří výsledek, propojuje obě řešení přímo -
  žádná korelace se nepoužívá, protože rozlišené proudění již konvekci nese.
  Řešič tepla v tekutinách předá pro každý prvek stěny dvojici `(h, Tf)`, která
  reprodukuje tepelný tok, jenž jeho vlastní řešení stěnou propouští; viz
  *Sdružený přenos tepla* v 1.6. Všude jinde - na povrchu, který nesousedí s
  vysíťovanou tekutinou, nebo v prvním průchodu svázaného výpočtu, než proběhne
  krok tepla v tekutinách - se `h` počítá z korelace pro vnější obtékání rovinné
  desky s použitím vlastností tekutiny, hydraulického průměru a zadaných hodnot
  *Fluid temperature* a *Velocity*. Log uvádí, který z obou režimů se používá.
- **Natural convection** (přirozená konvekce) počítá `h` z korelace pro
  vodorovné desky s použitím vlastností tekutiny, hydraulického průměru a
  rozdílu teplot mezi stěnou a tekutinou.

Je-li stejnému povrchu přiřazeno více podmínek konvekce, nesčítají se: jako
první se čte jednoduchá, nucená ji přepíše a přirozená přepíše obě.

### 1.5 Korelace konvekce

Obě počítané varianty sestavují obvyklá bezrozměrná čísla z vlastností
tekutiny zadaných s okrajovou podmínkou, s hydraulickým průměrem `d` jako
charakteristickým rozměrem:

```
Pr = c * mu / k                                        Prandtl
Re = rho * v * d / mu                                  Reynolds
Gr = | rho^2 * g * beta * (Ts - Tf) * d^3 / mu^2 |     Grashof,  g = 9.81
Ra = Gr * Pr                                           Rayleigh
```

a převádějí Nusseltovo číslo na součinitel přestupu tepla jako

```
h = Nu * k / d
```

| Varianta | Korelace | Nusseltovo číslo |
|---|---|---|
| Nucená | vnější obtékání, rovinná deska | `Nu = 0.036 * Re^(4/5) * Pr^(1/3)` |
| Přirozená | vodorovná deska, `Ra < 2e7` | `Nu = 0.54 * Ra^(1/4)` |
| Přirozená | vodorovná deska, `Ra >= 2e7` | `Nu = 0.14 * Ra^(1/3)` |

`k`, `c`, `rho`, `mu` a `beta` v těchto výrazech jsou vlastnosti **tekutiny**
zadané s okrajovou podmínkou, nikoli materiálové vlastnosti pevné látky.

Obě podmínky se vytvářejí s vlastnostmi **suchého vzduchu při 20 stupních
Celsia a 101.325 kPa**, takže tekutina je popsána za vás a zbývá zadat jen
proudění a geometrii:

| Složka | Výchozí hodnota |
|---|---|
| Density | `1.2041 kg/m^3` |
| Dynamic viscosity | `1.8205e-5 kg/(m*s)` |
| Heat capacity | `1005 J/(kg*K)` |
| Thermal conductivity | `0.02514 W/(m*K)` |
| Thermal expansion coefficient | `3.4112e-3 1/K`, hodnota ideálního plynu `1/T` |
| Fluid temperature | `293.15 K` |
| Hydraulic diameter | `1 m` |

Pro jakoukoli jinou tekutinu je nahraďte - jde o vlastnosti tekutiny, nikoli o
materiál stěny. *Natural convection* je proto po přiřazení připravena k řešení;
*Forced convection* stále potřebuje **rychlost**, kterou žádná výchozí hodnota
neuhodne - ledaže povrch sousedí s vysíťovanou tekutinou, kde se žádná z těchto
hodnot nečte.

Každé z těchto čísel dělí `mu`, `k` nebo `d` a `Re` a `Pr` klesnou na nulu,
klesne-li na ni `rho`, `c` nebo `v`, což by vedlo k `h = 0` a povrchu, který se
nikdy neochladí. Nulová hodnota se proto považuje za chybu, místo aby se
zamlčela: řešič se zastaví a uvede složku a entitu, například

```
Value of 'Velocity' configured in 'Forced convection' boundary condition on
entity 'Surface' is zero - the convection correlation can not be evaluated.
```

(hodnota *Velocity* nastavená v okrajové podmínce *Forced convection* na
entitě *Surface* je nulová - korelaci konvekce nelze vyhodnotit.)

Rychlost vypočtená řešičem proudění se do korelace nikdy nedosazuje. Stěna bez
skluzu drží tekutinu v klidu, takže rychlost u ní je nulová, a korelace by
potřebovala rychlost volného proudu a střední teplotu, které obecné
trojrozměrné proudění nedefinuje. Stěna sousedící s vysíťovanou tekutinou je
místo toho svázána s řešením tekutiny.

Stojí za to mít na paměti tři důsledky:

- Korelace je **pevná**. Řešič nebere v úvahu orientaci povrchu, zda jde o
  vnitřní nebo vnější proudění, ani zda je mezní vrstva laminární nebo
  turbulentní. Vše vstupuje přes hydraulický průměr, který je charakteristickým
  rozměrem korelace - je to vaše jediná páka na geometrii.
- Nulový hydraulický průměr by dal `h = 0`, tedy izolovanou stěnu, proto se
  řešič raději zastaví, než aby ho přijal.
- Přirozená konvekce závisí na **teplotě stěny**, která se bere z teploty prvku
  z **předchozího** řešení. Při prvním výpočtu ustálené analýzy je to počáteční
  teplota, takže se součinitel vyhodnotí při nesprávném rozdílu teplot. Abyste
  ho nechali zkonvergovat, zabalte krok do skupiny kroků úlohy s několika
  iteracemi, nebo použijte *Simple convection* se součinitelem, který jste
  spočítali sami. Mizivý rozdíl teplot dává `Nu = 1`.

### 1.6 Environmentální, počáteční a vazební podmínky

#### Environmentální podmínky

| Environmentální podmínka | Složky | Účinek |
|---|---|---|
| Temperature | Temperature `[K]` | teplota pozadí entity, pouze první výpočet |

#### Počáteční podmínky

| Počáteční podmínka | Účinek |
|---|---|
| Temperature | počáteční teplotní pole, použije se při prvním výpočtu |

Environmentální podmínky, počáteční podmínky a okrajové podmínky se čtou v
tomto pořadí a každá přepíše předchozí, takže **okrajová podmínka má přednost
před počáteční podmínkou, která má přednost před environmentální podmínkou**.
Environmentální a počáteční hodnoty se čtou **pouze při prvním výpočtu**; poté
pole pokračuje od předchozího výsledku.

#### Vazba s jinými řešiči

Tepelný řešič výsledky sdílené se zbytkem pořadí kroků spotřebovává i vytváří.

| Směr | Proměnná | Účinek |
|---|---|---|
| Přenos tepla zářením -> teplo | Heat - radiation part | přičte se ke zdrojovému členu **povrchových** prvků |
| Elektrostatika -> teplo | Joule heat | přičte se ke zdrojovému členu každého prvku |
| Napětí -> teplo | Displacement | síť se pro tepelné řešení deformuje a poté obnoví |
| Teplo -> napětí | Temperature | pohání člen tepelné roztažnosti strukturálního řešiče |
| Teplo -> libovolný řešič | Temperature | vybírá řádek každé materiálové tabulky závislé na teplotě |
| Teplo v tekutinách -> teplo | stěna `(h, Tf)` | pohání stěny *Forced convection* sousedící s vysíťovanou tekutinou |
| Teplo -> teplo v tekutinách | teplota pevné látky | drží tytéž stěny v řešení tekutiny na teplotě pevné látky |

**Přenos tepla zářením** (*Radiative heat transfer*) je samostatný typ úlohy s
vlastním řešičem. Vyžaduje krok *Heat transfer*, řeší soustavu radiozity
uzavřeného prostoru přes povrchové záplaty s pomocí view-faktorů a emisivity
materiálu a výsledek vrací jako povrchový zdroj tepla. Přidejte oba kroky do
jedné skupiny kroků úlohy a nechte skupinu iterovat: řešič záření hlásí
konvergenci a ukončí skupinu, jakmile se teplo jeho záplat přestane měnit.

**Jouleovo teplo** přichází stejným způsobem z kroku *Electro-statics*
umístěného před tepelným krokem, takže odporový ohřev nevyžaduje žádné další
nastavení.

Pokud není svázán s tekutinou, hlásí se tepelný řešič jako zkonvergovaný
bezpodmínečně, takže skupina kroků obsahující pouze tepelný krok se vždy zastaví
po jedné iteraci. Iterace má smysl, když je zpožděno něco, na čem tepelné
řešení závisí - teplo záření, součinitel přirozené konvekce nebo silně teplotně
závislá vodivost.

#### Sdružený přenos tepla

Tepelný řešič řeší **pouze pevné látky**. Objem, jehož materiál je tekutina -
plyn nebo kapalina, nebo materiál neurčeného skupenství, který má dynamickou
viskozitu, což pokrývá všechny tekutiny databáze materiálů - nikdy není
součástí tepelného řešení, bez ohledu na vlastnosti nebo podmínky, které nese.
Stejně tak bodová, čárová nebo povrchová entita ležící uvnitř tekutiny, například
vstup; podmínka na ní, kterou čte pouze tepelný řešič, se ohlásí v logu. Oblast
tekutiny patří typu úlohy *Heat transfer in fluids*, který přenáší teplo s
prouděním.

Obě oblasti se setkávají na stěnách s podmínkou **Forced convection**. S oběma
kroky v jedné skupině kroků úlohy - krok tepla v tekutinách jako první - provede
každý průchod skupiny následující:

1. Řešič tepla v tekutinách drží uzly stěny na teplotě, kterou tepelný řešič
   naposledy vypočítal v pevné látce (při úplně prvním průchodu, kdy taková
   teplota ještě neexistuje, je stěna izolovaná), vyřeší tekutinu a v každém
   uzlu stěny odečte tepelný tok vstupující do tekutiny. Tento tok je reziduem
   rovnic tekutiny v uzlu, tím, které by viděl jediný společný výpočet obou
   oblastí, takže tenká tepelná mezní vrstva nestojí žádnou přesnost.
2. Pro každý prvek stěny předá dvojici `(h, Tf)`. `h = k * G` je vodivost
   prvního prvku tekutiny za stěnou - u čtyřstěnu `k` děleno výškou prvku nad
   stěnou - a `Tf` je nastaveno tak, aby `q = h * (Tw - Tf)` byl tok z kroku 1.
3. Tepelný řešič je na stěnu použije tak, jako by použil podmínku
   *Simple convection*, vyřeší pevnou látku a vrátí teplotu své stěny.

Dvojice `(h, Tf)` je numerickou pomůckou, která obě řešení uvede do souladu -
`h` není součinitel přestupu tepla z příručky a mění se se sítí. Konverguje
teplota a tok stěnou, které odpovídají svázané úloze. Není potřeba nic vědět o
rychlosti tekutiny u stěny, a proto stěna bez skluzu - kde je proudění v klidu -
přenáší teplo přesně tak, jak má.

Teplota stěny předávaná tekutině se relaxuje **Aitkenovým faktorem**
vypočteným z posledních dvou průchodů. Bez něj může špatně vodivá pevná látka
vedle dobře rozlišené tekutiny potřebovat stovky průchodů; s ním se skupina
obvykle ustálí během několika. Oba řešiče hlásí jako svou konvergenci
relativní změnu svého teplotního pole, dokud jsou svázány, proto nastavte
hodnotu konvergence skupiny kroků - `1e-6` je rozumný začátek - a dejte jí
dostatek iterací. Nulová hodnota konvergence spustí všechny.

### 1.7 Ustálená analýza

Při vypnutém řešiči v čase řešič sestaví a vyřeší

```
K * T = f
```

`K` je symetrická pozitivně definitní, jakmile je předepsána alespoň jedna
teplota, takže se řeší metodou **sdružených gradientů** s Jacobiho
předpodmíněním.

Ustálený model **bez okrajové podmínky Temperature a bez povrchu s konvekcí**
je tepelnou obdobou nepodepřené konstrukce: energii lze dodávat, ale nemá kudy
odejít, matice je singulární a řešení nezkonverguje. Každý ustálený model
potřebuje alespoň jednu cestu ke známé teplotě.

### 1.8 Přechodová analýza

Při zapnutém řešiči v čase řešič kroku

```
M * dT/dt + K * T = f
```

schématem theta, kde `alpha` je koeficient aproximace časového kroku:

```
( M + alpha*dt*K ) * T_n+1 = dt*f + ( M - (1-alpha)*dt*K ) * T_n
```

Nastavení *Aproximace* řešiče v čase volí `alpha`:

| Aproximace | `alpha` | Výsledné krokování | Stabilita |
|---|---|---|---|
| Backward difference (stable) | `1` | `(M + dt*K)*T_n+1 = dt*f + M*T_n`, implicitní krok | bezpodmínečná |
| Central difference (accurate) | `0.5` | Crankovo-Nicolsonové schéma | bezpodmínečná, může oscilovat |
| Forward difference (fast) | `0` | `M*T_n+1 = dt*f + (M - dt*K)*T_n`, explicitní krok | podmíněná |

**Backward difference** (zpětná diference) je pro tepelný model bezpečnou
výchozí volbou. Stojí jedno řešení matice na krok, ale krok je omezen
požadovanou přesností, nikoli velikostí nejmenšího prvku. **Central
difference** (centrální diference) má přesnost druhého řádu a je lepší volbou,
jakmile je časový krok dost malý na rozlišení přechodového děje, i když při
prudkém zapnutí může zvonit. **Forward difference** (dopředná diference) dává
celou matici vedení na starý časový krok, díky čemuž je každý krok levný a
výpočet křehký.

Dopředná větev je stabilní jen tehdy, dokud časový krok zůstává pod přibližně

```
dt < rho * c * h^2 / ( 2 * k )
```

kde `h` je nejmenší velikost prvku. Protože teplotní vodivost `k/(rho*c)` kovu
je řádově `1e-5 m^2/s`, už prvek velikosti milimetru vynutí časový krok hluboko
pod desetinou sekundy. Pro cokoli jiného než velmi jemné časové rozlišení
použijte implicitní krokování.

Nestabilní přechodový výpočet se snadno pozná: teplotní pole osciluje z kroku
na krok s rostoucí amplitudou, často mezi fyzikálně nemožnými hodnotami,
místo aby se ustálilo. Lékem je menší časový krok nebo implicitní krokování.

### 1.9 Odvozené výsledky

Po vyřešení se prvkový tepelný tok získá z teplotního pole jako průměr přes
integrační body výrazu

```
Q = -k * grad(T)
```

vyhodnoceného v prvku a u čárových a povrchových prvků otočeného do globálních
souřadnic. Na povrchu s konvekcí se přičte konvektivní tok

```
Qh = h * ( Tf - T )
```

podél normály povrchu, takže vektor toku stěny s konvekcí míří tam, kudy teplo
skutečně odchází.

Tok je **hustotou** ve `W/m^2`, nezávislou na velikosti prvku, ve kterém byl
vypočten, takže ho lze porovnávat mezi prvky i s ručním výpočtem a se
zjemňováním sítě konverguje.

Ukládá se také **součinitel přestupu tepla**. Je to součinitel, který řešič na
každém povrchovém prvku skutečně použil, což je jediný způsob, jak zjistit, co
vytvořily korelace *Forced convection* a *Natural convection* - a na povrchu s
přirozenou konvekcí se mění prvek od prvku, protože závisí na místní teplotě
stěny. Na stěně svázané s tekutinou je to vodivost prvního prvku tekutiny,
která závisí na síti - viz *Sdružený přenos tepla* v 1.6. Na každém prvku, který
nenese podmínku konvekce, je nulový.

| Výsledek | Použití na | Význam |
|---|---|---|
| Temperature `[K]` | uzel | vyřešené teplotní pole |
| Heat flux `[W/m^2]` | prvek | vektor konduktivního toku, na površích s konvekcí plus konvektivní část |
| Heat transfer coefficient `[W/(m^2*K)]` | prvek | použitý součinitel přestupu tepla, nulový tam, kde není konvekce |

Po každém záznamu vypíše log řešiče statistiku všech tří a hodnoty ve všech
monitorovacích bodech.

### 1.10 Ustálená, nebo přechodová?

| | Ustálená | Přechodová |
|---|---|---|
| Řeší | `K*T = f` | `M*T' + K*T = f` v čase |
| Odpovídá na | „jak horké to nakonec bude“ | „jak dlouho to potrvá, než se tam dostane“ |
| Používá hustotu a tepelnou kapacitu | ne, ale musí být přítomny | ano |
| Potřebuje předepsanou teplotu nebo povrch s konvekcí | ano | ne, stačí kapacita |
| Záznamy | jeden | jeden na zapsaný časový krok |

**Ustálenou analýzu použijte pro** konečné rozložení teploty nepřetržitě
pracující součásti, dimenzování chladiče, tepelný most ve stěně nebo teplotní
pole, které napájí analýzu tepelného napětí. Je to výchozí volba a pokrývá
většinu práce.

**Přechodovou analýzu použijte pro** zahřívání a chladnutí, pracovní cyklus,
tepelný šok, kalení nebo cokoli, kde vás zajímá časová konstanta součásti.

---

## 2. Grafické uživatelské rozhraní

Vše podstatné se nachází v panelu **Řešič** (záložky `Úloha`,
`Okrajové podmínky`, `Počáteční podmínky`, `Environmentální podmínky`,
`Materiál`, `Výsledky`), v nabídce **Úloha** a v panelu **Model**.

### 2.1 Výběr úlohy

`Úloha` -> `Pořadí kroků úloh` (`Ctrl+P`) otevře dialog pořadí kroků úlohy.
Přidejte krok a vyberte **Heat transfer** - „Steady-state and transient
Heat-transfer“ (ustálený a přechodový přenos tepla).

Přenos tepla se volně kombinuje se všemi ostatními fyzikami kromě
*Modal analysis*. Řetězce, které stojí za to znát:

- *Heat transfer* a poté *Stress analysis* - tepelné napětí;
- *Heat transfer* a *Radiative heat transfer* v jedné iterující skupině kroků -
  vedení plus záření v uzavřeném prostoru;
- *Electro-statics* a poté *Heat transfer* - odporový ohřev.

Chcete-li svázanou dvojici iterovat, přidejte skupinu kroků a nastavte její
počet iterací; skupina se zastaví dříve, jakmile každý krok v ní ohlásí
konvergenci.

### 2.2 Záložka Úloha

**Řešič v čase** - zobrazuje se u přenosu tepla, protože jde o časově závislý
typ úlohy. Pro ustálenou analýzu ho nechte vypnutý.

| Pole | Význam |
|---|---|
| Zapnutí | zapíná a vypíná přechodové krokování |
| Aproximace | theta časového krokování: zpětná, centrální nebo dopředná, viz 1.8 |
| Počáteční čas | první hodnota času v sekundách; při restartu řešiče se ignoruje |
| Koncový čas | `start time + time-step size * number of time-steps`, pouze pro čtení |
| Velikost časového kroku | `dt` v sekundách |
| Počet časových kroků | kolik kroků se má vypočítat |
| Frekvence výstupu | zapsat soubor výsledků každých N kroků; `0` zapíše pouze poslední krok |

Tepelná úloha nemá vlastní skupinu nastavení. Vše ostatní se přiřazuje pro
každou entitu v záložkách podmínek.

### 2.3 Záložka Materiál

Záložka `Materiál` uvádí materiál přiřazený vybrané entitě. Přenos tepla
vyžaduje **všechny čtyři** tyto vlastnosti:

| Vlastnost | Jednotky | Poznámka |
|---|---|---|
| Thermal conductivity | `W/(m*K)` | `k`, asi `50` pro konstrukční ocel, `15` pro nerezovou, `200` pro hliník |
| Heat capacity | `J/(kg*K)` | `c`, asi `460` pro ocel |
| Density | `kg/m^3` | `rho`, asi `7850` pro ocel |
| Emissivity | `-` | používá ji jen přenos tepla zářením, ale je povinná |

Entita, které některá z nich chybí, se neřeší a kontrola nastavení na to
upozorní před spuštěním výpočtu. Pokud neřešíte přenos tepla zářením, splní
požadavek emisivita `0`.

Materiál musí být také **pevnou látkou**. Objem, jehož materiál je plyn nebo
kapalina - nebo nemá nastavené skupenství a má dynamickou viskozitu - je
přenechán typu úlohy *Heat transfer in fluids*, i když má všechny čtyři
vlastnosti.

Každá vlastnost je tabulkou v závislosti na teplotě, takže teplotně závislá
vodivost se respektuje - vyhodnocuje se při teplotě prvku z předchozího
výpočtu, a proto se při silné závislosti vyplatí krok iterovat.

### 2.4 Záložka Okrajové podmínky

Záložka je rozdělena na seznam dostupných okrajových podmínek nahoře a editor
vybrané podmínky dole. Nejprve vyberte entitu ve stromu `Model`; správce pak
nabídne podmínky platné pro daný typ entity a vybranou úlohu. Zaškrtnutím ji
přiřadíte a poté upravte její složky ve spodním stromu.

Nabízejí se podmínky z části 1.4. Poznámky k jejich použití:

- **Temperature** je jediná podmínka, která hodnotu drží, místo aby ji
  vynucovala. Přiřaďte ji stěně, hraně nebo bodu, jejichž teplotu znáte.
- **Heat** se nabízí na každém typu entity a přijímá celkový výkon ve `W`;
  řešič ho rozloží na entitu za vás. Je to podmínka, po které sáhnout, když
  znáte příkon toho, co teplo vytváří.
- **Heat rate (unit volume)** na objemu a **Heat rate (unit area)** na povrchu
  předepisují stejný zdroj jako hustotu, pro zdroj, který je přirozeně
  dimenzován na jednotku objemu nebo plochy.
- Tři podmínky **konvekce** platí pouze pro povrchy. Přiřaďte
  *Simple convection*, pokud nemáte konkrétní důvod nechat součinitel
  korelovat řešičem; je předvídatelná. *Forced convection* na stěně sousedící
  s vysíťovanou tekutinou sváže pevnou látku s řešením tepla v tekutině a
  nečte žádnou ze svých složek; jinde z nich koreluje součinitel.
- Každá složka je tabulkou v závislosti na čase, takže předepsané teplotě nebo
  zdroji tepla lze pro přechodový výpočet zadat časový průběh. Takto se
  modeluje pracovní cyklus nebo postupně náběhové topné těleso.

### 2.5 Počáteční a environmentální podmínky

Záložka `Počáteční podmínky` nabízí *Temperature*, která nastavuje výchozí
pole přechodového výpočtu. Přiřaďte ji celému modelu a zadejte teplotu, ze
které součást vychází.

Záložka `Environmentální podmínky` nabízí také *Temperature*, jako hodnotu
pozadí entity. Obě se čtou pouze při prvním výpočtu a okrajová podmínka obě
přepíše.

### 2.6 Nastavení maticového řešiče

`Úloha` -> `Nastavit maticový řešič úloh` nastavuje iterační řešiče. Tepelný
řešič používá položku **CG**.

Pokud ustálené řešení nekonverguje, obvyklou příčinou je model bez předepsané
teploty a bez povrchu s konvekcí, nikoli nastavení řešiče. Než zvýšíte počet
iterací, zkontrolujte okrajové podmínky.

### 2.7 Monitorovací body

`Úloha` -> `Definovat monitorovací body` umístí sondy na zadané souřadnice a
vybere proměnnou, která se má zaznamenávat - u tepelné úlohy *Temperature*. V
přechodovém výpočtu zobrazí historii `Zprávy` -> `Monitorovací body`, což je
nejrychlejší způsob, jak z modelu odečíst křivku zahřívání nebo časovou
konstantu.

### 2.8 Výsledky a záznamy

Záložka `Výsledky` uvádí vypočtené proměnné a ovládá 3D zobrazení. Teplota je
uzlový skalár, tepelný tok prvkový vektor a součinitel přestupu tepla prvkový
skalár, který je mimo povrchy s konvekcí nulový.

Záložka `Záznamy` panelu `Model` uvádí záznamy výsledků:

- **ustálená analýza** - jediný záznam;
- **přechodová analýza** - jeden záznam na zapsaný časový krok. Animace
  záznamů přehraje zahřívání.

`Zprávy` -> `Log soubor řešiče` ukazuje celý výstup řešiče včetně statistiky
teploty, tepelného toku a součinitele přestupu tepla po každém záznamu.

---

## 3. Návod - ustálený přenos tepla

**Cíl.** Vypočítat rozložení teploty v tyči, jejíž jeden konec je držen
horký a která je chlazena okolním vzduchem, a odečíst teplotu na studeném
konci.

Návod předpokládá vysíťovaný objemový model - dobře poslouží dlouhý kvádr.
Použijte `Soubor` -> `Nový model`, nakreslete kvádr pomocí `Geometrie` ->
`Vytvořit prvek` nebo importujte geometrii a tetraedrizujte ji; to pokrývá
návod *Nakreslit krychli*. Označte horkou koncovou stěnu a zbývající stěny jako
samostatné povrchové entity, aby jim bylo možné přiřadit podmínky.

### Krok 1 - vyberte úlohu

1. `Úloha` -> `Pořadí kroků úloh` (`Ctrl+P`).
2. Přidejte krok a vyberte **Heat transfer**.
3. Potvrďte tlačítkem `OK`.

### Krok 2 - přiřaďte materiál

1. Ve stromu `Model` vyberte objemovou entitu.
2. Otevřete záložku `Materiál` a přiřaďte materiál se všemi čtyřmi
   vlastnostmi, pro konstrukční ocel:
   - **Thermal conductivity** = `50` `W/(m*K)`
   - **Heat capacity** = `460` `J/(kg*K)`
   - **Density** = `7850` `kg/m^3`
   - **Emissivity** = `0`

Tepelnou kapacitu a hustotu ustálené řešení nepoužívá, ale musí být přítomny,
aby se entita vůbec řešila.

### Krok 3 - ponechte analýzu ustálenou

Otevřete záložku `Úloha` a ve skupině *Řešič v čase* nechte **Zapnutí**
nezaškrtnuté. Řešič pak jednou sestaví a vyřeší `K*T = f`.

### Krok 4 - předepište horký konec

1. Vyberte povrchovou entitu na horkém konci.
2. Otevřete záložku `Okrajové podmínky` a zaškrtněte **Temperature**.
3. Nastavte hodnotu na `353.15` K, tedy 80 stupňů Celsia.

Pamatujte, že všechny teploty v Range FEA jsou absolutní, v kelvinech.

### Krok 5 - ochlaďte zbytek

1. Vyberte povrchovou entitu pokrývající zbývající stěny.
2. Otevřete záložku `Okrajové podmínky` a zaškrtněte **Simple convection**.
3. Nastavte **Convection coefficient** = `10` `W/(m^2*K)` a
   **Fluid temperature** = `293.15` K.

Součinitel kolem `10` odpovídá povrchu v klidném vzduchu; nucené proudění
vzduchu je blíže `50` a voda je ve stovkách nebo tisících.

Tento krok činí model řešitelným. Bez něj má tyč přívod tepla, ale žádný odvod,
a ustálený stav neexistuje.

### Krok 6 - řešte

1. `Řešení` -> `Spustit řešič` (`Ctrl+R`).
2. Kontrola nastavení ohlásí chybějící materiály nebo okrajové podmínky ještě
   před spuštěním výpočtu.
3. Průběh sledujte v `Zprávy` -> `Log soubor řešiče`.

### Krok 7 - prohlédněte si výsledky

- Záložka `Výsledky`: zobrazte **Temperature**. Pole by mělo plynule klesat od
  předepsaného horkého konce k teplotě tekutiny.
- Nejchladnější bod by měl zůstat nad teplotou tekutiny. Teplota kdekoli pod
  `Tf` nebo nad předepsanými `353.15` K znamená, že něco dodává energii
  nesprávným směrem - zkontrolujte znaménko všech zdrojů tepla.
- **Heat flux** ukazuje, kudy energie putuje, ve `W/m^2`. Vynásobte hodnotu na
  stěnách s konvekcí jejich plochou a porovnejte s výkonem vstupujícím do tyče.
- **Heat transfer coefficient** by měl na stěnách s konvekcí ukazovat `10` a
  všude jinde `0`. Je to nejrychlejší způsob, jak ověřit, že podmínka konvekce
  dorazila do zamýšlených prvků, a jediný způsob, jak zjistit, co spočítala
  korelovaná podmínka.
- Monitorovací bod na *Temperature* na studeném konci dává odpověď jako číslo,
  a ne jako barvu.

### Krok 8 - volitelně přidejte zdroj tepla

Chcete-li modelovat topné těleso zabudované v tyči, vyberte objemovou entitu,
otevřete záložku `Okrajové podmínky` a zaškrtněte **Heat**. Chcete-li do tyče
dodat celkem `P` wattů, zadejte `P` - řešič ho rozloží na objem entity.

Je-li zdroj dimenzován na jednotku objemu, použijte místo toho
**Heat rate (unit volume)**; k dodání stejného celkového výkonu zadejte
`P / V` `W/m^3` - viz 1.4.

Ustálená teplota pak musí stoupat, dokud povrch s konvekcí nedokáže odvést
přesně tento výkon - což je kontrola energetické bilance z části 5.

### Krok 9 - volitelně tepelné napětí

Chcete-li výsledek přenést do strukturální analýzy:

1. Přidejte do pořadí kroků úlohy krok **Stress analysis** *za* tepelný krok a
   dejte entitě strukturální vlastnosti, které potřebuje.
2. Přiřaďte strukturální entitě environmentální podmínku **Temperature** s
   referenční teplotou bez napětí, například `293.15` K.

Řešič napětí převezme vypočtené teploty prvků a automaticky přidá člen tepelné
roztažnosti.

### Řešení problémů

| Příznak | Příčina |
|---|---|
| Řešení nekonverguje | žádná podmínka Temperature a žádný povrch s konvekcí, takže ustálený stav není definován |
| *Entity has material assigned which is missing required properties* | chybí jedna ze čtyř požadovaných vlastností - často emisivita |
| Celý model je na jedné teplotě | nevstupuje žádné teplo, nebo nic nevytváří gradient od předepsané hodnoty |
| Teploty jsou absurdně vysoké | do podmínky *Heat*, která přijímá celkovou hodnotu ve wattech, byla zadána hustota ve `W/m^3`, nebo je součinitel konvekce příliš malý |
| Stěna mezi pevnou látkou a tekutinou se neochlazuje | krok tepla v tekutinách běží po tepelném kroku, nebo skupina provádí jedinou iteraci - dejte krok tepla v tekutinách jako první a nechte skupinu iterovat |
| Tepelné řešení tekutinu ignoruje | záměrné - materiál tekutiny řeší pouze *Heat transfer in fluids* |
| Teploty vycházejí všude blízko `293.15` K | entita nebyla vyřešena - v logu hledejte chybějící materiálovou vlastnost |

---

## 4. Návod - přechodový přenos tepla

**Cíl.** Spustit tutéž tyč při pokojové teplotě, zapnout horký konec a zjistit,
jak dlouho trvá zahřátí.

### Krok 1 - vyberte úlohu

Stejný krok **Heat transfer** jako v návodu 3. Je-li model stále nastaven, stačí
přidat nastavení řešiče v čase a počáteční podmínku.

### Krok 2 - přiřaďte materiál

Stejné čtyři vlastnosti. Zde záleží na **tepelné kapacitě** a **hustotě**:
spolu s vodivostí určují teplotní vodivost `k/(rho*c)`, která řídí, jak rychle
se čelo šíří.

### Krok 3 - nastavte počáteční teplotu

1. Vyberte objemovou entitu.
2. Otevřete záložku `Počáteční podmínky` a zaškrtněte **Temperature**.
3. Nastavte ji na `293.15` K.

Bez ní model stále začne z výchozích `293.15` K, ale výslovné zadání činí model
samodokumentujícím a umožňuje začít z jakéhokoli jiného stavu.

### Krok 4 - nastavte řešič v čase

Otevřete záložku `Úloha` a ve skupině *Řešič v čase* nastavte:

| Pole | Hodnota |
|---|---|
| Zapnutí | zaškrtnuto |
| Aproximace | `Backward difference (stable)` |
| Počáteční čas | `0` |
| Velikost časového kroku | `1` |
| Počet časových kroků | `300` |
| Frekvence výstupu | `10` |

*Backward difference* je implicitní krokování - viz tabulka v 1.8 - a zde je
třeba použít právě ji. Zůstává stabilní při časovém kroku zvoleném podle fyziky,
nikoli podle sítě.

Časový krok zvolte podle očekávané časové konstanty a mířte na zhruba padesát
kroků přes zajímavou část křivky. Část 5 uvádí vzorec pro první odhad. Zápis
každého desátého kroku udrží soubor výsledků malý, aniž by se ztratil tvar
křivky.

### Krok 5 - ponechte okrajové podmínky

Předepsaná **Temperature** na horkém konci a **Simple convection** na
zbývajících stěnách zůstávají beze změny. Každá složka je tabulkou v závislosti
na čase, takže horkému konci lze místo toho zadat náběh: otevřete editor
složek a přidejte časové body, aby teplota během prvních několika sekund
postupně stoupala, místo aby skočila okamžitě.

### Krok 6 - přidejte monitorovací bod

`Úloha` -> `Definovat monitorovací body`, umístěte bod na studený konec a
nastavte jeho proměnnou na **Temperature**. Křivku zahřívání je mnohem snazší
číst jako historii než jako sled barevných obrázků.

### Krok 7 - řešte

`Řešení` -> `Spustit řešič` (`Ctrl+R`). Log vypíše jeden blok na časový krok:

```
time step:         1 of 300       | time =  0.000000e+00 [sec] | dt =  1.000000e+00 [sec]
```

následovaný iteracemi maticového řešiče a statistikou teploty v daném kroku.

### Krok 8 - prohlédněte si výsledky

- Panel `Model`, záložka `Záznamy`: jeden záznam na zapsaný krok. Procházejte je
  nebo je animujte a sledujte, jak tepelné čelo postupuje do tyče.
- Záložka `Výsledky`: zobrazte **Temperature**. Pevně nastavte rozsah
  zobrazení pro všechny záznamy, aby barvy v každém snímku znamenaly totéž,
  jinak se animace sama přeškáluje a postup není vidět.
- `Zprávy` -> `Monitorovací body` vykreslí křivku zahřívání na studeném konci.
  Odečtěte z ní časovou konstantu: dobu, za kterou se dosáhne asi 63 procent
  celkového nárůstu.
- Poslední záznam by se měl blížit ustálenému výsledku z návodu 3. Pokud ne,
  byl výpočet příliš krátký.

### Řešení problémů

| Příznak | Příčina |
|---|---|
| Teplota osciluje a krok od kroku roste | explicitní krokování s příliš velkým časovým krokem - přepněte aproximaci na *Backward difference (stable)* nebo zmenšete `dt` |
| Výsledek se během celého výpočtu téměř nemění | časový krok je hluboko pod časovou konstantou, nebo je výpočet příliš krátký |
| První krok skočí rovnou na konečnou odpověď | časový krok je vysoko nad časovou konstantou, takže krokování je fakticky ustálené |
| Zapsal se jen jeden záznam | frekvence výstupu je `0`, což zapíše pouze poslední krok |
| Přechodový výpočet nikdy nedosáhne ustáleného stavu | to bývá správně - porovnejte s ustáleným výpočtem, než začnete podezírat řešič |

---

## 5. Kontrola modelu

Řešič nemá vestavěnou sadu ověřovacích testů, proto se vyplatí nový model před
použitím ověřit vůči něčemu, co lze spočítat ručně. Čtyři levné kontroly, seřazené
podle užitečnosti:

**Rovinná stěna.** Pro desku tloušťky `L` a vodivosti `k` drženou na `T1` a `T2`
na jejích dvou stěnách je teplota napříč deskou lineární a tok je

```
q = k * ( T1 - T2 ) / L          [W/m^2]
```

Vymodelujte desku objemovými prvky, předepište obě stěny, potvrďte, že střední
rovina leží přesně v polovině a že vypočtený **Heat flux** odpovídá `q`. Tím
odhalíte chybné jednotky, chybnou vodivost i síť, která není propojena tak, jak
si myslíte. Jednou zjemněte síť a potvrďte, že obě čísla zůstanou stejná.

**Stěna s konvekcí.** Pro stejnou desku drženou na `T1` na jedné stěně a
chlazenou tekutinou o teplotě `Tf` se součinitelem `h` na druhé se chlazený
povrch ustálí na

```
Ts = ( (k/L)*T1 + h*Tf ) / ( k/L + h )
```

Porovnejte s vypočtenou teplotou povrchu. Tím se ověří podmínka konvekce, což
je část tepelného modelu, ve které se nejsnáze udělá chyba.

**Soustředěná časová konstanta.** Pro přechodovou kontrolu vezměte těleso o
objemu `V` a ploše povrchu `A` bez předepsané teploty, začínající na `T0` a
chlazené pouze konvekcí. Dokud Biotovo číslo `h*V/(A*k)` zůstává pod přibližně
`0.1`, je těleso téměř izotermické a

```
tau  = rho * c * V / ( h * A )
T(t) = Tf + ( T0 - Tf ) * exp( -t / tau )
```

Odečtěte `tau` z historie monitorovacího bodu a porovnejte. Tím se současně
ověří matice kapacity, podmínka konvekce i váš časový krok - a je to nejrychlejší
způsob, jak zvolit časový krok pro skutečný model, protože `tau/50` je dobrý
první odhad.

**Energetická bilance.** Pro ustálený model se známým příkonem `P` musí povrch
s konvekcí při konečné teplotě odvést přesně `P`:

```
P = h * A * ( Ts_mean - Tf )
```

Střední teplotu povrchu odečtěte ze statistiky v logu řešiče a zkontrolujte
bilanci. Je to jediná globální kontrola, která odhalí hodnotu Heat zadanou jako
celková hodnota místo hustoty.

U kterékoli z těchto kontrol zjemněte síť alespoň jednou a potvrďte, že odpověď
konverguje, a ne že se posouvá.

---

## 6. Omezení

### Omezení modelování

- **Pouze lineární vedení.** Vodivost, kapacita a hustota prvku jsou během
  řešení konstantní, čtené z materiálové tabulky při teplotě předchozího
  výpočtu. Silně teplotně závislá vlastnost potřebuje ke zkonvergování
  iterující skupinu kroků.
- **Žádná fázová přeměna.** Neexistuje skupenské teplo ani entalpická
  formulace, takže tání, tuhnutí, var a kondenzace se nemodelují. Materiálová
  tabulka nedokáže reprezentovat prodlevu skupenského tepla.
- **Žádná advekce.** Energie se pohybuje vedením uvnitř tělesa a konvekcí přes
  jeho hranici. Pohybující se médium s sebou žádnou energii nenese; k tomu
  použijte *Heat transfer in fluids*.
- **Konvekce je model hranice.** Korelované varianty používají každá jednu
  pevnou korelaci - vnější rovinnou desku pro nucené proudění, vodorovné desky
  pro přirozenou konvekci - s hydraulickým průměrem jako charakteristickým
  rozměrem. Orientace povrchu, režim proudění ani to, zda je proudění vnitřní
  nebo vnější, se nerozpoznávají. Pokud na součiniteli záleží, spočítejte ho
  sami a zadejte ho přes *Simple convection*.
- **Součinitel přirozené konvekce je zpožděný.** Vyhodnocuje se při teplotě
  stěny z předchozího řešení, takže ustálený výpočet s jediným průchodem
  fakticky používá počáteční rozdíl teplot.
- **Záření pouze přes samostatný typ úlohy.** Samotný tepelný řešič nemá žádný
  člen povrchové emisivity; záření v uzavřeném prostoru vyžaduje krok
  *Radiative heat transfer*, jeho matici view-faktorů a iteraci mezi oběma.
- **Povrchy jsou napříč tloušťkou izotermické.** Povrchová entita vede pouze ve
  své rovině. Stěnu, u které záleží na gradientu napříč tloušťkou, modelujte
  objemovými prvky.
- **Bodové prvky nevedou.** Bod je soustředěná kapacita a zdroj tepla, nic víc.
- **Ustálený model potřebuje odvod.** Bez předepsané teploty i bez povrchu s
  konvekcí je matice singulární a řešení nezkonverguje.

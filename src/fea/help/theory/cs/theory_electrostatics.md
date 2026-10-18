# Elektrostatika - Teoretická příručka a uživatelská příručka

Tento dokument popisuje řešič elektrostatiky (`RSolverElectrostatics`) v
Range FEA, který obsluhuje typ úlohy **Elektrostatika** (*Electro-statics*):
rovnice, které řeší, význam každého vstupu, který přijímá, části grafického
uživatelského rozhraní, které ho ovládají, a dva podrobné návody.

**Obsah**

1. [Teoretický základ](#1-teoretický-základ)
2. [Grafické uživatelské rozhraní](#2-grafické-uživatelské-rozhraní)
3. [Návod - potenciál a proud ve vodiči](#3-návod---potenciál-a-proud-ve-vodiči)
4. [Návod - odporový ohřev](#4-návod---odporový-ohřev)
5. [Kontrola modelu](#5-kontrola-modelu)
6. [Omezení](#6-omezení)

---

## 1. Teoretický základ

### 1.1 Primární neznámá - pole elektrického potenciálu

Primární neznámou je **elektrický potenciál** `V` v každém uzlu, ve voltech.
Každý uzel nese jediný stupeň volnosti, takže síť s `N` uzly dává soustavu `N`
rovnic. Elektrické pole, proudová hustota a vše, co se z nich odvozuje, se po
vyřešení získá z pole potenciálu.

Potenciál je skalár, díky čemuž je elektrostatický model stejně levný jako
tepelný model na stejné síti: jedna neznámá na uzel místo tří a matice s
devítinou prvků oproti strukturálnímu modelu.

Výchozí potenciál uzlu, kterého se nedotkne žádná podmínka ani předchozí
výsledek, je `0 V`.

Pole potenciálu je definováno pouze **až na aditivní konstantu**. Nic v
rovnicích neurčuje nulovou úroveň, takže každý model potřebuje alespoň jednu
okrajovou podmínku *Electric potential* - viz 1.7. Jde o elektrostatickou
obdobu nepodepřené konstrukce nebo tepelného modelu bez předepsané teploty.

### 1.2 Řídicí rovnice

Řešič implementuje **rovnici elektrostatického pole** - Poissonovu rovnici pro
potenciál lineárního, izotropního dielektrika:

```
div( e0 * er * grad(V) ) = -rho
```

a bez jakékoli hustoty náboje se redukuje na Laplaceovu rovnici

```
div( e0 * er * grad(V) ) = 0
```

Elektrické pole je záporný gradient potenciálu a proudová hustota plyne z
Ohmova zákona:

```
E = -grad(V)
J = sigma * E
```

| Symbol | Význam | Jednotky | Zdroj |
|---|---|---|---|
| `V` | elektrický potenciál | `V` | řešená veličina |
| `rho` | hustota náboje | `C/m^3`, viz 1.5 | podmínka Charge density |
| `e0` | permitivita vakua | `F/m` | vestavěná, `8.854187817e-12` |
| `er` | relativní permitivita | `-` | materiálová vlastnost |
| `sigma` | elektrická vodivost | `S/m` | materiálová vlastnost |
| `E` | elektrické pole | `V/m` | odvozené, viz 1.8 |
| `J` | proudová hustota | `A/m^2` | odvozená, viz 1.8 |

Permitivita vakua je konstantou řešiče, nikoli materiálovou vlastností ani
nastavením - s materiálem se zadává jen **relativní** permitivita.

**Pole je řízeno permitivitou, nikoli vodivostí.** To je vůbec nejdůležitější
věc, kterou je o tomto řešiči třeba vědět. Matice tuhosti se sestavuje pouze z
`e0*er`; vodivost do soustavy rovnic nikdy nevstupuje. Používá se až poté, k
převodu vypočteného pole na proudovou hustotu, rezistivitu a Jouleovo teplo.
Důsledek rozvádí 1.9: v tělese z jednoho materiálu to nehraje roli, ale v
tělese ze **dvou** materiálů se potenciál dělí v poměru permitivit, nikoli v
poměru odporů.

**Požadované materiálové vlastnosti.** Elektrostatická úloha vyžaduje **tři**:

| Vlastnost | Jednotky | Použití |
|---|---|---|
| Relative permittivity | `-` | matice tuhosti |
| Electrical conductivity | `S/m` | proudová hustota, rezistivita, Jouleovo teplo |
| Density | `kg/m^3` | nic - je povinná, ale nikdy se nečte |

Entita, jejímuž materiálu některá ze tří vlastností chybí, se **neřeší**, i
když tento řešič hustotu vůbec nečte. Kontrola nastavení na to upozorní před
spuštěním výpočtu. Jedinou výjimkou je entita, která nese elektrostatickou
okrajovou podmínku: ta je do soustavy zahrnuta bez ohledu na materiál a její
permitivita a vodivost jsou pak prostě nulové.

Nulová permitivita není neškodnou výchozí hodnotou - prvek s `er = 0` nemá
žádnou tuhost, takže do soustavy nic nepřispívá a jeho uzly nechává plovoucí.
Má-li se entita řešit, dejte jí skutečnou permitivitu: `1` pro vakuum nebo,
dostatečně přesně, pro vzduch.

Každá vlastnost je tabulkou v závislosti na teplotě, ale samotný krok
elektrostatiky nemá teplotní pole, při kterém by ji vyhodnotil, takže se každá
tabulka vyhodnocuje při `293.15 K`. Tabulky se respektují jen tehdy, pokud krok
*Heat transfer* dříve ve stejném pořadí kroků vypočítal teploty prvků, které se
sdílejí s následujícími kroky.

Relativní permitivita je **bezrozměrná** - v uživatelském rozhraní se zobrazuje
jako `N/A`, stejně jako emisivita a Poissonovo číslo - a hodnota se používá
jako prostý násobitel `e0`. Zadejte `1` pro vakuum, `1.00059` pro vzduch, `2.1`
pro PTFE, asi `4` pro sklo.

### 1.3 Diskretizace metodou konečných prvků

Slabá formulace rovnice pole dává pro každý prvek

```
K = INT( B^T * e0 * er * B )        stiffness
f = INT( N^T * rho )                charge source
```

(`stiffness` - tuhost, `charge source` - zdroj náboje), kde `B` je sestaveno z
derivací bázových funkcí a `N` jsou samotné bázové funkce. Každá matice prvku
má rozměr `n x n` pro prvek s `n` uzly - jeden řádek na uzel, ne tři. Neexistuje
matice hmotnosti ani kapacity: úloha je statická a 1.10 vysvětluje, co to
znamená pro pořadí kroků.

**Míry prvků.** Stejně jako jinde v Range FEA dostávají entity nižší dimenze
geometrickou míru, aby mohly reprezentovat skutečná tělesa:

| Entita | Míra | Kde se nastavuje |
|---|---|---|
| Objem | 1 | - |
| Povrch | tloušťka povrchu | geometrické vlastnosti entity |
| Čára | plocha průřezu čáry | geometrické vlastnosti entity |
| Bod | objem bodu | geometrické vlastnosti entity |

**Povrch** vede pole ve své vlastní rovině průřezem rovným své tloušťce.
Neexistuje gradient potenciálu napříč tloušťkou: povrchový prvek je silný jeden
uzel, takže je napříč tloušťkou z konstrukce ekvipotenciální. Povrch s **nulovou
tloušťkou** proto nemá vůbec žádnou tuhost a nic nespojuje - na rozdíl od
tepelného řešiče, kde povrch s nulovou tloušťkou stále přenáší konvekci, je zde
povrch s nulovou tloušťkou prostě nečinný. Kdykoli má povrchová entita vést,
dejte jí tloušťku, a pamatujte, že stěna s nulovou tloušťkou také nedostane
**žádné odvozené výsledky** (1.8).

**Čára** vede pole ve svém vlastním směru průřezem rovným své ploše průřezu.
**Bod** nemá směr, kterým by pole vedl, takže nepřispívá **vůbec žádnou
tuhostí**: nese pouze zdroj náboje. Bodový prvek, jehož uzel nesdílí žádný jiný
počítaný prvek, proto vytvoří v matici prázdný řádek, se kterým si metoda
sdružených gradientů neporadí - bod je místem pro vložení náboje nebo
předepsání potenciálu, nikoli vodičem.

### 1.4 Zdroje a okrajové podmínky

| Okrajová podmínka | Typ | Použití na | Složky |
|---|---|---|---|
| Electric potential | explicitní (Dirichletova) | bod, čára, povrch, objem | Electric potential `[V]` |
| Charge density | přirozená, zdroj | bod, čára, povrch, objem | Charge density, viz 1.5 |

Tyto dvě podmínky tvoří celý slovník typu úlohy. Neexistuje předepsaný tok,
povrchový náboj, podmínka vzdáleného pole ani otevřené hranice, ani podmínka
symetrie - viz část 6.

**Electric potential** (elektrický potenciál) je jedinou explicitní podmínkou
elektrostatické úlohy. Každý uzel entity, které je přiřazena, se odstraní ze
soustavy rovnic a drží se na zadané hodnotě; jeho příspěvek se přenese na pravou
stranu sousedních rovnic. Každý model potřebuje alespoň jednu z nich. Nově
přiřazená podmínka začíná na `1 V`.

Tam, kde se entity překrývají, **vyhrává entita nižší dimenze**: potenciál
předepsaný na povrchu přepíše ve sdílených uzlech potenciál předepsaný na
objemu za ním, čára přepíše povrch a bod přepíše vše. Potenciály se
nekombinují - na uzel připadá jen jeden stupeň volnosti, takže prostě převáží
poslední načtená podmínka.

**Charge density** (hustota náboje) je zdrojovým členem Poissonovy rovnice. Na
rozdíl od podmínky *Heat* tepelného řešiče se její hodnota **nerozkládá** na
míru entity: zadané číslo se použije na každý prvek entity tak, jak je, jako
hustota. Celkový náboj umístěný v objemové entitě je proto hodnota vynásobená
objemem této entity a při zvětšení entity roste - nezůstává stejný jako hodnota
*Heat* ve wattech.

| Entita | Co hodnota znamená |
|---|---|
| Objem | náboj na jednotku objemu |
| Povrch | náboj na jednotku objemu průřezu váženého tloušťkou |
| Čára | náboj na jednotku objemu průřezu |
| Bod | uzlový zdroj náboje |

Obě složky přijímají tabulku v závislosti na čase. Protože samotná úloha je
statická, má časový průběh vliv jen tehdy, sdílí-li pořadí kroků jiný, časově
závislý krok - a jak vysvětluje 1.10, je taková kombinace sama omezena.

### 1.5 Hodnota hustoty náboje

Složka *Charge density* je **objemová hustota náboje** v `C/m^3`, jak uvádí její
popisek v uživatelském rozhraní. Jde o hustotu, nikoli o celkovou hodnotu, a
proto nezávisí na síti - a jak uvádí 1.4, právě tím se liší od podmínky *Heat*
tepelného řešiče, která přijímá celkovou hodnotu ve wattech a rozkládá ji za
vás.

Znaménko se řídí obvyklou konvencí. Integrace `div(e0*er*grad(V)) = -rho` per
partes ponechá náboj na pravé straně s kladným znaménkem,

```
every entity type      f = +INT( N^T * rho )
```

(`every entity type` - každý typ entity), takže **kladná** hustota náboje
zvyšuje potenciál kolem sebe a záporná ho snižuje, na objemových, povrchových,
čárových i bodových entitách stejně.

Pamatujte, že samotný zdroj náboje z modelu řešitelný model neudělá. Potenciál
je stále definován jen až na konstantu, dokud ho něco neurčí, takže model
buzený nábojem potřebuje také alespoň jednu podmínku *Electric potential* - na
vnější hranici vysíťované oblasti, pokud nikde jinde. Viz 1.7.

### 1.6 Environmentální, počáteční a vazební podmínky

#### Environmentální podmínky

| Environmentální podmínka | Složky | Účinek |
|---|---|---|
| Temperature | Temperature `[K]` | nabízí se, ale nečte se - viz níže |

Environmentální podmínka *Temperature* se pro elektrostatickou úlohu nabízí,
protože materiálové vlastnosti jsou tabulkami v závislosti na teplotě. Řešič
elektrostatiky ji **nečte**: teploty prvků přebírá pouze ze sdílených dat, která
zanechal krok *Heat transfer*, a jinak vyhodnocuje každou tabulku při
`293.15 K`. Její přiřazení v modelu pouze s elektrostatikou nic nemění.

#### Počáteční podmínky

| Počáteční podmínka | Účinek |
|---|---|
| Electric potential | počáteční pole potenciálu, použije se při prvním výpočtu |

Počáteční potenciál se čte **pouze při prvním výpočtu**. Protože jde o přímé
řešení, nikoli o krokování, zkonvergovanou odpověď nemění - nastavuje pouze
výchozí bod iterace metody sdružených gradientů a hodnotu každého uzlu, který
řešič vynechá ze soustavy.

Environmentální podmínky, počáteční podmínky a okrajové podmínky se čtou v
tomto pořadí a každá přepíše předchozí, takže **okrajová podmínka má přednost
před počáteční podmínkou, která má přednost před environmentální podmínkou**.

#### Vazba s jinými řešiči

Řešič elektrostatiky vytváří dva výsledky, které spotřebovávají jiné řešiče.

| Směr | Proměnná | Účinek |
|---|---|---|
| Elektrostatika -> teplo | Joule heat | přičte se ke zdrojovému členu každého prvku |
| Elektrostatika -> teplo v tekutinách | Joule heat | přičte se ke zdrojovému členu každého prvku |
| Elektrostatika -> magnetostatika | Current density | zdroj magnetického pole |
| Teplo -> elektrostatika | Temperature | vybírá řádek každé materiálové tabulky závislé na teplotě |

**Jouleovo teplo** převezme krok *Heat transfer* umístěný v pořadí kroků úlohy
**za** krokem elektrostatiky, kde se přičte ke zdrojovému členu prvku přesně
tak, jako by to udělala podmínka *Heat rate (unit volume)*. Odporový ohřev proto
nevyžaduje žádné nastavení kromě seřazení obou kroků - provádí jím návod 4.

**Magnetostatika** je samostatný typ úlohy, který *vyžaduje* krok
elektrostatiky: čte prvkovou proudovou hustotu, kterou tento řešič ukládá - na
objemových, povrchových i čárových entitách - a vyhodnocuje magnetické pole
tohoto proudu Biotovým-Savartovým zákonem. Krok elektrostatiky přidejte jako
první.

Protože se řešič hlásí jako zkonvergovaný bezpodmínečně, skupina kroků
obsahující pouze krok elektrostatiky se vždy zastaví po jedné iteraci. Iterace
má smysl jen tehdy, když je zpožděno něco, na čem řešení závisí - například
silně na teplotě závislá permitivita napájená tepelným krokem.

### 1.7 Řešení soustavy

Řešič sestaví a vyřeší

```
K * V = f
```

`K` je symetrická pozitivně definitní, jakmile je předepsán alespoň jeden
potenciál, takže se řeší metodou **sdružených gradientů** s Jacobiho
předpodmíněním. Sestavení je paralelizováno přes prvky s vyrovnávacími
pamětmi pro každé vlákno, které se na konci jednou sloučí, takže výsledek
nezávisí na počtu vláken.

Model **bez okrajové podmínky Electric potential** je singulární: potenciál je
definován jen až na konstantu, matice má netriviální jádro a řešení
nezkonverguje. Každý model potřebuje alespoň jeden předepsaný potenciál a
model, který má vést proud, potřebuje dva s různými hodnotami.

Stojí za to znát tři další způsoby, jak dospět k singulární nebo prázdné
soustavě, protože všechny tři jsou tiché:

- entita, jejímuž materiálu chybí jedna ze tří požadovaných vlastností, se
  neřeší a její uzly ze soustavy vypadnou;
- entita s `er = 0` nemá žádnou tuhost, takže její uzly zůstanou v soustavě, ale
  nejsou s ničím spojeny;
- povrch s nulovou tloušťkou nebo čára s nulovou plochou průřezu také nemají
  žádnou tuhost.

### 1.8 Odvozené výsledky

Po vyřešení se prvkové výsledky získají z uzlového pole potenciálu. Elektrické
pole je záporný gradient potenciálu zprůměrovaný přes integrační body a u
čárových a povrchových prvků otočený do globálních souřadnic:

```
E = -grad(V)
J = sigma * E
w = e0 * er * |E|^2 / 2          electric energy
r = |E| / |J|                    electrical resistivity
q = sigma * |E|^2                Joule heat
```

(`electric energy` - elektrická energie, `electrical resistivity` - elektrická
rezistivita, `Joule heat` - Jouleovo teplo.)

| Výsledek | Použití na | Význam |
|---|---|---|
| Electric potential `[V]` | uzel | vyřešené pole potenciálu |
| Electric field `[V/m]` | prvek | vektor pole |
| Current density `[A/m^2]` | prvek | `sigma*E`, nulová tam, kde je vodivost nulová |
| Electric energy `[J/m^3]` | prvek | hustota energie pole |
| Electrical resistivity `[V*m/A]` | prvek | `1/sigma` všude, kde teče proud, jinde `0` |
| Joule heat `[W/m^3]` | prvek | odporová disipace, předávaná tepelnému řešiči |

Všechny jsou **hustotami**: derivace bázových funkcí se průměrují přes
integrační body prvku a neváží se jeho Jacobiánem a tloušťka povrchu a plocha
průřezu čáry zůstávají v tuhosti, kam patří, a nevstupují do odvozeného pole.
Pole, proudová hustota a elektrická energie proto nezávisí na velikosti prvku,
ve kterém byly vypočteny, takže je lze porovnávat mezi prvky i s ručním
výpočtem a se zjemňováním sítě konvergují.

**Elektrická rezistivita** je výsledek, po kterém sáhnout při kontrole modelu,
protože se dva činitele, ze kterých se skládá, vykrátí: `|E|/|sigma*E|` je
přesně `1/sigma`, ať pole dělá cokoli. Je to nejrychlejší potvrzení, že
vodivost určená pro entitu skutečně dorazila do jejích prvků. Na každém prvku,
kde proudová hustota klesne pod numerickou toleranci, se ukládá jako `0`, což
zahrnuje každý prvek s nulovou vodivostí.

**Jouleovo teplo** je hustota disipace `sigma*|E|^2` ve `W/m^3`. Přesně to
očekává krok *Heat transfer*, když hodnotu přičte ke svému zdrojovému členu a
integruje ji přes prvek, takže výkon přecházející do svázaného tepelného
výpočtu je výkon, který model skutečně disipuje. Stejně jako ostatní je to
hustota, takže nezávisí na velikosti prvku a při zjemňování konverguje.

Další dvě místa, kde odvozené výsledky prostě zůstanou nulové: **bodové prvky**
se nezpracovávají vůbec a **povrch s nulovou tloušťkou** nebo **čára s nulovou
plochou průřezu** se přeskakují, takže obě si ponechají nulové pole bez ohledu
na to, jak velký je gradient potenciálu přes ně.

Po zapsání záznamu vypíše log řešiče statistiku všech šesti proměnných a
hodnoty ve všech monitorovacích bodech.

### 1.9 Co řešič je a co není

Název je přesný: jde o **elektro-statický** řešič. Počítá pole rozložení náboje
a potenciálu v dielektriku. Proudová hustota, kterou hlásí, je to, co pro toto
pole dává Ohmův zákon - odvozená veličina vypočtená dodatečně, která pole samo
nikdy neovlivní.

Úloha **ustáleného proudového pole** se naproti tomu řídí rovnicí

```
div( sigma * grad(V) ) = 0
```

s vodivostí na místě, kam tento řešič dává permitivitu. Obě dávají v
homogenním tělese stejné pole potenciálu, protože se obě redukují na Laplaceovu
rovnici a konstanta se vykrátí. Rozcházejí se, jakmile je těleso vyrobeno z
více než jednoho materiálu:

| | Tento řešič | Řešič proudového pole |
|---|---|---|
| Potenciál se dělí v poměru | `1/er` každé oblasti | `1/sigma` každé oblasti |
| Vhodné pro | kondenzátor, dielektrikum, izolant | síť rezistorů, přípojnici, vodivou cestu |

Vodič z jednoho materiálu s předepsanými potenciály na koncích je tedy
modelován správně: potenciál je lineární a vydělením délkou se získá pole. Dva
vodiče s různou vodivostí v sérii nikoli - rozdělení sleduje permitivity, což
asi nebylo zamýšleno. Takový případ modelujte jako jediný materiál, nebo
rozdělení spočítejte ručně.

### 1.10 Statická analýza a řešič v čase

Elektrostatika je deklarována jako typ úlohy **bez** řešiče v čase. To má dva
viditelné důsledky.

V modelu pouze s elektrostatikou se skupina *Řešič v čase* v záložce `Úloha`
vůbec nezobrazí, výpočet vytvoří jediný záznam a není co nastavovat.

Překvapivější je, že přidání kroku elektrostatiky do pořadí kroků **vypne
řešič v čase pro celé pořadí kroků**. Řešič v čase se nabízí jen tehdy, když ho
podporuje *každý* typ úlohy v pořadí kroků, takže přechodová tepelná analýza,
ke které se přidá krok elektrostatiky, se stane ustálenou analýzou, bez
jakéhokoli upozornění kromě zmizení skupiny. Je-li pro odporový ohřívač potřeba
přechodová tepelná odpověď, spusťte krok elektrostatiky samostatně a jeho
disipaci přeneste do přechodového tepelného modelu jako podmínku *Heat*, místo
abyste oba kroky dávali do jednoho pořadí kroků.

---

## 2. Grafické uživatelské rozhraní

Vše podstatné se nachází v panelu **Řešič** (záložky `Úloha`,
`Okrajové podmínky`, `Počáteční podmínky`, `Environmentální podmínky`,
`Materiál`, `Výsledky`), v nabídce **Úloha** a v panelu **Model**.

### 2.1 Výběr úlohy

`Úloha` -> `Pořadí kroků úloh` (`Ctrl+P`) otevře dialog pořadí kroků úlohy.
Přidejte krok a vyberte **Electro-statics** - „Calculate electric field,
current density and generated Joule's heat“ (výpočet elektrického pole,
proudové hustoty a vzniklého Jouleova tepla).

Elektrostatika se volně kombinuje se všemi ostatními fyzikami kromě
*Modal analysis*. Řetězce, které stojí za to znát:

- *Electro-statics* a poté *Heat transfer* - odporový ohřev;
- *Electro-statics* a poté *Magneto-statics* - magnetické pole vypočteného
  proudu, což je jediný způsob, jak magnetostatickou analýzu vůbec spustit;
- *Electro-statics*, poté *Heat transfer* a poté *Stress analysis* - tepelné
  napětí od odporového ohřevu.

Pamatujte na 1.10: každé pořadí kroků, které obsahuje krok elektrostatiky, se
řeší jako ustálené.

### 2.2 Záložka Úloha

U modelu pouze s elektrostatikou je tato záložka **prázdná**. Řešič v čase zde
není, protože ho typ úlohy nepodporuje, a elektrostatika nemá vlastní skupinu
nastavení. Vše se přiřazuje pro každou entitu v záložkách podmínek.

Pokud záložka ukazuje skupinu *Řešič v čase*, umístil ji tam jiný krok v
pořadí kroků - a podle 1.10 zase zmizí, jakmile se přidá krok elektrostatiky.

### 2.3 Záložka Materiál

Záložka `Materiál` uvádí materiál přiřazený vybrané entitě. Elektrostatika
vyžaduje **všechny tři** tyto vlastnosti:

| Vlastnost | Jednotky | Poznámka |
|---|---|---|
| Relative permittivity | `N/A`, bezrozměrná | `er`, `1` pro vakuum, `1.00059` pro vzduch, asi `4` pro sklo |
| Electrical conductivity | `S/m` | `sigma`, `5.882e7` pro měď, `5e-15` pro vzduch |
| Density | `kg/m^3` | nutná k tomu, aby se entita řešila, nikdy se nečte |

Entita, které některá z nich chybí, se neřeší a kontrola nastavení před
spuštěním výpočtu upozorní hlášením *Entity ... has material assigned which is
missing required properties*. Protože tento řešič hustotu nikdy nepoužívá,
požadavek splní libovolná kladná hodnota.

Dodávaná knihovna materiálů už má všechny tři vlastnosti pro kovy, vzduch a
skla, takže k začátku stačí přiřadit *Copper* nebo *Air*.

Každá vlastnost je tabulkou v závislosti na teplotě, ale závislost na teplotě
se respektuje jen tehdy, když tepelný krok dříve v pořadí kroků vytvořil teploty
prvků - viz 1.6.

### 2.4 Záložka Okrajové podmínky

Záložka je rozdělena na seznam dostupných okrajových podmínek nahoře a editor
vybrané podmínky dole. Nejprve vyberte entitu ve stromu `Model`; správce pak
nabídne podmínky platné pro daný typ entity a vybranou úlohu. Zaškrtnutím ji
přiřadíte a poté upravte její složky ve spodním stromu.

Nabízejí se jen dvě podmínky z části 1.4. Poznámky k jejich použití:

- **Electric potential** je podmínka, která hodnotu drží, místo aby ji
  vynucovala, a nabízí se na každém typu entity. Přiřaďte ji stěně, hraně nebo
  bodu, jejichž potenciál je znám. Model bez ní se nevyřeší.
- **Charge density** je zdrojový člen a kladná hodnota zvyšuje potenciál kolem
  sebe. Před použitím si přečtěte 1.5: hodnota je hustotou a na rozdíl od
  podmínky *Heat* tepelného řešiče se nerozkládá na entitu.
- Obě složky jsou tabulkami v závislosti na čase, na čemž záleží jen v pořadí
  kroků řízeném jiným, časově závislým krokem.

### 2.5 Počáteční a environmentální podmínky

Záložka `Počáteční podmínky` nabízí *Electric potential*, který nastavuje
výchozí pole iteračního řešení. Zkonvergovanou odpověď nemění.

Záložka `Environmentální podmínky` nabízí *Temperature*, kterou tento řešič
ignoruje - viz 1.6.

### 2.6 Nastavení maticového řešiče

`Úloha` -> `Nastavit maticový řešič úloh` nastavuje iterační řešiče. Řešič
elektrostatiky používá položku **CG**.

Pokud řešení nekonverguje, obvyklou příčinou je model bez předepsaného
potenciálu nebo entita s nulovou permitivitou, nikoli nastavení řešiče. Než
zvýšíte počet iterací, zkontrolujte okrajové podmínky a materiál.

### 2.7 Monitorovací body

`Úloha` -> `Definovat monitorovací body` umístí sondy na zadané souřadnice a
vybere proměnnou, která se má zaznamenávat - u elektrostatické úlohy je obvyklou
volbou *Electric potential*, což je nejrychlejší způsob, jak z modelu odečíst
hodnotu jako číslo, a ne jako barvu. Hodnoty ukazuje
`Zprávy` -> `Monitorovací body`.

### 2.8 Výsledky a záznamy

Záložka `Výsledky` uvádí vypočtené proměnné a ovládá 3D zobrazení. Elektrický
potenciál je uzlový skalár; elektrické pole a proudová hustota jsou prvkové
vektory; elektrická energie, elektrická rezistivita a Jouleovo teplo jsou
prvkové skaláry.

Záložka `Záznamy` panelu `Model` uvádí záznamy výsledků. Elektrostatická
analýza je statická, takže je **jeden**.

`Zprávy` -> `Log soubor řešiče` ukazuje celý výstup řešiče včetně statistiky
všech šesti proměnných po zapsání záznamu.

---

## 3. Návod - potenciál a proud ve vodiči

**Cíl.** Vypočítat rozložení potenciálu podél měděné tyče držené na dvou
různých potenciálech a odečíst pole, proud a odpor.

Návod předpokládá vysíťovaný objemový model - dobře poslouží dlouhý kvádr.
Použijte `Soubor` -> `Nový model`, nakreslete kvádr pomocí `Geometrie` ->
`Vytvořit prvek` nebo importujte geometrii a tetraedrizujte ji; to pokrývá
návod *Nakreslit krychli*. Označte obě koncové stěny jako samostatné povrchové
entity, aby jim bylo možné přiřadit podmínky.

Uvažujte tyč dlouhou `0.1 m` s průřezem `0.01 m` krát `0.01 m`, díky čemuž se
ruční výpočty na konci snadno sledují.

### Krok 1 - vyberte úlohu

1. `Úloha` -> `Pořadí kroků úloh` (`Ctrl+P`).
2. Přidejte krok a vyberte **Electro-statics**.
3. Potvrďte tlačítkem `OK`.

Záložka `Úloha` zůstává prázdná - není co nastavovat.

### Krok 2 - přiřaďte materiál

1. Ve stromu `Model` vyberte objemovou entitu.
2. Otevřete záložku `Materiál` a přiřaďte **Copper** nebo materiál se všemi
   třemi vlastnostmi:
   - **Relative permittivity** = `2.8`
   - **Electrical conductivity** = `5.882e7` `S/m`
   - **Density** = `8960` `kg/m^3`

Hustotu tento řešič nepoužívá, ale musí být přítomna, aby se entita vůbec
řešila. Pole řídí permitivita; vodivost jen dodatečně převede pole na proud.

### Krok 3 - předepište oba konce

1. Vyberte povrchovou entitu na jednom konci.
2. Otevřete záložku `Okrajové podmínky` a zaškrtněte **Electric potential**.
3. Nastavte hodnotu na `0` V - to je zem.
4. Vyberte povrchovou entitu na druhém konci, znovu zaškrtněte
   **Electric potential** a nastavte ho na `10` V.

Tento krok činí model řešitelným. Jediný předepsaný potenciál je minimum; dva s
různými hodnotami vytvářejí pole.

Nepřidávejte podmínku *Charge density*. Vodič s předepsanými potenciály na
koncích nenese prostorový náboj, takže zdrojový člen patří na nulu a potenciál
je přímkou mezi oběma konci - díky čemuž je ruční kontrola v kroku 6 přesná.

### Krok 4 - řešte

1. `Řešení` -> `Spustit řešič` (`Ctrl+R`).
2. Kontrola nastavení ohlásí chybějící materiály ještě před spuštěním výpočtu.
3. Průběh sledujte v `Zprávy` -> `Log soubor řešiče`.

Výpočet je jediným řešením metodou sdružených gradientů a na síti této velikosti
skončí za několik sekund.

### Krok 5 - prohlédněte si výsledky

- Záložka `Výsledky`: zobrazte **Electric potential**. Podél tyče by měl
  lineárně klesat z `10` V na `0` V s rovinnými ekvipotenciálními plochami
  kolmými k její ose. Jakékoli zakřivení ve stejnorodé tyči znamená, že síť
  není propojena tak, jak se předpokládá.
- **Electrical resistivity** by měla na každém prvku tyče ukazovat
  `1/5.882e7 = 1.7e-8` `V*m/A`. To je nejlepší jednotlivá kontrola, že materiál
  dorazil do prvků: je přesná, nezávislá na síti a chybná ve chvíli, kdy je
  chybná vodivost.
- **Electric field** míří podél tyče a měl by rovnoměrně ukazovat
  `10/0.1 = 100` `V/m` a **Current density** `sigma*E = 5.88e9` `A/m^2`. Obě jsou
  hustoty, takže čísla platí bez ohledu na velikost prvku - krok 6 je odvozuje.
- Monitorovací bod na *Electric potential* v polovině tyče by měl ukazovat
  `5` V.

### Krok 6 - spočítejte čísla ručně

První dva řádky níže jsou to, co by výsledky **Electric field** a
**Current density** již měly ukazovat, takže jsou přímou kontrolou řešení.
Poslední tři jsou celkové hodnoty, nikoli hustoty, které žádný prvkový výsledek
nedává, a je nutné je spočítat. Pro stejnorodou tyč délky `L` s průřezem `A`
drženou na rozdílu potenciálů `U`:

```
E = U / L                       = 10 / 0.1            = 100      V/m
J = sigma * E                   = 5.882e7 * 100       = 5.88e9   A/m^2
I = J * A                       = 5.88e9 * 1e-4       = 5.88e5   A
R = L / ( sigma * A )           = 0.1 / (5.882e7*1e-4) = 1.7e-5  ohm
P = U^2 / R                     = 100 / 1.7e-5        = 5.88e6   W
```

Proud je obrovský, protože plná měděná tyč tohoto průřezu je velmi dobrý vodič
a `10` V na ní je zkrat - užitečná připomínka, že elektrostatický model ochotně
spočítá situaci, která by skutečnou součástku odpařila.

### Krok 7 - volitelně zjemněte síť

Jednou zjemněte síť a řešte znovu. Každý výsledek - **potenciál**,
**elektrické pole**, **proudová hustota**, **elektrická energie**,
**rezistivita** a **Jouleovo teplo** - si musí udržet svou hodnotu. Všechny jsou
hustotami, takže hodnota, která se při zjemňování posouvá, je známkou problému
modelu, nikoli jemnější sítě, a stojí za to ji vystopovat, než budete
pokračovat.

### Řešení problémů

| Příznak | Příčina |
|---|---|
| Řešení nekonverguje | nikde není podmínka *Electric potential*, takže potenciál je definován jen až na konstantu |
| *Entity has material assigned which is missing required properties* | chybí jedna ze tří požadovaných vlastností - často hustota, kterou řešič nikdy nepoužívá |
| Celý model je na jednom potenciálu | byl předepsán jen jeden potenciál, nebo druhý má stejnou hodnotu |
| Potenciál je všude nulový kromě předepsaných stěn | entita má `er = 0`, takže nemá tuhost a nic nespojuje |
| Rezistivita je všude nulová | elektrická vodivost je nulová, takže neteče žádný proud a rezistivita se ukládá jako nula |
| Povrchová entita neukazuje žádné pole | její tloušťka je nulová, takže se ani nesestavuje, ani nezpracovává |

---

## 4. Návod - odporový ohřev

**Cíl.** Vzít Jouleovo teplo vypočtené z elektrického pole a použít ho k ohřevu
tyče zřetězením kroku elektrostatiky a tepelného kroku.

### Krok 1 - sestavte pořadí kroků

1. `Úloha` -> `Pořadí kroků úloh` (`Ctrl+P`).
2. Přidejte krok **Electro-statics**.
3. **Za** něj přidejte krok **Heat transfer**.
4. Potvrďte tlačítkem `OK`.

Na pořadí záleží: tepelný řešič čte Jouleovo teplo, které zanechal řešič
elektrostatiky, takže krok elektrostatiky musí proběhnout jako první. K jejich
propojení není potřeba žádná podmínka ani nastavení.

Všimněte si, co se stane se záložkou `Úloha`: skupina *Řešič v čase*, kterou by
zobrazil samotný tepelný krok, zmizela, protože elektrostatika nemá řešič v čase
a pořadí kroků přebírá nejpřísnější nastavení - viz 1.10. Tento řetězec je
pouze ustálený.

### Krok 2 - doplňte materiál

Tepelný krok potřebuje kromě tří vlastností, které potřebuje krok
elektrostatiky, čtyři vlastní, takže materiál entity musí mít všechny:

| Vlastnost | Hodnota pro měď | Potřebuje |
|---|---|---|
| Relative permittivity | `2.8` | elektrostatika |
| Electrical conductivity | `5.882e7` `S/m` | elektrostatika |
| Density | `8960` `kg/m^3` | obě |
| Thermal conductivity | `401` `W/(m*K)` | teplo |
| Heat capacity | `385` `J/(kg*K)` | teplo |
| Emissivity | `0` | teplo |

Dodávaný materiál *Copper* je už má. Entitu, které chybí vlastnost požadovaná
**kterýmkoli** krokem, tento krok přeskočí.

### Krok 3 - ponechte elektrické okrajové podmínky

Obě podmínky **Electric potential** z návodu 3 zůstávají beze změny. Budicí
potenciál snižte na něco méně prudkého - `0.01` V na tyči stále vytvoří velký
proud a udrží tepelný zdroj v rozsahu, kde je tepelná odpověď čitelná.

### Krok 4 - přidejte tepelné okrajové podmínky

Tepelná polovina modelu potřebuje vlastní odvod tepla, přesně jako by ho
potřebovala samostatně:

1. Vyberte povrchovou entitu pokrývající boky tyče.
2. Otevřete záložku `Okrajové podmínky` a zaškrtněte **Simple convection**.
3. Nastavte **Convection coefficient** = `10` `W/(m^2*K)` a
   **Fluid temperature** = `293.15` K.

Bez povrchu s konvekcí nebo předepsané teploty má tepelný model zdroj, ale žádný
odvod, a ustálený stav neexistuje. Všimněte si, že stěna použitá pro konvekci
nepotřebuje pro tepelný řešič žádnou tloušťku, ale stěna s nulovou tloušťkou
také nevede žádné elektrické pole - elektrické podmínky ponechte na koncových
stěnách a konvekci na bocích, a obě se nebudou ovlivňovat.

### Krok 5 - řešte

`Řešení` -> `Spustit řešič` (`Ctrl+R`). Log ukazuje nejprve krok elektrostatiky
se statistikou jeho šesti proměnných a poté tepelný krok se statistikou
teploty, tepelného toku a součinitele přestupu tepla.

### Krok 6 - prohlédněte si výsledky

- Záložka `Výsledky`: zobrazte **Joule heat**. Je mostem mezi oběma fyzikami -
  všude, kde je nenulové, viděl tepelný řešič zdroj.
- Zobrazte **Temperature**. Tyč by měla být nejteplejší tam, kde je Jouleovo
  teplo největší, a směrem ke stěnám s konvekcí by se měla blížit teplotě
  tekutiny.
- Zobrazte **Electric potential** a potvrďte, že elektrická polovina modelu je
  stále lineárním polem z návodu 3.

### Krok 7 - zkontrolujte energetickou bilanci

Jouleovo teplo je hustota disipace ve `W/m^3`, takže celkový výkon, který
elektrická polovina modelu dodá tepelné polovině, je tato hustota krát objem, na
který působí. Pro stejnorodou tyč by měl vyjít jako `P = U^2/R` z návodu 3, s
`R = L/(sigma*A)`.

V ustáleném stavu musí povrch s konvekcí odvést přesně tento výkon, což dává
stejnou globální kontrolu, jakou popisuje příručka přenosu tepla:

```
P = h * A_s * ( Ts_mean - Tf )
```

Střední teplotu povrchu odečtěte ze statistiky v logu řešiče a potvrďte, že se
bilance uzavírá. Je to jediná kontrola, která pokrývá obě fyziky najednou:
odhalí chybnou vodivost, chybný součinitel konvekce i entitu, která v
kterékoli polovině potichu zůstala nevyřešená.

### Řešení problémů

| Příznak | Příčina |
|---|---|
| Teplota je všude `293.15` K | Jouleovo teplo je nulové - zkontrolujte elektrickou vodivost a že krok elektrostatiky běží jako první |
| Tepelné řešení nekonverguje | tepelná polovina nemá předepsanou teplotu ani povrch s konvekcí |
| Skupina *Řešič v čase* zmizela | očekávané - krok elektrostatiky činí celé pořadí kroků ustáleným, viz 1.10 |
| Nárůst teploty se při zjemnění sítě mění | neočekávané - každý elektrostatický výsledek je hustotou, proto zkontrolujte tepelnou polovinu modelu a kvalitu sítě |
| Zobrazují se jen výsledky elektrostatiky | tepelnému kroku chybí jedna z jeho čtyř požadovaných materiálových vlastností |

---

## 5. Kontrola modelu

Řešič nemá vestavěnou sadu ověřovacích testů, proto se vyplatí nový model před
použitím ověřit vůči něčemu, co lze spočítat ručně. Níže uvedené kontroly jsou
formulovány pro **potenciál**, který je primární neznámou i veličinou, ve které
se vyjadřuje každé řešení v uzavřeném tvaru; výsledky elektrického pole a
proudové hustoty by se měly shodovat s gradienty, které z něj plynou.

**Stejnorodá tyč.** Pro těleso délky `L` držené na `V1` a `V2` na dvou
protilehlých stěnách, z jednoho materiálu a bez náboje, je potenciál podél tyče
lineární a

```
V(x) = V1 + ( V2 - V1 ) * x / L
E    = ( V1 - V2 ) / L          [V/m]
```

Umístěte monitorovací bod do střední roviny a potvrďte, že ukazuje průměr obou
předepsaných hodnot, a další do čtvrtiny délky pro čtvrtinovou hodnotu. Tím
odhalíte nepropojenou síť, podmínku přiřazenou nesprávné entitě i nulovou
permitivitu. Jednou zjemněte síť a potvrďte, že čísla zůstanou stejná.

**Rovnoměrný prostorový náboj.** Tyč výše prověřuje jen tuhost. Chcete-li
zkontrolovat i zdrojový člen, vezměte stejné těleso, **obě** koncové stěny
držte na `0` V a objemu přiřaďte rovnoměrnou *Charge density* `rho`. Potenciál
je pak parabola, s `eps = e0*er`:

```
V(x)  = rho * x * ( L - x ) / ( 2 * eps )
Vmax  = rho * L^2 / ( 8 * eps )          at the mid-plane
```

(`at the mid-plane` - ve střední rovině.) Monitorovací bod ve střední rovině
dává přímo `Vmax`. Kontrolují se dvě věci najednou: velikost, která potvrzuje,
že permitivita a hustota náboje se čtou podle očekávání, a **znaménko** -
kladná hustota náboje musí mezi dvěma uzemněnými stěnami vytvořit kladný hrb
potenciálu. Zopakujte to s nábojem na povrchové nebo čárové entitě a na bodu a
potvrďte, že se polarita s typem entity nemění.

**Rezistivita.** Každý prvek, který vede proud, musí uložit elektrickou
rezistivitu přesně `1/sigma`. Odečtěte ji ze statistiky v logu řešiče a
porovnejte s vodivostí v materiálu. Je přesná, nikoli přibližná, nezávislá na
síti a nejrychlejším potvrzením, že materiál dorazil do prvků, do kterých měl.

**Koaxiální pole.** Pro mezikruží mezi poloměry `a` a `b` s vnitřním povrchem
na `V1` a vnějším na `V2` je potenciál logaritmický:

```
V(r) = V2 + ( V1 - V2 ) * ln( b / r ) / ln( b / a )
```

Vymodelujte mezikruží, předepište oba povrchy a porovnejte monitorovací body na
několika poloměrech. Tím se řešič ověří na poli, které není rovnoměrné, což tyč
nedokáže, a ukáže se, zda je síť u vnitřního vodiče dostatečně jemná.

**Dielektrika v sérii.** Pro dvě vrstvy tloušťky `d1`, `d2` a relativní
permitivity `er1`, `er2` naskládané mezi dva předepsané potenciály je potenciál
na rozhraní

```
Vi = ( V1*er1/d1 + V2*er2/d2 ) / ( er1/d1 + er2/d2 )
```

Porovnejte s vypočteným potenciálem na rozhraní. To přímo potvrzuje to, co
popisuje část 1.9: rozdělení se řídí **permitivitami**. Spuštění téhož modelu
se dvěma různými vodivostmi a stejnou permitivitou a pozorování, že se potenciál
na rozhraní nepohne, je ukázkou, že jde o elektrostatický řešič, a ne o řešič
proudového pole.

**Odpor a disipace.** Prvkové výsledky jsou hustotami, takže celková hodnota -
proud, odpor, výkon - musí vycházet z potenciálu. Pro stejnorodou tyč:

```
R = L / ( sigma * A )      [ohm]
I = U / R                  [A]
P = U * I = U^2 / R        [W]
```

Použijte je vždy, když má celková hodnota opustit model - pro dimenzování vodiče
nebo pro kontrolu správnosti vůči měření. `P` je také hodnota, na kterou se musí
přes objem tyče integrovat výsledek **Jouleova tepla**, což je kontrola, že se
elektrická a tepelná polovina svázaného výpočtu shodují.

U kterékoli z těchto kontrol zjemněte síť alespoň jednou a potvrďte, že
potenciál konverguje, a ne že se posouvá.

---

## 6. Omezení

### Omezení modelování

- **Elektrostatika, nikoli proudové pole.** Tuhost se sestavuje z permitivity;
  vodivost do soustavy rovnic nikdy nevstupuje. Potenciál v tělese ze dvou
  materiálů se proto dělí v poměru permitivit, nikoli odporů. Vodič z jednoho
  materiálu je modelován správně; síť rezistorů nikoli - viz 1.9.
- **Pouze statika.** Neexistuje řešič v čase, kapacita ani indukčnost, takže
  přechodové jevy nabíjení, střídavá pole, posuvný proud, vířivé proudy a
  povrchový jev jsou mimo model. Přidání kroku elektrostatiky do pořadí kroků
  navíc odstraní řešič v čase z celého pořadí kroků, takže tepelný krok
  zařazený za něj může být jen ustálený - viz 1.10.
- **Pouze lineární, izotropní dielektrika.** Permitivita a vodivost prvku jsou
  skaláry a během řešení jsou konstantní. Anizotropní, feroelektrické a na poli
  závislé materiály se nemodelují a neexistuje dielektrický průraz.
- **Pouze dvě okrajové podmínky.** Předepsaný potenciál a hustota náboje tvoří
  celý slovník. Neexistuje předepsaný povrchový náboj, předepsaný normálový tok,
  podmínka symetrie ani ošetření otevřené hranice nebo nekonečných prvků -
  neomezenou úlohu pole je nutné oříznout vysíťováním dostatečného objemu
  okolního vzduchu a předepsáním potenciálu na jeho vnější hranici.
- **Žádná redistribuce volného náboje.** Náboj se nepřesouvá na povrch vodiče;
  řešič nemá o vodiči žádnou představu nad rámec materiálových vlastností, které
  dostal. Vodič modelujte předepsáním jeho potenciálu, nikoli velkou vodivostí.
- **Žádná zpětná vazba od proudu.** Proudová hustota se dopočítává z pole, takže
  vodivost, která se mění s polem nebo s teplotou, kterou disipace vytváří, nemá
  v rámci jednoho řešení žádný vliv.
- **Povrchy jsou napříč tloušťkou ekvipotenciální.** Povrchová entita vede pole
  pouze ve své rovině. Povrch s nulovou tloušťkou nebo čára s nulovou plochou
  průřezu nemají vůbec žádnou tuhost a přeskakují se při sestavení i při
  zpracování výsledků.
- **Bodové prvky nevedou.** Bod nese zdroj náboje a může držet předepsaný
  potenciál, nic víc, a nikdy se nezpracovává. Bod, jehož uzel nepatří žádnému
  jinému počítanému prvku, zanechá v matici prázdný řádek.
- **Závislost materiálu na teplotě se nečte.** Tabulky se vyhodnocují při
  `293.15 K`, pokud tepelný krok dříve v pořadí kroků nevytvořil teploty prvků;
  environmentální podmínka *Temperature* na tento řešič nemá vliv.
- **Model potřebuje předepsaný potenciál.** Bez něj má matice netriviální jádro
  a řešení nezkonverguje.

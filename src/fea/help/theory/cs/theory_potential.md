# Proudění porézním prostředím - Teoretický manuál a uživatelská příručka

Tento dokument popisuje řešič proudění porézním prostředím (`RSolverPotential`)
v Range FEA, který řeší typ úlohy **Proudění porézním prostředím** (*Flow
through porous media*): rovnice, které řeší, význam každého vstupu, který
přijímá, části grafického uživatelského rozhraní, které jej ovládají, a dva
podrobně rozpracované tutoriály.

**Obsah**

1. [Teoretický základ](#1-teoretický-základ)
2. [Grafické uživatelské rozhraní](#2-grafické-uživatelské-rozhraní)
3. [Tutoriál - proudění pískovým filtrem poháněné tlakem](#3-tutoriál---proudění-pískovým-filtrem-poháněné-tlakem)
4. [Tutoriál - vzestupné prosakování svislým sloupcem](#4-tutoriál---vzestupné-prosakování-svislým-sloupcem)
5. [Kontrola modelu](#5-kontrola-modelu)
6. [Omezení](#6-omezení)

---

## 1. Teoretický základ

### 1.1 Primární neznámá - pole tlaku

Primární neznámou je **tlak** `p` pórové tekutiny v každém uzlu, v pascalech.
Každý uzel nese jediný stupeň volnosti, takže síť s `N` uzly dává soustavu `N`
rovnic - stejně levnou jako tepelný nebo elektrostatický model na téže síti.
Rychlost tekutiny se po řešení získá z pole tlaku.

Pole tlaku řízené touto rovnicí je určeno jen **až na aditivní konstantu**:
přičtení stejné hodnoty k tlaku všude nezmění žádnou rychlost. Každý model proto
potřebuje alespoň jednu okrajovou podmínku *Pressure (explicit)*, viz 1.7. Zda se
tlak chápe jako absolutní, nebo jako přetlak, je na uživateli - řešič pracuje
výhradně s jeho gradientem.

### 1.2 Řídicí rovnice

Řešič implementuje **Darcyho zákon** pro pomalé proudění jediné nestlačitelné
tekutiny, která vyplňuje póry tuhého porézního tělesa:

```
v = -( k / mu ) * ( grad(p) - rho * g )
```

spolu se zákonem zachování hmotnosti nestlačitelné tekutiny

```
div( v ) = 0
```

což dohromady dává potenciálovou rovnici (Laplaceova typu) pro tlak:

```
div( ( k / mu ) * ( grad(p) - rho * g ) ) = 0
```

| Symbol | Význam | Jednotky | Zdroj |
|---|---|---|---|
| `p` | tlak pórové tekutiny | `Pa` | řešená veličina |
| `v` | Darcyho (filtrační) rychlost | `m/s` | dopočtená, viz 1.8 |
| `k` | vnitřní propustnost porézního tělesa | `m^2` | vlastnost materiálu *Permeability to fluids* |
| `mu` | dynamická viskozita pórové tekutiny | `kg/(m*s)` | vlastnost materiálu *Dynamic viscosity* |
| `rho` | hustota pórové tekutiny | `kg/m^3` | vlastnost materiálu *Density* |
| `g` | tíhové zrychlení | `m/s^2` | environmentální podmínka, viz 1.6 |

Poměr `k/mu` hraje roli, jakou v tepelném modelu hraje vodivost. Bez gravitace je
rovnice Laplaceovou rovnicí pro tlak a proudění směřuje přímo proti gradientu
tlaku. S gravitací způsobí člen `rho*g`, že tekutina v klidu nese hydrostatický
gradient tlaku: `grad(p) = rho*g` dává `v = 0`.

**Rychlost je Darcyho rychlost.** Je to objemový průtok na jednotku **celkového**
průřezu, pevné fáze i pórů dohromady - nikoli rychlost tekutiny uvnitř pórů.
Střední rychlost v pórech je větší o převrácenou hodnotu pórovitosti `n`:

```
v_pore = v / n
```

Pórovitost do ustáleného proudění nevstupuje, takže ji řešič nepotřebuje ani
nepřijímá. Použijte ji ručně k odhadu doby průchodu.

**Propustnost a hydraulická vodivost.** Mechanika zemin a hydrogeologie často
uvádějí místo vnitřní propustnosti *hydraulickou vodivost* (součinitel filtrace)
`K` v `m/s` - součinitel Darcyho zákona zapsaného pro hydraulickou výšku. Obě
veličiny souvisejí přes vlastnosti tekutiny:

```
k = K * mu / ( rho * |g| )
```

Pro vodu při 20 °C odpovídá `K = 1e-4 m/s` hodnotě `k = 1.02e-11 m^2`. Ropné
inženýrství uvádí propustnost v darcy: `1 D = 9.869e-13 m^2`.

**Povinné vlastnosti materiálu.** Úloha proudění porézním prostředím vyžaduje
**tři** z nich:

| Vlastnost | Jednotky | Použití |
|---|---|---|
| Permeability to fluids | `m^2` | matice tuhosti - propustnost pevné fáze |
| Dynamic viscosity | `kg/(m*s)` | matice tuhosti - viskozita **tekutiny** |
| Density | `kg/m^3` | gravitační člen - hustota **tekutiny** |

**Hustota a viskozita patří pórové tekutině, ne poréznímu tělesu.** Materiál pro
tento typ úlohy proto popisuje *nasycené* prostředí: propustnost pevného skeletu
a hustotu a viskozitu tekutiny, která jej vyplňuje. Takto jsou sestaveny dodávané
materiály s názvem *... (water saturated)* (viz 2.3). Materiál uvádějící
objemovou hmotnost suché zeminy zkreslí hydrostatický tlak v poměru obou hustot.

Entita, jejímuž materiálu chybí některá ze tří vlastností, se **neřeší**. Kontrola
nastavení na to upozorní před spuštěním výpočtu. Jedinou výjimkou je entita s
okrajovou podmínkou tohoto typu úlohy: ta se do soustavy zahrne bez ohledu na
materiál a její propustnost je pak prostě nulová. Tak dostane přítok i okrajová
plocha bez materiálu.

Každá vlastnost je tabulka v závislosti na teplotě. Úloha proudění porézním
prostředím sama o sobě nemá pole teploty, takže každá tabulka se vyhodnotí při
`293.15 K`. Tabulky se uplatní jen tehdy, když úloha *Heat transfer* zařazená
dříve v témže pořadí kroků vypočítala teploty prvků - tak proudí teplejší
tekutina s nižší viskozitou rychleji.

### 1.3 Diskretizace metodou konečných prvků

Slabá formulace rovnice pro tlak dává pro každý prvek

```
K = INT( B^T * ( k / mu ) * B )                  tuhost
f = INT( B^T * ( k / mu ) * rho * g )            gravitační zatížení
  + INT_S( N^T * v_in )                          přítok přes hranici
```

kde `B` se sestavuje z derivací tvarových funkcí, `N` jsou tvarové funkce a
`v_in` je rychlost přítoku předepsaná na okrajové ploše. Každá matice prvku má
rozměr `n x n` pro prvek s `n` uzly. Matice hmotnosti ani akumulace neexistuje:
proudění je ustálené, viz 1.10.

**Míry prvků.** Jako jinde v Range FEA dostávají entity nižší dimenze
geometrickou míru, aby mohly reprezentovat skutečná tělesa:

| Entita | Míra | Kde se nastavuje |
|---|---|---|
| Objem | 1 | - |
| Plocha | tloušťka plochy | geometrické vlastnosti entity |
| Čára | průřez čáry | geometrické vlastnosti entity |
| Bod | - | body nevedou proudění |

**Plocha** s tloušťkou vede proudění ve své rovině - tak se sestavuje
dvourozměrný model (řez hrází nebo zvodní). Její gravitační zatížení používá
složku `g` ležící v rovině plochy. Plocha s **nulovou tloušťkou** - běžná okrajová
plocha objemového modelu - nemá tuhost a pouze přijímá přítok.

**Čára** s průřezem vede proudění podél svého směru, pro jednorozměrný model
sloupce nebo potrubí s náplní. **Bod** nepřispívá ničím. Bodový prvek, jehož uzel
nepatří žádnému jinému počítanému prvku, zanechá v matici prázdný řádek -
bodovým entitám porézní materiál nepřiřazujte.

### 1.4 Okrajové podmínky

| Okrajová podmínka | Typ | Použitelná na | Složky |
|---|---|---|---|
| Pressure (explicit) | explicitní (Dirichletova) | plocha | Pressure `[Pa]` |
| Velocity (inflow) | přirozená (předepsaný tok) | plocha | Velocity `[m/s]` |
| Volumetric flow rate (inflow) | přirozená (předepsaný tok) | plocha | Volumetric flow rate `[m^3/s]` |

Okrajová plocha bez kterékoli z nich je **nepropustná**: neprochází jí žádná
tekutina. To je přirozená podmínka rovnice, takže stěny, dno zvodně a roviny
symetrie nepotřebují žádnou podmínku.

**Pressure (explicit)** je jediná explicitní podmínka. Každý uzel plochy se
odstraní ze soustavy rovnic a drží se na zadaném tlaku. Je to místo, kde tekutina
model opouští (výtok, drén, volná voda nádrže) nebo do něj vstupuje pod známým
tlakem. Každý model potřebuje alespoň jednu. Nově přiřazená podmínka začíná na
`1000 Pa`.

**Velocity (inflow)** předepisuje normálovou Darcyho rychlost přes plochu.
**Kladná** hodnota je přítok do porézního tělesa, **záporná** odtok (odběr).
Hodnota je tok na jednotku plochy, takže celkový průtok roste s plochou.

**Volumetric flow rate (inflow)** předepisuje **celkový** průtok plochou v
`m^3/s`. Řešič jej vydělí plochou celé entity a výsledek použije jako
rovnoměrnou rychlost přítoku, takže průtok se nezmění při změně sítě ani
zvětšení plochy. Kladná hodnota je opět přítok.

Žádná z podmínek přítoku tlak nepředepisuje: tlak na vstupu je součástí
výsledku. Kde mají plocha s přítokem a plocha s *Pressure (explicit)* společné
uzly, v těchto uzlech vyhrává tlak.

Plocha s přítokem **nepotřebuje materiál ani tloušťku**: do soustavy ji zařadí
samotná podmínka. Má-li plocha porézní materiál a tloušťku, vede navíc proudění
ve své rovině.

Podmínka *Pressure (implicit)* řešiče nestlačitelného proudění se pro tento typ
úlohy nenabízí.

Všechny složky přijímají tabulku v závislosti na čase, která se uplatní při
zapnutém řešiči v čase, viz 1.10.

### 1.5 Gravitace

Gravitace vstupuje přes environmentální podmínku *Gravitational acceleration*.
**Každá entita ji má implicitně s `g = (0, 0, -9.80665) m/s^2`**, takže gravitace
směřuje podél záporné osy Z, pokud se nezmění. Dva důsledky stojí za to znát:

- V modelu, jehož osa Z směřuje vzhůru, dostane tekutina v klidu automaticky
  hydrostatický tlak rostoucí o `rho*|g|` na metr hloubky. To je obvykle žádoucí.
- Model postavený s jinou osou směřující vzhůru musí mít podmínku upravenou na
  každé porézní entitě, jinak tekutina „padá" do strany. Nastavením všech tří
  složek na nulu se gravitace vypne - například pro vodorovný model, ve kterém
  zajímá jen rozdíl tlaků.

Gravitace v kombinaci s plochou *Pressure (explicit)*, která není vodorovná,
vyžaduje pozornost: rovnoměrný tlak na svislé ploše **není** hydrostatický, takže
u této plochy tekutina cirkuluje. Buď zadejte tlak v závislosti na výšce
(rozdělte plochu na více entit), nebo gravitaci vypněte, není-li součástí otázky.

### 1.6 Environmentální, počáteční a vazební podmínky

#### Environmentální podmínky

| Environmentální podmínka | Složky | Účinek |
|---|---|---|
| Gravitational acceleration | g v X, Y, Z `[m/s^2]` | gravitační člen Darcyho zákona, viz 1.5 |
| Temperature | Temperature `[K]` | nabízí se, ale nečte - viz níže |

Environmentální podmínka *Temperature* se nabízí, protože vlastnosti materiálu
jsou tabulky v závislosti na teplotě. Řešič ji **nečte**: teploty prvků přebírá
jen ze sdílených dat, která zanechá úloha *Heat transfer*, a jinak vyhodnocuje
každou tabulku při `293.15 K`.

#### Počáteční podmínky

| Počáteční podmínka | Účinek |
|---|---|
| Pressure | nabízí se, nečte se |
| Velocity | nabízí se, nečte se |

Proudění je ustálené, takže výsledek nezávisí na počátečním poli a řešič nečte
ani jednu počáteční podmínku. Obě se nabízejí, protože je typ úlohy sdílí s
úlohou nestlačitelného proudění.

#### Vazba s jinými řešiči

| Směr | Veličina | Účinek |
|---|---|---|
| Teplo -> proudění porézním prostředím | Temperature | vybírá řádek každé teplotně závislé tabulky materiálu |

Tlak ani rychlost nepřebírá žádný jiný řešič. Proudění porézním prostředím lze
kombinovat s každou jinou fyzikou kromě *Incompressible viscous flow* a *Modal
analysis*.

Řešič hlásí konvergenci bezpodmínečně, takže skupina kroků obsahující jen úlohu
proudění porézním prostředím se vždy zastaví po jedné iteraci.

### 1.7 Řešení soustavy

Řešič sestaví a vyřeší

```
K * p = f
```

Matice `K` je symetrická a pozitivně definitní, jakmile je předepsán alespoň
jeden tlak, proto se řeší metodou **sdružených gradientů** s Jacobiho
předpodmíněním. Sestavení je paralelizováno přes prvky s vyrovnávacími paměťmi
pro každé vlákno, které se na konci jednou sloučí, takže výsledek nezávisí na
počtu vláken.

Propustnosti skutečných materiálů sahají od přibližně `1e-9 m^2` (štěrk) až po
`1e-20 m^2` (neporušený jíl), čímž by se prvky matice dostaly blízko k mezím
zaokrouhlování iteračního řešiče. Před řešením se proto každá rovnice vydělí
největším `k/mu` v modelu. Vydělení všech rovnic stejným číslem řešení nezmění;
jen udrží iteraci dobře podmíněnou.

Model **bez okrajové podmínky Pressure (explicit)** nemá jednoznačné řešení.
Řešič to zjistí a zastaví se s chybou *Flow through porous media requires at
least one 'Pressure (explicit)' boundary condition*.

Další způsoby, jak dospět k singulární nebo prázdné soustavě, všechny tiché:

- entita, jejímuž materiálu chybí jedna ze tří povinných vlastností, se neřeší a
  její uzly ze soustavy vypadnou;
- entita s nulovou propustností nebo nulovou viskozitou nemá tuhost, takže její
  uzly v soustavě zůstanou, ale nejsou s ničím spojeny;
- plocha s nulovou tloušťkou nebo čára s nulovým průřezem nevede proudění.

### 1.8 Odvozené výsledky

Po řešení se rychlost prvku dopočte z uzlového pole tlaku. Gradient tlaku se
zprůměruje přes integrační body prvku a zkombinuje s gravitačním členem; pro
čárové a plošné prvky se otočí do globálních souřadnic:

```
v = -( k / mu ) * ( grad(p) - rho * g )
```

| Výsledek | Aplikován na | Význam |
|---|---|---|
| Pressure `[Pa]` | uzel | vyřešené pole pórového tlaku |
| Velocity `[m/s]` | prvek | vektor Darcyho rychlosti |

Rychlost je **hustota** ve stejném smyslu jako výsledky polí ostatních řešičů:
nezávisí na velikosti prvku, v lineárním prvku je konstantní a při zjemňování sítě
konverguje. Okrajové plochy s nulovou tloušťkou ukazují nulovou rychlost -
výsledek zobrazujte na objemu.

Celkový průtok se jako výsledek neukládá. Vypočtěte jej ručně z rychlosti a plochy
průřezu nebo z podmínek přítoku, viz 5.

Po zapsání záznamu vypíše log řešiče statistiky obou veličin a hodnoty v
monitorovacích bodech.

### 1.9 Platnost Darcyho zákona

Darcyho zákon platí pro pomalé laminární proudění, ve kterém viskózní síly
převládají nad setrvačností tekutiny. Běžné kritérium používá Reynoldsovo číslo
vytvořené ze středního průměru zrna `d`:

```
Re = rho * |v| * d / mu  <  1 ... 10
```

Proudění podzemní vody v zeminách, prosakování hrázemi a násypy, proudění filtry,
sypanými vrstvami a většinou porézních technických materiálů jej splňuje s velkou
rezervou. Proudění hrubým štěrkem, kamenným záhozem nebo velmi otevřenými pěnami
při vysokých rychlostech nikoli; Darcyho zákon pak podhodnocuje tlakovou ztrátu
(Forchheimerův režim) a tento řešič vykáže příliš velkou rychlost.

### 1.10 Ustálené proudění a řešič v čase

Typ úlohy povoluje řešič v čase, ale samotné proudění je **ustálené**: tekutina i
porézní těleso jsou nestlačitelné a neexistuje člen akumulace. Při zapnutém
řešiči v čase řeší každý časový krok samostatný ustálený stav s hodnotami
podmínek v daném čase. To je správný model, když se okrajové podmínky mění pomalu
v porovnání s dobou, za kterou se tlak ustálí - hladina nádrže nad pískovým
filtrem měnící se v průběhu hodin - a umožňuje úloze proudění porézním prostředím
sdílet pořadí kroků s přechodovou tepelnou úlohou.

Konsolidace, odvodnění nenasycené zeminy, proudění stlačitelného plynu a jakýkoli
jiný jev, při kterém se množství tekutiny uložené v pórech mění v čase, jsou mimo
model, viz kapitola 6.

---

## 2. Grafické uživatelské rozhraní

Vše podstatné se nachází v doku **Řešič** (karty `Úloha`, `Okrajové podmínky`,
`Počáteční podmínky`, `Environmentální podmínky`, `Materiál`, `Výsledky`), v
nabídce **Úloha** a v doku **Model**.

### 2.1 Výběr úlohy

`Úloha` -> `Pořadí kroků úloh` (`Ctrl+P`) otevře dialog pořadí kroků úloh.
Přidejte krok a vyberte **Flow through porous media**.

Typ úlohy nelze kombinovat s *Incompressible viscous flow* ani s *Modal
analysis*. Krok *Heat transfer* zařazený před ním zajistí, že tabulky materiálu
budou sledovat vypočtenou teplotu.

### 2.2 Karta Úloha

Karta zobrazuje skupinu *Řešič v čase*, protože typ úlohy řešič v čase
podporuje - co přechodový výpočet znamená zde, vysvětluje 1.10. Proudění
porézním prostředím nemá vlastní skupinu nastavení. Vše se přiřazuje entitám na
kartách podmínek.

### 2.3 Karta Materiál

Karta `Materiál` zobrazuje materiál přiřazený vybrané entitě. Proudění porézním
prostředím vyžaduje **všechny tři** z těchto vlastností:

| Vlastnost | Jednotky | Poznámka |
|---|---|---|
| Permeability to fluids | `m^2` | vnitřní propustnost pevné fáze, viz tabulku níže |
| Dynamic viscosity | `kg/(m*s)` | pórové tekutiny, `1.003e-3` pro vodu při 20 °C |
| Density | `kg/m^3` | pórové tekutiny, `998.2` pro vodu při 20 °C |

Dodávaná knihovna obsahuje vodou nasycené porézní materiály připravené k použití.
Každý nese teplotně závislou hustotu a viskozitu vody a typickou propustnost:

| Materiál | Propustnost `[m^2]` | Hydraulická vodivost při 20 °C `[m/s]` |
|---|---|---|
| Gravel (water saturated) | `1e-9` | `1e-2` |
| Coarse Sand (water saturated) | `1e-10` | `1e-3` |
| Fine Sand (water saturated) | `1e-12` | `1e-5` |
| Sandstone (water saturated) | `1e-13` | `1e-6` |
| Silt (water saturated) | `1e-14` | `1e-7` |
| Clay (water saturated) | `1e-17` | `1e-10` |

Propustnost skutečné zeminy se může od těchto hodnot lišit o řád i více - jsou to
výchozí hodnoty, ne údaje o konkrétní lokalitě. K modelování jiné tekutiny
některý z nich zkopírujte a nahraďte hustotu a viskozitu. Obyčejný materiál
*Sand* je suchý sypký materiál bez viskozity, proto je pro tento typ úlohy
označen jako neplatný.

### 2.4 Karta Okrajové podmínky

Karta je rozdělena na seznam dostupných okrajových podmínek nahoře a editor
vybrané podmínky dole. Nejprve vyberte plošnou entitu ve stromu `Model`; seznam
pak nabídne podmínky z kapitoly 1.4. Zaškrtnutím ji přiřadíte a její složky
upravíte v dolním stromu.

- **Pressure (explicit)** drží tlak na výtoku. Model bez ní se nevyřeší.
- **Velocity (inflow)** a **Volumetric flow rate (inflow)** přivádějí do modelu
  známý průtok. Kladné hodnoty tečou dovnitř, záporné ven.
- Plochy bez podmínky jsou nepropustné.

### 2.5 Počáteční a environmentální podmínky

Karta `Environmentální podmínky` nabízí *Gravitational acceleration*, kterou už
každá entita má s hodnotou `(0, 0, -9.80665) m/s^2` (viz 1.5), a *Temperature*,
kterou tento řešič ignoruje.

Karta `Počáteční podmínky` nabízí *Pressure* a *Velocity*, které tento řešič
nečte.

### 2.6 Nastavení maticového řešiče

`Úloha` -> `Nastavit maticový řešič úloh` nastavuje iterační řešiče. Řešič
proudění porézním prostředím používá položku **CG**. Pokud řešení nekonverguje,
hledejte nejprve entitu s nulovou propustností nebo izolovaný bodový prvek, nikoli
nastavení řešiče.

### 2.7 Monitorovací body

`Úloha` -> `Definovat monitorovací body` umístí sondy na zadané souřadnice a
vybere zaznamenávanou veličinu - *Pressure* nebo *Velocity*. `Zprávy` ->
`Monitorovací body` zobrazí hodnoty.

### 2.8 Výsledky a záznamy

Karta `Výsledky` zobrazuje vypočtené veličiny a ovládá 3D zobrazení. Tlak je
uzlový skalár, rychlost vektor na prvcích. Zobrazte rychlost jako vektorové pole
na objemu, abyste viděli dráhy proudění.

Karta `Záznamy` doku `Model` obsahuje záznamy výsledků: jeden při ustáleném
výpočtu, jeden na každý výstupní časový krok při zapnutém řešiči v čase.

`Zprávy` -> `Log soubor řešiče` zobrazí úplný výstup řešiče včetně statistik obou
veličin.

---

## 3. Tutoriál - proudění pískovým filtrem poháněné tlakem

**Cíl.** Vypočítat proudění vody vodorovným pískovým filtrem poháněné známým
rozdílem tlaků a ověřit průtok podle Darcyho zákona.

Tutoriál předpokládá objemový model kvádru se sítí - použijte `Soubor` ->
`Nový model`, nakreslete kvádr přes `Geometrie` -> `Vytvořit prvek` nebo
importujte geometrii a vytvořte čtyřstěnovou síť; tutoriál *Draw cube* tento
postup popisuje. Obě koncové plochy označte jako samostatné plošné entity, aby
jim šly přiřadit podmínky.

Filtr nechť je `1 m` dlouhý podél osy X s průřezem `0.1 m` x `0.1 m`.

### Krok 1 - výběr úlohy

1. `Úloha` -> `Pořadí kroků úloh` (`Ctrl+P`).
2. Přidejte krok a vyberte **Flow through porous media**.
3. Potvrďte tlačítkem `OK`.

*Řešič v čase* nechte vypnutý - výsledkem je jediný ustálený stav.

### Krok 2 - přiřazení materiálu

1. Vyberte objemovou entitu ve stromu `Model`.
2. Otevřete kartu `Materiál` a přiřaďte **Coarse Sand (water saturated)**.

Materiál nese propustnost písku `1e-10 m^2` a hustotu a viskozitu vody.

### Krok 3 - vypnutí gravitace

Filtr je vodorovný a zajímá jen rozdíl tlaků, proto implicitní gravitaci
odstraňte:

1. Při vybrané objemové entitě otevřete kartu `Environmentální podmínky`.
2. Vyberte **Gravitational acceleration** a všechny tři složky nastavte na `0`.

S ponechanou gravitací by voda nesla i hydrostatický tlak přes výšku filtru
`0.1 m` a rovnoměrné tlaky na svislých koncových plochách by způsobily její
cirkulaci u nich - viz 1.5.

### Krok 4 - předepsání obou konců

1. Vyberte plošnou entitu v `x = 0`.
2. Otevřete kartu `Okrajové podmínky`, zaškrtněte **Pressure (explicit)** a
   nastavte `10000` Pa - přibližně jeden metr vodního sloupce.
3. Vyberte plošnou entitu v `x = 1 m`, zaškrtněte **Pressure (explicit)** a
   nastavte `0` Pa.

Čtyři boční plochy nedostanou žádnou podmínku: jsou to nepropustné stěny.

### Krok 5 - řešení

1. `Řešení` -> `Spustit řešič` (`Ctrl+R`).
2. Průběh sledujte v `Zprávy` -> `Log soubor řešiče`.

Výpočet je jediné řešení metodou sdružených gradientů.

### Krok 6 - prohlédnutí výsledků

- Karta `Výsledky`: zobrazte **Pressure**. Klesá lineárně z `10000` Pa na `0` Pa
  podél filtru, s rovinnými izobarami kolmými na jeho osu.
- Zobrazte **Velocity**. Směřuje podél +X a je stejná v každém prvku.
- Monitorovací bod na *Pressure* v polovině filtru ukáže `5000` Pa.

### Krok 7 - ruční výpočet

S `k = 1e-10 m^2` a `mu = 1.003e-3 kg/(m*s)` pro vodu při 20 °C:

```
v = ( k / mu ) * dp / L   = 9.97e-8 * 10000 / 1     = 9.97e-4  m/s
Q = v * A                 = 9.97e-4 * 0.01          = 9.97e-6  m^3/s  (0.6 l/min)
```

Výsledek **Velocity** musí ukázat `9.97e-4 m/s`. Reynoldsovo číslo při velikosti
zrna `1 mm` je `998 * 9.97e-4 * 1e-3 / 1.003e-3 = 1`, na hranici Darcyho režimu
(1.9). Při pórovitosti `0.35` se samotná voda pohybuje rychlostí
`9.97e-4 / 0.35 = 2.8e-3 m/s` a filtrem projde asi za šest minut.

### Řešení problémů

| Příznak | Příčina |
|---|---|
| *Flow through porous media requires at least one 'Pressure (explicit)' boundary condition* | nikde není předepsán tlak na výtoku |
| *Entity has material assigned which is missing required properties* | materiálu chybí viskozita - často suchý materiál zeminy jako *Sand* |
| Tlak se mění s výškou a rychlost má složku Z | implicitní gravitace je stále zapnutá, viz krok 3 |
| Rychlost je všude nulová | oba konce mají stejný tlak, nebo je propustnost nulová |

---

## 4. Tutoriál - vzestupné prosakování svislým sloupcem

**Cíl.** Protlačit vodu vzhůru svislým pískovým sloupcem známým průtokem a
ukázat, jak se tlak na vstupu dělí na hydrostatickou a třecí část.

Použijte kvádr široký `0.1 m` x `0.1 m` a vysoký `1 m` podél osy **Z**, se spodní
plochou (`z = 0`) a horní plochou (`z = 1 m`) jako samostatnými plošnými
entitami.

### Krok 1 - úloha a materiál

1. Vyberte **Flow through porous media** jako v tutoriálu 3.
2. Objemu přiřaďte **Coarse Sand (water saturated)**.

### Krok 2 - ponechání gravitace

Sloupec stojí podél osy Z, takže implicitní gravitace `(0, 0, -9.80665) m/s^2`
směřuje správně. Podmínku *Gravitational acceleration* nechte, jak je.

### Krok 3 - okrajové podmínky

1. Vyberte horní plochu, zaškrtněte **Pressure (explicit)** a nastavte `0` Pa:
   volná voda nad sloupcem.
2. Vyberte spodní plochu, zaškrtněte **Volumetric flow rate (inflow)** a
   nastavte `1e-6` m^3/s. Na ploše `0.01 m^2` to odpovídá rychlosti přítoku
   `1e-4 m/s`.

Horní plocha je vodorovná, takže rovnoměrný tlak na ní je v souladu s gravitací.

### Krok 4 - řešení a prohlédnutí

`Řešení` -> `Spustit řešič` (`Ctrl+R`), poté zobrazte **Pressure** a
**Velocity**.

- Rychlost je `1e-4 m/s` směrem vzhůru (+Z) v každém prvku.
- Tlak roste lineárně z `0` Pa nahoře po maximum na dně.

### Krok 5 - rozdělení tlaku na vstupu

Tlak na dně musí zvednout vodní sloupec a protlačit jej pískem:

```
p_hydrostatic = rho * g * L            = 998.2 * 9.80665 * 1       = 9789 Pa
p_friction    = mu * v * L / k         = 1.003e-3 * 1e-4 * 1 / 1e-10 = 1003 Pa
p_bottom      = p_hydrostatic + p_friction                         = 10792 Pa
```

Monitorovací bod na spodní ploše součet potvrdí. Nastavte průtok na nulu a řešte
znovu: rychlost zmizí a tlak na dně klesne na čistě hydrostatických `9789 Pa` -
řešič přesně reprodukuje tekutinu v klidu.

Zvyšujte průtok, dokud třecí část nedosáhne vztlakem snížené tíhy písku, přibližně
`(rho_s - rho) * g * (1 - n) * L`: to je začátek fluidizace (tekutý písek),
kterou tuhý skelet tohoto modelu nedokáže vystihnout - řešič prostě počítá stále
větší tlak.

### Řešení problémů

| Příznak | Příčina |
|---|---|
| Tlak na dně je jen třecí část | gravitace je na objemu vypnutá |
| Tlak na dně je jen hydrostatická část | přítok je na nesprávné ploše, nebo má nulovou hodnotu |
| Rychlost směřuje do strany | sloupec byl postaven podél jiné osy než Z; upravte podmínku gravitace |

---

## 5. Kontrola modelu

**Rovnoměrný sloupec.** Pro těleso délky `L` držené na `p1` a `p2` na dvou
protilehlých plochách, z jednoho materiálu a s vypnutou gravitací, je tlak podél
tělesa lineární a

```
p(x) = p1 + ( p2 - p1 ) * x / L
v    = ( k / mu ) * ( p1 - p2 ) / L
```

Výsledek rychlosti musí souhlasit na toleranci řešiče a tlak ve střední rovině
musí být průměrem obou předepsaných hodnot.

**Hydrostatika.** Těleso s gravitací, s podmínkou *Pressure (explicit)* jen na
vodorovné horní ploše a jinak nepropustnými stěnami musí ukázat nulovou rychlost
a tlak rostoucí o `rho*|g|` na metr hloubky. Tím se naráz ověří hustota i
podmínka gravitace.

**Bilance průtoku.** V ustáleném modelu musí veškerá tekutina, která vstoupí,
také vystoupit. Při přítoku předepsaném přes *Volumetric flow rate (inflow)* musí
rychlost v průřezu vynásobená jeho plochou vrátit předepsaný průtok. Rychlost ve
sloupci odečtěte monitorovacím bodem.

**Sériové vrstvy.** Pro dvě vrstvy délky `L1`, `L2` a propustnosti `k1`, `k2`
zapojené za sebou mezi dvěma předepsanými tlaky klade proudění odpor jako dva
odpory v sérii:

```
v  = ( p1 - p2 ) / ( mu * ( L1 / k1 + L2 / k2 ) )
pi = p1 - v * mu * L1 / k1          tlak na rozhraní
```

Na rozdíl od elektrostatického řešiče tento dělí tlak v poměru **propustností**,
jak by to dělala síť odporů.

**Radiální proudění ke studni.** Pro napjatou zvodeň tloušťky `H` mezi studní o
poloměru `rw` drženou na `pw` a vnějším poloměrem `R` drženým na `pR` je tlak
logaritmický a vydatnost studny je

```
p(r) = pw + ( pR - pw ) * ln( r / rw ) / ln( R / rw )
Q    = 2 * pi * H * ( k / mu ) * ( pR - pw ) / ln( R / rw )
```

Tím se řešič ověří na nerovnoměrném poli a ukáže se, zda je síť u studny dost
jemná.

Síť alespoň jednou zjemněte a ověřte, že tlak konverguje a neposouvá se.

---

## 6. Omezení

- **Ustálené, nestlačitelné, jednofázové proudění.** Neexistuje člen akumulace,
  takže konsolidace, proudění stlačitelného plynu a přechodová odezva napjaté
  zvodně se nemodelují. Přechodový výpočet je posloupnost ustálených stavů, viz
  1.10.
- **Jen plně nasycené prostředí.** Neexistuje volná hladina (depresní křivka) ani
  nenasycená zóna. Prosakování násypem, jehož hladinu podzemní vody je třeba
  najít, je mimo model, pokud se nasycená oblast nevytvoří explicitně sítí.
- **Tuhé porézní těleso.** Skelet se nedeformuje, takže neexistuje vazba na
  analýzu napětí, efektivní napětí, konsolidace ani fluidizace.
- **Izotropní propustnost.** Propustnost je skalár. Vrstevnaté zeminy s různou
  vodorovnou a svislou propustností je třeba modelovat jako samostatné vrstvy.
- **Jen Darcyho režim.** Setrvačné (Forchheimerovo) a turbulentní proudění ve
  velmi hrubých materiálech se nemodeluje, viz 1.9.
- **Okrajové podmínky jen na plochách.** Tlaky a přítoky se předepisují na
  plošných entitách; neexistuje výronová plocha, studna ani čárový zdroj, ani
  předepsaný tlak na čáře nebo v bodě.
- **Žádný výsledek celkového průtoku.** Průtoky průřezem je třeba vypočítat z
  rychlosti ručně.
- **Počáteční podmínky se nečtou** a environmentální podmínka *Temperature* nemá
  účinek; tabulky materiálu sledují teplotu jen přes tepelnou úlohu zařazenou
  dříve v pořadí kroků.
- **Model potřebuje předepsaný tlak.** Bez něj se řešič zastaví s chybou.

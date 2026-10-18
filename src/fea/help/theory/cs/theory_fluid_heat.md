# Přenos tepla v tekutinách - Teoretická příručka a uživatelská příručka

Tento dokument popisuje řešič přenosu tepla v tekutinách v Range FEA -
`RSolverFluidHeat`, který obsluhuje typ úlohy **Přenos tepla v tekutinách**
(*Heat transfer in fluids*): rovnici, kterou řeší, význam každého vstupu, který
přijímá, jeho vazbu na tepelný řešič pevné látky a části grafického
uživatelského rozhraní, které ho ovládají.

Přenos tepla v tekutinách se řeší na proudovém poli kroku **Nestlačitelné
viskózní proudění** (*Incompressible viscous flow*). Řešič proudění, jeho
okrajové podmínky, nelineární iterace a časový krok popisuje příručka
[Proudění tekutin](theory_fluid.md) a tepelný řešič pevných látek příručka
[Přenos tepla](theory_heat.md).

**Obsah**

1. [Teoretický základ](#1-teoretický-základ)
2. [Grafické uživatelské rozhraní](#2-grafické-uživatelské-rozhraní)
3. [Omezení](#3-omezení)

---

## 1. Teoretický základ

### 1.1 Primární neznámá - teplota

Primární neznámou je **teplota** `T` v `K`, jeden stupeň volnosti na uzel.

| Typ úlohy | Řešič | Primární neznámá | Jednotky | Stupně volnosti na uzel |
|---|---|---|---|---|
| Přenos tepla v tekutinách | `RSolverFluidHeat` | teplota | `K` | 1 |

Tento typ úlohy **vyžaduje** ve stejné úloze krok *Nestlačitelné viskózní
proudění*: pole rychlosti čte z jeho výsledku a samo ho nikdy nepočítá.

### 1.2 Řídicí rovnice

**Přenos tepla v tekutinách** řeší advekci a difúzi teploty na proudovém poli,

```
rho * c * ( dT/dt + v . grad(T) ) = div( k * grad(T) ) + q
```

se stejnou stabilizací SUPG konvektivního členu jako řešič proudění a se
stejným časovým krokováním theta. Rychlost `v` se **získává z výsledku
proudění**, zde se neřeší - proto tento typ úlohy vyžaduje krok
*Nestlačitelné viskózní proudění*.

| Symbol | Význam | Jednotky | Zdroj |
|---|---|---|---|
| `T` | teplota | `K` | řešená veličina |
| `v` | rychlost | `m/s` | výsledek proudění |
| `k` | tepelná vodivost | `W/(m*K)` | materiálová vlastnost |
| `c` | tepelná kapacita | `J/(kg*K)` | materiálová vlastnost |
| `rho` | hustota | `kg/m^3` | materiálová vlastnost |
| `q` | hustota zdroje tepla | `W/m^3` | podmínka Heat, Jouleovo teplo, záření |

Rozdíl oproti obyčejnému typu úlohy **Přenos tepla** (*Heat transfer*) je
právě člen `v . grad(T)`: energie je unášena pohybující se tekutinou, nejen
vedena. To také znamená, že tento řešič nemá okrajové podmínky konvekce - není
co korelovat, protože teplo přecházející ze stěny do tekutiny je rozlišeno
sítí, nikoli modelováno.

### 1.3 Okrajové podmínky

| Okrajová podmínka | Typ | Použití na | Složky |
|---|---|---|---|
| Temperature | explicitní (Dirichletova) | bod, čára, povrch, objem | Temperature `[K]` |
| Heat | přirozená, zdroj | bod, čára, povrch, objem | Heat `[W]` |

**Temperature** (teplota) drží uzly své entity na zadané hodnotě. Přiřaďte ji
vstupnímu povrchu, abyste nastavili teplotu tekutiny vstupující do oblasti, a
stěně, abyste modelovali ohřívanou nebo chlazenou hranici.

**Heat** (teplo) je zdroj a přijímá **celkový výkon ve `W`** dodaný do entity,
rozložený před sestavením na míru entity - stejná konvence jako u tepelného
řešiče. *Heat rate (unit area)* a *Heat rate (unit volume)* rozhraní pro tento
typ úlohy nabízí, řešič tepla v tekutinách je však **nečte**; jako zdroj zde
použijte *Heat*.

Každá složka je **tabulka v závislosti na čase**.

Podmínky proudění - stěny, vstupy a výstupy - patří kroku proudění a popisuje
je příručka [Proudění tekutin](theory_fluid.md).

### 1.4 Sdružený přenos tepla

Stěna mezi síťovanou tekutinou a síťovanou pevnou látkou, která nese podmínku
**Forced convection** (nucená konvekce) obyčejného řešiče *Přenos tepla*, obě
části propojuje. Jakmile proběhne tepelný řešič, řešič tepla v tekutinách drží
uzly stěny na teplotě pevné látky - do té doby je stěna izolovaná - a pro každý
prvek stěny vrací dvojici `(h, Tf)`, která reprodukuje tepelný tok, jenž jeho
řešení stěnou propouští. Tepelný řešič tuto dvojici použije jako podmínku
konvekce. Tok je reziduem rovnic tekutiny v uzlech stěny, konzistentním s
diskretizací, takže zůstává přesný i v tenké tepelné mezní vrstvě. Teplota
stěny se mezi průchody relaxuje Aitkenovým faktorem, takže se výměna ustálí
během několika průchodů. Schéma popisuje příručka [Přenos tepla](theory_heat.md)
v části *Sdružený přenos tepla*. Teplo záření a Jouleovo teplo přicházející z
jiných řešičů se přičítají ke zdrojovému členu stejně jako v tepelném řešiči.

### 1.5 Konvergence

Bez vazby se řešič tepla v tekutinách hlásí jako zkonvergovaný bezpodmínečně -
při pevném proudovém poli je lineární, takže jedno řešení je konečnou
odpovědí. Skupina obsahující krok proudění i krok tepla v tekutinách proto
skončí, jakmile zkonverguje krok proudění. Se svázanými stěnami místo toho
hlásí relativní změnu svého teplotního pole, takže skupina iteruje, dokud se
teplota stěny neustálí.

### 1.6 Zpětná vazba na proudění - vztlak

Teplota, kterou tento řešič vytváří, se předává zpět proudění. Při každém
průchodu vyhodnotí řešič proudění hustotu a dynamickou viskozitu každého prvku
při poslední teplotě, takže u materiálu, jehož hustota závisí na teplotě,
pohání objemová síla `rho(T)*g` **přirozenou konvekci** - teplá tekutina stoupá,
studená klesá. Dodávané materiály *Water* a *Air* mají hustotu závislou na
teplotě.

V ustáleném výpočtu vidí první průchod proudění pouze počáteční teplotu, takže
se vztlak projeví až od druhé iterace skupiny; dejte skupině několik iterací a
hodnotu konvergence. V přechodovém výpočtu s jednou iterací na časový krok
vidí proudění teplotu předchozího kroku. Schéma popisuje příručka
[Proudění tekutin](theory_fluid.md) v části 1.9.

### 1.7 Počáteční podmínky

| Počáteční podmínka | Účinek |
|---|---|
| Temperature | počáteční teplotní pole |

Počáteční podmínky se čtou **pouze při prvním výpočtu**; poté pole pokračuje
od předchozího výsledku. Tam, kde se setkají, má okrajová podmínka přednost
před počáteční podmínkou.

### 1.8 Výsledky

| Výsledek | Použití na | Význam |
|---|---|---|
| Temperature `[K]` | uzel | vyřešené teplotní pole |
| Heat flux `[W/m^2]` | prvek | konduktivní tok `-k*grad(T)` |

---

## 2. Grafické uživatelské rozhraní

Vše podstatné se nachází v panelu **Řešič** (záložky `Úloha`,
`Okrajové podmínky`, `Počáteční podmínky`, `Materiál`, `Výsledky`), v nabídce
**Úloha** a v panelu **Model**.

### 2.1 Výběr úlohy

`Úloha` -> `Pořadí kroků úloh` (`Ctrl+P`) otevře dialog pořadí kroků úlohy.
Kombinace, které stojí za to znát:

- *Incompressible viscous flow* a *Heat transfer in fluids* v jedné skupině -
  ohřívané nebo chlazené proudění. Pokud je proudění vztlakem poháněno nebo
  znatelně ovlivněno, dejte skupině několik iterací, aby se proudění a teplota
  ustálily společně - viz 1.6;
- *Heat transfer in fluids* a samostatný krok *Heat transfer* na síťované
  pevné látce, krok tepla v tekutinách jako první, v jedné skupině s několika
  iteracemi - sdružený přenos tepla stěnami s podmínkou *Forced convection*.

Tento typ úlohy vyžaduje přítomnost kroku proudění. Hodnoty skupiny kroků -
**počet iterací** a **Konvergence** - popisuje příručka
[Proudění tekutin](theory_fluid.md).

### 2.2 Záložka Úloha

**Řešič v čase** - zobrazuje se, protože typ úlohy je časově závislý. Pro
ustálený výpočet ho nechte vypnutý. Jeho pole popisuje příručka
[Proudění tekutin](theory_fluid.md).

Typ úlohy nemá vlastní skupinu nastavení. Vše ostatní se přiřazuje pro každou
entitu v záložkách podmínek.

### 2.3 Záložka Materiál

| Vlastnost | Jednotky | Poznámka |
|---|---|---|
| Density | `kg/m^3` | `rho`, asi `1000` pro vodu, `1.2` pro vzduch |
| Dynamic viscosity | `kg/(m*s)` | `mu`, asi `1.0e-3` pro vodu, `1.8e-5` pro vzduch |
| Thermal conductivity | `W/(m*K)` | `k`, asi `0.6` pro vodu, `0.025` pro vzduch |
| Heat capacity | `J/(kg*K)` | `c`, asi `4180` pro vodu, `1005` pro vzduch |

Entita, které chybí některá požadovaná vlastnost, se neřeší a kontrola
nastavení na to upozorní před spuštěním výpočtu. Dodávaná databáze materiálů
obsahuje **Water** a **Air**, což je obvykle celý tento krok. Každá vlastnost
je tabulkou v závislosti na teplotě, vyhodnocovanou při teplotě prvku z
předchozího výpočtu.

### 2.4 Záložka Okrajové podmínky

**Temperature** a **Heat** se zobrazí, jakmile je v pořadí kroků krok
*Heat transfer in fluids* - viz část 1.3. Každá složka je tabulkou v závislosti
na čase; kliknutím na **Upravit časově závislé hodnoty** zadáte hodnoty v
závislosti na čase.

### 2.5 Počáteční podmínky

Záložka `Počáteční podmínky` nabízí *Temperature*. Nastavuje výchozí pole
prvního výpočtu.

### 2.6 Nastavení maticového řešiče

`Úloha` -> `Nastavit maticový řešič úloh` nastavuje iterační řešiče. Řešič
tepla v tekutinách používá položku **GMRES**, protože jeho matice není
symetrická.

### 2.7 Monitorovací body a výsledky

`Úloha` -> `Definovat monitorovací body` umí zaznamenávat *Temperature*.
Záložka `Výsledky` zobrazuje teplotu jako uzlový skalár a tepelný tok jako
prvkový vektor.

---

## 3. Omezení

- **Vztlak bez roztažnosti tekutiny.** Teplota mění proudění prostřednictvím
  `rho(T)` a `mu(T)` (viz 1.6), ale tepelná roztažnost tekutiny nikdy nevstupuje
  do bilance hmoty `div(v) = 0`. To je přesné pro malé rozdíly hustoty u
  přirozené konvekce, nikoli pro plyn ohřátý o stovky kelvinů. Vztlakové
  proudění je laminární, jako každé proudění v Range FEA.
- **Vazba je střídavá, nikoli monolitická.** Proudění a teplota se řeší jedno
  po druhém a data si vyměňují mezi průchody. Ustálené vztlakové proudění
  potřebuje skupinu s několika iteracemi; při jedné iteraci na časový krok se
  přechodové proudění za teplotou zpožďuje o jeden krok.
- **Podmínky `Heat rate` se ignorují.** *Heat rate (unit area)* a *Heat rate
  (unit volume)* rozhraní pro *Heat transfer in fluids* nabízí, ale
  `RSolverFluidHeat` je nečte; dostane se k němu pouze podmínka *Heat*.
  Obyčejný typ úlohy *Heat transfer* čte všechny tři.
- **Žádné okrajové podmínky konvekce.** Teplo přecházející ze stěny do síťované
  tekutiny je rozlišeno, nikoli korelováno, takže podmínky *Simple*, *Forced* a
  *Natural convection* patří obyčejnému tepelnému řešiči. Sdružený přenos tepla
  se nastavuje jako krok *Heat transfer* na pevné látce, jejíž stěny s podmínkou
  *Forced convection* čtou vypočtený stav tekutiny.
- **Žádný model turbulence.** Proudění je laminární, viz příručka
  [Proudění tekutin](theory_fluid.md), takže turbulentní přenos tepla není
  zachycen.

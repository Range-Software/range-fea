# Rozptyl kontaminantu - Teoretická příručka a uživatelská příručka

Tento dokument popisuje řešič rozptylu kontaminantu v Range FEA -
`RSolverFluidParticle`, který obsluhuje typ úlohy **Rozptyl kontaminantu**
(*Contaminant dispersion*): rovnici, kterou řeší, význam každého vstupu, který
přijímá, části grafického uživatelského rozhraní, které ho ovládají, a
podrobný návod.

Rozptyl kontaminantu se řeší na proudovém poli kroku **Nestlačitelné viskózní
proudění** (*Incompressible viscous flow*). Řešič proudění, jeho okrajové
podmínky, nelineární iterace a časový krok popisuje příručka
[Proudění tekutin](theory_fluid.md), na kterou tato příručka navazuje.

**Obsah**

1. [Teoretický základ](#1-teoretický-základ)
2. [Grafické uživatelské rozhraní](#2-grafické-uživatelské-rozhraní)
3. [Návod - přechodové proudění s rozptylem kontaminantu](#3-návod---přechodové-proudění-s-rozptylem-kontaminantu)
4. [Kontrola modelu](#4-kontrola-modelu)
5. [Omezení](#5-omezení)

---

## 1. Teoretický základ

### 1.1 Primární neznámá - koncentrace částic

Primární neznámou je **koncentrace částic** `C` v `kg/m^3`, jeden stupeň
volnosti na uzel.

| Typ úlohy | Řešič | Primární neznámá | Jednotky | Stupně volnosti na uzel |
|---|---|---|---|---|
| Rozptyl kontaminantu | `RSolverFluidParticle` | koncentrace částic | `kg/m^3` | 1 |

Tento typ úlohy **vyžaduje** ve stejné úloze krok *Nestlačitelné viskózní
proudění*: pole rychlosti čte z jeho výsledku a samo ho nikdy nepočítá.

### 1.2 Řídicí rovnice

**Rozptyl kontaminantu** přenáší skalární koncentraci po proudovém poli,

```
dC/dt + v . grad(C) = div( D * grad(C) ) + s_eff

s_eff = s * ( 1 - C / C_sat )     for s > 0 and a maximum saturation set
s_eff = s                         otherwise
```

se stabilizací SUPG a časovým krokováním theta řešiče proudění; rychlost čte z
výsledku proudění. (V rovnici výše: `for s > 0 and a maximum saturation set`
znamená „pro `s > 0` a nastavené maximální nasycení“, `otherwise` znamená
„jinak“.)

| Symbol | Význam | Jednotky | Zdroj |
|---|---|---|---|
| `C` | koncentrace částic | `kg/m^3` | řešená veličina |
| `v` | rychlost | `m/s` | výsledek proudění |
| `D` | koeficient difúze | `m^2/s` | *Nastavení rozptylu kontaminantu*, výchozí `0` |
| `C_sat` | maximální nasycení | `kg/m^3` | *Nastavení rozptylu kontaminantu*, výchozí `0` (bez omezení) |
| `s` | intenzita zdroje částic | `kg/(m^3*s)` | podmínka Particle rate |

### 1.3 Nastavení rozptylu kontaminantu

Oba parametry typu úlohy se nastavují jednou pro model ve skupině
*Nastavení rozptylu kontaminantu* záložky `Úloha` (část 2.2). Oba mají výchozí
hodnotu nula, což odpovídá čisté advekci se zdrojem bez omezení.

#### Koeficient difúze

**Koeficient difúze** `D` rozptyluje kontaminant ve směru poklesu jeho
koncentrace. Při `D = 0` je přenos čistě advektivní: kontaminant je unášen
prouděním a šíří se pouze numerickou difúzí stabilizace, takže řešič odpovídá
na otázku „kam ho proudění zanese a jak dlouho to trvá“, ne „jak široký je
oblak“. Kladné `D` činí z šířky oblaku fyzikální výsledek.

Model turbulence není k dispozici (část 5), proto je `D` **efektivní**
difuzivita: míchání ve skutečném proudění je určeno turbulencí, kterou lze
odhadnout jako `D ~ nu_t / Sc_t` s turbulentním Schmidtovým číslem
`Sc_t ~ 0.7`.

| Molekulární difúze (fyzikální vlastnost) | `D [m^2/s]` |
|---|---|
| plyny a páry ve vzduchu (vodní pára `2.5e-5`, CO2 `1.6e-5`, páry rozpouštědel asi `1e-5`) | `1e-6` - `1e-4` |
| látky rozpuštěné ve vodě (O2 `2e-9`, soli asi `1.5e-9`) | `1e-10` - `1e-8` |
| aerosolové částice ve vzduchu, Brownův pohyb (10 nm `5e-8`, 0.1 um `7e-10`, 1 um `3e-11`) | `1e-11` - `1e-8` |

V inženýrských měřítkách je molekulární difúze vedle přenosu prouděním téměř
vždy zanedbatelná. Obvykle se zadává efektivní hodnota:

| Efektivní (turbulentní) difúze | `D_eff [m^2/s]` |
|---|---|
| větraná místnost, míchání vnitřního vzduchu | `1e-3` - `1e-2` |
| proudění v potrubí nebo kanálu, přibližně `D ~ 0.01 - 0.05 * U * L` (`U` střední rychlost, `L` rozměr potrubí) | `1e-3` - `1e-1` |
| řeky, příčné míchání | `1e-2` - `1` |
| atmosféra, horizontální míchání | `1` - `100` |

Zda na hodnotě záleží, závisí na síti. Pro střední rychlost `U`, délku
proudění `L`, velikost prvku `h` a časový krok `dt`:

- **kdy má `D` viditelný účinek** - kontaminant se během průchodu oblastí
  rozšíří přibližně o `sqrt( 2 * D * L / U )`. Je-li to méně než `h`, `D`
  nemá žádný účinek, takže `D` je podstatné až nad zhruba
  `U * h^2 / ( 2 * L )`. Pro kanál s `U ~ 1 m/s`, `L ~ 15 m` a `h ~ 0.1 m` je to
  asi `3e-4 m^2/s`;
- **kdy `D` převládá** - nad přibližně `U * L` (asi `15 m^2/s` pro stejný
  kanál) difúze vše rozmaže a na proudění téměř nezáleží;
- **časové krokování** - pokud `D * dt / h^2` překročí přibližně `1`, přepněte
  na *Backward difference (stable)*; centrální diference netlumí nejrychlejší
  difuzní módy a osciluje.

#### Maximální nasycení

**Maximální nasycení** `C_sat` je největší koncentrace, které může kontaminant
v tekutině dosáhnout - koncentrace nasycené páry nebo rozpustnost. Je-li
nastaveno, kladná *Particle rate* slábne, jak se koncentrace k této hodnotě
blíží, a při nasycení se zastaví, `s * ( 1 - C / C_sat )`; tím se modeluje
odpařování nebo rozpouštění a `s` je pak intenzita do čisté tekutiny, přibližně
`k_m * C_sat * A / V`. Propady (`s < 0`) omezeny nejsou. Omezení působí jen na
zdroj: koncentrace předepsaná okrajovou nebo počáteční podmínkou omezena není a
log řešiče upozorní, pokud překročí `C_sat`. Koncentrace se ani neořezává, takže
mírný překmit u strmého čela je stále možný.

Mez nasycení existuje jen tam, kde existuje termodynamická mez: tlak par nebo
rozpustnost. Pro páru ve vzduchu plyne koncentrace nasycení z tlaku nasycených
par, `C_sat = p_sat * M / ( R * T )`:

| Pára ve vzduchu | Teplota | `C_sat [kg/m^3]` |
|---|---|---|
| vodní pára | 0 degC | `0.0048` |
| vodní pára | 20 degC | `0.017` |
| vodní pára | 40 degC | `0.051` |
| vodní pára | 100 degC | `0.60` |
| ethanol nebo toluen | 20 degC | asi `0.11` |
| methanol | 20 degC | asi `0.17` |
| benzen | 20 degC | asi `0.32` |
| aceton | 20 degC | asi `0.59` |

Pro látku rozpuštěnou ve vodě je mezí její rozpustnost:

| Rozpuštěno ve vodě | `C_sat [kg/m^3]` |
|---|---|
| kyslík v rovnováze se vzduchem, 20 degC | `0.009` |
| oxid uhličitý při 1 atm | `1.7` |
| sádrovec | `2.4` |
| chlorid sodný | asi `360` |

Částice, prach a kouř nasycení nemají; ponechte hodnotu `0` (bez omezení). Jen
pro představu: expoziční limity na pracovišti jsou asi
`1e-6` - `1e-5 kg/m^3` (1 - 10 mg/m^3) a dolní meze výbušnosti prachu asi
`0.02` - `0.06 kg/m^3`.
Nasycení silně závisí na teplotě, kdežto řešič používá jednu konstantní
hodnotu: v neizotermickém případě použijte hodnotu při nejnižší relevantní
teplotě, kde by začala kondenzace.

### 1.4 Stabilizace

Parametr SUPG řešiče rozptylu je omezen časovým krokem a zahrnuje difúzi,

```
tau = [ ( 2 / dt )^2 + ( 2 * |v| / h )^2 + 9 * ( 4 * D / h^2 )^2 ]^(-1/2)
```

přičemž v ustálené analýze se člen s `dt` vynechává. Tam, kde je proudění
rychlé, se redukuje na `h / ( 2 * |v| )`, nikdy nepřekročí `dt / 2` a tam, kde
převládá difúze, se blíží `h^2 / ( 12 * D )` - limitě uzlově přesného
jednorozměrného parametru. Omezení časovým krokem je podstatné v pomalých a
recirkulačních oblastech: tam je Courantovo číslo prvku `|v| * dt / h` velmi
malé a neomezené `tau` by ze stabilizace udělalo šum, který vytváří
koncentraci před čelem. Tento parametr je vlastní řešiči rozptylu; řešič
proudění používá parametr popsaný v příručce [Proudění tekutin](theory_fluid.md).

Galerkinův advekční člen se sestavuje v **advektivním tvaru s divergenčním
propadem**, `v . grad(C) + 0.5 * max(div(v), 0) * C`, se spojitou uzlovou
rychlostí. Rychlost z řešiče proudění nikdy není přesně bezdivergenční - na
hrubé síti je u stěn a rohů běžná `div(v)` v řádu desítek `1/s`. Tam, kde se
proudění rozpíná (`div(v) > 0`), by prostá advekce sama vytvářela koncentraci -
šachovnicový vzor, který při malém časovém kroku roste krok od kroku; propad ho
ruší. Tam, kde proudění konverguje, se žádný člen nepřidává: symetrická oprava
`0.5 * div(v) * C` by tam působila jako zdroj a hromadila by kontaminant u
stěn. Pro přesně bezdivergenční rychlost je propad nulový.

Při zanedbatelném koeficientu difúze je přenos téměř čistě advektivní, což
Galerkinova metoda se SUPG neudrží monotónní: počítejte s překmity a
podkmity, největšími u stěn bez skluzu, kde je rychlost nulová. Efektivní
koeficient difúze (část 1.3) většinu z nich odstraní.

Na rozdíl od řešiče proudění používá řešič rozptylu váhu theta konzistentně,
takže *Central difference (accurate)* je zde skutečné Crankovo-Nicolsonové
schéma.

### 1.5 Okrajové podmínky

| Okrajová podmínka | Typ | Použití na | Složky |
|---|---|---|---|
| Particle concentration | explicitní (Dirichletova) | bod, čára, povrch | Particle concentration `[kg/m^3]` |
| Particle rate | přirozená, zdroj | bod, čára, povrch, objem | Particle rate `[kg/(m^3*s)]` |
| Particle flux (unit area) | přirozená (Neumannova) | povrch | Particle flux `[kg/(m^2*s)]` |
| Particle mass transfer | přirozená (Robinova) | povrch | Mass transfer coefficient `[m/s]`, Ambient particle concentration `[kg/m^3]` |

**Particle concentration** (koncentrace částic) drží své uzly na zadané
koncentraci. Přiřazená vstupnímu povrchu s tabulkou v závislosti na čase je
způsobem, jak modelovat únik: nula před událostí, koncentrace úniku během ní a
nula po ní. Jde o vůbec nejužitečnější postup tohoto typu úlohy a staví na něm
návod v části 3.

**Particle rate** (intenzita částic) je zdroj a jde o **hustotu** v
`kg/(m^3*s)`, nikoli o celkovou hodnotu - na rozdíl od podmínky *Heat* se
nerozkládá na míru entity. Modeluje zdroj, který uvnitř oblasti uvolňuje
kontaminant nepřetržitě.

**Particle flux (unit area)** (tok částic na jednotku plochy) předepisuje
hmotnostní tok `q` vstupující do oblasti povrchem, `D * grad(C) . n = q`, kde
`n` je vnější normála. Na rozdíl od *Particle rate* jde o tok na jednotku
plochy, takže celkový únik je `q * A`. Kladné `q` je emise - odpařující se
film, odplyňující stěna - a omezuje ho maximální nasycení stejně jako kladnou
intenzitu částic, `q * ( 1 - C / C_sat )`. Záporné `q` je pevná rychlost
odebírání bez ohledu na místní koncentraci a může ji stlačit pod nulu; depozice
úměrná koncentraci je *Particle mass transfer* s `C_inf = 0`. Na stěně tok
vstupuje pouze difúzí; na části hranice, kudy proudění vstupuje, se přítok
považuje za čistou tekutinu nesoucí tok, takže `q` se stává celkovým
vstupujícím hmotnostním tokem a `C ~ q / |v . n|`. Přiřazený vnitřnímu povrchu
mezi dvěma objemy uvolňuje `q` na jednotku plochy jako plošný zdroj.

**Particle mass transfer** (přestup hmoty částic) vyměňuje kontaminant s
okolním rezervoárem, `q = k_m * ( C_inf - C )`, kde `k_m` je
*Mass transfer coefficient* (součinitel přestupu hmoty) a `C_inf` je
*Ambient particle concentration* (okolní koncentrace částic). Je obdobou
jednoduché konvekce v přenosu tepla. S `C_inf = 0` modeluje depozici na povrchu
s depoziční rychlostí `k_m`; s `C_inf > 0` výměnu s okolním vzduchem otvorem
nebo rozpouštějící se povrch držený na `C_inf`. Typické hodnoty `k_m` jsou
`1e-4` - `1e-2 m/s` pro depozici částic v místnostech a `1e-3` - `1e-1 m/s` pro
odpařování do proudícího plynu. Velké `k_m` žene povrch k `C_inf` a blíží se
podmínce *Particle concentration*.

Každá složka je **tabulkou v závislosti na čase**, takže únik může během
přechodového výpočtu začít i skončit.

**Hranice bez podmínky.** Stěny a výstupy bez podmínky *Particle
concentration*, *Particle flux* nebo *Particle mass transfer* si ponechávají
přirozenou podmínku **nulového difuzního toku**: kontaminant stěnou nedifunduje
a výstupem odchází pouze s prouděním.

Tam, kde proudění takovou hranicí **vstupuje** - typicky zpětné proudění v
části výstupu *Pressure (implicit)* - se považuje za čistou tekutinu: řešič na
této části povrchu přidá slabý přítokový člen `max(-v . n, 0) * C`, rozložený
po stěně tak, aby tlumil i okrajové uzly, kde se výstup stýká se stěnou bez
skluzu.
Bez něj by hranice řešení napájela a koncentrace na výstupu by rostla nade
všechny meze. Pokud vracející se tekutina není čistá, prodlužte oblast tak,
aby výstup viděl pouze odtok.

Podmínky proudění - stěny, vstupy a výstupy - patří kroku proudění a popisuje
je příručka [Proudění tekutin](theory_fluid.md).

### 1.6 Počáteční podmínky

| Počáteční podmínka | Účinek |
|---|---|
| Particle concentration | počáteční pole koncentrace |

Počáteční podmínky se čtou **pouze při prvním výpočtu**; poté pole pokračuje
od předchozího výsledku. Tam, kde se setkají, má okrajová podmínka přednost
před počáteční podmínkou.

### 1.7 Ustálený, nebo přechodový výpočet?

Rozptyl kontaminantu je především **přechodová** úloha: úloha přenosu v čase,
která pro jednorázový únik nemá smysluplnou ustálenou odpověď. Ustálený
rozptyl odpovídá na jinou otázku - na jaké pole koncentrace se ustálí
nepřetržitý zdroj - a jde o jediné lineární řešení na proudovém poli (viz
část 2.6 o jeho maticovém řešiči).

Dvoukrokový postup návodu v části 3 je ten kanonický: nechat ustálené proudění
zkonvergovat a pak výpočet restartovat se zapnutým řešičem v čase a krokem
rozptylu.

### 1.8 Výsledky

| Výsledek | Použití na | Význam |
|---|---|---|
| Particle concentration `[kg/m^3]` | uzel | vyřešené pole koncentrace |
| Relative saturation `[-]` | uzel | `C / C_sat`, pouze je-li nastaveno maximální nasycení |

Relativní nasycení se ve výchozím stavu zobrazuje od `0` do `1` a odstraní se,
když se maximální nasycení nastaví zpět na nulu.

Pro tento typ úlohy je potřeba pouze materiálová vlastnost **hustota**
(*Density*).

---

## 2. Grafické uživatelské rozhraní

Vše podstatné se nachází v panelu **Řešič** (záložky `Úloha`,
`Okrajové podmínky`, `Počáteční podmínky`, `Materiál`, `Výsledky`), v nabídce
**Úloha** a v panelu **Model**.

### 2.1 Výběr úlohy

`Úloha` -> `Pořadí kroků úloh` (`Ctrl+P`) otevře dialog pořadí kroků úlohy.
Vložte krok **Contaminant dispersion** do stejné skupiny kroků jako krok
**Incompressible viscous flow** - rozptyl na proudění, klasická přechodová
dvojice. Tento typ úlohy vyžaduje přítomnost kroku proudění.

Hodnoty skupiny kroků - **počet iterací** a **Konvergence** - řídí nelineární
iteraci proudění a popisuje je příručka [Proudění tekutin](theory_fluid.md).
V přechodovém výpočtu jde o iterace *na jeden časový krok*.

### 2.2 Záložka Úloha

**Řešič v čase** - pro ustálený rozptyl ho nechte vypnutý, pro jednorázový
únik ho zapněte.

| Pole | Význam |
|---|---|
| Zapnutí | zapíná a vypíná přechodový výpočet |
| Aproximace | theta časového krokování: zpětná, centrální nebo dopředná |
| Počáteční čas | první hodnota času v sekundách |
| Koncový čas | `start time + time-step size * number of time-steps`, pouze pro čtení |
| Velikost časového kroku | `dt` v sekundách |
| Počet časových kroků | kolik kroků se má vypočítat |
| Frekvence výstupu | zapsat soubor výsledků každých N kroků; `0` zapíše pouze poslední krok |

**Nastavení rozptylu kontaminantu** - zobrazí se, jakmile je v pořadí kroků
krok *Contaminant dispersion*. Obsahuje oba parametry z části 1.3:

| Pole | Význam |
|---|---|
| Maximální nasycení `[kg/m^3]` | největší možná koncentrace; kladná intenzita částic se na ní zastaví a počítá se *Relative saturation*. `0` znamená bez omezení |
| Koeficient difúze `[m^2/s]` | efektivní difuzivita kontaminantu v tekutině. `0` znamená čistou advekci |

Typické hodnoty obou jsou uvedeny v části 1.3.

### 2.3 Záložka Materiál

| Vlastnost | Jednotky | Poznámka |
|---|---|---|
| Density | `kg/m^3` | `rho`, asi `1000` pro vodu, `1.2` pro vzduch |

Krok proudění ve stejné úloze potřebuje navíc dynamickou viskozitu. Dodávaná
databáze materiálů obsahuje **Water** a **Air**, což je obvykle celý tento
krok.

### 2.4 Záložka Okrajové podmínky

Podmínky z části 1.5 se nabízejí, jakmile je v pořadí kroků krok
*Contaminant dispersion*. Nejprve vyberte entitu ve stromu `Model`; správce pak
nabídne podmínky platné pro daný typ entity. Zaškrtnutím ji přiřadíte a poté
upravte její složky ve spodním stromu.

Každá složka je tabulkou v závislosti na čase. Kliknutím na **Upravit časově
závislé hodnoty** otevřete editor složek, kde se hodnoty zadávají v závislosti
na čase; mezi dvěma zadanými časy se hodnota lineárně interpoluje a po
posledním čase zůstává poslední hodnota. Takto se modeluje jednorázový únik.

### 2.5 Počáteční podmínky

Záložka `Počáteční podmínky` nabízí *Particle concentration*. Nastavuje
výchozí pole prvního výpočtu.

### 2.6 Nastavení maticového řešiče

`Úloha` -> `Nastavit maticový řešič úloh` nastavuje iterační řešiče. Řešič
rozptylu používá položku **GMRES**, protože jeho matice není symetrická.

Pozornost zde vyžaduje **ustálený rozptyl kontaminantu s difúzí**. Jde o
jediné lineární řešení, takže nejsou k dispozici iterace skupiny kroků, o které
by se dalo opřít, a soustava s převládající difúzí konverguje pomalu při
výchozím limitu GMRES 10 vnitřních krát 10 vnějších iterací. Maticový řešič
neupozorní, když dosáhne svého limitu: přečtěte si jeho tabulku iterací v logu
řešiče, a pokud dojde až k poslední vnější iteraci s reziduem stále výrazně nad
hodnotou konvergence řešiče, zvyšte počet vnějších iterací. Nezkonvergované
řešení dává špatnou odpověď bez jakéhokoli dalšího příznaku.

### 2.7 Monitorovací body

`Úloha` -> `Definovat monitorovací body` umístí sondy na zadané souřadnice a
vybere proměnnou, která se má zaznamenávat - *Particle concentration*,
*Particle rate* nebo *Relative saturation*. V přechodovém výpočtu zobrazí
historii `Zprávy` -> `Monitorovací body`.

Monitorovací bod v místě zájmu je odpověď ve tvaru, v jakém ji obvykle
chcete: koncentrace v závislosti na čase v daném místě, odečtená z grafu, a ne
ze sledu obrázků.

### 2.8 Výsledky a záznamy

Záložka `Výsledky` uvádí vypočtené proměnné a ovládá 3D zobrazení.
Koncentrace částic a relativní nasycení jsou uzlové skaláry.

Záložka `Záznamy` panelu `Model` uvádí záznamy výsledků - jeden pro každý
zapsaný časový krok přechodového výpočtu. Animace záznamů přehraje rozptyl a
tlačítko **Záznam** v dolní části stromu zapíše animaci do souboru videa.

---

## 3. Návod - přechodové proudění s rozptylem kontaminantu

**Cíl.** Uvolnit kontaminant do zkonvergovaného proudění a sledovat oblak v
čase, jak prochází kanálem.

Jde o dvoukrokový postup z části 1.7 a odpovídá dodávanému návodu
*Rozptyl kontaminantu v tekutinách*.

### Krok 1 - začněte od zkonvergovaného proudění

Nejprve nechte zkonvergovat ustálený návod s kanálem z příručky
[Proudění tekutin](theory_fluid.md). Výpočet rozptylu spuštěný z klidu promrhá
první časové kroky vývojem proudového pole místo přenosu čehokoli.

### Krok 2 - přidejte krok rozptylu

1. `Úloha` -> `Pořadí kroků úloh` (`Ctrl+P`).
2. Ponechte skupinu kroků a krok **Incompressible viscous flow**.
3. Přidejte krok **Contaminant dispersion** do stejné skupiny.
4. Snižte **počet iterací** skupiny na hodnotu kolem `20` - nyní jde o
   nelineární iterace *na jeden časový krok*, nikoli o iterace ustáleného
   řešení. Hodnotu **Konvergence** neměňte; většina kroků se ustálí před
   dvacátým průchodem a skončí dříve, a ty, které ne, za plných dvacet stojí.

*Contaminant dispersion* vyžaduje přítomnost kroku proudění; pole rychlosti čte
a nikdy ho nepočítá.

### Krok 3 - nastavte řešič v čase

Otevřete záložku `Úloha` a ve skupině *Řešič v čase* nastavte:

| Pole | Hodnota |
|---|---|
| Zapnutí | zaškrtnuto |
| Aproximace | `Backward difference (stable)` |
| Počáteční čas | `0` |
| Velikost časového kroku | podle Courantova doporučení níže |
| Počet časových kroků | dost na to, aby oblak prošel oblastí |
| Frekvence výstupu | `1` pro plynulou animaci, více pro menší soubor |

Časový krok zvolte podle sítě a rychlosti proudění tak, aby tekutina za jeden
krok prošla přibližně jedním prvkem:

```
dt ~ h / |v|
```

Počet kroků pak zvolte podle doby, kterou oblak potřebuje k průchodu oblastí,
`L / |v|`, dělené tímto krokem.

Ve skupině *Nastavení rozptylu kontaminantu* téže záložky ponechte pro čistou
advekci obě hodnoty na `0`, nebo zadejte efektivní **Koeficient difúze** z
tabulky v části 1.3, pokud záleží na šířce oblaku. **Maximální nasycení** má
smysl jen pro páru nebo rozpuštěnou látku; nesmí být nižší než koncentrace
úniku v kroku 4.

### Krok 4 - uvolněte kontaminant

1. Vyberte entitu vstupního povrchu.
2. Otevřete záložku `Okrajové podmínky` a zaškrtněte **Particle concentration**.
3. Místo zadání jediné hodnoty klikněte na **Upravit časově závislé hodnoty**.
4. V editoru složek zadejte profil úniku - například `0` v čase `0`,
   koncentraci úniku v okamžiku, kdy únik začíná, a opět `0`, když končí.

Mezi dvěma zadanými časy se hodnota lineárně interpoluje a po posledním čase
zůstává poslední hodnota, takže únik, který má začít náhle, potřebuje dvě
položky těsně u sebe. Právě tato časově závislá podmínka mění ustálený vstup
v jednorázový únik a je jádrem modelu rozptylu.

### Krok 5 - ponechte podmínky proudění

**Wall**, podmínka přítoku a výstup **Pressure (implicit)** zůstávají beze
změny. Proudění se stále řeší v každém časovém kroku - rozptyl se po něm veze.

### Krok 6 - přidejte monitorovací bod

`Úloha` -> `Definovat monitorovací body`, umístěte bod po proudu a nastavte
jeho proměnnou na **Particle concentration**. Čas příchodu a špičkovou
koncentraci v daném místě je mnohem snazší odečíst z historie než ze sledu
barevných obrázků.

### Krok 7 - řešte s restartem

1. `Řešení` -> `Spustit řešič` (`Ctrl+R`).
2. Zaškrtněte **Restartovat řešič / pokračovat**.

Díky restartu se zkonvergované ustálené pole stane výchozím bodem krokování.
Bez něj začne výpočet z klidu a první kroky přijdou nazmar.

### Krok 8 - prohlédněte si výsledky

- Panel `Model`, záložka `Záznamy`: jeden záznam na každý zapsaný krok.
  Procházejte je nebo je animujte a sledujte, jak oblak putuje.
- Záložka `Výsledky`: zobrazte **Particle concentration**. Pevně nastavte
  rozsah zobrazení pro všechny záznamy, aby barvy v každém snímku znamenaly
  totéž - jinak se animace sama přeškáluje a oblak se zdánlivě nezeslabuje.
- `Zprávy` -> `Monitorovací body` vykreslí historii koncentrace v sondě.
  Odečtěte z ní čas příchodu a porovnejte ho s `L / |v|`, dobou průchodu,
  kterou udává střední proudění - to je nejlevnější ověření celého modelu.
- Při nulovém koeficientu difúze se oblak šíří jen numerickou difúzí, takže
  jeho **šířka** je vlastností sítě a časového kroku, nikoli fyziky. S
  efektivním koeficientem difúze je šířka výsledkem - než jí budete věřit,
  ověřte ji na jemnější síti.

### Řešení problémů

| Příznak | Příčina |
|---|---|
| Koncentrace nikdy neopustí vstup | proudové pole je nulové - ve skupině chybí krok proudění, nebo výpočet nebyl restartován ze zkonvergovaného proudění |
| Oblak se téměř okamžitě rozmaže | Courantovo číslo je výrazně nad 1, nebo je síť podél dráhy příliš hrubá |
| Koncentrace je záporná nebo překmitává | časový krok je pro síť příliš velký, takže stabilizovaná advekce zvoní. Mírné podkmity u strmého čela jsou normální; řešení se neořezává na nulu, protože ořezání by kontaminant přidávalo |
| Zapsal se jen jeden záznam | frekvence výstupu je `0`, což zapíše pouze poslední krok |
| Reziduum každého kroku zůstává vysoké | příliš málo iterací na časový krok - proudění v jednotlivých krocích není zkonvergované |
| Oblak se nikdy nerozšíří do stran | koeficient difúze je nulový, takže boční šíření pochází jen z proudového pole - zadejte efektivní koeficient difúze, viz část 1.3 |
| Oblak se rozšíří po celé oblasti najednou | koeficient difúze je příliš velký - porovnejte ho s `U * L` |
| Log upozorňuje, že předepsaná koncentrace překračuje maximální nasycení | okrajová nebo počáteční podmínka je nad maximálním nasycením; omezení působí jen na intenzitu částic, proto zkontrolujte obě hodnoty |
| Ustálené řešení s difúzí vypadá špatně | maticový řešič se zastavil na limitu iterací - zvyšte počet vnějších iterací GMRES, viz část 2.6 |
| Koncentrace u stěn překmitává hodnotu na vstupu nebo je záporná, ale dál neroste | téměř čistá advekce - koeficient difúze je blízký nule. Zadejte efektivní koeficient difúze; na hrubé síti také ověřte, že proudové pole je zkonvergované |
| Koncentrace překračuje hodnotu na vstupu a roste do zašuměného vzoru | proudění není zkonvergované - `Residual ratio` zůstává v každém kroku blízko `1`. Jeho rychlost má daleko k bezdivergenčnosti a žádnému přenosu na ní nelze věřit; nejprve nechte zkonvergovat proudění (síť, Reynoldsovo číslo, iterace na krok) |

---

## 4. Kontrola modelu

**Doba průchodu.** Doba, za kterou značka urazí vzdálenost `L` při střední
rychlosti `V`, je `L/V`. Odečtěte čas příchodu z historie monitorovacího bodu
a porovnejte. Tím ověříte, že se krok rozptylu veze na tom proudovém poli, na
kterém si myslíte, a odhalíte výpočet, který nebyl restartován ze
zkonvergovaného proudění.

Samotné proudové pole je nejlepší zkontrolovat nejdříve, pomocí bilance hmoty
a analytických proudění uvedených v příručce [Proudění tekutin](theory_fluid.md).
Nezkonvergovaný model proudění neprojde žádnou kontrolou ze stejného důvodu a
žádnému přenosu na něm nelze věřit.

---

## 5. Omezení

- **Žádný model turbulence.** Proudění je laminární, viz příručka
  [Proudění tekutin](theory_fluid.md), takže míchání turbulentního proudění je
  nutné zadat jako efektivní koeficient difúze.
- **Konstantní, izotropní difúze.** Koeficient difúze je jediná hodnota pro
  celý model; nemůže se měnit v prostoru ani v čase, sledovat směr proudění ani
  pocházet z modelu turbulence. Je nutné ho odhadnout jako efektivní
  difuzivitu - viz část 1.3.
- **Konstantní maximální nasycení.** Nasycení nesleduje teplotu z výsledku
  *Heat transfer in fluids* a omezuje pouze kladnou intenzitu částic nebo tok
  částic. Koncentrace předepsané podmínkami omezeny nejsou a pole se neořezává.
- **Vazba na proudění je jednosměrná.** Proudění pohání přenos; přenos
  proudění nikdy nemění.
- **Nemonotónnost u téměř čisté advekce.** Galerkinova metoda se SUPG vytváří
  překmity a podkmity, když je koeficient difúze blízký nule, největší u stěn
  bez skluzu - viz část 1.4.

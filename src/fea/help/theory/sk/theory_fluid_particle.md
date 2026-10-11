# Rozptyl kontaminantu - Teoretický manuál a používateľská príručka

Tento dokument opisuje riešič rozptylu kontaminantu v Range FEA -
`RSolverFluidParticle`, ktorý rieši typ úlohy **Rozptyl kontaminantu**
(*Contaminant dispersion*): rovnicu, ktorú rieši, význam každého vstupu, ktorý
prijíma, časti grafického používateľského rozhrania, ktoré ho ovládajú, a
podrobne rozpracovaný tutoriál.

Rozptyl kontaminantu sa rieši na prúdovom poli kroku **Nestlačiteľné viskózne
prúdenie**. Riešič prúdenia, jeho okrajové podmienky, nelineárnu iteráciu a
schému v čase opisuje manuál [Prúdenie tekutín](theory_fluid.md), na ktorý tento
manuál nadväzuje.

**Obsah**

1. [Teoretický základ](#1-teoretický-základ)
2. [Grafické používateľské rozhranie](#2-grafické-používateľské-rozhranie)
3. [Tutoriál - prechodové prúdenie s rozptylom kontaminantu](#3-tutoriál---prechodové-prúdenie-s-rozptylom-kontaminantu)
4. [Kontrola modelu](#4-kontrola-modelu)
5. [Obmedzenia](#5-obmedzenia)

---

## 1. Teoretický základ

### 1.1 Primárna neznáma - koncentrácia častíc

Primárnou neznámou je **koncentrácia častíc** `C` v `kg/m^3`, jeden stupeň
voľnosti na uzol.

| Typ úlohy | Riešič | Primárna neznáma | Jednotky | Stupne voľnosti na uzol |
|---|---|---|---|---|
| Rozptyl kontaminantu | `RSolverFluidParticle` | koncentrácia častíc | `kg/m^3` | 1 |

Tento typ úlohy **vyžaduje** v tej istej úlohe krok *Nestlačiteľné viskózne
prúdenie*: pole rýchlosti číta z jeho výsledku a sám ho nikdy nepočíta.

### 1.2 Riadiaca rovnica

**Rozptyl kontaminantu** prenáša na prúdovom poli skalárnu koncentráciu,

```
dC/dt + v . grad(C) = div( D * grad(C) ) + s_eff

s_eff = s * ( 1 - C / C_sat )     for s > 0 and a maximum saturation set
s_eff = s                         otherwise
```

(obmedzenie `s * ( 1 - C / C_sat )` platí pre `s > 0` pri nastavenom maximálnom
nasýtení, inak `s_eff = s`), so stabilizáciou SUPG a theta schémou v čase
riešiča prúdenia a s rýchlosťou prevzatou z výsledku prúdenia.

| Symbol | Význam | Jednotky | Zdroj |
|---|---|---|---|
| `C` | koncentrácia častíc | `kg/m^3` | riešená |
| `v` | rýchlosť | `m/s` | výsledok prúdenia |
| `D` | koeficient difúzie | `m^2/s` | *Nastavenia rozptylu kontaminantu*, predvolene `0` |
| `C_sat` | maximálne nasýtenie | `kg/m^3` | *Nastavenia rozptylu kontaminantu*, predvolene `0` (bez obmedzenia) |
| `s` | rýchlosť zdroja častíc | `kg/(m^3*s)` | podmienka Rýchlosť tvorby častíc (*Particle rate*) |

### 1.3 Nastavenia rozptylu kontaminantu

Oba parametre tohto typu úlohy sa nastavujú raz pre celý model v skupine
*Nastavenia rozptylu kontaminantu* na záložke `Úloha` (časť 2.2). Obe majú
predvolenú hodnotu nula, čo zodpovedá čistej advekcii s neobmedzeným zdrojom.

#### Koeficient difúzie

**Koeficient difúzie** `D` rozširuje kontaminant v smere klesajúcej koncentrácie.
Pri `D = 0` je transport čisto advektívny: kontaminant je unášaný prúdením a
rozširuje sa iba numerickou difúziou stabilizácie, takže riešič odpovedá na
otázku „kam ho prúdenie zanesie a ako dlho to trvá", nie „aký široký je oblak".
Kladné `D` robí zo šírky oblaku fyzikálny výsledok.

Model turbulencie neexistuje (časť 5), takže `D` je **efektívny** koeficient
difúzie: miešanie v skutočnom prúdení je určené turbulenciou, ktorú možno
odhadnúť ako `D ~ nu_t / Sc_t` s turbulentným Schmidtovým číslom `Sc_t ~ 0.7`.

| Molekulová difúzia (fyzikálna vlastnosť) | `D [m^2/s]` |
|---|---|
| plyny a pary vo vzduchu (vodná para `2.5e-5`, CO2 `1.6e-5`, pary rozpúšťadiel približne `1e-5`) | `1e-6` - `1e-4` |
| rozpustené látky vo vode (O2 `2e-9`, soli približne `1.5e-9`) | `1e-10` - `1e-8` |
| aerosólové častice vo vzduchu, Brownov pohyb (10 nm `5e-8`, 0.1 um `7e-10`, 1 um `3e-11`) | `1e-11` - `1e-8` |

V inžinierskych mierkach je molekulová difúzia v porovnaní s transportom prúdením
takmer vždy zanedbateľná. Zvyčajne sa zadáva efektívna hodnota:

| Efektívna (turbulentná) difúzia | `D_eff [m^2/s]` |
|---|---|
| vetraná miestnosť, miešanie vzduchu v interiéri | `1e-3` - `1e-2` |
| prúdenie v potrubí alebo kanáli, približne `D ~ 0.01 - 0.05 * U * L` (`U` stredná rýchlosť, `L` rozmer potrubia) | `1e-3` - `1e-1` |
| rieky, priečne miešanie | `1e-2` - `1` |
| atmosféra, horizontálne miešanie | `1` - `100` |

To, či hodnota na niečom zmení, závisí od siete. So strednou rýchlosťou `U`,
dĺžkou prúdenia `L`, veľkosťou prvku `h` a časovým krokom `dt`:

- **kedy má `D` viditeľný účinok** - kontaminant sa počas prechodu oblasťou
  rozšíri približne o `sqrt( 2 * D * L / U )`. Ak je to menej ako `h`, `D` nemá
  žiadny účinok, takže `D` má význam až približne nad `U * h^2 / ( 2 * L )`. Pre
  kanál s `U ~ 1 m/s`, `L ~ 15 m` a `h ~ 0.1 m` je to približne `3e-4 m^2/s`;
- **kedy `D` prevláda** - približne nad `U * L` (pre ten istý kanál asi
  `15 m^2/s`) difúzia všetko rozmaže a na prúdení takmer nezáleží;
- **časové krokovanie** - ak `D * dt / h^2` prekročí približne `1`, prepnite na
  *Spätnú diferenciu*; centrálna diferencia netlmí najrýchlejšie difúzne módy a
  osciluje.

#### Maximálne nasýtenie

**Maximálne nasýtenie** `C_sat` je najväčšia koncentrácia, ktorú môže kontaminant
v tekutine dosiahnuť - koncentrácia nasýtených pár alebo rozpustnosť. Ak je
nastavené, kladná *Rýchlosť tvorby častíc* slabne, keď sa koncentrácia k nemu
blíži, a pri nasýtení sa zastaví, `s * ( 1 - C / C_sat )`; takto sa modeluje
odparovanie alebo rozpúšťanie a `s` je potom rýchlosť do čistej tekutiny,
približne `k_m * C_sat * A / V`. Záporné zdroje (`s < 0`) sa neobmedzujú.
Obmedzenie pôsobí iba na zdroj: koncentrácia predpísaná okrajovou alebo
počiatočnou podmienkou sa neobmedzuje a log riešiča upozorní, ak `C_sat`
prekračuje. Koncentrácia sa ani neorezáva, takže mierne prestrelenie pri strmom
čele je stále možné.

Hranica nasýtenia existuje iba tam, kde existuje termodynamická hranica: tlak
nasýtených pár alebo rozpustnosť. Pre paru vo vzduchu vyplýva koncentrácia
nasýtenia z tlaku nasýtených pár, `C_sat = p_sat * M / ( R * T )`:

| Para vo vzduchu | Teplota | `C_sat [kg/m^3]` |
|---|---|---|
| vodná para | 0 degC | `0.0048` |
| vodná para | 20 degC | `0.017` |
| vodná para | 40 degC | `0.051` |
| vodná para | 100 degC | `0.60` |
| etanol alebo toluén | 20 degC | približne `0.11` |
| metanol | 20 degC | približne `0.17` |
| benzén | 20 degC | približne `0.32` |
| acetón | 20 degC | približne `0.59` |

Pre látku rozpustenú vo vode je hranicou jej rozpustnosť:

| Rozpustené vo vode | `C_sat [kg/m^3]` |
|---|---|
| kyslík v rovnováhe so vzduchom, 20 degC | `0.009` |
| oxid uhličitý pri 1 atm | `1.7` |
| sadrovec | `2.4` |
| chlorid sodný | približne `360` |

Častice, prach a dym nasýtenie nemajú; ponechajte hodnotu `0` (bez obmedzenia).
Iba pre predstavu, limity expozície na pracovisku sú približne
`1e-6` - `1e-5 kg/m^3` (1 - 10 mg/m^3) a dolné medze výbušnosti prachu približne
`0.02` - `0.06 kg/m^3`.
Nasýtenie silne závisí od teploty, riešič však používa jednu konštantnú hodnotu:
v neizotermickom prípade použite hodnotu pri najnižšej relevantnej teplote, kde
by začala kondenzácia.

### 1.4 Stabilizácia

Parameter SUPG riešiča rozptylu je obmedzený časovým krokom a zahŕňa difúziu,

```
tau = [ ( 2 / dt )^2 + ( 2 * |v| / h )^2 + 9 * ( 4 * D / h^2 )^2 ]^(-1/2)
```

pričom v ustálenej analýze člen s `dt` odpadá. Tam, kde je prúdenie rýchle, sa
redukuje na `h / ( 2 * |v| )`, nikdy neprekročí `dt / 2` a tam, kde prevláda
difúzia, sa blíži k `h^2 / ( 12 * D )` - limite uzlovo presného
jednorozmerného parametra. Obmedzenie časovým krokom je dôležité v pomalých a
recirkulačných oblastiach: tam je Courantovo číslo prvku `|v| * dt / h` veľmi
malé a neobmedzené `tau` by zo stabilizácie urobilo šum, ktorý vytvára
koncentráciu pred čelom oblaku. Tento parameter je vlastným parametrom riešiča
rozptylu; riešič prúdenia používa parameter opísaný v manuáli
[Prúdenie tekutín](theory_fluid.md).

Galerkinov advekčný člen sa zostavuje v **advektívnom tvare s divergenčným
záchytom**, `v . grad(C) + 0.5 * max(div(v), 0) * C`, so spojitou uzlovou
rýchlosťou. Rýchlosť z riešiča prúdenia nikdy nie je presne nedivergentná - na
hrubej sieti je pri stenách a v rohoch bežná `div(v)` v desiatkach `1/s`. Tam,
kde sa prúdenie rozpína (`div(v) > 0`), by samotná advekcia vytvárala
koncentráciu - šachovnicový vzor, ktorý pri malom časovom kroku narastá z kroku
na krok; záchyt ho ruší. Tam, kde sa prúdenie zbieha, sa nepridáva nič:
symetrická oprava `0.5 * div(v) * C` by tam pôsobila ako zdroj a hromadila by
kontaminant pri stenách. Pre presne nedivergentnú rýchlosť je záchyt nulový.

Pri zanedbateľnom koeficiente difúzie je transport takmer čistou advekciou,
ktorú Galerkinova metóda so SUPG neudrží monotónnu: počítajte s prekmitmi a
podkmitmi, najväčšími pri stenách bez sklzu, kde je rýchlosť nulová. Efektívny
koeficient difúzie (časť 1.3) väčšinu z nich odstráni.

Na rozdiel od riešiča prúdenia riešič rozptylu aplikuje theta váhovanie
konzistentne, takže *Centrálna diferencia* je tu skutočnou Crankovou-Nicolsonovou
schémou.

### 1.5 Okrajové podmienky

| Okrajová podmienka | Typ | Aplikuje sa na | Zložky |
|---|---|---|---|
| Koncentrácia častíc (*Particle concentration*) | explicitná (Dirichletova) | bod, čiara, plocha | Koncentrácia častíc `[kg/m^3]` |
| Rýchlosť tvorby častíc (*Particle rate*) | prirodzená, zdroj | bod, čiara, plocha, objem | Rýchlosť tvorby častíc `[kg/(m^3*s)]` |
| Tok častíc (*Particle flux (unit area)*) | prirodzená (Neumannova) | plocha | Tok častíc `[kg/(m^2*s)]` |
| Prestup častíc (*Particle mass transfer*) | prirodzená (Robinova) | plocha | Súčiniteľ prestupu hmoty `[m/s]`, Okolitá koncentrácia častíc `[kg/m^3]` |

**Koncentrácia častíc** drží svoje uzly na zadanej koncentrácii. Priradená
vstupnej ploche s tabuľkou v čase je spôsobom, ako modelovať únik: nula pred
udalosťou, koncentrácia úniku počas nej, nula potom. Je to najužitočnejší postup
pri tomto type úlohy a stavia naň aj tutoriál z časti 3.

**Rýchlosť tvorby častíc** je zdroj a zadáva sa ako **hustota** v `kg/(m^3*s)`, nie
ako celková hodnota - na rozdiel od podmienky *Teplo* sa nerozkladá na mieru
entity. Modeluje zdroj, ktorý vnútri oblasti nepretržite uvoľňuje kontaminant.

**Tok častíc** zadáva hmotnostný tok `q`, ktorý cez plochu vstupuje do oblasti,
`D * grad(C) . n = q`, kde `n` je vonkajšia normála. Na rozdiel od *Rýchlosti
tvorby častíc* je to tok na jednotku plochy, celkové uvoľnenie je teda `q * A`.
Kladné `q` je emisia - odparujúci sa film, stena uvoľňujúca plyny - a obmedzuje
ho maximálne nasýtenie rovnako ako kladnú rýchlosť tvorby,
`q * ( 1 - C / C_sat )`. Záporné `q` je pevná rýchlosť odberu bez ohľadu na
miestnu koncentráciu a môže ju stlačiť pod nulu; usadzovanie úmerné
koncentrácii je *Prestup častíc* s `C_inf = 0`. Na stene
vstupuje tok iba difúziou; na časti hranice, kadiaľ prúdenie vstupuje, sa
vstupujúca tekutina považuje za čistú a nesie tok, `q` je potom celkový
vstupujúci hmotnostný tok a `C ~ q / |v . n|`. Priradený vnútornej ploche medzi
dvoma objemami uvoľňuje `q` na jednotku plochy ako plošný zdroj.

**Prestup častíc** vymieňa kontaminant s okolitým rezervoárom,
`q = k_m * ( C_inf - C )`, kde `k_m` je *Súčiniteľ prestupu hmoty* (*Mass
transfer coefficient*) a `C_inf` *Okolitá koncentrácia častíc* (*Ambient
particle concentration*). Je obdobou jednoduchej konvekcie pri prenose tepla.
Pri `C_inf = 0` modeluje usadzovanie na ploche s rýchlosťou usadzovania `k_m`;
pri `C_inf > 0` výmenu s okolitým vzduchom cez otvor alebo rozpúšťajúcu sa
plochu udržiavanú na `C_inf`. Typické hodnoty `k_m` sú `1e-4` - `1e-2 m/s` pre
usadzovanie častíc v miestnostiach a `1e-3` - `1e-1 m/s` pre odparovanie do
prúdiaceho plynu. Veľké `k_m` ženie plochu k `C_inf` a blíži sa podmienke
*Koncentrácia častíc*.

Každá zložka je **tabuľkou v čase**, takže únik môže počas prechodového výpočtu
začať aj skončiť.

**Hranice bez podmienky.** Steny a výstupy bez podmienky *Koncentrácia častíc*,
*Tok častíc* alebo *Prestup častíc* si ponechávajú prirodzenú podmienku
**nulového difúzneho toku**: kontaminant cez stenu nedifunduje a výstupom
odchádza iba s prúdením.

Tam, kde prúdenie takou hranicou **vstupuje** - typicky spätné prúdenie cez časť
výstupu s *Tlakom (implicitným)* - sa považuje za čistú tekutinu: riešič na tejto
časti plochy pridá slabý vstupný člen `max(-v . n, 0) * C`, rozložený po
ploche tak, aby tlmil aj uzly na hrane, kde sa výstup stretáva so stenou bez
sklzu. Bez neho by hranica
dodávala riešeniu energiu a koncentrácia na výstupe by neobmedzene rástla. Ak
vracajúca sa tekutina nie je čistá, predĺžte oblasť tak, aby výstupom prúdenie
iba odchádzalo.

Podmienky prúdenia - steny, vstupy a výstupy - patria kroku prúdenia a opisuje
ich manuál [Prúdenie tekutín](theory_fluid.md).

### 1.6 Počiatočné podmienky

| Počiatočná podmienka | Účinok |
|---|---|
| Koncentrácia častíc | počiatočné pole koncentrácie |

Počiatočné podmienky sa čítajú **iba pri prvom výpočte**; potom pole pokračuje z
predchádzajúceho výsledku. Kde sa stretne okrajová a počiatočná podmienka,
okrajová má prednosť.

### 1.7 Ustálená alebo prechodová analýza?

Rozptyl kontaminantu je predovšetkým **prechodovou** úlohou: úlohou transportu v
čase, ktorá pre udalosť úniku nemá zmysluplnú ustálenú odpoveď. Ustálený rozptyl
odpovedá na inú otázku - na aké pole koncentrácie sa ustáli trvalý zdroj - a je
jediným lineárnym riešením na prúdovom poli (o jeho maticovom riešiči pozri časť
2.6).

Dvojkrokový postup z tutoriálu v časti 3 je štandardný: nechajte skonvergovať
ustálené prúdenie a potom reštartujte so zapnutým časovým riešičom a krokom
rozptylu kontaminantu.

### 1.8 Výsledky

| Výsledok | Aplikuje sa na | Význam |
|---|---|---|
| Koncentrácia častíc `[kg/m^3]` | uzol | vyriešené pole koncentrácie |
| Relatívne nasýtenie `[-]` | uzol | `C / C_sat`, iba pri nastavenom maximálnom nasýtení |

Relatívne nasýtenie (*Relative saturation*) sa predvolene zobrazuje od `0` do
`1` a odstráni sa, keď sa maximálne nasýtenie nastaví späť na nulu.

Tento typ úlohy vyžaduje iba **hustotu** materiálu.

---

## 2. Grafické používateľské rozhranie

Všetko podstatné sa nachádza v paneli **Riešič** (záložky `Úloha`,
`Okrajové podmienky`, `Počiatočné podmienky`, `Materiál`, `Výsledky`), v menu
**Úloha** a v paneli **Model**.

### 2.1 Výber úlohy

`Úloha` -> `Poradie krokov úloh` (`Ctrl+P`) otvorí dialóg poradia krokov úloh.
Krok **Rozptyl kontaminantu** umiestnite do tej istej skupiny krokov ako krok
**Nestlačiteľné viskózne prúdenie** - rozptyl na prúdení, klasická prechodová
dvojica. Tento typ úlohy vyžaduje prítomnosť kroku prúdenia.

Hodnoty skupiny krokov - **počet iterácií** a **Konvergencia** - riadia
nelineárnu iteráciu prúdenia a opisuje ich manuál
[Prúdenie tekutín](theory_fluid.md). V prechodovom výpočte sú to iterácie *na
časový krok*.

### 2.2 Záložka Úloha

**Riešič v čase** - pre ustálený rozptyl ho nechajte vypnutý, pre udalosť úniku
ho zapnite.

| Pole | Význam |
|---|---|
| Povoliť | zapína a vypína krokovanie v čase |
| Aproximácia | theta schémy v čase: spätná, centrálna alebo dopredná |
| Počiatočný čas | prvá hodnota času v sekundách |
| Konečný čas | `počiatočný čas + veľkosť časového kroku * počet časových krokov`, iba na čítanie |
| Veľkosť časového kroku | `dt` v sekundách |
| Počet časových krokov | koľko krokov sa má vypočítať |
| Frekvencia výstupu | zapíše súbor výsledkov každých N krokov; `0` zapíše iba posledný krok |

**Nastavenia rozptylu kontaminantu** - zobrazuje sa, keď je v poradí krokov úloh
krok *Rozptyl kontaminantu*. Obsahuje dva parametre z časti 1.3:

| Pole | Význam |
|---|---|
| Maximálne nasýtenie `[kg/m^3]` | najväčšia možná koncentrácia; kladná rýchlosť tvorby častíc sa pri nej zastaví a počíta sa *Relatívne nasýtenie*. `0` znamená bez obmedzenia |
| Koeficient difúzie `[m^2/s]` | efektívny koeficient difúzie kontaminantu v tekutine. `0` znamená čistú advekciu |

Typické hodnoty oboch sú uvedené v časti 1.3.

### 2.3 Záložka Materiál

| Vlastnosť | Jednotky | Poznámka |
|---|---|---|
| Hustota | `kg/m^3` | `rho`, približne `1000` pre vodu, `1.2` pre vzduch |

Krok prúdenia v tej istej úlohe potrebuje aj dynamickú viskozitu. Dodaná
databáza materiálov obsahuje **Vodu** a **Vzduch**, čo zvyčajne tento krok celý
vybaví.

### 2.4 Záložka Okrajové podmienky

Podmienky z časti 1.5 sa ponúkajú, keď je v poradí krokov úloh krok *Rozptyl
kontaminantu*. Najprv vyberte entitu v strome `Model`; zoznam potom ponúkne
podmienky platné pre daný typ entity. Zaškrtnutím podmienku priradíte a potom
upravíte jej zložky v dolnom strome.

Každá zložka je tabuľkou v čase. Kliknutím na **Upraviť časovo závislé
hodnoty** otvoríte editor zložiek, kde sa hodnoty zadávajú v závislosti od času;
medzi dvoma zadanými časmi sa hodnota lineárne interpoluje a po poslednom
zadanom čase zostáva platná posledná hodnota. Takto sa modeluje udalosť úniku.

### 2.5 Počiatočné podmienky

Záložka `Počiatočné podmienky` ponúka *Koncentráciu častíc*. Nastavuje
počiatočné pole prvého výpočtu.

### 2.6 Nastavenie maticového riešiča

`Úloha` -> `Nastaviť maticový riešič úloh` konfiguruje iteračné riešiče. Riešič
rozptylu používa položku **GMRES**, pretože jeho matica nie je symetrická.

Pozornosť si tu vyžaduje **ustálený rozptyl kontaminantu s difúziou**. Ide o
jediné lineárne riešenie, takže neexistujú iterácie skupiny, na ktoré by sa dalo
spoľahnúť, a sústava s prevládajúcou difúziou konverguje pri predvolenom limite
GMRES 10 vnútorných krát 10 vonkajších iterácií pomaly. Maticový riešič pri
dosiahnutí limitu neupozorní: prečítajte si jeho tabuľku iterácií v logu riešiča
a ak dobehne do poslednej vonkajšej iterácie s reziduom stále výrazne nad
hodnotou konvergencie riešiča, zvýšte počet vonkajších iterácií. Nekonvergované
riešenie dá nesprávnu odpoveď bez akéhokoľvek iného náznaku.

### 2.7 Monitorovacie body

`Úloha` -> `Definovať monitorovacie body` umiestni sondy na zadané súradnice a
vyberie zaznamenávanú veličinu - *Koncentráciu častíc*, *Rýchlosť tvorby
častíc* alebo *Relatívne nasýtenie*. V prechodovom výpočte zobrazí históriu
`Výkaz` -> `Monitorovacie body`.

Monitorovací bod v mieste záujmu je odpoveďou vo forme, akú zvyčajne chcete:
koncentrácia v čase na danom mieste, odčítaná z grafu, a nie z postupnosti
obrázkov.

### 2.8 Výsledky a záznamy

Záložka `Výsledky` zobrazuje vypočítané veličiny a ovláda 3D zobrazenie.
Koncentrácia častíc a relatívne nasýtenie sú uzlové skaláry.

Záložka `Záznamy` panela `Model` zobrazuje záznamy výsledkov - jeden na každý
zapísaný časový krok prechodového výpočtu. Animovanie záznamov prehrá rozptyl a
tlačidlo **Záznam** v dolnej časti stromu zapíše animáciu do video súboru.

---

## 3. Tutoriál - prechodové prúdenie s rozptylom kontaminantu

**Cieľ.** Uvoľniť kontaminant do konvergovaného prúdenia a sledovať oblak v čase
pri jeho pohybe kanálom.

Ide o dvojkrokový postup z časti 1.7, ktorý zodpovedá dodanému tutoriálu
*Rozptyl kontaminantu v tekutinách*.

### Krok 1 - začnite z konvergovaného prúdenia

Najprv nechajte skonvergovať tutoriál ustáleného prúdenia kanálom z manuálu
[Prúdenie tekutín](theory_fluid.md). Výpočet rozptylu spustený z pokoja premrhá
prvé časové kroky vývojom prúdového poľa namiesto prenášania čohokoľvek.

### Krok 2 - pridanie kroku rozptylu

1. `Úloha` -> `Poradie krokov úloh` (`Ctrl+P`).
2. Ponechajte skupinu krokov úlohy a krok **Nestlačiteľné viskózne prúdenie**.
3. Do tej istej skupiny pridajte krok **Rozptyl kontaminantu**.
4. Znížte **počet iterácií** skupiny na približne `20` - teraz sú to nelineárne
   iterácie *na časový krok*, nie iterácie ustáleného riešenia. Hodnotu
   **Konvergencia** nemeňte; väčšina krokov sa ustáli pred dvadsiatym prechodom a
   skončí skôr, a tie, ktoré nie, sú práve tie, ktoré si plných dvadsať
   zaslúžia.

*Rozptyl kontaminantu* vyžaduje prítomnosť kroku prúdenia; pole rýchlosti číta a
nikdy ho nepočíta.

### Krok 3 - nastavenie časového riešiča

Otvorte záložku `Úloha` a v skupine *Riešič v čase* nastavte:

| Pole | Hodnota |
|---|---|
| Povoliť | zaškrtnuté |
| Aproximácia | `Backward difference (stable)` (spätná diferencia) |
| Počiatočný čas | `0` |
| Veľkosť časového kroku | podľa odporúčania Courantovho čísla nižšie |
| Počet časových krokov | dosť na to, aby oblak prešiel oblasťou |
| Frekvencia výstupu | `1` pre plynulú animáciu, viac pre menší súbor |

Časový krok zvoľte podľa siete a rýchlosti prúdenia tak, aby tekutina prešla za
krok približne jeden prvok:

```
dt ~ h / |v|
```

Potom zvoľte počet krokov podľa toho, ako dlho oblak potrebuje na prechod
oblasťou, `L / |v|`, delené týmto krokom.

V skupine *Nastavenia rozptylu kontaminantu* na tej istej záložke ponechajte pre
čistú advekciu obe hodnoty na `0`, alebo zadajte efektívny **Koeficient difúzie**
podľa tabuľky v časti 1.3, ak záleží na šírke oblaku. **Maximálne nasýtenie** má
zmysel iba pre paru alebo rozpustenú látku; nesmie byť nižšie ako koncentrácia
úniku z kroku 4.

### Krok 4 - uvoľnenie kontaminantu

1. Vyberte vstupnú plošnú entitu.
2. Otvorte záložku `Okrajové podmienky` a zaškrtnite **Koncentrácia častíc**
   (*Particle concentration*).
3. Namiesto zadania jednej hodnoty kliknite na **Upraviť časovo závislé
   hodnoty**.
4. V editore zložiek zadajte profil úniku - napríklad `0` v čase `0`,
   koncentráciu úniku v okamihu začiatku úniku a opäť `0` na jeho konci.

Medzi dvoma zadanými časmi sa hodnota lineárne interpoluje a po poslednom
zadanom čase zostáva platná posledná hodnota, takže únik, ktorý má začať náhle,
potrebuje dve tesne po sebe nasledujúce položky. Práve táto časovo závislá
podmienka robí z ustáleného vstupu udalosť úniku a je jadrom modelu rozptylu.

### Krok 5 - ponechanie podmienok prúdenia

**Stena**, podmienka prítoku a výstup s **Tlakom (implicitným)** zostávajú
nezmenené. Prúdenie sa naďalej rieši v každom časovom kroku - rozptyl sa na ňom
len veze.

### Krok 6 - pridanie monitorovacieho bodu

`Úloha` -> `Definovať monitorovacie body`, umiestnite bod po prúde a nastavte jeho
veličinu na **Koncentrácia častíc**. Čas príchodu a špičkovú koncentráciu na
danom mieste je oveľa jednoduchšie odčítať z histórie ako z postupnosti farebných
obrázkov.

### Krok 7 - riešenie s reštartom

1. `Riešenie` -> `Spustiť riešič` (`Ctrl+R`).
2. Zaškrtnite **Reštartovať riešič / pokračovať**.

Práve reštart robí z konvergovaného ustáleného poľa východiskový bod krokovania v
čase. Bez neho výpočet začne z pokoja a prvé kroky sú premrhané.

### Krok 8 - prezeranie výsledkov

- Panel `Model`, záložka `Záznamy`: jeden záznam na každý zapísaný krok.
  Prechádzajte ich alebo ich animujte a sledujte, ako sa oblak pohybuje.
- Záložka `Výsledky`: zobrazte **Koncentráciu častíc**. Zafixujte rozsah
  zobrazenia naprieč záznamami, aby farby znamenali v každom snímku to isté -
  inak sa animácia sama premieruje a oblak sa javí, akoby neslabol.
- `Výkaz` -> `Monitorovacie body` zobrazí históriu koncentrácie v sonde.
  Odčítajte z nej čas príchodu a porovnajte ho s `L / |v|`, časom prechodu, ktorý
  vyplýva zo strednej rýchlosti prúdenia - je to najlacnejšie overenie celého
  modelu.
- Pri nulovom koeficiente difúzie sa oblak rozširuje iba numerickou difúziou,
  takže jeho **šírka** je vlastnosťou siete a časového kroku, nie fyziky. S
  efektívnym koeficientom difúzie je šírka výsledkom - pred tým, než jej budete
  dôverovať, ju porovnajte s jemnejšou sieťou.

### Riešenie problémov

| Príznak | Príčina |
|---|---|
| Koncentrácia nikdy neopustí vstup | prúdové pole je nulové - v skupine chýba krok prúdenia alebo výpočet nebol reštartovaný z konvergovaného prúdenia |
| Oblak sa takmer okamžite rozmaže | Courantovo číslo je ďaleko nad 1 alebo je sieť pozdĺž dráhy príliš hrubá |
| Koncentrácia je záporná alebo prestrelí | časový krok je pre sieť príliš veľký, takže stabilizovaná advekcia „zvoní". Mierne podstrelenie pri strmom čele je normálne; riešenie sa neorezáva na nulu, pretože orezanie by pridávalo kontaminant |
| Zapísal sa iba jeden záznam | frekvencia výstupu je `0`, čo zapíše iba posledný krok |
| Reziduum každého kroku zostáva vysoké | príliš málo iterácií na časový krok - prúdenie v rámci kroku nie je konvergované |
| Oblak sa nikdy nerozšíri do strán | koeficient difúzie je nulový, takže bočné rozširovanie pochádza iba z prúdového poľa - zadajte efektívny koeficient difúzie, pozri časť 1.3 |
| Oblak sa naraz rozšíri po celej oblasti | koeficient difúzie je príliš veľký - porovnajte ho s `U * L` |
| Log upozorňuje, že predpísaná koncentrácia prekračuje maximálne nasýtenie | okrajová alebo počiatočná podmienka je nad maximálnym nasýtením; obmedzenie pôsobí iba na rýchlosť tvorby častíc, preto skontrolujte obe hodnoty |
| Ustálené riešenie s difúziou vyzerá nesprávne | maticový riešič sa zastavil na limite iterácií - zvýšte počet vonkajších iterácií GMRES, pozri časť 2.6 |
| Koncentrácia pri stenách prekračuje hodnotu na vstupe alebo je záporná, ale ďalej nenarastá | takmer čistá advekcia - koeficient difúzie je blízky nule. Zadajte efektívny koeficient difúzie; na hrubej sieti overte aj konvergenciu prúdenia |
| Koncentrácia prekročí hodnotu na vstupe a narastá do zašumeného vzoru | prúdenie nie je konvergované - `Residual ratio` zostáva v každom kroku blízko `1`. Jeho rýchlosť má ďaleko od nulovej divergencie a žiadnemu transportu na nej nemožno dôverovať; najprv nechajte skonvergovať prúdenie (sieť, Reynoldsovo číslo, iterácie na krok) |

---

## 4. Kontrola modelu

**Čas prechodu.** Čas, za ktorý značka prejde vzdialenosť `L` pri strednej
rýchlosti `V`, je rovný `L/V`. Odčítajte čas príchodu z histórie monitorovacieho
bodu a porovnajte. Overí to, že krok rozptylu sa veze na tom prúdovom poli, na
ktorom si myslíte, a odhalí výpočet, ktorý nebol reštartovaný z konvergovaného
prúdenia.

Samotné prúdové pole je najlepšie overiť ako prvé, bilanciou hmotnosti a
analytickými prúdeniami uvedenými v manuáli [Prúdenie tekutín](theory_fluid.md).
Nekonvergovaný model prúdenia z rovnakého dôvodu neprejde žiadnou kontrolou a
žiadnemu transportu na ňom nemožno dôverovať.

---

## 5. Obmedzenia

- **Žiadny model turbulencie.** Prúdenie je laminárne, pozri manuál
  [Prúdenie tekutín](theory_fluid.md), takže miešanie turbulentného prúdenia
  treba zadať ako efektívny koeficient difúzie.
- **Konštantná izotropná difúzia.** Koeficient difúzie je jedna hodnota pre celý
  model; nemôže sa meniť v priestore ani v čase, riadiť sa smerom prúdenia ani
  pochádzať z modelu turbulencie. Treba ho odhadnúť ako efektívny koeficient
  difúzie - pozri časť 1.3.
- **Konštantné maximálne nasýtenie.** Nasýtenie nesleduje teplotu z výsledku
  *Prestupu tepla v tekutinách* a obmedzuje iba kladnú rýchlosť tvorby častíc
  alebo kladný tok častíc. Koncentrácie predpísané podmienkami sa neobmedzujú a
  pole sa neorezáva.
- **Väzba na prúdenie je jednosmerná.** Prúdenie poháňa transport; transport
  nikdy nemení prúdenie.
- **Pri takmer čistej advekcii nie je riešenie monotónne.** Galerkinova metóda so
  SUPG pri koeficiente difúzie blízkom nule vytvára prekmity a podkmity, najväčšie
  pri stenách bez sklzu - pozri časť 1.4.

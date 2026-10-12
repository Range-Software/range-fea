# Magnetostatika - Teoretický manuál a používateľská príručka

Tento dokument opisuje riešič magnetostatiky (`RSolverMagnetostatics`) v Range
FEA, ktorý rieši typ úlohy **Magnetostatika** (*Magneto-statics*): zákon, ktorý
vyhodnocuje, vstupy, ktoré prijíma, časti grafického používateľského rozhrania,
ktoré ho ovládajú, podrobne rozpracovaný tutoriál a to, akú hodnotu má jeho
výsledok.

> **Stav.** Riešič vyhodnocuje magnetické pole prúdu vypočítaného krokom
> elektrostatiky priamo, pomocou **Biotovho-Savartovho zákona**. Neexistuje
> žiadna sústava rovníc ani okrajová podmienka: pole je určené iba prúdom a
> podmienka, že ďaleko od vodiča zaniká, je zabudovaná priamo v zákone. Pre
> zadanú prúdovú hustotu je výsledok presný až na kontrolovanú chybu kvadratúry,
> ktorá je výrazne pod `1e-4` špičkového poľa. Riešič pracuje vo **voľnom
> priestore** - magnetické materiály nemožno reprezentovať - a počíta iba pole
> modelovaného prúdu, takže prúdová dráha by mala byť uzavretá v rámci modelu.
> Obmedzenia uvádza časť 5.

**Obsah**

1. [Teoretický základ](#1-teoretický-základ)
2. [Grafické používateľské rozhranie](#2-grafické-používateľské-rozhranie)
3. [Tutoriál - pole vodiča s prúdom](#3-tutoriál---pole-vodiča-s-prúdom)
4. [Kontrola modelu](#4-kontrola-modelu)
5. [Obmedzenia](#5-obmedzenia)

---

## 1. Teoretický základ

### 1.1 Výsledok - magnetické pole

Výsledkom je **magnetická indukcia** `B` v každom uzle modelu, v teslách, ako
vektor s tromi zložkami.

Pole sa **vyhodnocuje, nerieši**. Keď je prúdová hustota známa, magnetické pole
vo voľnom priestore z nej vyplýva integrálom, takže riešič nezostavuje žiadnu
maticu a nespúšťa žiadnu iteráciu. Hodnotu dostane každý uzol modelu, či už leží
vo vodiči, na jeho povrchu alebo kdekoľvek inde v sieti.

### 1.2 Riadiaci zákon

Vo voľnom priestore statické magnetické pole spĺňa Ampérov zákon a nemá žiadne
zdroje:

```
curl(B) = u0 * J
div(B)  = 0
```

spolu s podmienkou, že `B` ďaleko od všetkých prúdov zaniká. Jediné pole, ktoré
spĺňa všetky tri, dáva **Biotov-Savartov zákon**

```
B(x) = u0/(4*pi) * INT_V  J(y) x (x - y) / |x - y|^3  dV(y)
```

| Symbol | Význam | Jednotky | Zdroj |
|---|---|---|---|
| `B` | magnetická indukcia | `T` | vypočítaná |
| `J` | prúdová hustota | `A/m^2` | z kroku elektrostatiky, pozri 1.4 |
| `u0` | permeabilita vákua | `H/m` | zabudovaná, `1.25663706212e-6` |
| `x` | bod poľa - uzol modelu | `m` | |
| `y` | bod zdroja - vnútri vodivého prvku | `m` | |

Tri vlastnosti vyplývajú zo samotného zákona, nie z niečoho, čo robí riešič:

- **`div(B) = 0` platí presne**, všade, nech je sieť akákoľvek.
- **Vzdialené pole je správne aj bez hranice.** `B` klesá so vzdialenosťou tak,
  ako predpisuje zákon; neexistuje vonkajšia plocha, žiadna oblasť vzduchu, ktorú
  treba pokryť sieťou, a žiadna podmienka, ktorú tam treba zadať.
- **`curl(B) = u0 * J` platí všade, kde je prúd uzavretý**, teda kde
  `div(J) = 0` a prúdová dráha tvorí slučky. Časť 1.4 vysvetľuje, čo to znamená
  pre elektrostatický model, ktorého prúd do telesa vstupuje a z neho vystupuje.

Permeabilita materiálu nikde nevstupuje. Neexistuje vlastnosť materiálu
relatívna permeabilita, žiadna magnetizácia ani rozlíšenie medzi `B` a `H` -
riešič pracuje vo voľnom priestore bez ohľadu na to, z čoho je model vyrobený.
Časť 1.9 vysvetľuje, prečo toto obmedzenie nemožno v rámci toho istého prístupu
odstrániť.

**Nevyžaduje sa žiadna vlastnosť materiálu.** Typ úlohy magnetostatika
nedeklaruje vôbec žiadnu a kontrola nastavenia nemá za neho čo kontrolovať. Krok
elektrostatiky, od ktorého typ úlohy závisí, má vlastné tri požiadavky - pozri
manuál elektrostatiky.

### 1.3 Diskretizácia

Riešič elektrostatiky vytvára prúdovú hustotu, ktorá je **konštantná v každom
prvku**. Integrál z 1.2 sa preto rozpadne na jeden integrál na prvok, každý s
konštantou `J_e`, ktorú možno vybrať pred integrál:

```
B(x) = u0/(4*pi) * SUM_e  J_e x G_e(x)
G_e(x) = INT_e  (x - y) / |x - y|^3  dV(y)
```

`G_e` závisí iba od geometrie prvku a od bodu poľa. Každý typ prvku, ktorý
riešič prijíma, má rovinné steny a pre tie má `G_e` uzavretý tvar, takže riešič
integruje každý prvok **presne**, nie približne:

- **Tetrahedrón.** Integrand je gradient `1/|x - y|` vzhľadom na `y`, takže
  objemový integrál sa zmení na súčet cez štyri steny,
  `G = SUM_f n_f * INT_f 1/R dA`. Integrál `1/R` cez rovinný trojuholník je známy
  v uzavretom tvare: logaritmus pozdĺž každej hrany a priestorový uhol, pod ktorým
  je trojuholník vidieť z bodu `x`,
  `INT 1/R dA = SUM_edges P_e * I_e - |d| * omega`.
- **Trojuholník** plošnej entity s hrúbkou `t`. Prúd je plošný prúd `K = J*t` v
  `A/m`. Tangenciálna časť `G` je opäť súčtom logaritmov hrán, `SUM_e m_e * I_e`,
  a normálová časť je priestorový uhol so znamienkom.
- **Štvoruholník.** Rozdelí sa na dva trojuholníky.
- **Úsečka** čiarovej entity s prierezom `A`. Prúd pozdĺž úsečky je
  `I = A * (J . t)` a pole priameho konečného drôtu je klasické:

  ```
  B = u0*I/(4*pi) * (a x b) * (|a| + |b|) / ( |a|*|b| * (|a|*|b| + a.b) )
  ```

  kde `a` a `b` sú vektory z bodu poľa do oboch koncov.

Tu `I_e` je integrál `1/R` pozdĺž hrany `e`, `P_e` vzdialenosť bodu poľa od tejto
hrany v rovine steny, `m_e` vonkajšia normála hrany v rovine steny, `d` výška bodu
poľa nad rovinou steny a `omega` priestorový uhol steny pri pohľade z bodu poľa.

**Vzdialené prvky** uzavretý tvar nepotrebujú. Integrand je tam hladký a
kvadratúrne pravidlo je takmer presné za zlomok nákladov:

| Vzdialenosť bodu poľa od stredu prvku | Pravidlo |
|---|---|
| pod 2-násobkom najdlhšej hrany prvku | uzavretý tvar |
| 2- až 6-násobok | pravidlo druhého stupňa - 3 body na trojuholníku, 4 na tetrahedróne |
| nad 6-násobok | pravidlo stredného bodu - prvok ako bodový prúd |

Úsečky vždy používajú uzavretý tvar, ktorý je rovnako lacný ako jeden
kvadratúrny bod. Celkovú chybu pravidiel merajú jednotkové testy voči uzavretému
tvaru sčítanému cez všetky prvky celého vodiča a v každom uzle - vnútri vodiča,
na jeho povrchu aj mimo neho - zostáva pod `1e-4` špičkového poľa.

**Pole priamo vo vodiči.**

- Pre **objemový prúd** je pole spojité a konečné všade, vrátane uzlov, hrán a
  stien vodivých prvkov. Uzol na povrchu masívneho vodiča dostane skutočné
  povrchové pole.
- **Čiarová** alebo **plošná** entita predstavuje vodič s nulovým prierezom a
  pole takého vodiča je singulárne: k drôtu rastie ako `1/r`, k okraju plošného
  vodiča logaritmicky a naprieč plošným vodičom skáče o `u0 * K x n`. V uzle na
  takom vodiči riešič vracia konečnú hodnotu: úsečka neprispieva do bodu na
  vlastnej priamke, plošný vodič prispieva priemerom polí na svojich dvoch
  stranách a logaritmus hrany plošného vodiča sa v uzle ležiacom na tejto hrane
  vynechá. Tieto hodnoty sú regularizované, nie fyzikálne. Pole tenkého vodiča
  odčítajte niekoľko veľkostí prierezu od neho, kde je model tenkého vodiča
  platný.

### 1.4 Zdroj - prúdová hustota

Magnetické pole poháňa jediný vstup: **prúdová hustota**, ktorú zanechal krok
*Elektrostatika* skôr v poradí krokov úloh. Nie je čo zadávať ani priraďovať.

Riešič číta **prvkovú** prúdovú hustotu, ktorú ukladá krok elektrostatiky, a
používa ju tak, ako je - nepriemeruje ju do uzlov. Prúd vedú tieto prvky:

| Entita | Podmienka | Zdroj |
|---|---|---|
| Objem | tetrahedróny | `J` |
| Plocha | hrúbka `> 0` | plošný prúd `J * hrúbka` |
| Čiara | prierez `> 0` | prúd `J * prierez` |

Prvky s nulovou prúdovou hustotou sa preskočia, čo sú všetky prvky, ktoré krok
elektrostatiky nepočítal. Preskočia sa aj prvky, ktorých prúdová hustota je pod
`1e-10` najväčšej v modeli: zle vodivá oblasť - vzduch s vodivosťou `5e-15 S/m`
vedľa medi - by inak pridala jeden zdroj na prvok a do poľa nič merateľné. Ak
žiadny prvok nevedie prúd - krok elektrostatiky neprebehol, potenciály sú rovnaké
alebo žiadny materiál nemá elektrickú vodivosť - log vypíše varovanie a pole je
všade nulové.

**Prúd sa musí uzavrieť.** Biotov-Savartov zákon dáva pole prúdu, ktorý dostane,
a nič iné. Fyzikálny prúd tečie v uzavretých slučkách, ale elektrostatický model
zvyčajne obsahuje iba časť slučky: tyč medzi dvoma elektródami, do ktorej prúd
vstupuje jedným koncom a vystupuje druhým. Prívody a zvyšok obvodu v modeli nie
sú a nie je v ňom ani ich pole. Riešič potom vypočíta pole modelovaného vodiča
tak, akoby sa prúd na jednom konci objavil a na druhom zmizol:

- **blízko vodiča** je to zvyčajne výborná aproximácia, pretože pole tam určuje
  hlavne blízky prúd;
- **ďaleko od neho**, vo vzdialenostiach porovnateľných so samotným vodičom, má
  chýbajúca časť slučky rovnakú váhu ako modelovaná a výsledok nie je poľom
  žiadneho skutočného obvodu.

Ak na vzdialenom poli záleží, zahrňte do elektrostatického modelu spätnú cestu,
aby prúd vytvoril slučku. Tenký spätný vodič možno reprezentovať čiarovou entitou
s prierezom.

### 1.5 Okrajové, počiatočné a environmentálne podmienky

**Žiadne** nie sú a žiadne nie sú potrebné.

| Druh podmienky | Ponúkané pre magnetostatiku |
|---|---|
| Okrajové podmienky | žiadne |
| Počiatočné podmienky | žiadne |
| Environmentálne podmienky | žiadne |
| Vlastnosti materiálu | žiadne |

Pole je úplne určené prúdom a požiadavkou, že ďaleko zaniká, ktorú Biotov-Savartov
zákon nesie so sebou. Neexistuje vonkajšia hranica, ktorú treba uzavrieť, ani
nič, čo by sa na nej predpisovalo.

Keď krok elektrostatiky a krok magnetostatiky zdieľajú poradie krokov - čo
vždy robia, pozri 1.6 - záložky podmienok aj tak ponúkajú **elektrostatické**
podmienky, pretože záložky sa plnia z typov úloh celého poradia krokov.
*Elektrický potenciál* (*Electric potential*) a *Hustota náboja* (*Charge
density*) patria kroku elektrostatiky a k magnetickému poľu sa dostanú cez
prúdovú hustotu, ktorú vytvoria.

### 1.6 Väzba s inými riešičmi

| Smer | Premenná | Účinok |
|---|---|---|
| Elektrostatika -> magnetostatika | Prúdová hustota | jediný zdroj magnetického poľa |

**Magnetostatika vyžaduje elektrostatiku.** Závislosť je deklarovaná v type úlohy
a používateľské rozhranie ju vynucuje: zaškrtnutie *Magnetostatiky* vo výbere
úloh automaticky zaškrtne *Elektrostatiku* a odškrtnutie *Elektrostatiky* s ňou
odškrtne aj *Magnetostatiku*. Predvolené poradie krokov dáva krok elektrostatiky
na prvé miesto, čo je poradie, ktoré väzba potrebuje.

Magnetické pole nič nespotrebúva. Je koncom svojho reťazca - žiadny riešič ho
nečíta a neexistuje Lorentzova sila, indukovaný prúd ani magnetický príspevok k
inej fyzike.

Keďže riešič hlási konvergenciu bezpodmienečne, skupina krokov obsahujúca krok
magnetostatiky sa vždy zastaví po jednej iterácii.

### 1.7 Vyhodnotenie poľa

Každý prvok vedúci prúd prispieva do každého uzla, takže práca rastie ako
**počet uzlov krát počet prvkov vedúcich prúd**. Vyhodnotenie beží paralelne cez
uzly, ktoré sa spracúvajú po blokoch, takže každý prvok sa z pamäte načíta raz na
blok, nie raz na uzol.

Pre orientáciu: tyč so `120 000` tetrahedrónmi vyhodnotená v jej `24 000` uzloch
trvá na 14 vláknach asi päť sekúnd. Zdvojnásobenie siete v každom smere znásobí
počet uzlov aj prvkov ôsmimi a čas približne šesťdesiatimi štyrmi; model s
niekoľkými stovkami tisíc prvkov trvá minúty, nie sekundy.

Počítajú sa iba prvky vedúce prúd. Veľká sieť okolo malého vodiča - oblasť
vzduchu, pozri 2.8 - pridá uzly, ale žiadne zdroje.

Nastavenie maticového riešiča sa nepoužíva.

### 1.8 Výsledky

| Výsledok | Aplikuje sa na | Význam |
|---|---|---|
| Magnetické pole `[T]` | uzol | vektor poľa |

To je celý výstup. `process()` nerobí nič, takže neexistuje žiadna odvodená
veličina - žiadna samostatne uložená veľkosť poľa, žiadny tok ani hustota
energie. 3D pohľad môže obvyklým spôsobom zobraziť veľkosť a zložky vektorovej
premennej a pre zobrazenie zložiek sú k dispozícii `Magnetic field in X
direction` a ekvivalenty pre `Y` a `Z`.

Magnetické pole sa ukladá ako **uzlová** premenná, na rozdiel od výsledkov
elektrostatického poľa, ktoré sú prvkovými premennými. Hodnotu má **každý uzol**
modelu, vrátane uzlov entít, ktoré nevedú prúd, takže sieť okolo vodiča ukazuje
pole v priestore okolo neho.

Po zapísaní záznamu log riešiča vypíše štatistiky magnetického poľa a hodnoty vo
všetkých monitorovacích bodoch.

### 1.9 Čo riešič je a čo nie je

**Je to integrál, nie rovnica poľa.** Vyhodnotenie Biotovho-Savartovho zákona
nepotrebuje žiadnu hranicu ani sieť mimo vodiča a pre prúd, ktorý dostane, je
presné. Cenou je, že funguje iba v **homogénnom** prostredí. S magnetickými
materiálmi pole prúdu závisí od toho, kde je železo, a to nemôže vyjadriť žiadny
integrál samotného prúdu; takáto úloha sa rieši potenciálovou formuláciou -
zvyčajne magnetickým vektorovým potenciálom na hranových prvkoch - čo je iný
riešič, nie rozšírenie tohto.

**Poháňa ho vypočítaný vodivý prúd.** Prúd nemožno zadať priamo, neexistuje
permanentný magnet ani vnútené pole; prúd musí vytvoriť riešenie elektrostatiky
na vodivom telese a jeho presnosť určuje presnosť magnetického výsledku. Manuál
elektrostatiky opisuje, čo táto prúdová hustota je a čo nie je. Prúdová hustota
konštantná na prvok je aproximáciou prvého rádu skutočnej hustoty a so
zjemňovaním siete konverguje.

**Počíta pole modelovaného prúdu.** Pozri 1.4 - ak záleží na poli ďaleko od
vodiča, prúd by sa mal uzavrieť v rámci modelu.

### 1.10 Statická analýza a časový riešič

Magnetostatika je deklarovaná ako typ úlohy **bez** časového riešiča, presne ako
elektrostatika. Skupina *Riešič v čase* sa na záložke `Úloha` nezobrazí, výpočet
vytvorí jediný záznam a nie je čo nastavovať.

Keďže poradie krokov s magnetostatikou vždy obsahuje aj krok elektrostatiky a
ani jeden časový riešič nepodporuje, pridanie ktoréhokoľvek z nich do poradia
krokov odstráni časový riešič z poradia ako celku. Krok prestupu tepla zaradený
za ne môže byť iba ustálený. Manuál elektrostatiky to rozoberá vo vlastnej časti
1.10.

---

## 2. Grafické používateľské rozhranie

Všetko podstatné sa nachádza v paneli **Riešič** (záložky `Úloha`, `Materiál`,
`Výsledky`), v menu **Úloha** a v paneli **Model**. Záložky podmienok pre tento
typ úlohy nič neobsahujú.

### 2.1 Výber úlohy

`Úloha` -> `Poradie krokov úloh` (`Ctrl+P`) otvorí dialóg poradia krokov úloh.
Pridajte krok a vyberte **Magnetostatika** - "Výpočet magnetického poľa".

**Elektrostatika sa zaškrtne za vás.** Výber vynucuje závislosť z 1.6 v oboch
smeroch: zaškrtnutie *Magnetostatiky* zaškrtne *Elektrostatiku* a odškrtnutie
*Elektrostatiky* odškrtne *Magnetostatiku*. Predvolené poradie krokov dáva krok
elektrostatiky na prvé miesto.

Magnetostatika sa kombinuje s každou inou fyzikou okrem *Modálnej analýzy*. V
praxi má zmysel iba reťazec, na ktorom je postavená:

- *Elektrostatika* a potom *Magnetostatika* - magnetické pole vypočítaného
  vodivého prúdu.

### 2.2 Záložka Úloha

Pre poradie krokov elektrostatika plus magnetostatika je táto záložka
**prázdna**. Ani jeden typ úlohy nemá časový riešič a ani jeden nemá vlastnú
skupinu nastavení.

### 2.3 Záložka Materiál

Magnetostatika od materiálu **nič** nevyžaduje. Krok elektrostatiky, od ktorého
závisí, vyžaduje tri vlastnosti - relatívnu permitivitu, elektrickú vodivosť a
hustotu - a entitu, ktorej materiálu niektorá z nich chýba, *tento* krok
preskočí, takže nemá prúdovú hustotu a teda ani príspevok k magnetickému poľu.

Materiál teda stále záleží, a to výlučne cez elektrostatickú polovicu reťazca.
Priraďte materiál so všetkými tromi vlastnosťami, napríklad dodávanú *Meď*
(*Copper*), a v manuáli elektrostatiky si pozrite, na čo sa každá z nich používa.

Neexistuje žiadna vlastnosť relatívna permeabilita, ktorú by bolo treba
priradiť. Riešič pracuje vo voľnom priestore.

### 2.4 Záložka Okrajové podmienky

Nič na tejto záložke nepatrí kroku magnetostatiky. Podmienky, ktoré ponúka -
*Elektrický potenciál* a *Hustota náboja* - sú elektrostatické a poháňajú
prúdovú hustotu, ktorá zase vytvára magnetické pole. Priraďte ich tak, ako
opisuje manuál elektrostatiky.

Žiadna magnetostatická okrajová podmienka neexistuje ani nie je potrebná - pozri
1.5.

### 2.5 Počiatočné a environmentálne podmienky

Ani jedna záložka pre magnetostatiku nič neponúka. Zobrazená počiatočná podmienka
*Elektrický potenciál* a environmentálna podmienka *Teplota* patria kroku
elektrostatiky.

### 2.6 Nastavenie maticového riešiča

Riešič magnetostatiky nerieši žiadnu sústavu rovníc a nastavenie maticového
riešiča nepoužíva. Položka **CG** nastavuje iba riešič elektrostatiky.

### 2.7 Monitorovacie body

`Úloha` -> `Definovať monitorovacie body` umiestni sondy na zadané súradnice a
vyberie premennú, ktorá sa má zaznamenávať. *Magnetické pole* je v zozname k
dispozícii spolu s elektrostatickými premennými a odčítava uzlové pole v mieste
sondy.

### 2.8 Výsledky a záznamy

Záložka `Výsledky` uvádza **Magnetické pole** ako uzlový vektor, vedľa
elektrostatických výsledkov, ktoré vytvoril prvý krok. Záložka `Záznamy` panela
`Model` obsahuje presne jeden záznam: oba typy úloh sú statické.

`Výkaz` -> `Log súbor riešiča` zobrazí celý výstup: najprv štatistiky
elektrostatiky, potom počet prvkov vedúcich prúd a uzlov, pre ktoré sa
Biotov-Savartov integrál vyhodnotil, a potom štatistiky magnetického poľa.

**Zobrazenie poľa okolo vodiča.** Pole sa počíta v každom uzle, takže ho ukazuje
čokoľvek, čo je okolo vodiča pokryté sieťou. Okolitý priestor pokryte sieťou ako
samostatnú objemovú entitu - oblasť vzduchu - a dajte jej materiál bez
elektrickej vodivosti: krok elektrostatiky ju preskočí, takže nevedie prúd, ale
jej uzly dostanú magnetické pole. Oblasť stojí uzly, ale nie zdroje (1.7), a
nepotrebuje žiadnu konkrétnu veľkosť ani hranicu, pretože sa na nej nič nerieši.

Dodávaný materiál *Vzduch* (*Air*) elektrickú vodivosť má, aj keď nepatrnú, takže
krok elektrostatiky by oblasť zahrnul do vlastného riešenia. Použite jeho kópiu s
odstránenou vlastnosťou elektrická vodivosť. Kontrola nastavenia potom upozorní,
že materiálu entity chýbajú požadované vlastnosti; pre oblasť vzduchu je to
očakávané a výpočet pokračuje.

---

## 3. Tutoriál - pole vodiča s prúdom

**Cieľ.** Spustiť reťazec elektrostatika a magnetostatika od začiatku do konca na
medenej tyči, ktorou tečie prúd, overiť magnetické pole ručným výpočtom a ukázať,
kde sa prejavia aproximácie z 1.4.

Tento tutoriál predpokladá tyč so sieťou z manuálu elektrostatiky - kváder dlhý
`0.1 m` so štvorcovým prierezom `0.01 m`, rozdelený na tetrahedróny, s dvoma
koncovými stenami označenými ako samostatné plošné entity. Ak je tento model
stále nastavený, zmení sa iba poradie krokov.

### Krok 1 - výber úloh

1. `Úloha` -> `Poradie krokov úloh` (`Ctrl+P`).
2. Zaškrtnite **Magnetostatika**. Sledujte, ako sa **Elektrostatika** zaškrtne
   sama.
3. Potvrďte tlačidlom `OK`.

Poradie krokov teraz obsahuje dva kroky, elektrostatiku ako prvú. Záložka `Úloha`
zostane prázdna.

### Krok 2 - priradenie materiálu

Vyberte objemovú entitu a na záložke `Materiál` priraďte **Meď**, alebo materiál
s tromi vlastnosťami, ktoré potrebuje krok elektrostatiky:

- **Relatívna permitivita** = `2.8`
- **Elektrická vodivosť** = `5.882e7` `S/m`
- **Hustota** = `8960` `kg/m^3`

Nič z toho krok magnetostatiky nečíta. Potrebuje ich krok elektrostatiky a bez
prúdovej hustoty nie je žiadne magnetické pole.

### Krok 3 - vytvorenie prúdu

1. Vyberte plošnú entitu na jednom konci, otvorte záložku `Okrajové podmienky` a
   zaškrtnite **Elektrický potenciál**. Nastavte ho na `0` V.
2. Vyberte plošnú entitu na druhom konci, zaškrtnite **Elektrický potenciál** a
   nastavte ho na `0.01` V.

Prúd tyčou je

```
I = sigma * dV * A / L = 5.882e7 * 0.01 * 1e-4 / 0.1 = 588 A
```

Tyč bez rozdielu potenciálov nevedie prúd a dáva magnetické pole presne nulové -
čo je prvá vec, ktorú treba skontrolovať, ak výsledok vyjde prázdny.

### Krok 4 - riešenie

1. `Riešenie` -> `Spustiť riešič` (`Ctrl+R`).
2. Priebeh sledujte v `Výkaz` -> `Log súbor riešiča`.

Log ukazuje krok elektrostatiky s jeho premennými a potom krok magnetostatiky:
počet prvkov vedúcich prúd, počet uzlov a štatistiky magnetického poľa. Oba
prebehnú raz.

### Krok 5 - kontrola výsledku

- Záložka `Výsledky`: zobrazte **Magnetické pole**. Je to uzlový vektor, takže 3D
  pohľad ponúka jeho veľkosť a tri zložky.
- Pole **obieha okolo tyče**, kolmo na prúd, v zmysle pravidla pravej ruky.
- Na **osi** tyče je **nulové** a rastie smerom k povrchu, kde je najväčšie - v
  strede každej dlhej steny, nie v rohoch.
- **Pri koncoch** tyče je **slabšie** ako v strede. To je otvorená prúdová dráha
  z 1.4: pri konci chýba polovica toho, čo by bolo dlhším vodičom.

### Krok 6 - kontrola veľkosti

Pre kruhový vodič s polomerom `R` je povrchové pole `u0*I/(2*pi*R)`. Štvorcová
tyč so stranou `a` nie je kruhová, ale stred steny vo vzdialenosti `a/2` od osi
je tomu blízko: s `a/2 = 0.005 m`,

```
u0*I/(2*pi*r) = 1.2566e-6 * 588 / (2*pi*0.005) = 0.0235 T
```

Vypočítané pole v strede dlhej steny, v polovici dĺžky tyče, je asi `0.020 T`,
teda o nejakých 15 % menej - štvorcový prierez umiestňuje veľkú časť prúdu
ďalej od bodu, než by to urobil kruhový. V rohu, `a/sqrt(2)` od osi, je ešte
nižšie, asi `0.019 T`.

Ostrejšia kontrola potrebuje bod poľa mimo tyče, kde už na priereze nezáleží.
Pridajte oblasť vzduchu podľa 2.8 a odčítajte pole vo vzdialenosti `r = 0.02 m`
od osi, v polovici dĺžky. Pre priamy konečný vodič dĺžky `L`,

```
B = u0*I/(4*pi*r) * 2 * (L/2) / sqrt( (L/2)^2 + r^2 )
  = 1e-7 * 588 / 0.02 * 2 * 0.05 / sqrt(0.0025 + 0.0004) = 5.46e-3 T
```

a vypočítaná hodnota by sa mala zhodovať asi na `0.1 %`, pričom rozdiel spôsobuje
štvorcový prierez tyče. Táto hodnota je pole samotnej tyče, ako vysvetľuje 1.4 -
skutočný obvod by pridal pole svojich prívodov.

### Riešenie problémov

| Príznak | Príčina |
|---|---|
| Magnetické pole je všade nulové a log upozorňuje, že žiadny prvok nevedie prúd | chýba prúdová hustota - skontrolujte, že krok elektrostatiky prebehol ako prvý, že sa potenciály líšia a že materiál má elektrickú vodivosť |
| **Magnetostatiku** nemožno odškrtnúť samostatne | očakávané - vyžaduje *Elektrostatiku* a jej odškrtnutie odškrtne obe |
| Chýba skupina *Riešič v čase* | očakávané - ani jeden typ úlohy nemá časový riešič, pozri 1.10 |
| Pole je na koncoch vodiča slabšie | očakávané - prúdová dráha je otvorená, pozri 1.4 |
| Pole ďaleko od vodiča nezodpovedá obvodu | prívody a zvyšok obvodu nie sú v modeli, pozri 1.4 |
| Pole na čiarovom alebo tenkom plošnom vodiči je nevyspytateľné | očakávané - pole vodiča s nulovým prierezom je tam singulárne, pozri 1.3 |
| Výpočet trvá dlho | náklady sú uzly krát prvky vedúce prúd, pozri 1.7 |

---

## 4. Kontrola modelu

**Priamy vodič.** Pre dlhý vodič s polomerom `R`, ktorým rovnomerne tečie celkový
prúd `I`, je pole obvodové a

```
outside, r > R    B = u0 * I / ( 2 * pi * r )
inside,  r < R    B = u0 * I * r / ( 2 * pi * R^2 )
```

(mimo vodiča a vnútri vodiča) s `u0 = 1.2566e-6`. Pole je najväčšie na povrchu
vodiča, `B = u0*I/(2*pi*R)`, a za ním klesá ako `1/r`. Pre prúd `1 A` vo vodiči s
polomerom `5 mm` je táto špička `4e-5 T`, približne veľkosť magnetického poľa
Zeme.

**Konečný vodič.** Pre priamy vodič dĺžky `L`, vo vzdialenosti `r` od jeho osi a
v polovici dĺžky,

```
B = u0 * I / ( 4 * pi * r ) * L / sqrt( (L/2)^2 + r^2 )
```

čo s rastúcim `L` prechádza na vzorec pre dlhý vodič. Mimo vodiča tvar prierezu
prestáva záležať, takže podľa toho možno overiť akýkoľvek prierez.

**Drôt.** Čiarová entita s prierezom vedie svoj prúd pozdĺž úsečiek a riešič pre
každú z nich používa uzavretý tvar priameho drôtu. Priama čiarová entita preto
reprodukuje vzorec pre konečný vodič presne, až na zaokrúhlenie, v akejkoľvek
vzdialenosti.

**Solenoid.** Pre dlhý solenoid s `n` závitmi na meter, ktorým tečie `I`, je pole
vnútri homogénne a smeruje pozdĺž osi,

```
B = u0 * n * I
```

a mimo neho je takmer nulové. Vinutie možno modelovať ako čiarovú entitu
sledujúcu závity, s prierezom drôtu, a krok elektrostatiky ním poháňa prúd z
potenciálov na jeho dvoch koncoch.

**Divergencia.** Pre akékoľvek magnetické pole platí všade `div(B) = 0`.
Biotov-Savartov zákon ju spĺňa presne, takže tento riešič netestuje; testovala by
iba interpoláciu medzi uzlami.

**Konvergencia so sieťou.** Samotná integrácia je presná (1.3); od siete závisí
prúdová hustota, ktorú krok elektrostatiky počíta ako konštantnú na prvok.
Zjemnenie siete vo vodiči a porovnanie poľa v tom istom bode je kontrolou tejto
časti.

---

## 5. Obmedzenia

### Obmedzenia modelovania

- **Iba voľný priestor.** Do formulácie nevstupuje žiadna permeabilita.
  Neexistuje vlastnosť materiálu relatívna permeabilita, rozlíšenie medzi `B` a
  `H`, magnetizácia ani nasýtenie, takže železo, ferity a tienenie nemožno
  reprezentovať vôbec. Prečo to potrebuje iný riešič, vysvetľuje 1.9.
- **Iba pole modelovaného prúdu.** Pole prívodov a zvyšku obvodu nie je
  zahrnuté. Keď je prúdová dráha v modeli otvorená, pole ďaleko od vodiča nie je
  poľom skutočného obvodu - pozri 1.4.
- **Žiadne permanentné magnety ani vnútené pole.** Jediným zdrojom je vodivý
  prúd vypočítaný krokom elektrostatiky. Magnet, pole pozadia ani vonkajšie
  budenie nemajú žiadnu reprezentáciu.
- **Prúd nemožno zadať priamo.** Prúd vzniká riešením elektrostatiky na vodivom
  telese. Cievku možno modelovať ako čiarovú entitu sledujúcu vinutie, ale jej
  prúd sa aj tak nastavuje cez potenciály.
- **Tenké vodiče sú singulárne.** Pole v uzle na čiarovej entite alebo na okraji
  plošnej entity je regularizovaná hodnota, nie fyzikálna - pozri 1.3.
- **Typy prvkov.** Prúd vedú tetrahedróny, trojuholníky, štvoruholníky a
  dvojuzlové úsečky; ostatné typy prvkov neprispievajú ničím.
- **Náklady.** Práca rastie ako počet uzlov krát počet prvkov vedúcich prúd -
  pozri 1.7.
- **Žiadne odvodené výsledky.** `process()` nič nepočíta, takže vektor
  magnetického poľa je celým výstupom. Neexistuje tok, energia, sila ani veľkosť
  poľa uložená ako samostatná premenná.
- **Výsledok nič nespotrebúva.** Žiadny iný riešič magnetické pole nečíta.
  Neexistuje Lorentzova sila na vodič, indukovaný prúd ani spätná väzba do
  elektrostatickej, tepelnej alebo štrukturálnej fyziky.
- **Iba statická analýza.** Neexistuje časový riešič a pridanie kroku
  magnetostatiky - ktorý so sebou prinesie krok elektrostatiky - odstráni časový
  riešič z celého poradia krokov, takže každá iná fyzika zaradená za ním je tiež
  ustálená.

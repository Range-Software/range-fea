# Prúdenie tekutín - Teoretický manuál a používateľská príručka

Tento dokument opisuje riešiče prúdenia tekutín v Range FEA - `RSolverFluid`,
ktorý rieši typ úlohy **Nestlačiteľné viskózne prúdenie** (*Incompressible
viscous flow*), `RSolverFluidHeat`, ktorý rieši **Prestup tepla v tekutinách**
(*Heat transfer in fluids*), a `RSolverFluidParticle`, ktorý rieši **Rozptyl
kontaminantu** (*Contaminant dispersion*): rovnice, ktoré riešia, význam každého
vstupu, ktorý prijímajú, časti grafického používateľského rozhrania, ktoré ich
ovládajú, a dva podrobne rozpracované tutoriály.

**Obsah**

1. [Teoretický základ](#1-teoretický-základ)
2. [Grafické používateľské rozhranie](#2-grafické-používateľské-rozhranie)
3. [Tutoriál - ustálené prúdenie kanálom](#3-tutoriál---ustálené-prúdenie-kanálom)
4. [Tutoriál - prechodové prúdenie s rozptylom kontaminantu](#4-tutoriál---prechodové-prúdenie-s-rozptylom-kontaminantu)
5. [Kontrola modelu](#5-kontrola-modelu)
6. [Obmedzenia](#6-obmedzenia)

---

## 1. Teoretický základ

### 1.1 Primárne neznáme - pole rýchlosti a tlaku

Primárnymi neznámymi **Nestlačiteľného viskózneho prúdenia** sú vektor
**rýchlosti** `v` v `m/s` a **tlak** `p` v `Pa`, oboje v každom uzle. Každý uzol
nesie **štyri** stupne voľnosti - `vx`, `vy`, `vz` a `p` - takže sieť s `N`
uzlami dáva sústavu `4N` rovníc.

Preto je model prúdenia fyzikálne najnáročnejším typom úlohy v Range FEA: štyri
neznáme na uzol oproti trom pri štrukturálnom modeli a jednej pri tepelnom
modeli, matica, ktorá nie je ani symetrická, ani pozitívne definitná, a
nelineárna iterácia z časti 1.6 okolo každého lineárneho riešenia.

Dva sprievodné typy úloh pridávajú každý jednu skalárnu veličinu, riešenú na
prúdovom poli, ktoré vypočítal riešič prúdenia:

| Typ úlohy | Riešič | Primárna neznáma | Jednotky | Stupne voľnosti na uzol |
|---|---|---|---|---|
| Nestlačiteľné viskózne prúdenie | `RSolverFluid` | rýchlosť a tlak | `m/s`, `Pa` | 4 |
| Prestup tepla v tekutinách | `RSolverFluidHeat` | teplota | `K` | 1 |
| Rozptyl kontaminantu | `RSolverFluidParticle` | koncentrácia častíc | `kg/m^3` | 1 |

Oba sprievodné typy **vyžadujú** v tej istej úlohe krok *Nestlačiteľné viskózne
prúdenie*: pole rýchlosti čítajú z jeho výsledku a samy ho nikdy nepočítajú.

Predvolený stav uzla, ktorého sa nedotýka žiadna podmienka ani predchádzajúci
výsledok, je nulová rýchlosť a nulový tlak.

### 1.2 Riadiace rovnice

Riešič implementuje **Navierove-Stokesove rovnice nestlačiteľného prúdenia** pre
newtonskú tekutinu s konštantnou hustotou `rho` a konštantnou dynamickou
viskozitou `mu`. Bilancia hybnosti a hmotnosti znie

```
rho * ( dv/dt + (v . grad) v ) = -grad(p) + div( mu * ( grad(v) + grad(v)^T ) ) + rho * g

div(v) = 0
```

a pri vypnutom časovom riešiči prechodový člen odpadne a zostane ustálená
bilancia

```
rho * (v . grad) v = -grad(p) + div( mu * ( grad(v) + grad(v)^T ) ) + rho * g

div(v) = 0
```

| Symbol | Význam | Jednotky | Zdroj |
|---|---|---|---|
| `v` | rýchlosť | `m/s` | riešená |
| `p` | tlak | `Pa` | riešený |
| `rho` | hustota | `kg/m^3` | vlastnosť materiálu |
| `mu` | dynamická viskozita | `kg/(m*s)`, teda `Pa*s` | vlastnosť materiálu |
| `g` | gravitačné zrýchlenie | `m/s^2` | environmentálna podmienka Gravitačné zrýchlenie |

Tlak tu nie je termodynamická veličina. Nestlačiteľnosť z neho robí Lagrangeov
multiplikátor, ktorý vynucuje `div(v) = 0`, čo má dva praktické dôsledky, ktoré
treba mať na pamäti pred čítaním akéhokoľvek výsledku:

- Zmysel majú iba **rozdiely tlaku**. Model, v ktorom žiadna podmienka nikde
  nefixuje tlak, určuje tlakové pole iba až na aditívnu konštantu a matica je v
  tejto konštante singulárna - pozri 1.7.
- Tlak reaguje okamžite v celej oblasti. V tejto formulácii neexistuje
  akustická vlna ani rýchlosť zvuku, takže tlak predpísaný na výstupe sa na
  vstupe prejaví v tom istom riešení.

**Reynoldsovo číslo** vytvorené z charakteristickej dĺžky `L` a rýchlosti `V`,

```
Re = rho * V * L / mu
```

je jediné číslo, ktoré rozhoduje o tom, či sa model prúdenia bude správať
rozumne. Približne pod `Re = 1` je konvekčný člen zanedbateľný a úloha je takmer
lineárna; do niekoľkých tisíc je prúdenie laminárne a riešič je vo svojom živle;
výrazne nad touto hodnotou je skutočné prúdenie turbulentné a tento riešič
turbulenciu nemodeluje - pozri časť 6.

### 1.3 Diskretizácia metódou konečných prvkov a stabilizácia

Rýchlosť a tlak sa na každom prvku interpolujú **rovnakými** lineárnymi tvarovými
funkciami - interpolácia rovnakého rádu `P1/P1` a `Q1/Q1`. Táto kombinácia sama
osebe nespĺňa podmienku inf-sup (Babuškovu-Brezziho) a viedla by k
šachovnicovému tlakovému poľu, preto je formulácia **stabilizovaná**. Pridávajú
sa tri stabilizačné členy, každý s vlastným parametrom na úrovni prvku:

| Člen | Účel |
|---|---|
| **SUPG** - streamline upwind Petrov-Galerkin | stabilizuje konvekčný člen, ktorý by inak pri prúdení s prevládajúcou konvekciou vytváral oscilácie od uzla k uzlu |
| **PSPG** - pressure stabilising Petrov-Galerkin | umožňuje použiť rýchlosť a tlak rovnakého rádu a odstraňuje šachovnicový tlakový mód |
| **LSIC** - najmenšie štvorce na podmienke nestlačiteľnosti | tlmí chybu divergencie a zlepšuje zachovanie hmotnosti prvok po prvku |

Parametre SUPG a PSPG sú zostavené z Reynoldsovho čísla prvku

```
h   = element length along the flow direction
Re  = rho * |v| * h / ( 2 * mu )            element Reynolds number

tau = h * Re / ( 6 * |v| )      for  0 < Re <= 3
tau = h / ( 2 * |v| )           for  Re > 3
```

(`h` je dĺžka prvku v smere prúdenia, `Re` Reynoldsovo číslo prvku), takže
proti-prúdová stabilizácia je silná tam, kde v prvku prevláda konvekcia, a
zaniká tam, kde prevláda difúzia. Parameter LSIC je `|v| * h / 2`. Tieto
parametre patria riešiču prúdenia; riešič rozptylu kontaminantu používa vlastný
parameter SUPG obmedzený časovým krokom, pozri časť 1.10.

Parameter SUPG používa **lokálnu** rýchlosť a dĺžku prvku, takže sa prispôsobuje
prvok po prvku. Parameter PSPG namiesto toho používa **globálnu prúdovú
rýchlosť** a globálnu mierku dĺžky prvku, čo udržuje tlakovú stabilizáciu v celej
sieti rovnomernú. Prúdová rýchlosť sa berie z podmienok prítoku pri prvom
prechode každého riešenia - v prechodovom výpočte pri každom časovom kroku - a
nie z riešeného poľa,

```
V_stream = sum( |Q| over inflow surfaces ) / sum( area of inflow surfaces )
```

teda plochou vážená stredná rýchlosť prítoku (súčet `|Q|` cez plochy prítoku
delený súčtom ich plôch), s náhradnou hodnotou `1 m/s`, ak model nemá žiadnu
podmienku prítoku. Mierka dĺžky prvku sa odvodzuje z objemu prvku:
`cbrt(6*V/pi)` pre tetrahedrón, `cbrt(V)` pre hexahedrón.

**Podporované prvky.** Rovnice prúdenia sa zostavujú **iba na objemových
prvkoch**, a to iba na **lineárnych tetrahedrónoch** (`TETRA1`) a **lineárnych
hexahedrónoch** (`HEXA1`). Akýkoľvek iný typ objemového prvku zastaví výpočet s
chybou, ktorá daný typ uvádza. Plošné prvky sa zúčastňujú iba tam, kde nesú
podmienku *Tlak (implicitný)*, a prispievajú tlakovou silou a ničím iným. Bodové
a čiarové entity neprispievajú vôbec - na rozdiel od tepelného modelu nie sú
vodičmi nižšej dimenzie, jednoducho chýbajú.

Toto je najvýraznejší praktický rozdiel oproti ostatným fyzikálnym úlohám v
Range FEA: model tekutiny musí byť **objem so sieťou**. Pred tým, než od riešenia
prúdenia budete niečo očakávať, vygenerujte tetrahedrónovú sieť pomocou
`Geometria` -> `Objem` -> `Generovať tetrahedrónovú sieť`.

### 1.4 Okrajové podmienky

| Okrajová podmienka | Typ | Aplikuje sa na | Zložky |
|---|---|---|---|
| Stena (*Wall*) | explicitná | bod, čiara, plocha | žiadne |
| Stena (bez trenia) (*Wall (frictionless)*) | explicitná | bod, čiara, plocha | žiadne |
| Rýchlosť (prítok) (*Velocity (inflow)*) | explicitná | plocha | Rýchlosť `[m/s]` |
| Objemový prietok (prítok) (*Volumetric flow rate (inflow)*) | explicitná | plocha | Objemový prietok `[m^3/s]` |
| Tlak (explicitný) (*Pressure (explicit)*) | explicitná | plocha | Tlak `[Pa]` |
| Tlak (implicitný) (*Pressure (implicit)*) | prirodzená, silová | plocha | Tlak `[Pa]` |

**Stena** je podmienka nulového sklzu (no-slip). Každému uzlu entity sa zo
sústavy odoberú všetky tri zložky rýchlosti a držia sa na nule. Nemá žiadne
zložky - nie je čo zadať, zaškrtnutie je celé nastavenie. Je to podmienka pre
každú pevnú hranicu, ktorej sa tekutina dotýka.

**Stena (bez trenia)** je podmienka sklzu, alebo symetrie: tekutina sa môže
pohybovať pozdĺž plochy, ale nie cez ňu. Aplikuje sa tak, že pre normálu každého
plošného prvku sa nájde **dominantná globálna os** a na nule sa drží iba táto
jedna zložka rýchlosti, ostatné dve zostanú voľné. Všimnite si, čo to znamená:
podmienka je presná iba pre plochu, ktorej normála leží pozdĺž `x`, `y` alebo
`z`. Na ploche naklonenej voči globálnym osiam obmedzuje nesprávny smer a čím
bližšie je normála k 45 stupňom medzi dvoma osami, tým horšia je aproximácia.
Používajte ju pre rovinné roviny symetrie modelu zarovnaného s globálnymi
osami, inde použite *Stenu*.

**Rýchlosť (prítok)** predpisuje rýchlosť danej veľkosti smerujúcu pozdĺž
**priemernej normály** plošnej entity, do tekutiny. Znamienko sa určuje z
orientácie prvkov entity voči objemu za nimi, takže kladná hodnota znamená
prúdenie vstupujúce do oblasti a záporná prúdenie, ktoré ju opúšťa. Zadáva sa
rýchlosť, nie vektor - smer vyplýva z geometrie.

**Objemový prietok (prítok)** predpisuje to isté z celkového prietoku: riešič
vydelí prietok plochou entity a výslednú strednú rýchlosť aplikuje pozdĺž tej
istej priemernej normály.

```
v = Q / A          A = area of the entity
```

(`A` je plocha entity.) Siahnite po nej vždy, keď je veličinou, ktorú skutočne
poznáte, prietok - parameter čerpadla, potrubie v `m^3/s` - a po *Rýchlosti
(prítok)*, keď poznáte rýchlosť. Obe držia všetky tri zložky rýchlosti v každom
uzle entity, takže obe vytvárajú na vstupe rovnomerný piestový profil; ani jedna
nedokáže na vstupnej ploche vytvoriť vyvinutý profil. Ak na profile záleží, dajte
modelu nábehovú dĺžku.

**Tlak (explicitný)** odoberie tlakový stupeň voľnosti každému uzlu entity a drží
ho na zadanej hodnote. Je to tlakový ekvivalent predpísanej rýchlosti a jeden zo
spôsobov, ako určiť aditívnu konštantu z časti 1.2.

**Tlak (implicitný)** je prirodzený náprotivok: namiesto fixovania uzlového tlaku
aplikuje na plošné prvky **tlakovú silu** a rýchlosť tam ponechá voľnú, takže o
tom, koľko a s akým profilom vyteká, rozhodne tekutina. Sila zahŕňa hydrostatický
člen,

```
t = -( p + rho * g * h ) * n
```

kde `h` je výška uzla meraná **v smere gravitácie od najnižšieho bodu** modelu a
`n` je normála plochy. Smer gravitácie je priemer environmentálnych podmienok
*Gravitačné zrýchlenie* priradených plochám, ktoré nesú túto podmienku. Bez
gravitácie hydrostatický člen zaniká a sila je iba zadaný tlak.

Toto je výstupná podmienka, po ktorej treba siahnuť takmer v každom modeli:
umožňuje vyvinúť sa výstupnému profilu namiesto jeho vynútenia ako rovného a
zadanie `0` robí z výstupu referenciu pri atmosférickom tlaku. *Tlak
(explicitný)* je tuhšia, preskriptívnejšia voľba.

Každá zložka každej z týchto podmienok je **tabuľkou v čase**, takže vstup možno
počas prvých sekúnd prechodového výpočtu postupne nábehovať alebo prietoku dať
pracovný cyklus.

**Priorita.** Rýchlostné a tlakové podmienky sú nezávislé - obmedzujú rôzne
stupne voľnosti, takže vstupná plocha môže niesť rýchlostnú podmienku a výstupná
tlakovú bez vzájomného ovplyvnenia. Medzi podmienkami rovnakého druhu na tej istej
entite prevládne naposledy načítaná a *Stena* vynuluje rýchlosť svojich uzlov po
aplikovaní všetkých ostatných rýchlostných podmienok. Entita, ktorá je
vstupom aj stenou, je preto stenou.

### 1.5 Environmentálne a počiatočné podmienky

#### Environmentálne podmienky

| Environmentálna podmienka | Zložky | Účinok |
|---|---|---|
| Gravitačné zrýchlenie (*Gravitational acceleration*) | `gx`, `gy`, `gz` `[m/s^2]` | objemová sila `rho*g` na entitu a hydrostatický člen podmienky *Tlak (implicitný)* |
| Teplota (*Temperature*) | Teplota `[K]` | teplota okolia; vyberá riadok teplotne závislých tabuliek materiálu |

**Gravitačné zrýchlenie** má predvolenú hodnotu `(0, 0, -9.80665) m/s^2`, teda
gravitáciu v zápornom smere `Z`. Priraďte ho objemovej entite kvôli objemovej
sile a výstupnej ploche, aby sa hydrostatický člen *Tlaku (implicitného)* meral
v správnom smere.

Gravitácia vstupuje do rovnice hybnosti ako konštantná objemová sila `rho*g`.
Keďže hustota je konštantná, rovnomerné gravitačné pole v uzavretej oblasti
vytvorí hydrostatický gradient tlaku a žiadny pohyb - čo je správna odpoveď.
Hustota v tomto riešiči **nezávisí** od teploty, takže neexistuje prúdenie
poháňané vztlakom: výsledok *Prestupu tepla v tekutinách* nikdy nepoháňa
prúdenie, na ktorom bol vypočítaný. Pozri časť 6.

#### Počiatočné podmienky

| Počiatočná podmienka | Účinok |
|---|---|
| Rýchlosť | počiatočné pole rýchlosti, aplikované pri prvom výpočte |
| Tlak | počiatočné tlakové pole, aplikované pri prvom výpočte |
| Teplota | počiatočné teplotné pole pre *Prestup tepla v tekutinách* |
| Koncentrácia častíc | počiatočné pole koncentrácie pre *Rozptyl kontaminantu* |

Počiatočné podmienky sa čítajú **iba pri prvom výpočte**; potom polia pokračujú
z predchádzajúceho výsledku. Práve to umožňuje postup s reštartom z časti 1.7:
prechodový výpočet spustený s voľbou *Reštartovať riešič / pokračovať* prevezme
konvergované ustálené pole namiesto toho, aby opäť začínal z pokoja.

Kde sa stretne okrajová a počiatočná podmienka, okrajová má prednosť.

### 1.6 Nelineárna iterácia

Konvekčný člen `(v . grad) v` je v neznámej kvadratický, takže rovnice prúdenia
sú **nelineárne** a nemožno ich vyriešiť jedným prechodom. Riešič ich linearizuje
a iteruje: každý prechod zostaví reziduum bilancie hybnosti a hmotnosti pri
aktuálnom poli, vyrieši **prírastok** a pripočíta ho,

```
J * dx = -R(x)
x <- x + dx
```

kde `x` obsahuje uzlové rýchlosti a tlaky. Ľavá strana je približný jakobián, nie
presný, takže schéma je modifikovanou Newtonovou iteráciou: v blízkosti riešenia
konverguje pomalšie ako plná Newtonova metóda, ale riešenie, ku ktorému
konverguje, je určené **reziduom** `R`, teda samotnými diskretizovanými
Navierovými-Stokesovými rovnicami.

**Krok je tlmený.** Keďže matica nie je presnou deriváciou rezidua, plný krok
`dx` môže prestreliť a zanechať pole ďalej od riešenia, než bolo na začiatku -
reziduum nasledujúceho prechodu potom vyjde vyššie ako predchádzajúce. Riešič
sleduje presne túto situáciu a aplikuje iba časť vypočítaného kroku,

```
x <- x + omega * dx
```

so začiatkom pri `omega = 1`. Prechod, ktorý reziduum zhoršil, ho zmenší na
polovicu, najviac na spodnú hranicu `0.1`; prechod, ktorý ho zlepšil, nechá
`omega` narásť späť vždy o štvrtinu, takže ústup je rýchly a návrat rozvážny.
Platná hodnota sa v logu vypisuje ako `Relaxation` a pri výpočte, ktorý nikdy
neprestrelí, zostáva `1` - dobre sa správajúci model prúdenia o tomto mechanizme
ani nevie. `omega` sa na začiatku každého riešenia, v prechodovom výpočte teda pri
každom časovom kroku, nastaví späť na `1`.

Z toho vyplývajú dva dôsledky, ktoré určujú celý postup práce s modelom prúdenia:

- **Jeden prechod nikdy nestačí.** Jediná iterácia kroku prúdenia vráti to, čo
  náhodou dal prvý prírastok, a to nie je riešením ničoho. Krok prúdenia musí byť
  zabalený do **skupiny krokov úlohy** s počtom iterácií - stovky pre ustálený
  výpočet, desiatky na časový krok pre prechodový.
- **Počet je horná hranica, nie cieľ.** Skupina nesie aj **hodnotu
  konvergencie** a ukončí iterácie, len čo krok prúdenia ohlási konvergenciu pod
  touto hodnotou. Nastavte počet dostatočne vysoko pre najťažší krok a nechajte
  hodnotu konvergencie zastaviť každý krok, keď sa ustáli.

Po každej iterácii riešič zapíše do súboru konvergencie a do logu štyri čísla:

| Veličina | Význam |
|---|---|
| `Residual` | norma vektora rezidua, vzdialenosť od riešenia |
| `Convergence-R` | zmena rezidua od predchádzajúcej iterácie |
| `Convergence-V` | relatívna veľkosť prírastku rýchlosti, `\|\|dv\|\| / \|\|v\|\|` |
| `Convergence-P` | relatívna veľkosť prírastku tlaku, `\|\|dp\|\| / \|\|p\|\|` |

S hodnotou konvergencie sa porovnávajú `Convergence-V` a `Convergence-P`. Obe sú
bezrozmerné a obe klesajú, ako sa iterácia ustaľuje: merajú, aký veľký krok pole
ešte robí v pomere k veľkosti samotného poľa, takže hodnota `1e-5` znamená, že
riešenie sa v danom prechode posunulo o stotisícinu seba samého.

Malý krok však sám osebe nie je riešením. Takmer singulárna sústava - model bez
tlakovej referencie alebo sieť, ktorá neunesie požadované Reynoldsovo číslo -
robí malé kroky, pretože sa **nemôže pohnúť**, nie preto, že dorazila k
riešeniu, a jej reziduum stojí na mieste alebo rastie. Test má preto dve časti a
skupina sa zastaví, iba ak platia obe:

| Časť | Podmienka |
|---|---|
| Pole sa prestalo pohybovať | `Convergence-V` aj `Convergence-P` sú pod hodnotou konvergencie |
| Pole skutočne kleslo | reziduum kleslo na **desatinu** hodnoty pri prvom prechode tohto riešenia |

Druhá časť je pevná a nedá sa nastaviť - je kontrolou zmysluplnosti prvej, nie
druhým parametrom na ladenie. Log ju vypisuje ako `Residual ratio` spolu s
cieľovou hodnotou, takže je vidieť, ako blízko sa k zastaveniu dostal výpočet,
ktorý sa nezastavil skôr. V prechodovom výpočte je každý časový krok samostatným
riešením, takže pomer sa meria znova od začiatku každého kroku.

`Výkaz` -> `Konvergencia riešiča` zobrazí všetky štyri čísla. Konvergovaný
ustálený výpočet ukazuje reziduum klesajúce o niekoľko rádov a potom sa
vyrovnávajúce, pričom `Convergence-V` a `Convergence-P` monotónne klesajú k nule.
Reziduum, ktoré sa vyrovná vysoko, osciluje alebo rastie, znamená, že výpočet
nekonvergoval - a skupina pri takom výpočte predčasne neskončí, nech robia
prírastky čokoľvek.

Hodnota konvergencie `0` vypne predčasné ukončenie a spustí plný počet iterácií,
čo zodpovedá správaniu riešiča pred zavedením tejto hodnoty. Použite ju, ak
chcete pevné množstvo práce na krok bez ohľadu na to, čo robí reziduum.

### 1.7 Ustálená analýza

Pri vypnutom časovom riešiči riešič iteruje ustálenú bilanciu hybnosti a
hmotnosti k pevnému bodu. Lineárna sústava každej iterácie nie je symetrická,
preto sa rieši metódou **GMRES** s Jacobiho predpodmienením, a nie metódou
združených gradientov, ktorú používajú symetrické fyzikálne úlohy.

Ustálený model prúdenia potrebuje minimálne:

- **objem** so sieťou s priradenou hustotou a dynamickou viskozitou;
- **Stenu** na každej pevnej hranici;
- podmienku **prítoku** alebo rozdiel tlakov, ktorý poháňa prúdenie;
- **tlakovú referenciu** - plochu s *Tlakom (explicitným)* alebo *Tlakom
  (implicitným)*. Bez nej je tlak určený iba až na konštantu a iterácia sa
  neustáli.

Model, ktorého hranice tvoria výlučne steny a predpísané rýchlosti, má ešte jednu
pascu: predpísaný prítok a odtok musia byť **v rovnováhe**, pretože nestlačiteľná
tekutina sa nemôže hromadiť. Predpísanie 5 m^3/s na vstupe a 4 m^3/s na výstupe
žiada nemožné a riešenie nekonverguje. Ponechanie výstupu ako *Tlak
(implicitný)* túto otázku úplne obíde, čo je hlavný dôvod uprednostniť ho.

Obvyklým postupom je najprv nechať skonvergovať ustálený výpočet a použiť ho ako
počiatočné pole prechodového výpočtu, pretože prechodový výpočet spustený z
pokoja strávi prvé časové kroky riešením nábehového deja namiesto fyziky, o
ktorú ide.

### 1.8 Prechodová analýza

Pri zapnutom časovom riešiči riešič postupuje v čase nestacionárnymi rovnicami
pomocou theta schémy, kde `alpha` je koeficient aproximácie časového kroku:

| Aproximácia | `alpha` | Výsledná schéma | Stabilita |
|---|---|---|---|
| Spätná diferencia (*Backward difference (stable)*) | `1` | plne implicitný krok | bezpodmienečná |
| Centrálna diferencia (*Central difference (accurate)*) | `0.5` | Crank-Nicolson | bezpodmienečná, môže oscilovať |
| Dopredná diferencia (*Forward difference (fast)*) | `0` | explicitný krok | podmienečná |

Pre model prúdenia použite **spätnú diferenciu**. Čas stojí nelineárna iterácia v
rámci každého časového kroku a implicitná schéma umožňuje zvoliť časový krok
podľa fyziky, a nie podľa siete.

Aj pri implicitnej schéme je časový krok modelu prúdenia obmedzený presnosťou,
prostredníctvom **Courantovho čísla**

```
C = |v| * dt / h
```

kde `h` je veľkosť prvku v smere prúdenia. Courantovo číslo okolo `1` znamená, že
tekutina prejde za krok približne jeden prvok, čo je správny rád na zachytenie
prechodového deja; pri oveľa väčšom čísle riešič rozmazáva transport, ktorý má
zachytiť, bez ohľadu na stabilitu schémy. Zvoľte `dt` podľa siete a očakávanej
rýchlosti:

```
dt ~ h / |v|
```

Každý časový krok spúšťa plnú nelineárnu iteráciu z časti 1.6, takže cena
prechodového výpočtu je počet časových krokov krát počet iterácií na krok. Desať
až päťdesiat iterácií na krok je typický rozsah, keď je prúdové pole už
konvergované z ustáleného výpočtu. Krok, ktorého reziduum na konci iterácií
neklesne, je krok s nekonvergovanou odpoveďou a chyba sa prenáša do každého
nasledujúceho kroku.

### 1.9 Prestup tepla v tekutinách

**Prestup tepla v tekutinách** (`RSolverFluidHeat`) rieši advekciu a difúziu
teploty na prúdovom poli,

```
rho * c * ( dT/dt + v . grad(T) ) = div( k * grad(T) ) + q
```

s rovnakou stabilizáciou SUPG konvekčného člena ako riešič prúdenia a s rovnakou
theta schémou v čase. Rýchlosť `v` sa **preberá z výsledku prúdenia**, nerieši sa
tu - preto tento typ úlohy vyžaduje krok *Nestlačiteľné viskózne prúdenie*.

| Symbol | Význam | Jednotky | Zdroj |
|---|---|---|---|
| `T` | teplota | `K` | riešená |
| `v` | rýchlosť | `m/s` | výsledok prúdenia |
| `k` | tepelná vodivosť | `W/(m*K)` | vlastnosť materiálu |
| `c` | tepelná kapacita | `J/(kg*K)` | vlastnosť materiálu |
| `rho` | hustota | `kg/m^3` | vlastnosť materiálu |
| `q` | hustota tepelného zdroja | `W/m^3` | podmienka Teplo, Joulovo teplo, sálanie |

Rozdiel oproti obyčajnému typu úlohy **Prestup tepla** je práve člen
`v . grad(T)`: energia je unášaná pohybujúcou sa tekutinou, nielen vedená. To
tiež znamená, že tento riešič nemá konvekčné okrajové podmienky - nie je čo
korelovať, pretože teplo odchádzajúce zo steny do tekutiny je rozlíšené sieťou, a
nie modelované.

| Okrajová podmienka | Typ | Aplikuje sa na | Zložky |
|---|---|---|---|
| Teplota (*Temperature*) | explicitná (Dirichletova) | bod, čiara, plocha, objem | Teplota `[K]` |
| Teplo (*Heat*) | prirodzená, zdroj | bod, čiara, plocha, objem | Teplo `[W]` |

**Teplota** drží uzly svojej entity na zadanej hodnote. Priraďte ju vstupnej
ploche na nastavenie teploty tekutiny vstupujúcej do oblasti a stene na
modelovanie ohrievanej alebo chladenej hranice.

**Teplo** je zdroj a zadáva sa ako **celkový výkon vo `W`** privedený do entity,
ktorý sa pred zostavením rozloží na mieru entity - rovnaká konvencia ako v
tepelnom riešiči. *Tepelný výkon (na jednotku plochy)* a *Tepelný výkon (na
jednotku objemu)* rozhranie pre tento typ úlohy ponúka, ale riešič prestupu tepla
v tekutinách ich **nečíta**; ako zdroj tu použite *Teplo*.

**Väzba.** Stena medzi tekutinou so sieťou a pevnou látkou so sieťou, ktorá nesie
podmienku **Nútená konvekcia** (*Forced convection*) obyčajného riešiča *Prestupu
tepla*, tieto dve oblasti prepája. Po prebehnutí tepelného riešiča drží riešič
prestupu tepla v tekutinách uzly steny na teplote pevnej látky - dovtedy je stena
izolovaná - a pre každý prvok steny vráti dvojicu `(h, Tf)`, ktorá reprodukuje
tepelný tok, ktorý jeho riešenie cez stenu prenáša. Tepelný riešič túto dvojicu
aplikuje ako konvekčnú podmienku. Tok je reziduom rovníc tekutiny v uzloch steny,
konzistentný s diskretizáciou, takže zostáva presný aj v tenkej tepelnej hraničnej
vrstve. Teplota steny sa z prechodu na prechod relaxuje Aitkenovým faktorom, čo
výmenu ustáli v niekoľkých prechodoch. Teoretický manuál prestupu tepla opisuje
túto schému v časti *Conjugate heat transfer*. Sálavé a Joulovo teplo prichádzajúce
z iných riešičov sa pripočítavajú k zdrojovému členu rovnako ako v tepelnom
riešiči.

Bez väzby riešič prestupu tepla v tekutinách hlási konvergenciu bezpodmienečne -
po fixovaní prúdového poľa je lineárny, takže jedno riešenie je konečnou
odpoveďou. Skupina obsahujúca oba kroky preto skončí, keď skonverguje krok
prúdenia. So spriahnutými stenami namiesto toho hlási relatívnu zmenu svojho
teplotného poľa, takže skupina iteruje, kým sa teplota steny neustáli.

Výsledky: **Teplota** ako uzlový skalár a **Tepelný tok** ako prvkový vektor,
vodivý tok `-k*grad(T)`.

### 1.10 Rozptyl kontaminantu

**Rozptyl kontaminantu** (`RSolverFluidParticle`) prenáša na tom istom prúdovom
poli skalárnu koncentráciu,

```
dC/dt + v . grad(C) = div( D * grad(C) ) + s_eff

s_eff = s * ( 1 - C / C_sat )     for s > 0 and a maximum saturation set
s_eff = s                         otherwise
```

(obmedzenie `s * ( 1 - C / C_sat )` platí pre `s > 0` pri nastavenom maximálnom
nasýtení, inak `s_eff = s`), opäť so stabilizáciou SUPG a rovnakou schémou v čase a
opäť s rýchlosťou prevzatou z výsledku prúdenia.

| Symbol | Význam | Jednotky | Zdroj |
|---|---|---|---|
| `C` | koncentrácia častíc | `kg/m^3` | riešená |
| `v` | rýchlosť | `m/s` | výsledok prúdenia |
| `D` | koeficient difúzie | `m^2/s` | *Nastavenia rozptylu kontaminantu*, predvolene `0` |
| `C_sat` | maximálne nasýtenie | `kg/m^3` | *Nastavenia rozptylu kontaminantu*, predvolene `0` (bez obmedzenia) |
| `s` | rýchlosť zdroja častíc | `kg/(m^3*s)` | podmienka Rýchlosť tvorby častíc (*Particle rate*) |

#### Nastavenia rozptylu kontaminantu

Oba parametre tohto typu úlohy sa nastavujú raz pre celý model v skupine
*Nastavenia rozptylu kontaminantu* na záložke `Úloha` (časť 2.2). Obe majú
predvolenú hodnotu nula, čo zodpovedá čistej advekcii s neobmedzeným zdrojom.

**Koeficient difúzie** `D` rozširuje kontaminant v smere klesajúcej koncentrácie.
Pri `D = 0` je transport čisto advektívny: kontaminant je unášaný prúdením a
rozširuje sa iba numerickou difúziou stabilizácie, takže riešič odpovedá na
otázku „kam ho prúdenie zanesie a ako dlho to trvá", nie „aký široký je oblak".
Kladné `D` robí zo šírky oblaku fyzikálny výsledok.

Model turbulencie neexistuje (časť 6), takže `D` je **efektívny** koeficient
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

Steny a výstupy bez podmienky *Koncentrácia častíc* si ponechávajú prirodzenú
podmienku **nulového difúzneho toku**: kontaminant cez stenu nedifunduje a
výstupom odchádza iba s prúdením.

Tam, kde prúdenie takou hranicou **vstupuje** - typicky spätné prúdenie cez časť
výstupu s *Tlakom (implicitným)* - sa považuje za čistú tekutinu: riešič na tejto
časti plochy pridá slabý vstupný člen `max(-v . n, 0) * C`. Bez neho by hranica
dodávala riešeniu energiu a koncentrácia na výstupe by neobmedzene rástla. Ak
vracajúca sa tekutina nie je čistá, predĺžte oblasť tak, aby výstupom prúdenie
iba odchádzalo.

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

#### Stabilizácia

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
koncentráciu pred čelom oblaku.

Galerkinov advekčný člen sa zostavuje v **šikmo symetrickom tvare**,
`v . grad(C) + 0.5 * div(v) * C`, so spojitou uzlovou rýchlosťou. Rýchlosť z
riešiča prúdenia nikdy nie je presne nedivergentná a bez tohto tvaru môže
diskretizácia sama vytvárať koncentráciu - šachovnicový vzor, ktorý pri malom
časovom kroku narastá z kroku na krok. V šikmo symetrickom tvare advekcia
kontaminant iba presúva; pre presne nedivergentnú rýchlosť je pridaný člen
nulový.

Na rozdiel od riešiča prúdenia (časť 6) riešič rozptylu aplikuje theta váhovanie
konzistentne, takže *Centrálna diferencia* je tu skutočnou Crankovou-Nicolsonovou
schémou.

#### Okrajové podmienky

| Okrajová podmienka | Typ | Aplikuje sa na | Zložky |
|---|---|---|---|
| Koncentrácia častíc (*Particle concentration*) | explicitná (Dirichletova) | bod, čiara, plocha | Koncentrácia častíc `[kg/m^3]` |
| Rýchlosť tvorby častíc (*Particle rate*) | prirodzená, zdroj | bod, čiara, plocha, objem | Rýchlosť tvorby častíc `[kg/(m^3*s)]` |

**Koncentrácia častíc** drží svoje uzly na zadanej koncentrácii. Priradená
vstupnej ploche s tabuľkou v čase je spôsobom, ako modelovať únik: nula pred
udalosťou, koncentrácia úniku počas nej, nula potom. Je to najužitočnejší postup
pri tomto type úlohy a stavia naň aj tutoriál *Rozptyl kontaminantu v
tekutinách*.

**Rýchlosť tvorby častíc** je zdroj a zadáva sa ako **hustota** v `kg/(m^3*s)`, nie
ako celková hodnota - na rozdiel od podmienky *Teplo* sa nerozkladá na mieru
entity. Modeluje zdroj, ktorý vnútri oblasti nepretržite uvoľňuje kontaminant.

Tento typ úlohy vyžaduje iba **hustotu** materiálu.

Výsledky: **Koncentrácia častíc** ako uzlový skalár a pri nastavenom maximálnom
nasýtení **Relatívne nasýtenie** (*Relative saturation*) `C / C_sat` ako
bezrozmerný uzlový skalár. Relatívne nasýtenie sa predvolene zobrazuje od `0` do
`1` a odstráni sa, keď sa maximálne nasýtenie nastaví späť na nulu.

### 1.11 Odvodené výsledky

| Výsledok | Aplikuje sa na | Vytvára | Význam |
|---|---|---|---|
| Rýchlosť `[m/s]` | uzol | prúdenie | vyriešené pole rýchlosti, tri zložky |
| Tlak `[Pa]` | uzol | prúdenie | vyriešené tlakové pole |
| Teplota `[K]` | uzol | prestup tepla v tekutinách | vyriešené teplotné pole |
| Tepelný tok `[W/m^2]` | prvok | prestup tepla v tekutinách | vodivý tok `-k*grad(T)` |
| Koncentrácia častíc `[kg/m^3]` | uzol | rozptyl kontaminantu | vyriešené pole koncentrácie |
| Relatívne nasýtenie `[-]` | uzol | rozptyl kontaminantu | `C / C_sat`, iba pri nastavenom maximálnom nasýtení |

Rýchlosť sa ukladá ako uzlový **vektor**, takže záložka `Výsledky` môže zobraziť
jej veľkosť farebne, jednotlivé zložky alebo šípky. Pre výsledok prúdenia sa
oplatí poznať dva zobrazovacie nástroje:

- **rez** objemom, pretože zaujímavá štruktúra prúdenia je vnútri oblasti, nie na
  jej povrchu;
- **prúdnice**, vytvorené cez `Geometria` -> `Prúdnica`, ktoré sledujú pole
  rýchlosti a sú najrýchlejším spôsobom, ako uvidieť recirkuláciu alebo mŕtvu
  zónu.

Po každom zázname vypíše log riešiča štatistiky rýchlosti a tlaku a hodnoty vo
všetkých monitorovacích bodoch.

### 1.12 Ustálená alebo prechodová analýza?

| | Ustálená | Prechodová |
|---|---|---|
| Rieši | ustálenú bilanciu, iteračne | nestacionárne rovnice, krokovaním v čase |
| Odpovedá na | „ako vyzerá vyvinuté prúdenie" | „ako sa prúdenie vyvinie a čo unáša" |
| Iterácie | stovky, v jednej skupine krokov | desiatky na časový krok |
| Záznamy | jeden | jeden na každý zapísaný časový krok |
| Potrebuje tlakovú referenciu | áno | áno |

**Ustálenú analýzu použite pre** vyvinuté prúdové pole v potrubí, rozdeľovači
alebo kanáli výmenníka tepla, tlakovú stratu na komponente a ako počiatočné pole
pre akýkoľvek prechodový výpočet. Tu začína takmer každý model prúdenia.

**Prechodovú analýzu použite pre** nábeh a odstavenie, pulzujúci alebo cyklický
vstup, odtrhávanie vírov a - predovšetkým - pre **rozptyl kontaminantu**, ktorý je
úlohou transportu v čase a pre udalosť úniku nemá zmysluplnú ustálenú odpoveď.

Dvojkrokový postup z tutoriálu *Rozptyl kontaminantu v tekutinách* je
štandardný: nechajte skonvergovať ustálené prúdenie a potom reštartujte so
zapnutým časovým riešičom a krokom rozptylu kontaminantu.

---

## 2. Grafické používateľské rozhranie

Všetko podstatné sa nachádza v paneli **Riešič** (záložky `Úloha`,
`Okrajové podmienky`, `Počiatočné podmienky`, `Environmentálne podmienky`,
`Materiál`, `Výsledky`), v menu **Úloha** a v paneli **Model**.

### 2.1 Výber úlohy

`Úloha` -> `Poradie krokov úloh` (`Ctrl+P`) otvorí dialóg poradia krokov úloh.
Pridajte krok a vyberte **Nestlačiteľné viskózne prúdenie** - „Ustálené a
prechodové prúdenie newtonských tekutín".

Krok prúdenia patrí do **skupiny krokov úlohy**, ktorá nesie dve hodnoty
riadiace nelineárnu iteráciu z časti 1.6. Každú z nich upravíte dvojitým
kliknutím:

| Hodnota | Význam |
|---|---|
| **počet iterácií** | najväčší počet prechodov, ktoré môže urobiť jedno riešenie - alebo jeden časový krok |
| **Konvergencia** | skupina ukončí prechody, keď sa krok prúdenia ustáli pod touto hodnotou **a** jeho reziduum kleslo; `0` vždy spustí plný počet |

Predvolená hodnota konvergencie je `1e-5`, čo je pre model prúdenia bezpečná
relatívna veľkosť kroku. Umiestniť krok prúdenia mimo skupiny, kde prebehne
presne raz, nemá zmysel - pozri 1.6.

Kombinácie, ktoré sa oplatí poznať:

- samotné *Nestlačiteľné viskózne prúdenie* v skupine s niekoľkými stovkami
  iterácií - ustálené prúdové pole;
- *Nestlačiteľné viskózne prúdenie* a *Rozptyl kontaminantu* v jednej skupine -
  rozptyl na prúdení, klasická prechodová dvojica;
- *Nestlačiteľné viskózne prúdenie* a *Prestup tepla v tekutinách* v jednej
  skupine - ohrievané alebo chladené prúdenie;
- *Prestup tepla v tekutinách* a samostatný krok *Prestup tepla* na pevnej látke
  so sieťou, krok prestupu tepla v tekutinách ako prvý, v jednej skupine s
  niekoľkými iteráciami - združený prestup tepla cez steny s podmienkou *Nútená
  konvekcia*.

Prúdenie vylučuje *Prúdenie cez porézne prostredie* a *Modálnu analýzu*; oba
sprievodné typy úloh vyžadujú prítomnosť kroku prúdenia.

### 2.2 Záložka Úloha

**Riešič v čase** - zobrazuje sa pre všetky tri typy úloh prúdenia tekutín,
pretože všetky sú časovo závislé. Pre ustálený výpočet prúdenia ho nechajte
vypnutý.

| Pole | Význam |
|---|---|
| Povoliť | zapína a vypína krokovanie v čase |
| Aproximácia | theta schémy v čase: spätná, centrálna alebo dopredná, pozri 1.8 |
| Počiatočný čas | prvá hodnota času v sekundách; pri reštarte riešiča sa ignoruje |
| Konečný čas | `počiatočný čas + veľkosť časového kroku * počet časových krokov`, iba na čítanie |
| Veľkosť časového kroku | `dt` v sekundách, pozri odporúčanie Courantovho čísla v 1.8 |
| Počet časových krokov | koľko krokov sa má vypočítať |
| Frekvencia výstupu | zapíše súbor výsledkov každých N krokov; `0` zapíše iba posledný krok |

**Nastavenia rozptylu kontaminantu** - zobrazuje sa, keď je v poradí krokov úloh
krok *Rozptyl kontaminantu*. Obsahuje dva parametre z časti 1.10:

| Pole | Význam |
|---|---|
| Maximálne nasýtenie `[kg/m^3]` | najväčšia možná koncentrácia; kladná rýchlosť tvorby častíc sa pri nej zastaví a počíta sa *Relatívne nasýtenie*. `0` znamená bez obmedzenia |
| Koeficient difúzie `[m^2/s]` | efektívny koeficient difúzie kontaminantu v tekutine. `0` znamená čistú advekciu |

Typické hodnoty oboch sú uvedené v časti 1.10.

Typy úloh prúdenia a prestupu tepla v tekutinách vlastnú skupinu nastavení
nemajú. Všetko ostatné sa priraďuje entitám na záložkách podmienok a počet
iterácií - na ktorom tu záleží viac ako kdekoľvek inde v Range FEA - sa nastavuje
v dialógu poradia krokov úloh, nie na tejto záložke.

### 2.3 Záložka Materiál

Záložka `Materiál` zobrazuje materiál priradený vybranej entite. Čo je potrebné,
závisí od toho, ktoré typy úloh prúdenia tekutín sú v poradí krokov úloh:

| Vlastnosť | Jednotky | Vyžaduje | Poznámka |
|---|---|---|---|
| Hustota | `kg/m^3` | všetky tri | `rho`, približne `1000` pre vodu, `1.2` pre vzduch |
| Dynamická viskozita | `kg/(m*s)` | prúdenie, prestup tepla v tekutinách | `mu`, približne `1.0e-3` pre vodu, `1.8e-5` pre vzduch |
| Tepelná vodivosť | `W/(m*K)` | prestup tepla v tekutinách | `k`, približne `0.6` pre vodu, `0.025` pre vzduch |
| Tepelná kapacita | `J/(kg*K)` | prestup tepla v tekutinách | `c`, približne `4180` pre vodu, `1005` pre vzduch |

Entita, ktorej chýba niektorá vlastnosť vyžadovaná typom úlohy v poradí krokov,
sa nerieši a kontrola nastavení na to pred spustením výpočtu upozorní. Dodaná
databáza materiálov obsahuje **Vodu** a **Vzduch**, čo zvyčajne tento krok celý
vybaví.

Všimnite si, že sa požaduje **dynamická** viskozita `mu` v `Pa*s`, nie
kinematická viskozita `nu = mu/rho` v `m^2/s`. Zadanie `1.0e-6` pre vodu, lebo
je to známe číslo, je najčastejšou chybou materiálu v modeli prúdenia a urobí
tekutinu tisíckrát menej viskóznou, než má byť - čo sa zvyčajne prejaví
výpočtom, ktorý nekonverguje.

Každá vlastnosť je tabuľkou v závislosti od teploty, vyhodnocovanou pri teplote
prvku z predchádzajúceho výpočtu.

### 2.4 Záložka Okrajové podmienky

Záložka je rozdelená na zoznam dostupných okrajových podmienok hore a editor
vybranej podmienky dole. Najprv vyberte entitu v strome `Model`; zoznam potom
ponúkne podmienky platné pre daný typ entity a vybrané typy úloh. Zaškrtnutím
podmienku priradíte a potom upravíte jej zložky v dolnom strome.

Ponúkajú sa podmienky z častí 1.4, 1.9 a 1.10. Poznámky k ich použitiu:

- **Stena** patrí na každú pevnú hranicu a nemá žiadnu hodnotu. Plošná entita
  objemu so sieťou, ktorá nenesie žiadnu podmienku, *nie je* stenou - je to
  neobmedzená hranica, čo je zriedka to, čo bolo zamýšľané. Steny sú prvá vec,
  ktorú treba skontrolovať pri modeli, ktorý sa správa zvláštne.
- **Stena (bez trenia)** je určená pre roviny symetrie zarovnané s globálnymi
  osami. Na naklonenej ploche obmedzuje nesprávny smer - pozri 1.4.
- **Rýchlosť (prítok)** aj **Objemový prietok (prítok)** poháňajú prúdenie cez
  plochu pozdĺž jej priemernej normály. Vyberte tú, ktorá zodpovedá číslu, ktoré
  skutočne máte. Záporná hodnota poháňa prúdenie opačne, čo je jeden zo spôsobov,
  ako zadať výstup.
- **Tlak (implicitný)** je uprednostňovaná výstupná podmienka: umožňuje vyvinúť
  sa výstupnému profilu a so zadanou hodnotou `0` robí z výstupu tlakovú
  referenciu. **Tlak (explicitný)** namiesto toho fixuje uzlový tlak.
- **Teplota** a **Teplo** sa objavia, keď je v poradí krokov úloh krok
  *Prestup tepla v tekutinách*; **Koncentrácia častíc** a **Rýchlosť tvorby
  častíc**, keď je tam krok *Rozptyl kontaminantu*.
- Každá zložka je tabuľkou v čase. Kliknutím na **Upraviť časovo závislé
  hodnoty** otvoríte editor zložiek, kde sa hodnoty zadávajú v závislosti od času;
  medzi dvoma zadanými časmi sa hodnota lineárne interpoluje a po poslednom
  zadanom čase zostáva platná posledná hodnota. Takto sa modeluje udalosť úniku,
  postupne nabiehajúci vstup alebo pracovný cyklus.

### 2.5 Počiatočné a environmentálne podmienky

Záložka `Počiatočné podmienky` ponúka pre úlohu prúdenia *Rýchlosť* a *Tlak*, pre
prestup tepla v tekutinách *Teplotu* a pre rozptyl *Koncentráciu častíc*.
Nastavujú počiatočné pole prvého výpočtu a umožňujú, aby prechodový výpočet
nezačínal z pokoja.

Záložka `Environmentálne podmienky` ponúka *Gravitačné zrýchlenie* s predvolenou
hodnotou `(0, 0, -9.80665) m/s^2`. Priraďte ho objemu kvôli objemovej sile a
každej ploche s podmienkou *Tlak (implicitný)*, aby sa hydrostatický člen tejto
podmienky meral v správnom smere.

### 2.6 Nastavenie maticového riešiča

`Úloha` -> `Nastaviť maticový riešič úloh` konfiguruje iteračné riešiče. Všetky
tri riešiče prúdenia tekutín používajú položku **GMRES**, pretože ich matice nie
sú symetrické.

Pred akoukoľvek zmenou tu rozlišujte dve vnorené iterácie: iterácie **maticového
riešiča** riešia jednu linearizovanú sústavu, kým iterácie **skupiny krokov
úlohy** riešia nelinearitu z časti 1.6. Model prúdenia, ktorý nekonverguje,
takmer vždy potrebuje viac iterácií skupiny, lepšiu sieť alebo opravené okrajové
podmienky - nie vyšší počet iterácií GMRES.

Výnimkou je **ustálený rozptyl kontaminantu s difúziou**. Ide o jediné lineárne
riešenie, takže neexistujú iterácie skupiny, na ktoré by sa dalo spoľahnúť, a
sústava s prevládajúcou difúziou konverguje pri predvolenom limite GMRES 10
vnútorných krát 10 vonkajších iterácií pomaly. Maticový riešič pri dosiahnutí
limitu neupozorní: prečítajte si jeho tabuľku iterácií v logu riešiča a ak
dobehne do poslednej vonkajšej iterácie s reziduom stále výrazne nad hodnotou
konvergencie riešiča, zvýšte počet vonkajších iterácií. Nekonvergované riešenie
dá nesprávnu odpoveď bez akéhokoľvek iného náznaku.

### 2.7 Monitorovacie body

`Úloha` -> `Definovať monitorovacie body` umiestni sondy na zadané súradnice a
vyberie zaznamenávanú veličinu - *Rýchlosť*, *Tlak*, *Teplotu*, *Koncentráciu
častíc*, *Rýchlosť tvorby častíc* alebo *Relatívne nasýtenie*. V prechodovom
výpočte zobrazí históriu `Výkaz` -> `Monitorovacie body`.

Pre model rozptylu je monitorovací bod v mieste záujmu odpoveďou vo forme, akú
zvyčajne chcete: koncentrácia v čase na danom mieste, odčítaná z grafu, a nie z
postupnosti obrázkov.

### 2.8 Výsledky a záznamy

Záložka `Výsledky` zobrazuje vypočítané veličiny a ovláda 3D zobrazenie.
Rýchlosť je uzlový vektor, tlak uzlový skalár, teplota uzlový skalár, tepelný tok
prvkový vektor a koncentrácia častíc a relatívne nasýtenie sú uzlové skaláry.

Záložka `Záznamy` panela `Model` zobrazuje záznamy výsledkov:

- **ustálená analýza** - jediný záznam;
- **prechodová analýza** - jeden záznam na každý zapísaný časový krok.
  Animovanie záznamov prehrá prúdenie a tlačidlo **Záznam** v dolnej časti stromu
  zapíše animáciu do video súboru.

`Výkaz` -> `Konvergencia riešiča` zobrazí históriu konvergencie opísanú v 1.6 a
po každom výpočte prúdenia je to výkaz, ktorý treba otvoriť ako prvý. `Výkaz` ->
`Log súbor riešiča` zobrazí úplný výstup riešiča vrátane štatistík rýchlosti a
tlaku po každom zázname.

---

## 3. Tutoriál - ustálené prúdenie kanálom

**Cieľ.** Vypočítať vyvinuté prúdové pole a tlakovú stratu tekutiny poháňanej
kanálom a overiť, že výsledok je konvergovaný.

Tento tutoriál predpokladá objemový model so sieťou - dodaný model
**Channel.tmsh** je presne takýto prípad a používa ho aj tutoriál *Rozptyl
kontaminantu v tekutinách*. Ak si zostavujete vlastný, nakreslite kváder,
rozdeľte ho na tetrahedróny a označte vstupnú plochu, výstupnú plochu a ostatné
plochy ako tri samostatné plošné entity, aby im bolo možné priradiť podmienky.

### Krok 1 - výber úlohy

1. `Úloha` -> `Poradie krokov úloh` (`Ctrl+P`).
2. Pridajte **skupinu krokov úlohy** a dvojitým kliknutím na hodnotu nastavte
   jej **počet iterácií** na `2000`.
3. **Konvergenciu** ponechajte na predvolenej hodnote `1e-5`.
4. Do skupiny pridajte krok **Nestlačiteľné viskózne prúdenie**.
5. Potvrďte tlačidlom `OK`.

O tieto dve hodnoty v tomto kroku ide predovšetkým. Výpočet urobí najviac 2000
prechodov a zastaví sa, len čo sa pole ustáli pod `1e-5` - pozri 1.6. Počet
nastavte veľkoryso: je to strop a výpočet zvyčajne ukončí hodnota konvergencie.

### Krok 2 - generovanie siete

`Geometria` -> `Objem` -> `Generovať tetrahedrónovú sieť`.

Model prúdenia sa rieši iba na objemových prvkoch. Bez objemovej siete riešič
nemá čo zostaviť.

### Krok 3 - priradenie materiálu

1. V strome `Model` vyberte objemovú entitu.
2. Otvorte záložku `Materiál` a priraďte **Vodu**, alebo zadajte:
   - **Hustota** = `1000` `kg/m^3`
   - **Dynamická viskozita** = `1.0e-3` `kg/(m*s)`

Skontrolujte, že viskozita je dynamická v `Pa*s`, a nie kinematická - pozri 2.3.

### Krok 4 - ponechanie ustálenej analýzy

Otvorte záložku `Úloha` a v skupine *Riešič v čase* ponechajte **Povoliť**
nezaškrtnuté.

### Krok 5 - priradenie stien

1. Vyberte plošnú entitu pokrývajúcu pevné hranice kanála.
2. Otvorte záložku `Okrajové podmienky` a zaškrtnite **Stena** (*Wall*).

Nie je čo zadať. Je to podmienka nulového sklzu a práve vďaka nej sa v prúdení
vyvinie profil, namiesto aby tekutina preklzla ako piest.

### Krok 6 - pohon prúdenia

1. Vyberte vstupnú plošnú entitu.
2. Zaškrtnite **Objemový prietok (prítok)** (*Volumetric flow rate (inflow)*) a
   zadajte prietok v `m^3/s` - alebo namiesto toho zaškrtnite **Rýchlosť
   (prítok)** (*Velocity (inflow)*) a zadajte rýchlosť v `m/s`.

Skôr než budete pokračovať, odhadnite Reynoldsovo číslo zo strednej rýchlosti a
šírky kanála:

```
Re = rho * V * L / mu
```

Do niekoľkých tisíc je model v rozsahu, ktorý tento riešič zvláda. Ďaleko nad
ním očakávajte, že výpočet nebude konvergovať, a skôr než mu budete venovať čas,
prečítajte si časť 6.

### Krok 7 - otvorenie výstupu

1. Vyberte výstupnú plošnú entitu.
2. Zaškrtnite **Tlak (implicitný)** (*Pressure (implicit)*) a nastavte
   **Tlak** = `0` `Pa`.

Toto je krok, ktorý robí model riešiteľným. Dáva tlakovému poľu referenciu a
umožňuje vyvinúť sa výstupnému profilu namiesto jeho vynútenia ako rovného. Model
so vstupom a stenami, ale bez tlakovej podmienky kdekoľvek, má singulárny tlak a
nebude konvergovať.

### Krok 8 - riešenie

1. `Riešenie` -> `Spustiť riešič` (`Ctrl+R`).
2. Kontrola nastavení pred spustením výpočtu ohlási chýbajúce materiály alebo
   chýbajúce okrajové podmienky.
3. Priebeh sledujte vo `Výkaz` -> `Log súbor riešiča`.

Môže to chvíľu trvať - model prúdenia akejkoľvek veľkosti je skutočná práca a
log vypisuje jeden blok na iteráciu. Sledujte v logu `Convergence target`:
prechod, ktorý ho dosiahne, je označený a skupina potom ohlási *All sub-tasks
have converged* a zastaví sa.

### Krok 9 - kontrola konvergencie

`Výkaz` -> `Konvergencia riešiča`.

Urobte to skôr, než sa pozriete na akýkoľvek obrázok. Reziduum by malo klesnúť o
niekoľko rádov a potom sa vyrovnať a `Convergence-V` a `Convergence-P` by sa mali
blížiť k nule. Ak reziduum pri poslednej iterácii stále strmo klesá, výpočet bol
príliš krátky - zvýšte počet iterácií a reštartujte. Ak osciluje alebo rastie,
odpoveď nie je riešením a treba sa pozrieť do časti 6.

### Krok 10 - prezeranie výsledkov

- Záložka `Výsledky`: zobrazte **Rýchlosť**. Profil naprieč kanálom by mal byť
  hladký, na stenách nulový a najrýchlejší pri strede.
- Vytvorte rez objemom - zaujímavá štruktúra prúdenia je vnútri oblasti, nie na
  jej povrchu.
- Zobrazte **Tlak**. Mal by monotónne klesať od vstupu k výstupu a na výstupe,
  kde ste ho nastavili, mať hodnotu `0`.
- Vytvorte **prúdnice** cez `Geometria` -> `Prúdnica`, aby ste hneď videli
  recirkuláciu alebo mŕtve zóny.
- Skontrolujte bilanciu hmotnosti: stredná výstupná rýchlosť krát plocha výstupu
  by sa mala rovnať objemovému prietoku predpísanému na vstupe. Je to najrýchlejšia
  globálna kontrola, že model robí to, čo ste chceli - pozri časť 5.

### Riešenie problémov

| Príznak | Príčina |
|---|---|
| Reziduum nikdy neklesne | nikde nie je tlaková podmienka, takže tlak je neurčený |
| Reziduum osciluje alebo rastie | Reynoldsovo číslo je pre sieť príliš vysoké alebo bola viskozita zadaná ako kinematická |
| Tekutina preklzne bez profilu | na pevných hraniciach chýba podmienka *Stena* |
| Výsledok vyzerá ako prvá iterácia | krok nebol umiestnený v skupine krokov úlohy, takže prebehol raz |
| Výpočet sa zastavil ďaleko pred počtom iterácií | konvergoval - prírastky aj reziduum splnili test z 1.6 |
| Výpočet sa nikdy nezastaví skôr, nech je hodnota konvergencie akokoľvek voľná | reziduum neklesá - prečítajte si v logu `Residual ratio` a považujte prúdové pole za nekonvergované |
| *Failed to calculate element scales. Unsupported element type* | sieť obsahuje iné objemové prvky ako lineárne tetrahedróny alebo hexahedróny |
| Nevyriešilo sa vôbec nič | nebola vygenerovaná objemová sieť alebo materiálu chýba hustota alebo viskozita |
| Predpísaný prítok a odtok nie sú v rovnováhe | nestlačiteľná tekutina sa nemôže hromadiť - na výstupe použite *Tlak (implicitný)* |

---

## 4. Tutoriál - prechodové prúdenie s rozptylom kontaminantu

**Cieľ.** Uvoľniť kontaminant do konvergovaného prúdenia z tutoriálu 3 a sledovať
oblak v čase pri jeho pohybe kanálom.

Ide o dvojkrokový postup z časti 1.12, ktorý zodpovedá dodanému tutoriálu
*Rozptyl kontaminantu v tekutinách*.

### Krok 1 - začnite z konvergovaného prúdenia

Najprv nechajte tutoriál 3 skonvergovať. Výpočet rozptylu spustený z pokoja
premrhá prvé časové kroky vývojom prúdového poľa namiesto prenášania čohokoľvek.

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
podľa tabuľky v časti 1.10, ak záleží na šírke oblaku. **Maximálne nasýtenie** má
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
| Oblak sa nikdy nerozšíri do strán | koeficient difúzie je nulový, takže bočné rozširovanie pochádza iba z prúdového poľa - zadajte efektívny koeficient difúzie, pozri časť 1.10 |
| Oblak sa naraz rozšíri po celej oblasti | koeficient difúzie je príliš veľký - porovnajte ho s `U * L` |
| Log upozorňuje, že predpísaná koncentrácia prekračuje maximálne nasýtenie | okrajová alebo počiatočná podmienka je nad maximálnym nasýtením; obmedzenie pôsobí iba na rýchlosť tvorby častíc, preto skontrolujte obe hodnoty |
| Ustálené riešenie s difúziou vyzerá nesprávne | maticový riešič sa zastavil na limite iterácií - zvýšte počet vonkajších iterácií GMRES, pozri časť 2.6 |
| Koncentrácia prekročí hodnotu na vstupe a narastá do zašumeného vzoru | prúdenie nie je konvergované - `Residual ratio` zostáva v každom kroku blízko `1`. Jeho rýchlosť má ďaleko od nulovej divergencie a žiadnemu transportu na nej nemožno dôverovať; najprv nechajte skonvergovať prúdenie (sieť, Reynoldsovo číslo, iterácie na krok) |

---

## 5. Kontrola modelu

Riešič nemá zabudovanú sadu overovacích testov, preto sa oplatí nový model pred
tým, než mu začnete dôverovať, overiť voči niečomu, čo viete vypočítať ručne. Päť
lacných kontrol v poradí podľa užitočnosti:

**Bilancia hmotnosti.** Pre každý konvergovaný nestlačiteľný model musí to, čo
vtečie, aj vytiecť:

```
sum( v . n * A ) over the inlets  =  sum( v . n * A ) over the outlets
```

(súčet cez vstupy sa rovná súčtu cez výstupy). Odčítajte strednú rýchlosť na
výstupe zo štatistík v logu riešiča, vynásobte ju plochou výstupu a porovnajte s
predpísaným prítokom. Táto jediná globálna kontrola odhalí nekonvergovaný výpočet,
chýbajúcu stenu aj okrajovú podmienku na nesprávnej entite a nič nestojí.

**Hydrostatika.** Vypnite všetky prítoky, ponechajte oblasť uzavretú stenami,
priraďte environmentálnu podmienku *Gravitačné zrýchlenie* a riešte. Tekutina sa
musí upokojiť a tlak sa musí meniť lineárne s hĺbkou:

```
p(h) = p_ref + rho * g * h
```

Akýkoľvek zvyškový pohyb je numerický. Táto kontrola v jednom výpočte overí
gravitačnú objemovú silu, tlakovú referenciu aj jednotky materiálu a je to
najrýchlejší prvý model, ktorý možno v novom nastavení zostaviť.

**Rovinné Poiseuillovo prúdenie.** Pre ustálené laminárne prúdenie medzi dvoma
rovnobežnými doskami vo vzdialenosti `H` je profil parabolický, rýchlosť v osi je
`1.5`-násobkom strednej a gradient tlaku je

```
dp/dx = 12 * mu * V_mean / H^2
```

Vymodelujte priamy kanál so stenami hore a dole, predpísaným prítokom a výstupom
s *Tlakom (implicitným)* a porovnajte vypočítanú rýchlosť v osi a tlakovú stratu s
týmito hodnotami. Je to najostrejšia dostupná kontrola: naraz overí viskózny člen,
stenu s nulovým sklzom a jednotky viskozity. Pred meraním dajte modelu dostatočnú
nábehovú dĺžku na vyvinutie profilu, pretože vstupná podmienka predpisuje rovný
profil.

**Hagenovo-Poiseuillovo prúdenie.** Tá istá kontrola v kruhovej rúre s priemerom
`D` a dĺžkou `L`, kde rýchlosť v osi je `2`-násobkom strednej a

```
dp = 128 * mu * Q * L / ( pi * D^4 )
```

platí, kým je prúdenie laminárne, teda približne pod `Re = 2300`.

**Čas prechodu.** Pre model rozptylu je čas, za ktorý značka prejde vzdialenosť
`L` pri strednej rýchlosti `V`, rovný `L/V`. Odčítajte čas príchodu z histórie
monitorovacieho bodu a porovnajte. Overí to, že krok rozptylu sa veze na tom
prúdovom poli, na ktorom si myslíte, a odhalí výpočet, ktorý nebol reštartovaný z
konvergovaného prúdenia.

Pri každej z týchto kontrol aspoň raz zjemnite sieť a overte, že odpoveď
konverguje a neodpláva - a skôr než budete čokoľvek porovnávať, overte, že sa
história konvergencie vyrovnala. Nekonvergovaný model prúdenia z rovnakého dôvodu
neprejde žiadnou z týchto kontrol.

---

## 6. Obmedzenia

### Obmedzenia modelovania

- **Žiadny model turbulencie.** Riešič integruje laminárne Navierove-Stokesove
  rovnice. Členy SUPG, PSPG a LSIC sú numerickou stabilizáciou, nie uzáverom
  turbulencie - nepredstavujú miešanie ani dodatočnú disipáciu turbulentného
  prúdenia. Približne nad `Re = 2300` v rúre je skutočné prúdenie turbulentné a
  tento riešič ho nemodeluje: výpočet môže aj tak konvergovať, ale odpoveď opisuje
  laminárne prúdenie, ktoré neexistuje. Neexistuje model `k`-`e`, stenová
  funkcia ani formulácia simulácie veľkých vírov.
- **Iba nestlačiteľné a newtonské.** Hustota je konštantou materiálu a viskózne
  napätie je lineárne v rýchlosti deformácie. Stlačiteľné prúdenie, vplyvy
  Machovho čísla, voľné hladiny, viac fáz a nenewtonské správanie sú mimo
  formulácie.
- **Žiadny vztlak.** Hustota nezávisí od teploty, takže výsledok *Prestupu tepla v
  tekutinách* nikdy nepoháňa prúdenie, na ktorom bol vypočítaný. Prirodzenú
  konvekciu, tepelné vlečky ani stratifikáciu nemožno modelovať. Gravitácia
  vstupuje iba ako konštantná objemová sila, ktorá v oblasti s konštantnou
  hustotou vytvára hydrostatický tlak a žiadny pohyb.
- **Väzba na teplotu a koncentráciu je jednosmerná.** Prúdenie poháňa transport;
  transport nikdy nemení prúdenie.
- **Konštantná izotropná difúzia pri rozptyle kontaminantu.** Koeficient difúzie
  je jedna hodnota pre celý model; nemôže sa meniť v priestore ani v čase, riadiť
  sa smerom prúdenia ani pochádzať z modelu turbulencie. Keďže model turbulencie
  neexistuje, treba ho odhadnúť ako efektívny koeficient difúzie - pozri časť
  1.10.
- **Konštantné maximálne nasýtenie.** Nasýtenie nesleduje teplotu z výsledku
  *Prestupu tepla v tekutinách* a obmedzuje iba kladnú rýchlosť tvorby častíc.
  Koncentrácie predpísané podmienkami sa neobmedzujú a pole sa neorezáva.
- **Podmienky `Heat rate` riešič prestupu tepla v tekutinách ignoruje.** *Tepelný
  výkon (na jednotku plochy)* a *Tepelný výkon (na jednotku objemu)* rozhranie pre
  *Prestup tepla v tekutinách* ponúka, ale `RSolverFluidHeat` ich nečíta; dostane
  sa k nemu iba podmienka *Teplo*. Obyčajný typ úlohy *Prestup tepla* číta všetky
  tri.
- **Žiadne konvekčné okrajové podmienky pri prestupe tepla v tekutinách.** Teplo
  odchádzajúce zo steny do tekutiny so sieťou sa rozlišuje, nie koreluje, takže
  podmienky *Jednoduchá*, *Nútená* a *Prirodzená konvekcia* patria obyčajnému
  tepelnému riešiču. Združený prestup tepla sa nastavuje ako krok *Prestup tepla*
  na pevnej látke, ktorej steny s *Nútenou konvekciou* čítajú vypočítaný stav
  tekutiny.
- **Iba objemové prvky, iba lineárne tetrahedróny a hexahedróny.** Model prúdenia
  musí byť objem so sieťou z prvkov `TETRA1` alebo `HEXA1`; čokoľvek iné výpočet
  zastaví. Plochy sa zúčastňujú iba ako nositelia tlakovej sily *Tlaku
  (implicitného)* a bodové a čiarové entity neprispievajú ničím.
- **Stena bez trenia je zarovnaná s osami.** Obmedzuje jedinú dominantnú globálnu
  zložku normály každého prvku, takže je presná iba na ploche, ktorej normála
  leží pozdĺž `x`, `y` alebo `z`.
- **Vstupy sú rovnomerné.** *Rýchlosť (prítok)* aj *Objemový prietok (prítok)*
  predpisujú rovný profil pozdĺž priemernej normály plochy. Vyvinutý vstupný
  profil treba vytvoriť nábehovou dĺžkou v sieti.

### Obmedzenia riešiča

- **Iterácia je modifikovanou Newtonovou schémou**, takže konvergencia v
  blízkosti riešenia je lineárna, nie kvadratická. Pri ustálenom výpočte
  očakávajte stovky iterácií a počet iterácií skupiny krokov nastavte tak vysoko,
  aby výpočet ukončila hodnota konvergencie, a nie počet. Tlmenie z časti 1.6
  udržuje taký výpočet klesajúci, ale kvadratickým ho urobiť nedokáže.
- **Centrálna diferencia riešiča prúdenia nie je Crankova-Nicolsonova schéma.**
  Theta váhovanie sa aplikuje na maticu, ale nie na reziduum, takže *Centrálna
  diferencia* rieši reziduum spätného Eulera s maticou s polovičnou tuhosťou.
  Odpoveďou je odpoveď spätnej diferencie, dosiahnutá pomalšie. Pre prúdenie
  používajte *Spätnú diferenciu*, kým sa to neopraví. Riešiča rozptylu sa to
  netýka - je lineárny a jeho schéma je skutočne Crankova-Nicolsonova.
- **Stabilizačné parametre sa nederivujú.** `Tsupg`, `Tpspg`, `Tlsic` aj dĺžka
  prvku závisia od rýchlosti a všetky vstupujú do matice ako konštanty
  vyhodnotené pri aktuálnom poli. Je to bežná prax pri stabilizovanej formulácii
  a jeden z dôvodov, prečo jakobián nie je presný.
- **Reziduálna časť testu konvergencie je pevná.** Riešenie sa považuje za
  konvergované, až keď jeho reziduum klesne na desatinu hodnoty na začiatku tohto
  riešenia, a túto desatinu nemožno zmeniť. Je zámerne konzervatívna: pomaly
  konvergujúci model ju nemusí nikdy splniť a potom prebehne plný počet iterácií.
  `Residual ratio` v logu ukazuje, ako blízko sa taký výpočet dostal.
- **Je potrebná tlaková referencia.** Bez plochy s *Tlakom (explicitným)* alebo
  *Tlakom (implicitným)* je tlak určený iba až na konštantu a iterácia sa
  neustáli.
- **Predpísané prietoky musia byť v rovnováhe.** Model, ktorého hranice tvoria
  výlučne steny a predpísané rýchlosti, musí mať prítok presne rovný odtoku;
  nestlačiteľná tekutina rozdiel nepohltí.
- **Cena.** Štyri neznáme na uzol, nesymetrická matica a nelineárna iterácia
  okolo každého lineárneho riešenia robia z modelu prúdenia s veľkým náskokom
  najnáročnejší typ úlohy v Range FEA. Podľa toho dimenzujte sieť a pred
  spustením akéhokoľvek prechodového výpočtu nechajte skonvergovať ustálené pole.

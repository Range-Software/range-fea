# Magnetostatika - Teoretická příručka a uživatelská příručka

Tento dokument popisuje řešič magnetostatiky (`RSolverMagnetostatics`) v
Range FEA, který obsluhuje typ úlohy **Magnetostatika** (*Magneto-statics*):
zákon, který vyhodnocuje, vstupy, které přijímá, části grafického
uživatelského rozhraní, které ho ovládají, podrobný návod a to, jakou hodnotu
má jeho výsledek.

> **Stav.** Řešič vyhodnocuje magnetické pole proudu vypočteného krokem
> elektrostatiky přímo, pomocí **Biotova-Savartova zákona**. Neexistuje žádná
> soustava rovnic ani okrajová podmínka: pole je určeno pouze proudem a
> podmínka, že daleko od vodiče zaniká, je v zákoně obsažena. Pro zadanou
> proudovou hustotu je výsledek přesný až na řízenou chybu kvadratury hluboko
> pod `1e-4` špičkového pole. Řešič pracuje ve **volném prostoru** - magnetické
> materiály nelze reprezentovat - a počítá pouze pole modelovaného proudu,
> takže dráha proudu by měla být uzavřena v rámci modelu. Omezení uvádí část 5.

**Obsah**

1. [Teoretický základ](#1-teoretický-základ)
2. [Grafické uživatelské rozhraní](#2-grafické-uživatelské-rozhraní)
3. [Návod - pole vodiče protékaného proudem](#3-návod---pole-vodiče-protékaného-proudem)
4. [Kontrola modelu](#4-kontrola-modelu)
5. [Omezení](#5-omezení)

---

## 1. Teoretický základ

### 1.1 Výsledek - magnetické pole

Výsledkem je **magnetická indukce** `B` v každém uzlu modelu, v teslech, jako
vektor o třech složkách.

Pole se **vyhodnocuje, neřeší**. Jakmile je známa proudová hustota, plyne z ní
magnetické pole ve volném prostoru integrálem, takže řešič nesestavuje žádnou
matici a neprovádí žádnou iteraci. Hodnotu dostane každý uzel modelu, ať leží
ve vodiči, na jeho povrchu nebo kdekoli jinde v síti.

### 1.2 Řídicí zákon

Ve volném prostoru splňuje statické magnetické pole Ampérův zákon a nemá
zdroje:

```
curl(B) = u0 * J
div(B)  = 0
```

spolu s podmínkou, že `B` daleko od všech proudů zaniká. Jediné pole
splňující všechny tři podmínky udává **Biotův-Savartův zákon**

```
B(x) = u0/(4*pi) * INT_V  J(y) x (x - y) / |x - y|^3  dV(y)
```

| Symbol | Význam | Jednotky | Zdroj |
|---|---|---|---|
| `B` | magnetická indukce | `T` | vypočtená |
| `J` | proudová hustota | `A/m^2` | z kroku elektrostatiky, viz 1.4 |
| `u0` | permeabilita vakua | `H/m` | vestavěná, `1.25663706212e-6` |
| `x` | bod pole - uzel modelu | `m` | |
| `y` | zdrojový bod - uvnitř vodivého prvku | `m` | |

Tři vlastnosti plynou ze samotného zákona, nikoli z čehokoli, co řešič dělá:

- **`div(B) = 0` platí přesně**, všude, bez ohledu na síť.
- **Vzdálené pole je správné bez hranice.** `B` klesá se vzdáleností tak, jak
  zákon předepisuje; neexistuje žádný vnější povrch, žádná vzduchová oblast k
  síťování ani podmínka, kterou by tam bylo nutné nastavit.
- **`curl(B) = u0 * J` platí všude, kde je proud uzavřen**, tedy kde
  `div(J) = 0` a dráha proudu tvoří smyčky. Část 1.4 vysvětluje, co to znamená
  pro elektrostatický model, jehož proud do tělesa vstupuje a vystupuje z něj.

Permeabilita materiálu nevstupuje nikam. Neexistuje materiálová vlastnost
relativní permeability, žádná magnetizace ani rozlišení mezi `B` a `H` - řešič
pracuje ve volném prostoru bez ohledu na to, z čeho je model vyroben. Část 1.9
vysvětluje, proč to v rámci téhož přístupu nelze odstranit.

**Není potřeba žádná materiálová vlastnost.** Typ úlohy magnetostatika žádnou
nedeklaruje a kontrola nastavení za něj nemá co kontrolovat. Krok
elektrostatiky, na kterém typ úlohy závisí, má vlastní tři požadavky - viz
příručka elektrostatiky.

### 1.3 Diskretizace

Řešič elektrostatiky vytváří proudovou hustotu, která je **v každém prvku
konstantní**. Integrál z 1.2 se proto rozpadá na jeden integrál na prvek, každý
s konstantním `J_e`, které lze vytknout:

```
B(x) = u0/(4*pi) * SUM_e  J_e x G_e(x)
G_e(x) = INT_e  (x - y) / |x - y|^3  dV(y)
```

`G_e` závisí jen na geometrii prvku a na bodu pole. Každý typ prvku, který
řešič přijímá, má rovinné stěny a pro ně má `G_e` uzavřený tvar, takže řešič
integruje každý prvek **přesně**, nikoli přibližně:

- **Čtyřstěn.** Integrand je gradient `1/|x - y|` vzhledem k `y`, takže se
  objemový integrál změní na součet přes čtyři stěny,
  `G = SUM_f n_f * INT_f 1/R dA`. Integrál `1/R` přes rovinný trojúhelník je
  znám v uzavřeném tvaru: logaritmus podél každé hrany a prostorový úhel, pod
  kterým je trojúhelník vidět z `x`,
  `INT 1/R dA = SUM_edges P_e * I_e - |d| * omega`.
- **Trojúhelník** povrchové entity s tloušťkou `t`. Proud je plošný proud
  `K = J*t` v `A/m`. Tečná část `G` je opět součtem logaritmů hran,
  `SUM_e m_e * I_e`, a normálová část je znaménkový prostorový úhel.
- **Čtyřúhelník.** Rozdělí se na dva trojúhelníky.
- **Úsečka** čárové entity s plochou průřezu `A`. Proud je `I = A * (J . t)`
  podél úsečky a pole přímého konečného vodiče je klasické:

  ```
  B = u0*I/(4*pi) * (a x b) * (|a| + |b|) / ( |a|*|b| * (|a|*|b| + a.b) )
  ```

  kde `a` a `b` jsou vektory z bodu pole k oběma koncům.

Zde `I_e` je integrál `1/R` podél hrany `e`, `P_e` vzdálenost bodu pole od této
hrany v rovině stěny, `m_e` vnější normála hrany v rovině stěny, `d` výška bodu
pole nad rovinou stěny a `omega` prostorový úhel stěny viděné z bodu pole.

**Vzdálené prvky** uzavřený tvar nepotřebují. Integrand je tam hladký a
kvadraturní pravidlo je stejně dobré jako přesné, za zlomek ceny:

| Vzdálenost bodu pole od středu prvku | Pravidlo |
|---|---|
| pod 2 x nejdelší hrana prvku | uzavřený tvar |
| 2 až 6 x | pravidlo druhého řádu - 3 body na trojúhelníku, 4 na čtyřstěnu |
| nad 6 x | pravidlo středního bodu - prvek jako bodový proud |

Úsečky vždy používají uzavřený tvar, který je stejně levný jako jeden
kvadraturní bod. Celkovou chybu pravidel měří jednotkové testy vůči uzavřenému
tvaru sečtenému přes všechny prvky celého vodiče a v každém uzlu zůstává pod
`1e-4` špičkového pole - uvnitř vodiče, na jeho povrchu i za ním.

**Pole přímo ve vodiči.**

- Pro **objemový proud** je pole všude spojité a konečné, včetně uzlů, hran a
  stěn vodivých prvků. Uzel na povrchu masivního vodiče dostane skutečné
  povrchové pole.
- **Čárová** nebo **povrchová** entita představuje vodič s nulovým průřezem a
  pole takového vodiče je singulární: roste jako `1/r` směrem k drátu a
  logaritmicky směrem k okraji plochy a přes plochu skočí o `u0 * K x n`. V uzlu
  na takovém vodiči vrací řešič konečnou hodnotu: úsečka nepřispívá do bodu na
  vlastní přímce, plocha přispívá průměrem polí na svých dvou stranách a
  logaritmus hrany plochy se v uzlu ležícím na této hraně vynechá. Tyto hodnoty
  jsou regularizované, nikoli fyzikální. Pole tenkého vodiče odečítejte několik
  rozměrů průřezu od něj, kde je tenký model platný.

### 1.4 Zdroj proudové hustoty

Magnetické pole je buzeno jediným vstupem: **proudovou hustotou**, kterou
zanechal krok *Electro-statics* dříve v pořadí kroků úlohy. Není co zadávat ani
přiřazovat.

Řešič čte **prvkovou** proudovou hustotu, kterou ukládá krok elektrostatiky, a
používá ji tak, jak je - neprovádí se žádné průměrování do uzlů. Proud vedou
tyto prvky:

| Entita | Podmínka | Zdroj |
|---|---|---|
| Objem | čtyřstěny | `J` |
| Povrch | tloušťka `> 0` | plošný proud `J * thickness` |
| Čára | plocha průřezu `> 0` | proud `J * cross area` |

Prvky s nulovou proudovou hustotou se přeskakují, což jsou všechny prvky, které
krok elektrostatiky nepočítal. Stejně tak prvky, jejichž proudová hustota je
pod `1e-10` největší hodnoty v modelu: špatně vodivá oblast - vzduch s
vodivostí `5e-15 S/m` vedle mědi - by jinak přidala zdroj na každý prvek a k
poli nic měřitelného. Pokud žádný prvek nevede proud - krok elektrostatiky
neproběhl, potenciály jsou stejné nebo žádný materiál nemá elektrickou
vodivost - log vypíše upozornění a pole je všude nulové.

**Proud se musí uzavřít.** Biotův-Savartův zákon dává pole proudu, který je mu
zadán, a nic jiného. Fyzikální proud teče v uzavřených smyčkách, ale
elektrostatický model obvykle obsahuje jen část smyčky: tyč mezi dvěma
elektrodami, do které proud vstupuje jedním koncem a druhým vystupuje.
Přívodní vodiče a zbytek obvodu v modelu nejsou, a tedy ani jejich pole. Řešič
pak počítá pole modelovaného vodiče, jako by se proud na jednom konci objevil a
na druhém zmizel:

- **v blízkosti vodiče** je to obvykle výborná aproximace, protože pole tam
  určuje blízký proud;
- **daleko od něj**, ve vzdálenostech srovnatelných se samotným vodičem, je
  chybějící část smyčky stejně důležitá jako ta modelovaná a výsledek není
  polem žádného skutečného obvodu.

Záleží-li na vzdáleném poli, zahrňte do elektrostatického modelu zpětnou
cestu, aby proud tvořil smyčku. Tenký zpětný vodič lze reprezentovat čárovou
entitou s plochou průřezu.

### 1.5 Okrajové, počáteční a environmentální podmínky

**Žádné** nejsou a žádné nejsou potřeba.

| Druh podmínky | Nabízeno pro magnetostatiku |
|---|---|
| Okrajové podmínky | žádné |
| Počáteční podmínky | žádné |
| Environmentální podmínky | žádné |
| Materiálové vlastnosti | žádné |

Pole je zcela určeno proudem a požadavkem, aby daleko zanikalo, který
Biotův-Savartův zákon obsahuje. Neexistuje vnější hranice, kterou by bylo nutné
uzavřít, a nic, co by na ní bylo nutné předepsat.

Pokud krok elektrostatiky a krok magnetostatiky sdílejí pořadí kroků - což
vždy dělají, viz 1.6 - záložky podmínek stále nabízejí **elektrostatické**
podmínky, protože se záložky plní z typů úloh celého pořadí kroků.
*Electric potential* a *Charge density* patří kroku elektrostatiky a k
magnetickému poli se dostanou prostřednictvím proudové hustoty, kterou vytvoří.

### 1.6 Vazba s jinými řešiči

| Směr | Proměnná | Účinek |
|---|---|---|
| Elektrostatika -> magnetostatika | Current density | jediný zdroj magnetického pole |

**Magnetostatika vyžaduje elektrostatiku.** Závislost je deklarována v typu
úlohy a uživatelské rozhraní ji vynucuje: zaškrtnutí *Magneto-statics* ve
výběru typu úlohy automaticky zaškrtne *Electro-statics* a zrušení zaškrtnutí
*Electro-statics* zruší spolu s ním i *Magneto-statics*. Výchozí pořadí kroků
umístí krok elektrostatiky jako první, což je pořadí, které vazba potřebuje.

Magnetické pole nic nespotřebovává. Je koncem svého řetězce - žádný řešič ho
nečte a neexistuje Lorentzova síla, indukovaný proud ani magnetický příspěvek k
jakékoli jiné fyzice.

Protože se řešič hlásí jako zkonvergovaný bezpodmínečně, skupina kroků
obsahující krok magnetostatiky se vždy zastaví po jedné iteraci.

### 1.7 Vyhodnocení pole

Každý prvek vedoucí proud přispívá do každého uzlu, takže práce roste jako
**počet uzlů krát počet prvků vedoucích proud**. Vyhodnocení probíhá paralelně
přes uzly, které se zpracovávají po blocích, takže se každý prvek načítá z
paměti jednou na blok, nikoli jednou na uzel.

Pro představu: tyč o `120 000` čtyřstěnech vyhodnocená ve svých `24 000`
uzlech zabere na 14 vláknech asi pět sekund. Zdvojnásobení sítě v každém směru
znásobí počet uzlů i prvků osmkrát a čas přibližně šedesátčtyřikrát; model o
několika stech tisících prvků trvá spíše minuty než sekundy.

Počítají se jen prvky vedoucí proud. Velká síť kolem malého vodiče - vzduchová
oblast, viz 2.8 - přidá uzly, ale žádné zdroje.

Nastavení maticového řešiče se nepoužívá.

### 1.8 Výsledky

| Výsledek | Použití na | Význam |
|---|---|---|
| Magnetic field `[T]` | uzel | vektor pole |

To je celý výstup. `process()` nedělá nic, takže neexistuje žádná odvozená
veličina - žádná samostatně uložená velikost pole, žádný tok ani hustota
energie. 3D zobrazení umí obvyklým způsobem ukázat velikost a složky vektorové
proměnné a pro zobrazení složek jsou k dispozici
`Magnetic field in X direction` a obdobné proměnné pro `Y` a `Z`.

Magnetické pole se ukládá jako **uzlová** proměnná, na rozdíl od výsledků
elektrostatického pole, které jsou prvkovými proměnnými. Hodnotu má **každý
uzel** modelu, včetně uzlů entit, které nevedou proud, takže síť kolem vodiče
ukazuje pole v prostoru kolem něj.

Po zapsání záznamu vypíše log řešiče statistiku magnetického pole a hodnoty ve
všech monitorovacích bodech.

### 1.9 Co řešič je a co není

**Je to integrál, nikoli rovnice pole.** Vyhodnocení Biotova-Savartova zákona
nepotřebuje žádnou hranici ani síť mimo vodič a pro zadaný proud je přesné.
Cenou je, že funguje jen ve **stejnorodém** prostředí. U magnetických materiálů
závisí pole proudu na tom, kde je železo, a žádný integrál samotného proudu to
nedokáže vyjádřit; taková úloha se řeší potenciálovou formulací - obvykle
magnetickým vektorovým potenciálem na hranových prvcích - což je jiný řešič,
nikoli rozšíření tohoto.

**Je buzen vypočteným kondukčním proudem.** Proud nelze zadat přímo, neexistuje
permanentní magnet ani vnucené pole; proud musí vzniknout elektrostatickým
řešením na vodivém tělese a jeho přesnost určuje přesnost magnetického
výsledku. Příručka elektrostatiky popisuje, čím tato proudová hustota je a čím
není. Proudová hustota konstantní na prvek je aproximací prvního řádu skutečné
hustoty a se zjemňováním sítě konverguje.

**Počítá pole modelovaného proudu.** Viz 1.4 - pokud záleží na poli daleko od
vodiče, měl by se proud uzavřít v rámci modelu.

### 1.10 Statická analýza a řešič v čase

Magnetostatika je deklarována jako typ úlohy **bez** řešiče v čase, stejně
jako elektrostatika. Skupina *Řešič v čase* se v záložce `Úloha` nezobrazuje,
výpočet vytvoří jediný záznam a není co nastavovat.

Protože pořadí kroků s magnetostatikou vždy obsahuje i krok elektrostatiky a
ani jeden nepodporuje řešič v čase, přidání kteréhokoli z nich do pořadí kroků
odstraní řešič v čase z celého pořadí kroků. Tepelný krok zařazený za ně může
být jen ustálený. Příručka elektrostatiky to popisuje ve své části 1.10.

---

## 2. Grafické uživatelské rozhraní

Vše podstatné se nachází v panelu **Řešič** (záložky `Úloha`, `Materiál`,
`Výsledky`), v nabídce **Úloha** a v panelu **Model**. Záložky podmínek pro
tento typ úlohy nic neobsahují.

### 2.1 Výběr úlohy

`Úloha` -> `Pořadí kroků úloh` (`Ctrl+P`) otevře dialog pořadí kroků úlohy.
Přidejte krok a vyberte **Magneto-statics** - „Calculate magnetic field“
(výpočet magnetického pole).

**Electro-statics se zaškrtne za vás.** Výběr vynucuje závislost z 1.6 v obou
směrech: zaškrtnutí *Magneto-statics* zaškrtne *Electro-statics* a zrušení
zaškrtnutí *Electro-statics* zruší i *Magneto-statics*. Výchozí pořadí kroků
umístí krok elektrostatiky jako první.

Magnetostatika se kombinuje se všemi ostatními fyzikami kromě *Modal analysis*.
V praxi má smysl jen řetězec, na kterém je postavena:

- *Electro-statics* a poté *Magneto-statics* - magnetické pole vypočteného
  kondukčního proudu.

### 2.2 Záložka Úloha

Pro pořadí kroků s elektrostatikou a magnetostatikou je tato záložka
**prázdná**. Ani jeden typ úlohy nemá řešič v čase ani vlastní skupinu
nastavení.

### 2.3 Záložka Materiál

Magnetostatika od materiálu **nic** nepotřebuje. Krok elektrostatiky, na kterém
závisí, potřebuje tři vlastnosti - relativní permitivitu, elektrickou vodivost
a hustotu - a entitu, jejímuž materiálu některá z nich chybí, *tento* krok
přeskočí, takže nebude mít žádnou proudovou hustotu, a tedy ani příspěvek k
magnetickému poli.

Na materiálu tedy stále záleží, a to výhradně přes elektrostatickou polovinu
řetězce. Přiřaďte materiál se všemi třemi vlastnostmi, například dodávaný
*Copper*, a k čemu každá slouží, najdete v příručce elektrostatiky.

Vlastnost relativní permeability k přiřazení neexistuje. Řešič pracuje ve
volném prostoru.

### 2.4 Záložka Okrajové podmínky

Nic v této záložce nepatří magnetostatickému kroku. Podmínky, které nabízí -
*Electric potential* a *Charge density* - jsou elektrostatické a budí proudovou
hustotu, která zase vytváří magnetické pole. Přiřaďte je tak, jak popisuje
příručka elektrostatiky.

Žádná magnetostatická okrajová podmínka neexistuje ani není potřeba - viz 1.5.

### 2.5 Počáteční a environmentální podmínky

Žádná z těchto záložek pro magnetostatiku nic nenabízí. Zobrazená počáteční
podmínka *Electric potential* a environmentální podmínka *Temperature* patří
kroku elektrostatiky.

### 2.6 Nastavení maticového řešiče

Řešič magnetostatiky neřeší žádnou soustavu rovnic a nastavení maticového
řešiče nepoužívá. Položka **CG** nastavuje pouze řešič elektrostatiky.

### 2.7 Monitorovací body

`Úloha` -> `Definovat monitorovací body` umístí sondy na zadané souřadnice a
vybere proměnnou, která se má zaznamenávat. *Magnetic field* je v seznamu k
dispozici spolu s elektrostatickými proměnnými a odečítá uzlové pole v sondě.

### 2.8 Výsledky a záznamy

Záložka `Výsledky` uvádí **Magnetic field** jako uzlový vektor vedle
elektrostatických výsledků, které vytvořil první krok. Záložka `Záznamy`
panelu `Model` obsahuje přesně jeden záznam: oba typy úloh jsou statické.

`Zprávy` -> `Log soubor řešiče` ukazuje celý výstup: nejprve statistiku
elektrostatiky, pak počet prvků vedoucích proud a uzlů, pro které byl vyhodnocen
Biotův-Savartův integrál, a nakonec statistiku magnetického pole.

**Zobrazení pole kolem vodiče.** Pole se počítá v každém uzlu, takže ho ukáže
cokoli, co je kolem vodiče síťované. Okolní prostor vysíťujte jako samostatnou
objemovou entitu - vzduchovou oblast - a dejte jí materiál bez elektrické
vodivosti: krok elektrostatiky ji přeskočí, takže nevede proud, ale její uzly
dostanou magnetické pole. Oblast stojí uzly, ale žádné zdroje (1.7), a
nepotřebuje žádnou zvláštní velikost ani hranici, protože se na ní nic neřeší.

Dodávaný materiál *Air* elektrickou vodivost má, i když nepatrnou, takže krok
elektrostatiky by oblast zahrnul do vlastního řešení. Použijte jeho kopii s
odstraněnou vlastností elektrické vodivosti. Kontrola nastavení pak upozorní,
že materiálu entity chybí požadované vlastnosti; u vzduchové oblasti je to
očekávané a výpočet pokračuje.

---

## 3. Návod - pole vodiče protékaného proudem

**Cíl.** Provést řetězec elektrostatiky a magnetostatiky od začátku do konce na
měděné tyči protékané proudem, ověřit magnetické pole ručním výpočtem a ukázat,
kde se projevují aproximace z 1.4.

Návod předpokládá vysíťovanou tyč z příručky elektrostatiky - kvádr dlouhý
`0.1 m` se čtvercovým průřezem `0.01 m`, tetraedrizovaný, s oběma koncovými
stěnami označenými jako samostatné povrchové entity. Je-li tento model stále
nastaven, mění se pouze pořadí kroků.

### Krok 1 - vyberte úlohy

1. `Úloha` -> `Pořadí kroků úloh` (`Ctrl+P`).
2. Zaškrtněte **Magneto-statics**. Sledujte, jak se **Electro-statics** zaškrtne
   samo.
3. Potvrďte tlačítkem `OK`.

Pořadí kroků nyní obsahuje dva kroky, elektrostatiku jako první. Záložka
`Úloha` zůstává prázdná.

### Krok 2 - přiřaďte materiál

Vyberte objemovou entitu a v záložce `Materiál` přiřaďte **Copper**, nebo
materiál se třemi vlastnostmi, které krok elektrostatiky potřebuje:

- **Relative permittivity** = `2.8`
- **Electrical conductivity** = `5.882e7` `S/m`
- **Density** = `8960` `kg/m^3`

Nic z toho magnetostatický krok nečte. Potřebuje je elektrostatický krok a bez
proudové hustoty není magnetické pole.

### Krok 3 - vybuďte proud

1. Vyberte povrchovou entitu na jednom konci, otevřete záložku
   `Okrajové podmínky` a zaškrtněte **Electric potential**. Nastavte ho na
   `0` V.
2. Vyberte povrchovou entitu na druhém konci, zaškrtněte **Electric potential**
   a nastavte ho na `0.01` V.

Proud tyčí je

```
I = sigma * dV * A / L = 5.882e7 * 0.01 * 1e-4 / 0.1 = 588 A
```

Tyč bez rozdílu potenciálů nevede proud a dává magnetické pole přesně nulové -
což je první věc, kterou je třeba zkontrolovat, pokud výsledek vyjde prázdný.

### Krok 4 - řešte

1. `Řešení` -> `Spustit řešič` (`Ctrl+R`).
2. Průběh sledujte v `Zprávy` -> `Log soubor řešiče`.

Log ukazuje krok elektrostatiky s jeho proměnnými a pak krok magnetostatiky:
počet prvků vedoucích proud, počet uzlů a statistiku magnetického pole. Oba
proběhnou jednou.

### Krok 5 - prohlédněte si výsledek

- Záložka `Výsledky`: zobrazte **Magnetic field**. Je to uzlový vektor, takže
  3D zobrazení nabízí jeho velikost a tři složky.
- Pole **obíhá kolem tyče**, kolmo k proudu, ve smyslu pravidla pravé ruky.
- Na **ose** tyče je **nulové** a roste k povrchu, kde je největší - uprostřed
  každé dlouhé stěny, nikoli v rozích.
- **U konců** tyče je **slabší** než uprostřed. To je otevřená dráha proudu z
  1.4: u konce chybí polovina toho, co by bylo delším vodičem.

### Krok 6 - ověřte velikost

Pro kruhový vodič o poloměru `R` je pole na povrchu `u0*I/(2*pi*R)`. Čtvercová
tyč o straně `a` kruhová není, ale střed stěny ve vzdálenosti `a/2` od osy se
tomu blíží: s `a/2 = 0.005 m`,

```
u0*I/(2*pi*r) = 1.2566e-6 * 588 / (2*pi*0.005) = 0.0235 T
```

Vypočtené pole uprostřed dlouhé stěny, v polovině délky tyče, je asi
`0.020 T`, tedy zhruba o 15 % méně - čtvercový průřez umisťuje velkou část
proudu dál od bodu, než by to udělal kruhový. V rohu, `a/sqrt(2)` od osy, je
ještě nižší, asi `0.019 T`.

Přesnější kontrola potřebuje bod pole mimo tyč, kde už na průřezu nezáleží.
Přidejte vzduchovou oblast podle 2.8 a odečtěte pole ve vzdálenosti
`r = 0.02 m` od osy, v polovině délky. Pro přímý konečný vodič délky `L`,

```
B = u0*I/(4*pi*r) * 2 * (L/2) / sqrt( (L/2)^2 + r^2 )
  = 1e-7 * 588 / 0.02 * 2 * 0.05 / sqrt(0.0025 + 0.0004) = 5.46e-3 T
```

a vypočtená hodnota by se měla shodovat přibližně na `0.1 %`, přičemž rozdíl
způsobuje čtvercový průřez tyče. Tato hodnota je polem samotné tyče, jak
vysvětluje 1.4 - skutečný obvod by přidal pole svých přívodů.

### Řešení problémů

| Příznak | Příčina |
|---|---|
| Magnetické pole je všude nulové a log upozorňuje, že žádný prvek nevede proud | žádná proudová hustota - zkontrolujte, že krok elektrostatiky proběhl jako první, že se potenciály liší a že materiál má elektrickou vodivost |
| **Magneto-statics** nelze odškrtnout samostatně | očekávané - vyžaduje *Electro-statics* a jeho odškrtnutí odškrtne obě |
| Chybí skupina *Řešič v čase* | očekávané - ani jeden typ úlohy nemá řešič v čase, viz 1.10 |
| Pole je u konců vodiče slabší | očekávané - dráha proudu je otevřená, viz 1.4 |
| Pole daleko od vodiče neodpovídá obvodu | přívody a zbytek obvodu nejsou v modelu, viz 1.4 |
| Pole u čárového nebo tenkého povrchového vodiče je nevyzpytatelné | očekávané - pole vodiče s nulovým průřezem je tam singulární, viz 1.3 |
| Výpočet trvá dlouho | cena je počet uzlů krát počet prvků vedoucích proud, viz 1.7 |

---

## 4. Kontrola modelu

**Přímý vodič.** Pro dlouhý vodič o poloměru `R`, kterým rovnoměrně protéká
celkový proud `I`, je pole obvodové a

```
outside, r > R    B = u0 * I / ( 2 * pi * r )
inside,  r < R    B = u0 * I * r / ( 2 * pi * R^2 )
```

(`outside` - vně, `inside` - uvnitř) s `u0 = 1.2566e-6`. Pole je největší na
povrchu vodiče, `B = u0*I/(2*pi*R)`, a za ním klesá jako `1/r`. Pro proud
`1 A` ve vodiči o poloměru `5 mm` je tato špička `4e-5 T`, přibližně velikost
zemského pole.

**Konečný vodič.** Pro přímý vodič délky `L` ve vzdálenosti `r` od jeho osy a v
polovině délky,

```
B = u0 * I / ( 4 * pi * r ) * L / sqrt( (L/2)^2 + r^2 )
```

což se s rostoucím `L` blíží vzorci pro dlouhý vodič. Mimo vodič přestává
tvar průřezu hrát roli, takže lze takto kontrolovat libovolný průřez.

**Drát.** Čárová entita s plochou průřezu vede svůj proud podél úseček a řešič
pro každou z nich používá uzavřený tvar přímého drátu. Přímá čárová entita
proto reprodukuje vzorec konečného vodiče přesně, až na zaokrouhlení, v
jakékoli vzdálenosti.

**Solenoid.** Pro dlouhý solenoid s `n` závity na metr protékaný proudem `I` je
pole uvnitř homogenní a míří podél osy,

```
B = u0 * n * I
```

a vně je téměř nulové. Vinutí lze modelovat jako čárovou entitu sledující
závity s plochou průřezu drátu a krok elektrostatiky jím žene proud z
potenciálů na jeho dvou koncích.

**Divergence.** Pro každé magnetické pole platí všude `div(B) = 0`.
Biotův-Savartův zákon ji splňuje přesně, takže tento řešič netestuje; testovala
by pouze interpolaci mezi uzly.

**Konvergence se sítí.** Samotná integrace je přesná (1.3); na síti závisí
proudová hustota, kterou krok elektrostatiky počítá jako konstantní na prvek.
Zjemnění sítě ve vodiči a porovnání pole ve stejném bodě je kontrolou této
části.

---

## 5. Omezení

### Omezení modelování

- **Pouze volný prostor.** Do formulace nevstupuje žádná permeabilita.
  Neexistuje materiálová vlastnost relativní permeability, rozlišení mezi `B` a
  `H`, magnetizace ani nasycení, takže železo, ferity a stínění nelze vůbec
  reprezentovat. Proč to vyžaduje jiný řešič, vysvětluje 1.9.
- **Pouze pole modelovaného proudu.** Pole přívodů a zbytku obvodu není
  zahrnuto. Je-li dráha proudu v modelu otevřená, pole daleko od vodiče není
  polem skutečného obvodu - viz 1.4.
- **Žádné permanentní magnety a žádné vnucené pole.** Jediným zdrojem je
  kondukční proud vypočtený krokem elektrostatiky. Magnet, pozadové pole ani
  vnější buzení nemají žádnou reprezentaci.
- **Proud nelze zadat přímo.** Proud vzniká elektrostatickým řešením na vodivém
  tělese. Cívku lze modelovat jako čárovou entitu sledující vinutí, ale její
  proud se stále nastavuje přes potenciály.
- **Tenké vodiče jsou singulární.** Pole v uzlu na čárové entitě nebo na okraji
  povrchové entity je regularizovaná hodnota, nikoli fyzikální - viz 1.3.
- **Typy prvků.** Proud vedou čtyřstěny, trojúhelníky, čtyřúhelníky a
  dvouuzlové úsečky; ostatní typy prvků nepřispívají ničím.
- **Cena.** Práce roste jako počet uzlů krát počet prvků vedoucích proud - viz
  1.7.
- **Žádné odvozené výsledky.** `process()` nic nepočítá, takže vektor
  magnetického pole je celým výstupem. Neexistuje tok, energie, síla ani
  velikost pole uložená jako samostatná proměnná.
- **Výsledek nic nespotřebovává.** Žádný jiný řešič magnetické pole nečte.
  Neexistuje Lorentzova síla na vodič, indukovaný proud ani zpětná vazba do
  elektrostatické, tepelné nebo strukturální fyziky.
- **Pouze statika.** Neexistuje řešič v čase a přidání kroku magnetostatiky -
  který s sebou přináší krok elektrostatiky - odstraní řešič v čase z celého
  pořadí kroků, takže jakákoli další fyzika zařazená za něj je rovněž
  ustálená.

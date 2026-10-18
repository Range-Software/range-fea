# Akustická analýza - Teoretická příručka a uživatelská příručka

Tento dokument popisuje akustický řešič (`RSolverAcoustic`) v Range FEA:
rovnice, které řeší, význam každého vstupu, který přijímá, části grafického
uživatelského rozhraní, které ho ovládají, a dva podrobné návody.

**Obsah**

1. [Teoretický základ](#1-teoretický-základ)
2. [Grafické uživatelské rozhraní](#2-grafické-uživatelské-rozhraní)
3. [Návod - přechodová akustická analýza](#3-návod---přechodová-akustická-analýza)
4. [Návod - harmonická akustická analýza](#4-návod---harmonická-akustická-analýza)
5. [Ověření](#5-ověření)
6. [Omezení](#6-omezení)

---

## 1. Teoretický základ

### 1.1 Primární neznámá - rychlostní potenciál

Akustický řešič neřeší přímo tlak. Jeho primární neznámou je **rychlostní
potenciál** `phi`, skalární pole s jednotkami `m^2/s`. Obě fyzikální veličiny,
které nás zajímají, se z něj získají:

```
u = -grad(phi)                  particle velocity      [m/s]
p = rho * dphi/dt               acoustic pressure      [Pa]
```

(`particle velocity` - akustická rychlost částic, `acoustic pressure` -
akustický tlak.)

Použití potenciálu místo tlaku má dvě praktické výhody. Úloha zůstává
skalární, přestože rychlost částic je vektor, a předepsaná rychlost stěny -
způsob, jakým se popisuje většina akustických zdrojů - vstupuje jako přirozená
okrajová podmínka, nikoli jako vazba.

Výše uvedená znaménková konvence (`u = -grad(phi)`, `p = +rho * dphi/dt`) se
používá konzistentně v celém řešiči i v uložených výsledcích.

### 1.2 Řídicí rovnice

Zvuk malé amplitudy v klidné, homogenní tekutině se řídí tlumenou vlnovou
rovnicí pro rychlostní potenciál:

```
laplace(phi) - (1/c^2) * d2phi/dt2 - (beta/c^2) * dphi/dt = 0
```

| Symbol | Význam | Jednotky | Zdroj |
|---|---|---|---|
| `phi` | rychlostní potenciál | `m^2/s` | řešená veličina |
| `c` | rychlost zvuku | `m/s` | materiálová vlastnost |
| `rho` | hustota tekutiny | `kg/m^3` | materiálová vlastnost |
| `beta` | objemový součinitel tlumení | `1/s` | materiálová vlastnost, výchozí `0` |

Rovnice je lineární. Jediné řešení na časový krok nebo na frekvenci je proto
konečnou odpovědí - neprovádí se žádná vnější nelineární iterace a
`hasConverged()` po vyřešení lineární soustavy vždy hlásí úspěch.

**Rychlost zvuku.** `c` lze zadat přímo materiálovou vlastností
*Speed of sound*. Pokud chybí nebo je nulová, odvodí se z objemové tuhosti a
hustoty:

```
c = sqrt(E / rho)
```

kde `E` je materiálová vlastnost *Modulus of elasticity*. U plynu je `E`
adiabatický objemový modul (`gamma * p0`, asi `1.42e5 Pa` pro vzduch za
atmosférického tlaku), což spolu s `rho = 1.2 kg/m^3` dává známou hodnotu
`c = 344 m/s`. Přímé zadání `c` je obvykle méně náchylné k chybám.

Prvky, které nemají vlastní materiál - například hraniční povrch ohraničující
vysíťovaný objem - přebírají průměrné `c` a `rho` počítané oblasti, aby se na
nich členy impedance a vyzařování vyhodnotily správně.

### 1.3 Diskretizace metodou konečných prvků

Vynásobení váhovou funkcí `w`, integrace přes oblast, použití Gaussovy věty a
vynásobení `c^2` dává slabou formulaci

```
c^2 * INT( grad(phi).grad(w) )  +  INT( d2phi/dt2 * w )  +  beta * INT( dphi/dt * w )
    +  SUM_boundaries( (c/zeta) * SURF( dphi/dt * w ) )
    =  c^2 * SURF( v_n * w )
```

která se po dosazení bázových funkcí prvku `N` stane soustavou druhého řádu

```
M * phi'' + C * phi' + K * phi = f
```

kde

| Matice | Definice | Poznámka |
|---|---|---|
| `M` | `INT( N_m * N_n )` | matice hmotnosti |
| `K` | `c^2 * INT( grad(N_m).grad(N_n) )` | matice tuhosti |
| `C` | `beta * M + (c/zeta) * SURF( N_m * N_n )` | objemové tlumení plus impedance hranice |
| `f` | `c^2 * SURF( v_n * N_m )` | zdroj předepsané normálové rychlosti |

`M` a `K` jsou pozitivně semidefinitní; `C` je pozitivně semidefinitní, pokud
je poměr impedance `zeta` kladný.

**Míry prvků.** Členy oblasti (`M`, `K`, objemová část `C`) se škálují
geometrickou mírou entity, ke které patří:

| Entita | Míra | Kde se nastavuje |
|---|---|---|
| Objem | 1 | - |
| Povrch | tloušťka povrchu | geometrické vlastnosti entity |
| Čára | plocha průřezu čáry | geometrické vlastnosti entity |
| Bod | objem bodu | geometrické vlastnosti entity |

Hraniční členy (impedanční část `C` a zdroj `f`) působí na prostou míru prvku -
plochu povrchového prvku, délku čárového prvku, jednotkovou plochu bodového
prvku. Povrch ohraničující vysíťovaný objem proto přispívá jen svými hraničními
členy, protože jeho tloušťka je ve výchozím stavu nulová.

Bodové prvky nemají žádné integrační body. Jejich příspěvky se soustřeďují
přímo do jejich jediného uzlu, díky čemuž fungují bodové okrajové podmínky na
koncích jednorozměrných potrubí.

### 1.4 Okrajové podmínky

| Okrajová podmínka | Typ | Použití na | Složka |
|---|---|---|---|
| Velocity potential | explicitní (Dirichletova) | bod, čára, povrch | Potential `[m^2/s]` |
| Velocity | přirozená (zdroj) | bod, čára, povrch | Velocity `[m/s]` |
| Absorbing boundary | přirozená (tlumení) | bod, čára, povrch | Acoustic absorption coefficient `[-]` |
| Acoustic impedance | přirozená (tlumení) | bod, čára, povrch | Acoustic impedance `[Pa*s/m]` |

**Velocity** (rychlost). Klasický akustický zdroj: povrch, který kmitá s
předepsanou normálovou rychlostí, jako membrána reproduktoru nebo píst. Kladná
hodnota pohání tekutinu, tj. míří *do* akustické oblasti. Do pravé strany
vstupuje jako `f = c^2 * SURF(v_n * N)`.

Uzel bez jakékoli okrajové podmínky je ideální **tuhá stěna** - přirozenou
podmínkou slabé formulace je nulová normálová rychlost. Pro dutinu s tuhými
stěnami není nutné nic přiřazovat.

**Acoustic impedance** (akustická impedance). Předepisuje poměr tlaku k vnější
normálové rychlosti částic na hranici, `p = Z * u_n`. Interně se vyjadřuje
bezrozměrným **poměrem měrné impedance**

```
zeta = Z / (rho * c)
```

a přispívá hraniční maticí tlumení `(c/zeta) * SURF(N_m * N_n)`.
`Z = rho*c` (`zeta = 1`) je charakteristická impedance tekutiny a dává dokonale
neodrážející zakončení. Pro vzduch za pokojových podmínek je `rho*c` blízko
`415 Pa*s/m`, což je hodnota, kterou okrajová podmínka nabízí ve výchozím
stavu.

**Absorbing boundary** (pohltivá hranice). Stejný člen, ale parametrizovaný
**činitelem pohltivosti** `alpha` při kolmém dopadu v intervalu `[0, 1]`, který
uvádějí katalogové listy materiálů. Na poměr impedance se převádí přes činitel
odrazu rovinné vlny `R = (zeta - 1) / (zeta + 1)` a `alpha = 1 - |R|^2`:

```
zeta = (1 + sqrt(1 - alpha)) / (1 - sqrt(1 - alpha))
```

| `alpha` | `zeta` | Chování |
|---|---|---|
| `1.0` | `1` | plně pohltivé, bezodrazové zakončení, žádný odraz |
| `0.5` | `5.83` | částečně pohltivé |
| `0.1` | `38.0` | slabě pohltivé |
| `0.0` | nekonečno | tuhá stěna, žádné tlumení |

Použijte `alpha = 1` na každém umělém řezu neomezenou oblastí, abyste
napodobili volné vyzařování, a naměřenou hodnotu `alpha` na obložené stěně.
Jsou-li stejné entitě přiřazeny podmínky *Acoustic impedance* i
*Absorbing boundary*, má přednost impedance.

### 1.5 Počáteční podmínky

| Počáteční podmínka | Nastavuje | Poznámka |
|---|---|---|
| Velocity potential | `phi` při `t = 0` | sama o sobě je zřídka nenulová |
| Velocity potential / s | `dphi/dt` při `t = 0` | tato nastavuje počáteční **tlak**, `p = rho * dphi/dt` |

Obě se uplatní pouze v prvním časovém kroku. Chcete-li přechodový výpočet
začít z natlakované oblasti, přiřaďte této entitě *Velocity potential / s*
`= p0 / rho`. Na harmonickou analýzu nemají počáteční podmínky žádný vliv.

### 1.6 Přechodová analýza

Soustava druhého řádu se integruje **Newmarkovým** schématem s `gamma = 1/2` a

```
beta_N = max( time-march approximation coefficient / 2 , 1/4 )
```

(`time-march approximation coefficient` je koeficient aproximace časového
kroku.) Omezení zaručuje `beta_N >= 1/4`, tj. alespoň bezpodmínečně stabilní
variantu průměrného zrychlení, a brání dělení nulou u dopředné aproximace,
jejíž koeficient je `0`.

Každý krok řeší

```
( K + a0*M + a1*C ) * phi_n+1 = f + M*(a0*phi_n + a2*phi'_n + a3*phi''_n)
                                  + C*(a1*phi_n + a4*phi'_n + a5*phi''_n)
```

s obvyklými koeficienty

```
a0 = 1/(beta_N*dt^2)   a1 = gamma/(beta_N*dt)   a2 = 1/(beta_N*dt)
a3 = 1/(2*beta_N) - 1  a4 = gamma/beta_N - 1    a5 = dt/2*(gamma/beta_N - 2)
a6 = dt*(1 - gamma)    a7 = dt*gamma
```

následovanými aktualizací stavu

```
phi''_n+1 = a0*(phi_n+1 - phi_n) - a2*phi'_n - a3*phi''_n
phi'_n+1  = phi'_n + a6*phi''_n + a7*phi''_n+1
```

Matice soustavy je symetrická pozitivně definitní, takže se řeší metodou
**sdružených gradientů** s Jacobiho předpodmíněním.

`phi'` a `phi''` se zapisují do souboru výsledků jako *Potential / s* a
*Potential / s^2*. Představují stav časového integrátoru, takže musí být k
dispozici, aby restart nebo další časový krok pokračovaly správně.

**Přechodová analýza vyžaduje zapnutý řešič v čase.** Bez něj členy hmotnosti
a tlumení zmizí a úloha degeneruje v nesmyslnou Laplaceovu úlohu; řešič to
hlásí jako chybu, místo aby vytvářel nesmysly.

**Volba časového kroku.** Schéma je bezpodmínečně stabilní, takže `dt` je
určeno přesností, nikoli stabilitou. Užitečné pravidlo je rozlišit nejvyšší
sledovanou frekvenci `f_max` alespoň 20 kroky na periodu:

```
dt <= 1 / (20 * f_max)
```

**Volba sítě.** Nejkratší vlnovou délku `lambda_min = c / f_max` rozlište
alespoň 10 lineárními prvky:

```
h <= c / (10 * f_max)
```

### 1.7 Harmonická analýza

Pro ustálenou odezvu při jediné úhlové frekvenci `omega = 2*pi*f` zapíšeme
`phi(x,t) = Re{ PHI(x) * exp(i*omega*t) }`. Pak
`dphi/dt -> i*omega*PHI` a `d2phi/dt2 -> -omega^2*PHI` a soustava se stane
komplexní **Helmholtzovou** úlohou

```
( K - omega^2*M + i*omega*C ) * PHI = F
```

Maticové řešiče Range FEA jsou reálné, proto se komplexní soustava řeší jako
ekvivalentní reálná bloková soustava dvojnásobné velikosti. S `S = K - omega^2*M`
a `D = omega*C`:

```
| S   -D |   | PHI_re |   | F_re |
|        | * |        | = |      |
| D    S |   | PHI_im |   | F_im |
```

Tato bloková matice je nesymetrická a nad statickou mezí indefinitní, takže se
řeší metodou **GMRES** s Jacobiho předpodmíněním. Restartovaný GMRES na
indefinitních soustavách stagnuje, kdykoli je Krylovův prostor mnohem menší
než soustava, proto řešič automaticky rozšíří restart GMRES na velikost
soustavy, omezenou pevným rozpočtem 256 MB pro obě Krylovovy báze. Nastavený
restart se pouze zvyšuje, nikdy nesnižuje, a změna se hlásí v logu řešiče:

```
Increasing GMRES restart from 10 to 403 for the frequency domain system.
```

Harmonická analýza prochází seznam frekvencí

```
f_i = first frequency + i * frequency step,   i = 0 .. number of frequencies - 1
```

(`first frequency` - první frekvence, `frequency step` - frekvenční krok,
`number of frequencies` - počet frekvencí.) Každá frekvence je nezávislým
řešením a vytvoří vlastní záznam výsledků, stejně jako přechodová analýza
vytvoří jeden záznam na časový krok. Řešič v čase zde nehraje žádnou roli;
pokud je náhodou zapnutý, s upozorněním se přeskočí a jeho nastavení zůstane
nedotčeno, aby se při přepnutí zpět na přechodovou analýzu neztratilo.

### 1.8 Odvozené výsledky

| Výsledek | Použití na | Přechodová | Harmonická |
|---|---|---|---|
| Potential `[m^2/s]` | uzel | `phi(t)` | `Re{PHI}` |
| Potential (imaginary part) `[m^2/s]` | uzel | - | `Im{PHI}` |
| Potential / s `[m^2/s^2]` | uzel | `dphi/dt` | `omega * abs(PHI)` |
| Potential / s^2 `[m^2/s^3]` | uzel | `d2phi/dt2` | `omega^2 * abs(PHI)` |
| Acoustic pressure `[Pa]` | uzel | `rho * dphi/dt`, okamžitá hodnota | `omega * rho * abs(PHI)`, amplituda |
| Acoustic phase `[Deg]` | uzel | - | `arg(P)` |
| Sound pressure level `[dB]` | uzel | `20*log10(abs(p) / p_ref)` | `20*log10(abs(P)/sqrt(2) / p_ref)` |
| Acoustic particle velocity `[m/s]` | prvek | `-grad(phi)` | amplituda `-grad(PHI)`, po složkách |
| Acoustic intensity `[W/m^2]` | prvek | `p * u`, okamžitá hodnota | `0.5 * Re{P * conj(U)}`, časově středovaná |

**Hladina akustického tlaku** (*Sound pressure level*) se počítá vůči
*Referenčnímu tlaku* z nastavení akustiky, ve výchozím stavu `2e-5 Pa` - práh
slyšení ve vzduchu. V harmonické analýze se používá efektivní hodnota
harmonického signálu, tedy amplituda dělená `sqrt(2)`; v přechodové analýze se
používá okamžitá velikost, takže hlášená hladina v rámci periody kolísá.

**Akustická intenzita** v harmonické analýze je časově středovaná činná
intenzita, tj. čistý tok akustického výkonu. V čisté stojaté vlně je
identicky nulová a v rovinné postupné vlně se rovná `0.5 * |p| * |u|`.

### 1.9 Přechodová, nebo harmonická?

| | Přechodová | Harmonická |
|---|---|---|
| Řeší | `M*phi'' + C*phi' + K*phi = f` v čase | `(K - w^2*M + i*w*C)*PHI = F` pro každou frekvenci |
| Odpovídá na | „co se stane, když ...“ | „jaká je ustálená odezva při ...“ |
| Buzení | libovolný časový průběh, včetně jediného pulzu | vždy jedna sinusoida |
| Cena | jedno řešení SPD na časový krok | jedno indefinitní řešení dvojnásobné velikosti na frekvenci |
| Záznamy | jeden na časový krok | jeden na frekvenci |
| Fázová informace | implicitně v časovém průběhu | explicitně, jako výsledkové pole |

**Přechodovou analýzu použijte pro**

- impulzní odezvu, studium ozvěny a dozvuku;
- přechodové děje - ránu, uzavření ventilu, bouchnutí dveří;
- širokopásmové buzení, kdy jediný výpočet následovaný Fourierovou
  transformací monitorovacího bodu pokryje celé spektrum najednou;
- jakýkoli případ, kdy je odpovědí samotný časový průběh.

**Harmonickou analýzu použijte pro**

- frekvenční přenosové funkce a křivky útlumu průchodem;
- rezonance dutin a potrubí a rozložení tlaku při rezonanci;
- účinnost tlumičů, tlumičů hluku a absorbérů ve frekvenčním pásmu;
- mapy hladiny akustického tlaku pro jediný tón;
- jakýkoli případ, kdy záleží jen na ustáleném stavu. Harmonický výpočet je
  obvykle mnohem levnější než krokovat přechodový výpočet tak dlouho, až
  přechodové jevy odezní.

Praktickým kompromisem je najít rezonance hrubým harmonickým rozmítáním a pak
zjemnit jen zajímavé pásmo.

---

## 2. Grafické uživatelské rozhraní

Vše, co souvisí s akustikou, se nachází v panelu **Řešič** (záložky `Úloha`,
`Okrajové podmínky`, `Počáteční podmínky`, `Materiál`, `Výsledky`), v nabídce
**Úloha** a v panelu **Model**.

### 2.1 Výběr akustické úlohy

`Úloha` -> `Pořadí kroků úloh` (`Ctrl+P`) otevře dialog pořadí kroků úlohy.
Přidejte krok a vyberte **Acoustics** - „Customized wave equation for solving
acoustic problems“ (upravená vlnová rovnice pro řešení akustických úloh).

Akustika se vzájemně vylučuje s **Wave - general** a s **Modal analysis**. Lze
ji kombinovat s jinými fyzikami; v takovém případě určuje pořadí provádění
pořadí kroků.

### 2.2 Záložka Úloha

Jakmile je vybrána akustika, zobrazí záložka `Úloha` panelu Řešič dvě skupiny.

**Řešič v čase** - zobrazí se, protože akustika je časově závislý typ úlohy.

| Pole | Význam |
|---|---|
| Zapnutí | zapíná a vypíná přechodové časové krokování |
| Aproximace | aproximace časového kroku; převádí se na Newmarkovo `beta_N` podle 1.6 |
| Počáteční čas | první hodnota času v sekundách; při restartu řešiče se ignoruje |
| Koncový čas | `start time + time-step size * number of time-steps`, pouze pro čtení |
| Velikost časového kroku | `dt` v sekundách |
| Počet časových kroků | kolik kroků se má vypočítat |
| Frekvence výstupu | zapsat soubor výsledků každých N kroků; `0` zapíše pouze poslední krok |

**Nastavení akustické analýzy** - skupina přidaná pro akustiku.

| Pole | Význam |
|---|---|
| Typ analýzy | *Transient* (přechodová) nebo *Harmonic* (harmonická) |
| První frekvence `[Hz]` | první frekvence harmonického rozmítání |
| Frekvenční krok `[Hz]` | přírůstek mezi dvěma po sobě jdoucími frekvencemi |
| Počet frekvencí | kolik frekvencí se má vyřešit; každá vytvoří vlastní záznam |
| Rozmítání | popisek pouze pro čtení s výsledným rozsahem frekvencí |
| Referenční tlak `[Pa]` | reference pro hladinu akustického tlaku, ve výchozím stavu `2e-5 Pa` |

Tři frekvenční pole jsou při volbě *Transient* zašedlá. Referenční tlak platí
pro oba typy analýzy.

### 2.3 Záložka Materiál

Záložka `Materiál` uvádí materiál přiřazený vybrané entitě a jeho
materiálové vlastnosti. Pro akustiku jsou relevantní tyto vlastnosti:

| Vlastnost | Jednotky | Povinná | Poznámka |
|---|---|---|---|
| Density | `kg/m^3` | ano | `rho` |
| Speed of sound | `m/s` | jedna z těchto dvou | `c`, použije se přímo, je-li nenulová |
| Modulus of elasticity | `Pa` | jedna z těchto dvou | náhrada, `c = sqrt(E/rho)` |
| Acoustic damping factor | `1/s` | ne | `beta`, objemový útlum, ve výchozím stavu `0` |

Entitu akustický řešič řeší, má-li hustotu **a** buď rychlost zvuku, nebo
modul pružnosti. Protože jde o alternativy, kontrola nastavení nehlásí ani
jednu jako chybějící; model, který nemá ani jednu, řešič odmítne s výslovnou
zprávou.

Každá vlastnost je tabulkou v závislosti na teplotě, takže lze zadat rychlost
zvuku závislou na teplotě, která se bude interpolovat, pokud souběžně běží
úloha přenosu tepla.

### 2.4 Záložka Okrajové podmínky

Záložka je rozdělena na seznam dostupných okrajových podmínek nahoře a editor
vybrané podmínky dole. Nejprve vyberte entitu ve stromu `Model`; správce pak
nabídne okrajové podmínky platné pro daný typ entity a vybranou úlohu.
Zaškrtnutím ji přiřadíte a poté upravte její složky ve spodním stromu.

Pro akustiku správce nabízí:

- **Velocity potential** - explicitní, body, čáry a povrchy. Složka:
  *Potential* `[m^2/s]`.
- **Velocity** - body, čáry a povrchy. Složka: *Velocity* `[m/s]`.
- **Absorbing boundary** - body, čáry a povrchy. Složka: *Acoustic absorption
  coefficient*, bezrozměrná, výchozí `1.0`.
- **Acoustic impedance** - body, čáry a povrchy. Složka: *Acoustic impedance*
  `[Pa*s/m]`, výchozí `415`.

Každá složka je tabulkou v závislosti na čase, takže zdroji lze zadat časový
průběh: přidáním řádků lze rychlost postupně zvyšovat nebo vyslat krátký
pulz. V harmonické analýze se použije hodnota v aktuálním čase řešiče, což je
u jednohodnotové tabulky prostě tato hodnota.

Chcete-li, aby stěna byla akusticky tuhá, nechte ji bez okrajové podmínky.

### 2.5 Záložka Počáteční podmínky

Stejné rozvržení jako záložka okrajových podmínek. Pro akustiku nabízí
**Velocity potential** a **Velocity potential / s**. Ta druhá nastavuje
počáteční pole tlaku.

### 2.6 Nastavení maticového řešiče

`Úloha` -> `Nastavit maticový řešič úloh` nastavuje iterační řešiče.
Přechodová akustická analýza používá položku **CG**, harmonická analýza
položku **GMRES**.

Výchozí restart GMRES `10` je pro blokovou soustavu ve frekvenční oblasti příliš
malý. Řešič ho zvýší automaticky, ale počet *vnějších* iterací a hodnota
konvergence se přebírají tak, jak jsou nastaveny, takže je zde zvyšte, pokud
harmonický výpočet nekonverguje.

### 2.7 Monitorovací body

`Úloha` -> `Definovat monitorovací body` umístí sondy na zadané souřadnice a
vybere proměnnou, která se má zaznamenávat. Monitorovací bod na
*Acoustic pressure* je přirozeným mikrofonem: v přechodovém výpočtu vytvoří
časový průběh tlaku v daném místě, který lze prohlédnout přes
`Zprávy` -> `Monitorovací body`.

### 2.8 Výsledky

Po výpočtu uvádí záložka `Výsledky` vypočtené proměnné a ovládá, co zobrazuje
3D pohled. Akustické proměnné jsou uvedeny v části 1.8.

Uzlové proměnné (tlak, hladina akustického tlaku, fáze, potenciál) se
vykreslují jako hladká pole; prvkové proměnné (rychlost částic, intenzita) jsou
vektory a nejlépe se zobrazují vektorovým polem nebo řezem.

Záložka `Záznamy` panelu `Model` uvádí jednotlivé záznamy výsledků - jeden na
časový krok u přechodového výpočtu, jeden na frekvenci u harmonického
rozmítání - a umožňuje je procházet nebo animovat.

`Zprávy` -> `Log soubor řešiče` ukazuje celý výstup řešiče včetně bloku
statistiky vypisovaného pro každou akustickou proměnnou po každém záznamu.

---

## 3. Návod - přechodová akustická analýza

**Cíl.** Vypočítat impulzní odezvu uzavřené vzduchové dutiny a odečíst
časový průběh tlaku v poloze mikrofonu.

Návod předpokládá vysíťovaný objemový model. Použijte `Soubor` -> `Nový model`,
nakreslete kvádr pomocí `Geometrie` -> `Vytvořit prvek` nebo importujte
geometrii a tetraedrizujte ji - to pokrývá návod *Nakreslit krychli*.

### Krok 1 - vyberte úlohu

1. `Úloha` -> `Pořadí kroků úloh` (`Ctrl+P`).
2. Přidejte krok a vyberte **Acoustics**.
3. Potvrďte tlačítkem `OK`.

Záložka `Úloha` panelu Řešič nyní zobrazuje skupiny *Řešič v čase* a
*Nastavení akustické analýzy*.

### Krok 2 - přiřaďte materiál

1. Ve stromu `Model` vyberte objemovou entitu.
2. Otevřete záložku `Materiál`.
3. Přiřaďte materiál nebo vytvořte nový a zadejte:
   - **Density** = `1.2` `kg/m^3`
   - **Speed of sound** = `344` `m/s`
4. **Acoustic damping factor** ponechte na `0` pro netlumenou dutinu, nebo ho
   nastavte na malou hodnotu, například `1.0` `1/s`, aby odezva doznívala.

### Krok 3 - zvolte typ analýzy a časový krok

1. Otevřete záložku `Úloha`.
2. V *Nastavení akustické analýzy* nastavte **Typ analýzy** = `Transient`.
3. V *Řešiči v čase*:
   - zaškrtněte **Zapnutí**;
   - **Počáteční čas** = `0`;
   - **Velikost časového kroku** = `2e-5` s. Podle `dt <= 1/(20*f_max)` to
     rozliší frekvence přibližně do `2.5 kHz`;
   - **Počet časových kroků** = `500`, což dává `10 ms` simulovaného času;
   - **Frekvence výstupu** = `5`, což zapíše 100 záznamů výsledků.

Zkontrolujte síť podle `h <= c/(10*f_max)`: pro `f_max = 2.5 kHz` a
`c = 344 m/s` by velikost prvku neměla překročit přibližně `14 mm`.

### Krok 4 - vybuďte dutinu

Dvě možnosti, vyberte jednu.

**A - pohyblivá stěna.** Vyberte povrch, který představuje zdroj, otevřete
záložku `Okrajové podmínky` a zaškrtněte **Velocity**. Do tabulky složky
zadejte časový průběh, například krátký pulz:

| Čas `[s]` | Rychlost `[m/s]` |
|---|---|
| `0.0` | `0.0` |
| `0.0002` | `0.01` |
| `0.0004` | `0.0` |
| `0.01` | `0.0` |

**B - na počátku natlakovaná oblast.** Vyberte entitu, která má začínat
natlakovaná, otevřete záložku `Počáteční podmínky` a zaškrtněte
**Velocity potential / s**. Zadejte `p0 / rho`; pro `p0 = 1 Pa` a
`rho = 1.2 kg/m^3` je to `0.833`.

Všechny ostatní povrchy nechte bez okrajové podmínky, aby dutina měla tuhé
stěny, nebo zaškrtněte **Absorbing boundary** s činitelem pohltivosti
odpovídajícím obložení stěny, aby energie mohla unikat.

### Krok 5 - přidejte mikrofon

1. `Úloha` -> `Definovat monitorovací body`.
2. Přidejte bod v místě zájmu.
3. Jako jeho proměnnou vyberte **Acoustic pressure**.

### Krok 6 - řešte

1. `Řešení` -> `Spustit řešič` (`Ctrl+R`).
2. Kontrola nastavení ohlásí chybějící materiály nebo okrajové podmínky ještě
   před spuštěním výpočtu.
3. Průběh sledujte v `Zprávy` -> `Log soubor řešiče`. Každý časový krok vypíše
   konvergenci CG a statistiku každé akustické proměnné.

### Krok 7 - prohlédněte si výsledky

- Panel `Model`, záložka `Záznamy`: procházejte nebo animujte časové záznamy.
- Záložka `Výsledky`: zobrazte na modelu **Acoustic pressure** nebo
  **Sound pressure level** a **Acoustic particle velocity** jako vektorové pole.
- `Zprávy` -> `Monitorovací body`: časový průběh tlaku v mikrofonu. Rychlost
  jeho doznívání dává dobu dozvuku; jeho Fourierova transformace dává rezonance
  dutiny.

### Řešení problémů

| Příznak | Příčina |
|---|---|
| *"Transient acoustic analysis requires an enabled time solver"* | v *Řešiči v čase* není zaškrtnuto *Zapnutí*, nebo by typ analýzy měl být *Harmonic* |
| *"Acoustic solver requires a positive time step size"* | velikost časového kroku je nulová |
| *"No element with a valid speed of sound was found"* | žádná entita nemá hustotu spolu s rychlostí zvuku nebo modulem pružnosti |
| Odezva nikdy nedoznívá | nikde není žádné tlumení - u dokonale tuhé bezeztrátové dutiny očekávané; přidejte pohltivou hranici nebo akustický součinitel tlumení |
| Zvlnění před čelem vlny | síť nebo časový krok jsou pro frekvenční obsah příliš hrubé; zjemněte obojí |

---

## 4. Návod - harmonická akustická analýza

**Cíl.** Vypočítat frekvenční odezvu potrubí s bezodrazovým zakončením a mapu
hladiny akustického tlaku pro každou frekvenci.

### Krok 1 - vyberte úlohu

`Úloha` -> `Pořadí kroků úloh` (`Ctrl+P`), přidejte krok, vyberte
**Acoustics**.

### Krok 2 - přiřaďte materiál

Jako v přechodovém návodu: **Density** `1.2 kg/m^3` a **Speed of sound**
`344 m/s` na entitě tekutiny.

Síť dimenzujte pro **nejvyšší** frekvenci rozmítání:
`h <= c / (10 * f_max)`.

### Krok 3 - nastavte rozmítání

Otevřete záložku `Úloha` a v *Nastavení akustické analýzy* nastavte:

| Pole | Hodnota |
|---|---|
| Typ analýzy | `Harmonic` |
| První frekvence | `50` Hz |
| Frekvenční krok | `50` Hz |
| Počet frekvencí | `20` |
| Referenční tlak | `2e-5` Pa |

Popisek *Rozmítání* potvrdí `50 - 1000 [Hz]`, tj. 20 řešení a 20 záznamů
výsledků.

Skupina *Řešič v čase* může zůstat zapnutá - harmonická analýza ji ignoruje a
zapíše do logu upozornění. Její nastavení se zachová.

### Krok 4 - vybuďte potrubí

Vyberte vstupní povrch, otevřete záložku `Okrajové podmínky` a zaškrtněte
**Velocity**. Zadejte jediný řádek s rychlostí pístu, například `0.01` `m/s`.
Hodnota se chápe jako amplituda harmonického buzení při každé frekvenci
rozmítání.

### Krok 5 - zakončete potrubí

Vyberte výstupní povrch a zaškrtněte buď:

- **Absorbing boundary** s **Acoustic absorption coefficient** = `1.0` pro
  dokonale neodrážející zakončení; nebo
- **Acoustic impedance** s **Acoustic impedance** = `rho * c` = `413`
  `Pa*s/m`, což je totéž vyjádřené jinak.

Menší činitel pohltivosti použijte k modelování částečně odrážejícího
zakončení a výstup nechte bez jakékoli podmínky k modelování uzavřeného, tuhého
konce.

Stěny potrubí nepotřebují žádnou okrajovou podmínku - ve výchozím stavu jsou
tuhé.

### Krok 6 - zkontrolujte maticový řešič

`Úloha` -> `Nastavit maticový řešič úloh`, položka **GMRES**. Restart se zvýší
automaticky, ale pokud log ukazuje stagnující reziduum, zvyšte počet vnějších
iterací a podle potřeby uvolněte nebo zpřísněte hodnotu konvergence.

### Krok 7 - řešte

`Řešení` -> `Spustit řešič` (`Ctrl+R`). Log ukazuje jeden blok na frekvenci:

```
Frequency:         1 of 20        | f =  5.000000e+01 [Hz]
{
    Increasing GMRES restart from 10 to ... for the frequency domain system.
    ...
    Acoustic pressure [Pa]
    Sound pressure level [dB]
    ...
}
```

### Krok 8 - prohlédněte si výsledky

- Panel `Model`, záložka `Záznamy`: jeden záznam na frekvenci. Jejich
  procházení je frekvenčním rozmítáním.
- Záložka `Výsledky`: zobrazte **Sound pressure level** pro mapu v decibelech,
  **Acoustic pressure** pro amplitudu a **Acoustic phase** pro zobrazení čel
  vln.
- **Acoustic intensity** ukazuje, kudy proudí akustický výkon. V dobře
  zakončeném potrubí je rovnoměrná a směřuje k výstupu; v rezonanční dutině je
  téměř nulová.
- Monitorovací bod na **Acoustic pressure** zaznamená amplitudu při každé
  frekvenci, tj. frekvenční přenosovou funkci v daném místě.

### Interpretace výsledku

Pro přímé potrubí buzené rychlostí `v` a bezodrazově zakončené je analytické
řešení rovinné vlny

```
|p| = rho * c * v        constant along the duct
|u| = v
|PHI| = v / k            with k = omega / c
I = 0.5 * |p| * |u|
```

(`constant along the duct` - konstantní podél potrubí, `with` - kde.) Pro
`rho = 1.2`, `c = 344`, `v = 0.01` je to `|p| = 4.13 Pa`, hladina akustického
tlaku `103.3 dB` a intenzita `0.0206 W/m^2`. Odchylky od konstantního `|p|`
podél potrubí znamenají, že zakončení odráží - buď záměrně, nebo proto, že
činitel pohltivosti není `1`.

S tuhým zakončením se totéž potrubí stane rezonančním. Amplituda tlaku má
maxima při čtvrtvlnných rezonancích `f = (2n+1) * c / (4L)` a fáze je
konstantní `+/-90` stupňů s nulovou čistou intenzitou.

### Řešení problémů

| Příznak | Příčina |
|---|---|
| *"Harmonic acoustic analysis requires a positive frequency"* | první frekvence je nulová a frekvenční krok je nulový |
| Reziduum stagnuje, výsledky vypadají zašuměně | GMRES nezkonvergoval; zvyšte vnější iterace v nastavení maticového řešiče nebo zhrubněte síť |
| Amplituda při jedné frekvenci prudce vzroste | rozmítání narazilo na netlumenou rezonanci; přidejte pohltivou hranici nebo akustický součinitel tlumení |
| Tlak se podél údajně bezodrazového potrubí silně mění | zakončení odráží; zkontrolujte činitel pohltivosti nebo hodnotu impedance |

---

## 5. Ověření

Jednotkový test `tst_solver_acoustic` (v `range-solver-lib/tests`, sestavuje se
s `-DRANGE_BUILD_TESTS=ON`) ověřuje řešič vůči řešením v uzavřeném tvaru na
jednorozměrném potrubí o délce `1 m` diskretizovaném 200 prutovými prvky, s
výchozím nastavením maticového řešiče.

**Harmonická, bezodrazové zakončení** - `rho = 1.2 kg/m^3`, `c = 340 m/s`,
`v = 0.01 m/s`, `f = 500 Hz`:

| Veličina | Analyticky | Vypočteno |
|---|---|---|
| Amplituda tlaku | `rho*c*v = 4.0800 Pa` | `4.0800 - 4.0804` v každém uzlu |
| Amplituda potenciálu | `v/k = 1.0823e-3 m^2/s` | v rámci 1 % |
| Rychlost částic | `0.01 m/s` | `0.009999 m/s` |
| Hladina akustického tlaku | `103.18 dB` | v rámci `0.1 dB` |
| Intenzita | `0.0204 W/m^2` | v rámci 2 % |
| Gradient fáze | `k * dx` | v rámci 5 stupňů |

**Harmonická, tuhé zakončení** - stojatá vlna dává fázi přesně `+/-90` stupňů
a nulovou čistou intenzitu a špičková amplituda převyšuje amplitudu postupné
vlny.

**Přechodová** - buzené potrubí s bezodrazovým zakončením zůstává omezené,
Newmarkův stav je zveřejňován, takže integrace mezi kroky pokračuje správně, a
vybuzené pole je všude nenulové a konečné.

---

## 6. Omezení

- **Žádná vazba konstrukce a akustiky.** Akustiku nelze kombinovat s modální
  analýzou a neexistuje rozhraní tekutina-konstrukce s `RSolverStress`.
  Kmitající konstrukci je nutné reprezentovat předepsanou okrajovou podmínkou
  *Velocity*.
- **Pouze lineární akustika.** Malá amplituda, klidné prostředí. Žádné střední
  proudění, žádné konvektivní jevy, žádné nelineární šíření.
- **Pouze reálná impedance.** Pohltivá a impedanční okrajová podmínka
  přijímají reálnou hodnotu. Komplexní, frekvenčně závislá povrchová impedance
  není podporována.
- **Materiálová data nezávislá na frekvenci.** Rychlost zvuku a součinitel
  tlumení jsou tabelovány v závislosti na teplotě, nikoli na frekvenci, takže
  jsou v celém rozmítání konstantní.
- **Hraniční členy používají prostou míru prvku.** U jedno- nebo
  dvourozměrného modelu, jehož plocha průřezu nebo tloušťka není `1`, se jí
  členy zdroje a impedance neškálují, kdežto členy oblasti ano. Nesouladu se
  vyhnete jednotkovou plochou průřezu nebo plným trojrozměrným modelem.
- **Harmonické buzení je reálné.** Předepsaná rychlost vstupuje jako reálná
  část komplexního zatížení; zdroje s relativním fázovým posunem nelze
  definovat.

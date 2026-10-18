# Analýza napětí - Teoretická příručka a uživatelská příručka

Tento dokument popisuje strukturální řešič (`RSolverStress`) v Range FEA, který
obsluhuje typy úloh **Analýza napětí** (*Stress analysis*) i **Modální
analýza** (*Modal analysis*): rovnice, které řeší, význam každého vstupu, který
přijímá, části grafického uživatelského rozhraní, které ho ovládají, a dva
podrobné návody.

**Obsah**

1. [Teoretický základ](#1-teoretický-základ)
2. [Grafické uživatelské rozhraní](#2-grafické-uživatelské-rozhraní)
3. [Návod - statická analýza napětí](#3-návod---statická-analýza-napětí)
4. [Návod - modální analýza](#4-návod---modální-analýza)
5. [Kontrola modelu](#5-kontrola-modelu)
6. [Omezení](#6-omezení)

---

## 1. Teoretický základ

### 1.1 Primární neznámá - pole posunutí

Primární neznámou je **vektor posunutí** `u = (ux, uy, uz)` v každém uzlu, v
metrech. Každý uzel nese tři stupně volnosti, takže síť s `N` uzly dává soustavu
`3N` rovnic. Přetvoření, napětí a reakční síly se po vyřešení získají z pole
posunutí.

### 1.2 Řídicí rovnice

Řešič implementuje **lineární izotropní pružnost s malými přetvořeními**. Je
definována třemi vztahy.

**Rovnováha** tělesa zatíženého objemovým zatížením `b` a povrchovým zatížením
`t`:

```
div(sigma) + b = 0
```

Vztah **přetvoření - posunutí**, malá přetvoření:

```
eps_xx = dux/dx        gamma_xy = dux/dy + duy/dx
eps_yy = duy/dy        gamma_yz = duy/dz + duz/dy
eps_zz = duz/dz        gamma_xz = dux/dz + duz/dx
```

**Konstitutivní** vztah s tepelným členem:

```
sigma = D * ( eps - alpha * dT )
```

| Symbol | Význam | Jednotky | Zdroj |
|---|---|---|---|
| `u` | posunutí | `m` | řešená veličina |
| `sigma` | Cauchyho napětí | `Pa` | odvozené |
| `eps` | inženýrské přetvoření | `-` | odvozené |
| `E` | modul pružnosti | `Pa` | materiálová vlastnost |
| `v` | Poissonovo číslo | `-` | materiálová vlastnost |
| `rho` | hustota | `kg/m^3` | materiálová vlastnost |
| `alpha` | součinitel teplotní roztažnosti | `1/K` | materiálová vlastnost |
| `dT` | nárůst teploty nad stav bez napětí | `K` | viz 1.5 |

Všechny čtyři materiálové vlastnosti jsou **povinné** - entita, které některá z
nich chybí, se neřeší. Dva Rayleighovy součinitele tlumení dynamické analýzy
(viz 1.7) jsou volitelné.

### 1.3 Matice pružnosti

**Objemové prvky** používají úplnou trojrozměrnou izotropní matici pružnosti ve
Voigtově zápisu s vektorem napětí uspořádaným jako
`[sigma_xx, sigma_yy, sigma_zz, tau_yz, tau_xz, tau_xy]`:

```
        E                | 1-v   v    v      0        0        0     |
D = -----------------  * |  v   1-v   v      0        0        0     |
    (1+v)*(1-2v)         |  v    v   1-v     0        0        0     |
                         |  0    0    0   (1-2v)/2    0        0     |
                         |  0    0    0      0     (1-2v)/2    0     |
                         |  0    0    0      0        0     (1-2v)/2 |
```

**Povrchové prvky** používají dvourozměrný tvar **rovinné deformace** s
vektorem napětí `[sigma_xx, sigma_yy, tau_xy]`, vyhodnocený v lokální rovině
prvku a otočený zpět do globálních souřadnic:

```
        E                | 1-v   v      0     |
D = -----------------  * |  v   1-v     0     |
    (1+v)*(1-2v)         |  0    0   (1-2v)/2 |
```

Jde o rovinnou **deformaci**, nikoli o rovinnou napjatost. Tenká deska zatížená
ve své rovině proto vyjde tužší, než by předpověděla idealizace rovinné
napjatosti.

**Čárové prvky** se považují za **pruty**: sestavuje se pouze osová tuhost
`E * A` ve směru prvku, kde `A` je plocha průřezu čáry. Neexistuje ohybová,
smyková ani torzní tuhost, a to ani u typů prvků nazvaných „nosník“.

### 1.4 Diskretizace metodou konečných prvků

Slabá formulace rovnováhy dává pro každý prvek

```
K = INT( B^T * D * B )        stiffness matrix
M = INT( rho * N^T * N )      consistent mass matrix
f = load vector, see 1.5
```

(`stiffness matrix` - matice tuhosti, `consistent mass matrix` - konzistentní
matice hmotnosti, `load vector, see 1.5` - vektor zatížení, viz 1.5), kde `B` je
matice přetvoření - posunutí sestavená z derivací bázových funkcí. Každá matice
prvku má rozměr `3n x 3n` pro prvek s `n` uzly, se stupni volnosti prokládanými
jako `[ux1, uy1, uz1, ux2, uy2, uz2, ...]`.

**Míry prvků.** Stejně jako jinde v Range FEA dostávají entity nižší dimenze
geometrickou míru, aby mohly reprezentovat skutečná tělesa:

| Entita | Míra | Kde se nastavuje |
|---|---|---|
| Objem | 1 | - |
| Povrch | tloušťka povrchu | geometrické vlastnosti entity |
| Čára | plocha průřezu čáry | geometrické vlastnosti entity |
| Bod | objem bodu | geometrické vlastnosti entity |

Povrch s nulovou tloušťkou nebo čára s nulovou plochou průřezu nepřispívají
žádnou tuhostí ani hmotností - stále však mohou nést zatížení, což je přesně to,
co je potřeba u zatěžované stěny vysíťovaného tělesa.

Matice hmotnosti se sestavuje jen tehdy, když je potřeba, tedy pro dynamickou
přechodovou analýzu nebo pro modální analýzu.

### 1.5 Zatížení, okrajové a environmentální podmínky

#### Okrajové podmínky

| Okrajová podmínka | Typ | Použití na | Složky |
|---|---|---|---|
| Displacement | explicitní (Dirichletova) | bod, čára, povrch, objem | Displacement in X, Y, Z `[m]` |
| Normal displacement | explicitní, lokální soustava | povrch | Displacement `[m]` |
| Roller displacement | explicitní, lokální soustava | bod, čára, povrch | Displacement `[m]` |
| Force | přirozená | bod, čára, povrch | Force in X, Y, Z `[N]` |
| Force (unit area) | přirozená | povrch | Force per unit area in X, Y, Z `[N/m^2]` |
| Pressure | přirozená | povrch | Pressure `[Pa]` |
| Weight | přirozená | bod, čára, povrch | Weight `[kg]` |

**Displacement** (posunutí) je podmínka s **volitelnými** složkami: každou z
jejích tří složek lze zapnout nebo vypnout samostatně. Zapnutá složka váže
daný globální směr každého uzlu entity na zadanou hodnotu; vypnutá složka
ponechá daný směr zcela volný. Vypnutím *Displacement in Y* a
*Displacement in Z* se tedy vytvoří podpora, která drží X a nechá uzel klouzat v
rovině YZ.

Pokud volné směry nejsou globální, ale leží v tečné rovině zakřiveného povrchu,
použijte místo toho *Roller displacement*.

**Normal displacement** (normálové posunutí) a **Roller displacement**
(posuvná podpora) pracují v **lokální soustavě** vytvořené pro každý vázaný
uzel. Ve výchozím stavu soustava sleduje geometrii:

- na **povrchu** je lokální osa X průměrnou normálou sousedních povrchových
  prvků;
- na **čáře** se lokální soustava vytvoří ze směru prvku;
- na **bodu** není geometrie, kterou by bylo možné sledovat, proto se vždy
  použije směr zadaný s okrajovou podmínkou.

Zaškrtnutí **Use entered local direction** to pro povrch nebo čáru potlačí a
lokální osu X převezme ze směru zadaného pod ním. Použijte ho, když směr, který
chcete vázat, není ten, který náhodou dává síť - například posuvná podpora,
která má klouzat po fasetovaném povrchu podél globální osy.

Vazby z různých entit se na uzlu, který sdílejí, **kombinují**. Řešič shromáždí
každou vazbu posunutí působící na uzel, z nichž každá je výrokem `d . u = v` o
jednom směru, a Gramovou-Schmidtovou ortogonalizací ji zredukuje na nejvýše tři
navzájem kolmé držené směry, přičemž stejnými operacemi provede i předepsané
hodnoty. Tyto držené směry se stanou vedoucími osami soustavy uzlu; co zbude,
soustavu doplní a zůstane volné. Uzel tak může být v jednom směru držen
globálním *Displacement*, v jiném *Roller displacement* na nakloněné stěně a ve
třetím zůstat volný - obě podmínky se navzájem nepřepisují a globálně
formulovaná složka se nikdy potichu nečte v lokální soustavě někoho jiného.

Dvě entity, které předepisují **různé hodnoty ve stejném směru**, nemohou být
splněny obě. Řešič to během ortogonalizace zjistí a zastaví se s chybou, která
uvede uzel, místo aby spor rozhodl ve prospěch podmínky, která byla náhodou
načtena jako poslední.

Matice prvků a vektory zatížení vázaného uzlu se otočí do této soustavy, vazba
se uplatní a výsledné posunutí se otočí zpět. V této soustavě:

- **Normal displacement** uzamkne všechny tři lokální složky a předepsaný
  skalár se uplatní podél lokální normály. Uzel se posune přesně o zadanou
  hodnotu podél normály a jinak je pevný.
- **Roller displacement** uzamkne na povrchu nebo bodu pouze lokální normálovou
  složku, takže uzel může volně klouzat v tečné rovině. Na čárové entitě místo
  toho uzamkne dvě složky kolmé k čáře, takže uzel může volně klouzat podél
  čáry.

**Force** (síla) je celková síla v newtonech rozložená na entitu, které je
přiřazena - u čáry se dělí délkou entity, u povrchu plochou entity a u bodové
entity počtem bodů. Zjemnění sítě proto celkové zatížení nemění.

**Force (unit area)** (síla na jednotku plochy) je povrchové zatížení v
`N/m^2` působící v zadaném globálním směru, integrované přes plochu prvku.

**Pressure** (tlak) působí podél **normály** povrchu. Řešič pro každý hraniční
povrch zjistí, zda jeho normála míří do sousedního počítaného objemového
prvku, a podle toho otočí znaménko, takže kladný tlak vždy tlačí na těleso.
Povrch, který neohraničuje žádný objemový prvek, se nikdy nezařadí jako
mířící dovnitř, takže tam zatížení působí podél **záporné** normály povrchu - v
takovém případě zkontrolujte směr výsledného zatížení.

**Weight** (hmotnost) je hmotnost v kilogramech připojená k entitě. Násobí se
gravitačním zrychlením z environmentální podmínky a rozkládá se na entitu
stejně jako Force. Použijte ji pro nemodelované připojené hmoty.

#### Environmentální podmínky

| Environmentální podmínka | Složky | Účinek |
|---|---|---|
| Gravitational acceleration | G acceleration in X, Y, Z `[m/s^2]` | vlastní tíha a okrajová podmínka Weight |
| Temperature | Temperature `[K]` | referenční teplota stavu bez napětí |

**Vlastní tíha** se automaticky generuje pro každou entitu, která má hustotu i
geometrickou míru, jako `rho * g` integrované přes prvek. Objeví se, jakmile je
přiřazena environmentální podmínka gravitačního zrychlení; výchozí hodnota je
`-9.80665 m/s^2` v Z.

**Tepelné zatížení.** Rozdíl teplot, který pohání člen tepelné roztažnosti, je

```
dT = element temperature - environment temperature
```

(`element temperature` - teplota prvku, `environment temperature` - teplota
prostředí.) **Teplota prostředí** pochází z environmentální podmínky
*Temperature* a je referenčním stavem bez napětí. **Teplota prvku** pochází z
řešiče přenosu tepla, který běží dříve ve stejném pořadí kroků úlohy, a předává
se přes sdílená data řešičů. Spuštění *Heat transfer* a poté *Stress analysis*
v jednom pořadí kroků proto dává analýzu tepelného napětí bez dalšího
nastavování. Bez tepelného řešiče zůstává teplota prvku na výchozí hodnotě a
tepelný člen je rozdílem pouze vůči environmentální podmínce.

#### Počáteční podmínky

| Počáteční podmínka | Účinek |
|---|---|
| Displacement | počáteční pole posunutí dynamické přechodové analýzy |
| Structural velocity | počáteční pole rychlosti dynamické přechodové analýzy |
| Pressure | počáteční povrchové tlakové zatížení |

Počáteční podmínky se čtou pouze při prvním výpočtu přechodové analýzy.
Dynamická analýza bez nich začíná z klidu. Vázané směry nemají vlastní
počáteční rychlost, sledují předepsané posunutí.

#### Vazba s řešičem tekutin

Řešič převezme výsledek **Pressure** předchozího řešení a použije ho jako
povrchové zatížení všude tam, kde není přiřazena explicitní tlaková okrajová
podmínka. Krok tekutiny následovaný krokem napětí ve stejném pořadí kroků proto
automaticky přenese tlak tekutiny na konstrukci.

V opačném směru každý nestrukturální řešič před svým během posune síť o
uložený výsledek **Displacement** a poté ji vrátí zpět, takže řešení tekutiny
nebo tepla, které následuje po řešení napětí, vidí deformovaný tvar.

### 1.6 Statická analýza

Při vypnutém řešiči v čase a typu úlohy *Stress analysis* řešič sestaví a
vyřeší

```
K * u = f
```

`K` je symetrická pozitivně definitní, jakmile vazby posunutí odstraní módy
tuhého tělesa, takže se řeší metodou **sdružených gradientů** s Jacobiho
předpodmíněním a velikostí bloku 3.

Je-li model nedostatečně vázaný, matice je singulární a řešení nezkonverguje.
Každý statický model potřebuje dostatek vazeb k potlačení všech šesti módů
tuhého tělesa - tří posuvů a tří rotací.

### 1.7 Přechodová analýza

Při zapnutém řešiči v čase volí *Nastavení přechodové analýzy napětí* jeden ze
dvou typů analýzy.

#### Dynamická analýza

Výchozí volba. Řešič integruje rovnice lineární elastodynamiky

```
M * d2u/dt2 + C * du/dt + K * u = f(t)
```

takže setrvačnost, napěťové vlny, kmitání, dynamické zesílení a rezonance jsou
všechny zachyceny.

**Tlumení.** `C` je Rayleighovo tlumení, sestavované prvek po prvku ze dvou
volitelných materiálových vlastností,

```
C = alpha_M * M + beta_K * K
```

*Rayleigh mass damping coefficient* `alpha_M` `[1/s]` tlumí nízké frekvence,
*Rayleigh stiffness damping coefficient* `beta_K` `[s]` vysoké. Poměrný útlum
módu s kruhovou frekvencí `omega` je

```
zeta(omega) = alpha_M / (2*omega) + beta_K * omega / 2
```

takže cílového poměrného útlumu `zeta` při dvou frekvencích `omega_1` a
`omega_2` se dosáhne s

```
alpha_M = 2*zeta * omega_1*omega_2 / (omega_1 + omega_2)
beta_K  = 2*zeta / (omega_1 + omega_2)
```

Obě frekvence převezměte z modální analýzy, kolem pásma, které vás zajímá.
Typické poměrné útlumy konstrukcí jsou 0.01 až 0.05. Bez těchto dvou vlastností
neexistuje vůbec žádné fyzikální tlumení.

**Časová integrace.** Newmarkova metoda s modifikací Hilber-Hughes-Taylor
(HHT). S posunutím `u`, rychlostí `v` a zrychlením `a` na začátku kroku řeší
každý krok

```
( a0*M + (1+alpha)*(K + a1*C) ) * u_n+1 =
      (1+alpha)*f_n+1 - alpha*f_n
    + M * ( a0*u_n + a2*v_n + a3*a_n )
    + (1+alpha) * C * ( a1*u_n + a4*v_n + a5*a_n )
    + alpha * ( C*v_n + K*u_n )
```

```
a0 = 1/(beta*dt^2)   a1 = gamma/(beta*dt)   a2 = 1/(beta*dt)
a3 = 1/(2*beta) - 1  a4 = gamma/beta - 1    a5 = dt/2 * (gamma/beta - 2)
```

a poté aktualizuje

```
a_n+1 = a0*(u_n+1 - u_n) - a2*v_n - a3*a_n
v_n+1 = v_n + dt*( (1-gamma)*a_n + gamma*a_n+1 )
```

Efektivní matice je symetrická pozitivně definitní, takže se řeší stejnou
metodou CG jako statická analýza.

- S **HHT alpha = 0** (výchozí) je schéma prostým Newmarkovým schématem s
  `gamma = 1/2` a `beta = max(theta/2, 1/4)`, kde `theta` je koeficient
  aproximace časového kroku (`1` zpětná, `0.5` centrální, `0` dopředná).
  Centrální i dopředná dávají schéma průměrného zrychlení - bezpodmínečně
  stabilní, s přesností druhého řádu a bez numerického tlumení. Zpětná dává
  `beta = 1/2`, což je rovněž bezpodmínečně stabilní, ale prodlužuje periody.
- S **HHT alpha** mezi `-1/3` a `0` je `gamma = 1/2 - alpha` a
  `beta = (1 - alpha)^2 / 4`. Schéma zůstává bezpodmínečně stabilní s přesností
  druhého řádu a tlumí vysoké frekvence, které hrubá síť rozlišuje špatně,
  přičemž nízké módy nechává téměř nedotčené. `-0.05` až `-0.1` je běžná volba,
  když je odezva znečištěna vysokofrekvenčním šumem.

**Počáteční zrychlení.** V prvním kroku se zrychlení řeší z rovnováhy,
`M*a = f - K*u - C*v`, takže zatížení působící od začátku vytvoří správné
počáteční zrychlení. Směry bez hmotnosti dostanou nulu.

**Soustředěná hmotnost.** Volba *Soustředěná matice hmotnosti* nahradí
konzistentní hmotnost diagonální se stejnou celkovou hmotností, škálováním
diagonály konzistentní matice (soustředění HRZ, které zůstává kladné i u
kvadratických prvků). Mírně snižuje vlastní frekvence a dává hladší odezvu za
ostrými čely vln.

**Volba časového kroku.** Schéma je stabilní pro jakékoli `dt`, ale přesnost
vyžaduje zhruba 20 a více kroků na periodu nejvyššího módu, který chcete
rozlišit. Pro vlnu šířící se sítí je dobrým začátkem krok blízký době, kterou
vlna potřebuje k průchodu jedním prvkem (`dt ~ h/c`, `c = sqrt(E/rho)`).

**Výsledky.** Kromě posunutí a napětí ukládá dynamická analýza uzlové vektory
*Structural velocity* a *Structural acceleration*. Ty také přenášejí stav do
dalšího časového kroku a přes restart řešiče.

#### Kvazistatická analýza

Každý časový krok je samostatným statickým řešením

```
K * u(t) = f(t)
```

se zanedbanou setrvačností a tlumením. Použijte ji pro průběh zatížení
působící pomalu ve srovnání s periodami konstrukce, kdy záleží jen na posloupnosti
rovnovážných stavů. Nepotřebuje hustotu ani omezení časového kroku.

### 1.8 Modální analýza

Při typu úlohy *Modal analysis* řešič sestaví `K` a `M` a řeší zobecněnou úlohu
vlastních čísel

```
K * phi = lambda * M * phi        with  lambda = omega^2
```

(`with` - kde.) Každá vlastní dvojice je **módem**: `phi` je tvar módu, ukládaný
a zobrazovaný jako pole posunutí, a `lambda` je vlastní číslo, ze kterého plyne
vlastní frekvence jako

```
omega = sqrt(lambda)      [rad/s]
f     = sqrt(lambda) / (2*pi)      [Hz]
```

Řešič tento převod provádí sám: nastavení modální analýzy a 3D zobrazení uvádějí
vlastní frekvenci v Hz, zatímco log řešiče vedle ní vypisuje příslušné vlastní
číslo. Mód, který se iteraci vlastních čísel nepodařilo vyřešit, se hlásí jako
`0` Hz s upozorněním v logu.

K dispozici jsou dvě metody, volené nastavením modální metody:

| Modální metoda | Řešič vlastních čísel | Extrahuje |
|---|---|---|
| Multiple modes | podprostorová iterace s Rayleighovou-Ritzovou projekcí | požadovaný počet módů |
| Only most dominant mode | inverzní mocninná metoda s Rayleighovým podílem | jediný mód |

Obě metody iterují na `K^-1 * M`, takže konvergují k **nejnižším**
frekvencím - které jsou ty podstatné. Každý krok je lineárním řešením s `K` s
použitím nastavení metody sdružených gradientů z nastavení maticového řešiče,
takže cena modální analýzy je zhruba cenou statického řešení vynásobenou počtem
podprostorových vektorů a iterací.

Podprostorová iterace nese blok vektorů, promítne `K` a `M` do podprostoru,
který vektory generují, a výslednou malou plnou úlohu vlastních čísel řeší
přesně. Kromě požadovaného počtu módů se nese několik vektorů navíc, protože
výrazně urychlují konvergenci hledaných módů. Pro `p` požadovaných módů obsahuje
blok `min(2p, p+8)` vektorů, což je obvyklý kompromis mezi rychlostí konvergence
a cenou iterace. Vlastní čísla vycházejí přímo jako `lambda` a jsou seřazena
vzestupně, takže **mód 0 je základní mód**.

**Zde se spotřebuje doba výpočtu.** Jedna podprostorová iterace provede jedno
úplné lineární řešení na každý vektor bloku, takže požadavek na 10 módů stojí
18 řešení na iteraci a požadavek na 100 stojí 108. Iterace se zastaví dříve jen
tehdy, když se **každé** požadované vlastní číslo ustálí v rámci hodnoty
konvergence, a nejvyšší požadovaný mód se ustálí vždy jako poslední - takže
velký počet módů jednak prodražuje každou iteraci, jednak činí předčasné
zastavení nepravděpodobným. Žádejte módy, které se skutečně chystáte prohlížet.

Pokud se požadované módy do poslední iterace neustálí, řešič to v logu uvede a
ohlásí dosaženou změnu, místo aby nezkonvergované hodnoty předložil bez
komentáře. Nejnižší módy takového výpočtu jsou obvykle stále v pořádku;
nejvyšší nikoli.

Matice hmotnosti se pro modální analýzu sestavuje vždy, takže hustota musí být
přítomna na každé entitě, která se analýzy účastní.

**Modální analýza s předpětím.** Před sestavením matic posune modální analýza
každý uzel o výsledek posunutí předchozího řešení a poté síť obnoví. Spuštění
*Stress analysis* a poté *Modal analysis* v jednom pořadí kroků proto dává módy
**zatížené, deformované** konstrukce, nikoli nedeformované. Uložený tvar módu je
vlastní vektor přičtený k tomuto počátečnímu posunutí.

Aktualizuje se pouze geometrie - matice tuhosti je stále lineárně pružná,
vyhodnocená na posunuté síti. Zachycuje to změnu tvaru, nikoli geometrické
zpevnění taženého prvku.

Modální analýza je **výlučná**: nelze ji kombinovat s žádnou jinou fyzikou
kromě předcházející analýzy napětí.

### 1.9 Odvozené výsledky

Po vyřešení se napětí prvků získají z pole posunutí jako průměr přes
integrační body výrazu

```
sigma = D * ( B * u  -  eps_th )
```

s tepelným přetvořením `eps_th = alpha * dT * [1, 1, 1, 0, 0, 0]` pro objemové
prvky. Povrchové prvky jsou v rovinné deformaci: roztažnost mimo jejich rovinu
je potlačena, což zvyšuje tepelné přetvoření v rovině na
`eps_th = (1+v) * alpha * dT * [1, 1, 0]`. Stejné tepelné přetvoření vstupuje do
tepelného zatížení `f_th = INT( B^T * D * eps_th )`. Čárové prvky hlásí
`E * (eps - alpha * dT)`.

Ze šesti složek řešič vytvoří dva invarianty a kvadraticky je zkombinuje do
napětí podle von Misese:

```
normal part  QN = sqrt( sx^2 + sy^2 + sz^2 - (sx*sy + sy*sz + sz*sx) )
shear part   QS = sqrt( 3 * (tyz^2 + txz^2 + txy^2) )
von Mises    QVM = sqrt( QN^2 + QS^2 )
```

(`normal part` - normálová část, `shear part` - smyková část), což je klasická
definice

```
QVM = sqrt( sx^2 + sy^2 + sz^2 - (sx*sy + sy*sz + sz*sx) + 3*(tyz^2 + txz^2 + txy^2) )
```

Pro povrchové prvky se používají dvourozměrné ekvivalenty, `QN` ze dvou
normálových napětí v rovině a `QS = sqrt(3 * tau_xy^2)`. Pro čárové prvky se
hlásí jen osová hodnota a smyková část je nulová.

**Uzlové síly** se získávají prvek po prvku jako vnitřní pružná síla

```
f = K * u - f_th
```

a sčítají se v uzlech. Ve vázaném uzlu je to reakční síla; v nezatíženém volném
uzlu by u zkonvergovaného statického řešení měla vyjít téměř nulová, což z
výsledku Force činí užitečnou kontrolu rezidua. Těleso, které se může volně
tepelně roztahovat, proto neukazuje ani síly, ani napětí.

| Výsledek | Použití na | Význam |
|---|---|---|
| Displacement `[m]` | uzel | vektor posunutí, v modální analýze tvar módu |
| Von Mises stress `[Pa]` | prvek | `sqrt(QN^2 + QS^2)` jako výše |
| Normal stress `[Pa]` | prvek | `QN` |
| Shear stress `[Pa]` | prvek | `QS` |
| Normal stress in X, Y, Z `[Pa]` | prvek | `sigma_xx`, `sigma_yy`, `sigma_zz` |
| Shear stress in YZ, XZ, XY `[Pa]` | prvek | `tau_yz`, `tau_xz`, `tau_xy` |
| Force `[N]` | uzel | získaná uzlová síla, ve vázaných uzlech reakce |

Šest složek napětí se pro objemové prvky hlásí v **globálních** souřadnicích.
Povrchové a čárové prvky vyhodnocují napětí ve své vlastní lokální soustavě
prvku, takže jejich složky se hlásí v této soustavě: povrch vyplňuje
`sigma_xx`, `sigma_yy` a `tau_xy` v rovině a čára vyplňuje pouze osové
`sigma_xx`.

### 1.10 Statická, přechodová, nebo modální?

| | Statická | Dynamická přechodová | Kvazistatická přechodová | Modální |
|---|---|---|---|---|
| Řeší | `K*u = f` | `M*u'' + C*u' + K*u = f(t)` v čase | `K*u = f(t)` v každém čase | `K*phi = omega^2*M*phi` |
| Odpovídá na | „jak moc se prohne a jak vysoké je napětí“ | „jak reaguje na rychle se měnící zatížení“ | „jak sleduje pomalu se měnící zatížení“ | „při jakých frekvencích a v jakém tvaru chce kmitat“ |
| Potřebuje hmotnost | ne | ano | ne | ano |
| Potřebuje vazby proti pohybu tuhého tělesa | ano | ano | ano | ano |
| Záznamy | jeden | jeden na časový krok | jeden na časový krok | jeden na extrahovaný mód |

**Statickou analýzu použijte pro** kontroly pevnosti a tuhosti, meze průhybu,
reakční síly, vlastní tíhu, tlakové nádoby, tepelné napětí. Je to výchozí volba
a pokrývá většinu práce.

**Dynamickou přechodovou analýzu použijte pro** rázy, náhle působící nebo
uvolněná zatížení, průběhy otřesů a vibrací, šíření vln a odezvu na harmonické
buzení v blízkosti rezonance - všude tam, kde záleží na setrvačnosti.

**Kvazistatickou přechodovou analýzu použijte pro** zatížení, které se mění v
čase dostatečně pomalu, aby ho konstrukce sledovala v rovnováze.

**Modální analýzu použijte pro** vlastní frekvence a tvary módů, vyhnutí se
rezonanci se známým buzením, posouzení, zda je konstrukce dynamicky dost tuhá,
a kontrolu nezamýšlených mechanismů - téměř nulové vlastní číslo obvykle
znamená chybějící vazbu nebo odpojenou část.

---

## 2. Grafické uživatelské rozhraní

Vše podstatné se nachází v panelu **Řešič** (záložky `Úloha`,
`Okrajové podmínky`, `Počáteční podmínky`, `Environmentální podmínky`,
`Materiál`, `Výsledky`), v nabídce **Úloha** a v panelu **Model**.

### 2.1 Výběr úlohy

`Úloha` -> `Pořadí kroků úloh` (`Ctrl+P`) otevře dialog pořadí kroků úlohy.
Přidejte krok a vyberte buď:

- **Stress analysis** - „Stress and linear elasto-dynamics analysis including
  modal analysis“ (analýza napětí a lineární elastodynamiky včetně modální
  analýzy);
- **Modal analysis** - „Stress modal analysis“ (modální analýza).

Analýza napětí se volně kombinuje s jinými fyzikami. Dva užitečné řetězce jsou
*Heat transfer* a poté *Stress analysis* pro tepelné napětí a
*Incompressible viscous flow* a poté *Stress analysis* pro tlakové zatížení.

Modální analýza vylučuje všechny ostatní typy úloh kromě předcházející analýzy
napětí, která z ní udělá modální analýzu s předpětím.

### 2.2 Záložka Úloha

**Řešič v čase** - zobrazuje se u analýzy napětí, protože jde o časově závislý
typ úlohy. Pro statickou analýzu ho nechte vypnutý.

| Pole | Význam |
|---|---|
| Zapnutí | zapíná a vypíná přechodové krokování |
| Aproximace | nastavuje Newmarkovo beta dynamické analýzy, viz 1.7; doporučenou volbou je centrální |
| Počáteční čas | první hodnota času v sekundách; při restartu řešiče se ignoruje |
| Koncový čas | `start time + time-step size * number of time-steps`, pouze pro čtení |
| Velikost časového kroku | `dt` v sekundách |
| Počet časových kroků | kolik kroků se má vypočítat |
| Frekvence výstupu | zapsat soubor výsledků každých N kroků; `0` zapíše pouze poslední krok |

**Nastavení přechodové analýzy napětí** - zobrazuje se u analýzy napětí a je
aktivní pouze při zapnutém řešiči v čase.

| Pole | Význam | Výchozí |
|---|---|---|
| Typ analýzy | *Dynamic* (se setrvačností) nebo *Quasi-static* (statické řešení v každém časovém kroku) | Dynamic |
| Numerické tlumení (HHT alpha) | od `-1/3` do `0`; záporné hodnoty tlumí vysokofrekvenční šum | 0 |
| Soustředěná matice hmotnosti | použít diagonální matici hmotnosti místo konzistentní | vypnuto |

Poslední dvě platí pouze pro dynamickou analýzu a jinak jsou zašedlé.

**Nastavení modální analýzy** - zobrazuje se, když je typ úlohy
*Modal analysis*.

| Pole | Význam | Výchozí |
|---|---|---|
| Modální metoda | *Only most dominant mode* nebo *Multiple modes* | Multiple modes |
| Iterace | iterace řešiče vlastních čísel; pro metodu dominantního módu vypnuto | 100 |
| Extrahovat módy | kolik módů extrahovat; pro metodu dominantního módu vypnuto | 10 |
| Hodnota konvergence | práh konvergence řešiče vlastních čísel | 1.0e-9 |

Oba počty jsou nezávislé: iterace zpřesňují celou sadu módů společně,
nevytvářejí je jeden po druhém.

**Extrahovat módy je nastavení, které rozhoduje o délce výpočtu.** Každý mód
přidá do každé iterace jedno lineární řešení a výpočet se může zastavit dříve,
až když se ustálí všechny požadované módy, takže cena roste rychleji než počet.
Deset je rozumný začátek a je výchozí hodnotou; sto je výpočet jiného řádu a na
velké síti může trvat hodiny. Zvyšujte ho, jen když víte, že vyšší módy
potřebujete.

Pokud výpočet trvá déle, než čekáte, log řešiče uvádí podprostorový vektor, na
kterém právě pracuje, takže je vidět postup v rámci iterace, a po každé iteraci
vypisuje dosaženou míru konvergence vedle požadované hodnoty.

### 2.3 Záložka Materiál

Záložka `Materiál` uvádí materiál přiřazený vybrané entitě. Analýza napětí
vyžaduje **všechny čtyři** tyto vlastnosti:

| Vlastnost | Jednotky | Poznámka |
|---|---|---|
| Modulus of elasticity | `Pa` | `E`, asi `2.1e11` pro ocel |
| Poisson ratio | `-` | `v`, asi `0.3` pro ocel; musí zůstat pod `0.5` |
| Density | `kg/m^3` | potřebná pro vlastní tíhu, dynamickou a modální analýzu |
| Thermal expansion coefficient | `1/K` | nastavte na `0`, není-li tepelné zatížení |

Entita, které některá z nich chybí, se neřeší a kontrola nastavení na to
upozorní před spuštěním výpočtu.

Dvě další vlastnosti jsou volitelné a čte je pouze dynamická přechodová
analýza:

| Vlastnost | Jednotky | Poznámka |
|---|---|---|
| Rayleigh mass damping coefficient | `1/s` | `alpha_M`, viz 1.7; chybí-li, je nulový |
| Rayleigh stiffness damping coefficient | `s` | `beta_K`, viz 1.7; chybí-li, je nulový | Každá vlastnost je tabulkou v závislosti na teplotě, takže
teplotně závislý modul se respektuje, pokud nejprve běží tepelný řešič.

### 2.4 Záložka Okrajové podmínky

Záložka je rozdělena na seznam dostupných okrajových podmínek nahoře a editor
vybrané podmínky dole. Nejprve vyberte entitu ve stromu `Model`; správce pak
nabídne podmínky platné pro daný typ entity a vybranou úlohu. Zaškrtnutím ji
přiřadíte a poté upravte její složky ve spodním stromu.

Nabízejí se podmínky z části 1.5. Poznámky k jejich použití:

- **Displacement** zobrazuje u každé ze svých tří složek zaškrtávací políčko.
  Vázány jsou jen zaškrtnuté složky, takže stěnu lze držet v X a nechat volnou v
  Y a Z. Pole hodnoty nezaškrtnuté složky je zašedlé.
- **Normal displacement** a **Roller displacement** přebírají lokální soustavu
  ve výchozím stavu z geometrie - z průměrných normál prvků povrchu, ze směru
  prvků čáry. Na **bodové** entitě není geometrie, kterou by bylo možné sledovat,
  proto se editor *Local direction* zobrazuje a používá vždy.
- Na povrchu nebo čáře zobrazuje strom podmínky zaškrtávací políčko **Use
  entered local direction**. Zaškrtnutím odkryjete editor *Local direction* a
  soustava se převezme ze zadaného směru místo ze sítě. Šipky směru vykreslené ve
  3D zobrazení sledují tu z obou možností, která platí. Pokud se vazba povrchu s
  nezaškrtnutým políčkem chová podivně, zkontrolujte, že jsou normály povrchu
  konzistentní, a synchronizujte je nástroji geometrie.
- Vazby z několika entit, které se setkávají v jednom uzlu, se kombinují, takže
  stěnu lze současně držet v globálním směru podmínkou *Displacement* a nechat ji
  klouzat po nakloněné rovině. Pokud dvě z nich požadují různé hodnoty ve stejném
  směru, výpočet se zastaví s chybou, která uvede uzel.
- **Pressure** a **Force (unit area)** platí pouze pro povrchy.
- Každá složka je tabulkou v závislosti na čase, takže zatížení lze pro
  přechodový výpočet zadat časový průběh.

### 2.5 Počáteční a environmentální podmínky

V záložce `Environmentální podmínky` se zapíná **gravitace**. Přiřaďte
*Gravitational acceleration* entitě, která má nést svou vlastní tíhu - obvykle
celému modelu - a nastavte vektor; výchozí hodnota je `-9.80665` v Z. Přiřaďte
*Temperature* pro nastavení referenční teploty stavu bez napětí pro analýzu
tepelného napětí.

Záložka `Počáteční podmínky` nabízí *Displacement*, *Structural velocity* a
*Pressure*, které se uplatní při prvním výpočtu přechodové analýzy.

### 2.6 Nastavení maticového řešiče

`Úloha` -> `Nastavit maticový řešič úloh` nastavuje iterační řešiče.
Strukturální řešič používá položku **CG**, a to pro statické i přechodové
řešení i pro vnitřní řešení iterace vlastních čísel.

Pokud statické řešení nekonverguje, obvyklou příčinou je nedostatečně vázaný
model, nikoli nastavení řešiče. Než zvýšíte počet iterací, zkontrolujte vazby.

### 2.7 Monitorovací body

`Úloha` -> `Definovat monitorovací body` umístí sondy na zadané souřadnice a
vybere proměnnou, která se má zaznamenávat - obvykle *Displacement* nebo
*Von Mises stress*. V přechodovém výpočtu zobrazí historii
`Zprávy` -> `Monitorovací body`.

### 2.8 Výsledky a záznamy

Záložka `Výsledky` uvádí vypočtené proměnné a ovládá 3D zobrazení. Posunutí je
uzlový vektor; tři míry napětí jsou prvkové skaláry. Dynamická přechodová
analýza přidává uzlové vektory *Structural velocity* a *Structural
acceleration*.

Pro zobrazení deformovaného tvaru zobrazte *Displacement* a v nastavení
zobrazení zapněte škálování deformace - skutečná posunutí jsou obvykle příliš
malá na to, aby byla ve skutečném měřítku vidět.

Záložka `Záznamy` panelu `Model` uvádí záznamy výsledků:

- **statická analýza** - jediný záznam;
- **přechodová analýza** - jeden záznam na zapsaný časový krok;
- **modální analýza** - jeden záznam na extrahovaný mód, číslovaný od
  základního. Procházení záznamů prochází módy a jejich animace animuje tvary
  módů.

U modálního záznamu označí 3D zobrazení záznam číslem módu a vlastní frekvencí
daného módu v Hz.

`Zprávy` -> `Log soubor řešiče` ukazuje celý výstup řešiče včetně vlastního
čísla vypsaného pro každý extrahovaný mód a statistiky posunutí, napětí podle
von Misese a síly po každém záznamu.

---

## 3. Návod - statická analýza napětí

**Cíl.** Vypočítat průhyb a napětí konzoly zatížené vlastní tíhou a zatížením
na konci a odečíst reakci v podpoře.

Návod předpokládá vysíťovaný objemový model - dobře poslouží dlouhý kvádr.
Použijte `Soubor` -> `Nový model`, nakreslete kvádr pomocí `Geometrie` ->
`Vytvořit prvek` nebo importujte geometrii a tetraedrizujte ji; to pokrývá
návod *Nakreslit krychli*. Označte vetknutý konec a zatížený konec jako
samostatné povrchové entity, aby jim bylo možné přiřadit okrajové podmínky.

### Krok 1 - vyberte úlohu

1. `Úloha` -> `Pořadí kroků úloh` (`Ctrl+P`).
2. Přidejte krok a vyberte **Stress analysis**.
3. Potvrďte tlačítkem `OK`.

### Krok 2 - přiřaďte materiál

1. Ve stromu `Model` vyberte objemovou entitu.
2. Otevřete záložku `Materiál` a přiřaďte materiál se všemi čtyřmi
   vlastnostmi, pro konstrukční ocel:
   - **Modulus of elasticity** = `2.1e11` `Pa`
   - **Poisson ratio** = `0.3`
   - **Density** = `7850` `kg/m^3`
   - **Thermal expansion coefficient** = `1.2e-5` `1/K`

### Krok 3 - ponechte analýzu statickou

Otevřete záložku `Úloha` a ve skupině *Řešič v čase* nechte **Zapnutí**
nezaškrtnuté. Řešič pak jednou sestaví a vyřeší `K*u = f`.

### Krok 4 - zadejte vazby modelu

1. Vyberte povrchovou entitu na vetknutém konci.
2. Otevřete záložku `Okrajové podmínky` a zaškrtněte **Displacement**.
3. Ponechte všechny tři složky zaškrtnuté a nastavené na `0`.

Tím se stěna zcela upevní a odstraní se všech šest módů tuhého tělesa.
Zkontrolujte, že nic dalšího nezůstalo volně - odpojená část bez vazby činí
soustavu singulární a řešení nezkonverguje.

Pro podporu, která má klouzat, a ne být zcela upevněná, použijte na povrchu
místo toho **Roller displacement**. Uzamkne pouze směr normály povrchu a uzlu
nechá volný pohyb v tečné rovině.

### Krok 5 - zadejte zatížení

**Vlastní tíha.** Vyberte objemovou entitu, otevřete záložku
`Environmentální podmínky` a zaškrtněte **Gravitational acceleration**.
Ponechte výchozí `-9.80665` v Z.

**Zatížení na konci.** Vyberte povrchovou entitu na volném konci, otevřete
záložku `Okrajové podmínky` a zaškrtněte **Force**. Zadejte celkovou sílu v
newtonech, například `-1000` v Z. Hodnota je celkovou hodnotou na entitu, takže
se při zjemnění sítě nemění.

Alternativy podle toho, čím zatížení fyzikálně je:

- **Pressure** pro rovnoměrný normálový tlak na stěnu, v `Pa`;
- **Force (unit area)** pro směrové povrchové zatížení v `N/m^2`;
- **Weight** pro připojenou hmotnost v `kg`, která není modelována jako
  geometrie.

### Krok 6 - řešte

1. `Řešení` -> `Spustit řešič` (`Ctrl+R`).
2. Kontrola nastavení ohlásí chybějící materiály nebo okrajové podmínky ještě
   před spuštěním výpočtu.
3. Průběh sledujte v `Zprávy` -> `Log soubor řešiče`.

### Krok 7 - prohlédněte si výsledky

- Záložka `Výsledky`: zobrazte **Displacement** pro průhyb a
  **Von Mises stress** pro rozložení napětí. V nastavení zobrazení zapněte
  škálování deformace, aby byl průhyb vidět.
- **Normal stress** a **Shear stress** oddělují dva příspěvky, ze kterých se
  skládá hlášená hodnota podle von Misese.
- **Force** na vetknuté stěně je reakcí. Její součet by měl vyvažovat zadané
  zatížení plus vlastní tíhu - rychlá a účinná kontrola, že je model zatížen tak,
  jak jste zamýšleli.
- Monitorovací bod na *Displacement* na konci dává průhyb jako číslo, a ne jako
  barvu.

### Krok 8 - volitelně tepelné napětí

Chcete-li přidat tepelné zatížení:

1. Přidejte do pořadí kroků úlohy krok **Heat transfer** *před* krok napětí a
   nastavte jeho okrajové podmínky a materiálové vlastnosti.
2. Přiřaďte strukturální entitě environmentální podmínku **Temperature** s
   referenční teplotou bez napětí, například `293.15` K.

Řešič napětí převezme vypočtené teploty prvků z tepelného řešiče a automaticky
přidá člen tepelné roztažnosti.

### Řešení problémů

| Příznak | Příčina |
|---|---|
| Řešení nekonverguje | model je nedostatečně vázaný; nějaká část je volná, nebo podpory neodstraňují všech šest módů tuhého tělesa |
| *Entity has material assigned which is missing required properties* | chybí jedna ze čtyř požadovaných vlastností |
| Posunutí jsou všude nulová | do modelu nedorazilo žádné zatížení - zkontrolujte, že je přiřazena gravitace nebo že složky Force jsou nenulové |
| Posunutí jsou obrovská | modul pružnosti je chybný o řády, nebo má povrchová entita nulovou tloušťku, a tedy žádnou tuhost |
| Zatížení se při zjemnění sítě mění | *Force (unit area)* nebo *Pressure* byly zamýšleny jako celková síla, nebo naopak |
| Tlak tlačí nesprávným směrem | normály povrchu nejsou konzistentní, nebo povrch neohraničuje objem |

---

## 4. Návod - modální analýza

**Cíl.** Extrahovat několik prvních vlastních módů téže konzoly.

### Krok 1 - vyberte úlohu

`Úloha` -> `Pořadí kroků úloh` (`Ctrl+P`), přidejte krok, vyberte
**Modal analysis**.

Modální analýzu nelze kombinovat s jinými fyzikami, s jedinou výjimkou analýzy
napětí umístěné před ní, která z ní udělá modální analýzu s předpětím.

### Krok 2 - přiřaďte materiál

Stejné čtyři vlastnosti jako ve statickém návodu. Na **hustotě** zde záleží víc
než kdekoli jinde: tvoří celou matici hmotnosti a frekvence se škálují jako
`1/sqrt(rho)`.

### Krok 3 - zadejte vazby modelu

Přiřaďte na vetknutém konci stejnou okrajovou podmínku **Displacement**.

Vazby odlišují módy konzoly od módů volně plovoucí tyče. Necháte-li model
nevázaný, prvních šest vlastních čísel bude téměř nulových - módy tuhého
tělesa - a zajímavé módy se posunou dál v seznamu.

### Krok 4 - nastavte modální analýzu

Otevřete záložku `Úloha` a v *Nastavení modální analýzy* nastavte:

| Pole | Hodnota |
|---|---|
| Modální metoda | `Multiple modes` |
| Iterace | `100` |
| Extrahovat módy | `6` |
| Hodnota konvergence | `1.0e-9` |

Šest módů je rozumný začátek a méně než výchozích deset. Každý mód navíc přidá
do každé iterace jedno lineární řešení, takže velký počet módů na velké síti je
pomalý.

Je-li potřeba jen jediný mód, zvolte `Only most dominant mode`; používá
inverzní mocninnou metodu na jediném vektoru a je mnohem levnější.

### Krok 5 - zatížení není potřeba

Modální analýza nemá vektor zatížení - úloha vlastních čísel je homogenní.
Zatížení přiřazená modelu se ignorují s jedinou výjimkou: pokud předtím
proběhla analýza napětí, její výsledek posunutí zdeformuje síť pro modální
sestavení.

### Krok 6 - řešte

`Řešení` -> `Spustit řešič` (`Ctrl+R`). Log hlásí podprostorové iterace a jejich
míru konvergence a poté jeden blok na extrahovaný mód:

```
Extracting mode 6
{
    Eigen-value = ...
    Storing results
    ...
}
```

Módy se zpracovávají od nejvyššího extrahovaného indexu dolů k základnímu, takže
základní mód se počítá jako poslední a ukládá se jako záznam 1.

### Krok 7 - prohlédněte si módy

- Panel `Model`, záložka `Záznamy`: jeden záznam na mód, číslovaný od
  základního. Procházejte je a prohlédněte si tvar každého módu.
- Záložka `Výsledky`: zobrazte **Displacement**. Tvar módu je definován jen až
  na měřítko, takže velikosti jsou libovolné - záleží na tvaru. Zapněte
  škálování deformace a animujte záznam, abyste mód zviditelnili.
- Vlastní číslo každého módu se vypisuje v logu řešiče a zobrazuje se jako
  popisek záznamu ve 3D zobrazení.

### Krok 8 - odečtěte vlastní frekvence

3D zobrazení označí každý modální záznam číslem módu a jeho vlastní frekvencí v
Hz. Log řešiče uvádí stejnou frekvenci pro každý extrahovaný mód spolu s
příslušným vlastním číslem:

```
Eigen-value = 1e+06, frequency = 159.155 [Hz]
```

Mód, který iterace vlastních čísel nedokázala vyřešit, se hlásí jako `0` Hz a
nese upozornění v logu.

Stejně jako u každého výsledku se vyplatí při prvním nastavení modelu ověřit
základní frekvenci ručním výpočtem - část 5 uvádí vzorec pro konzolu.

### Krok 9 - volitelně módy s předpětím

1. V pořadí kroků úlohy umístěte krok **Stress analysis** *před* krok
   **Modal analysis**.
2. Nastavte statická zatížení a vazby jako v návodu 3.

Modální analýza pak běží na deformované síti. Takto lze zjistit, jak statické
zatížení mění tvar, ve kterém konstrukce kmitá.

### Řešení problémů

| Příznak | Příčina |
|---|---|
| První vlastní čísla jsou téměř nulová | model je nedostatečně vázaný nebo je část odpojená - skutečný mechanismus |
| Řešič vlastních čísel nekonverguje | příliš málo iterací, nebo špatně podmíněná matice hmotnosti, protože entita nemá hustotu nebo geometrickou míru |
| Extrakce módů je velmi pomalá | požadováno příliš mnoho módů; každý mód stojí úplné lineární řešení |
| Výpočet trvá hodiny nebo se zdá, že nikdy neskončí | požadováno příliš mnoho módů - každý stojí lineární řešení v každé iteraci a výpočet se nemůže zastavit dříve, dokud se neustálí všechny. Sledujte počet podprostorových vektorů v logu, abyste viděli postup v rámci iterace |
| Frekvence vypadá absurdně | podprostorová iterace nezkonvergovala - log to na konci nezkonvergovaného výpočtu uvádí; extrahujte méně módů, zvyšte počet iterací nebo zkontrolujte hustotu a vazby |
| Tvary módů vypadají stejně | blízko položené módy symetrické konstrukce, nebo příliš málo podprostorových iterací k jejich oddělení |
| Méně záznamů, než bylo požadováno módů | podprostor ztratil nezávislé směry; log uvádí, kolik módů bylo extrahováno, a ukládá jen ty |

---

## 5. Kontrola modelu

Řešič nemá vestavěnou sadu ověřovacích testů, proto se vyplatí nový model před
použitím ověřit vůči něčemu, co lze spočítat ručně. Tři levné kontroly, seřazené
podle užitečnosti:

**Rovnováha reakcí.** Sečtěte výsledek *Force* přes vázané uzly. Musí vyvažovat
zadaná zatížení plus vlastní tíhu `rho * V * g`. Tím odhalíte chybné velikosti
zatížení, chybné jednotky i chybné směry tlaku.

**Průhyb konce konzoly.** Pro nosník délky `L`, kvadratického momentu průřezu
`I` a koncového zatížení `F` dává Eulerova-Bernoulliho teorie

```
delta = F * L^3 / (3 * E * I)
```

Vymodelujte nosník objemovými prvky a porovnejte. Počítejte s tím, že výsledek
metody konečných prvků bude na hrubé síti mírně tužší a se zjemňováním sítě
bude konvergovat zdola. Pro tuto kontrolu nepoužívejte čárové prvky - nemají
žádnou ohybovou tuhost.

**Základní frekvence konzoly.** Pro tentýž nosník s plochou průřezu `A`,

```
f1 = (1.875^2 / (2*pi)) * sqrt( E*I / (rho * A * L^4) )
```

Porovnejte se `sqrt(lambda_0) / (2*pi)` z modální analýzy. Tím se současně ověří
matice hmotnosti, matice tuhosti i váš převod vlastního čísla.

**Náhle zatížená tyč.** Tyč vetknutá na jednom konci a na druhém zatížená osovou
silou `F` zapnutou v čase `t = 0` je kontrolou dynamické analýzy. Napěťová vlna
běží k vetknutému konci a zpět rychlostí `c = sqrt(E/rho)`, takže volný konec se
lineárně posune na dvojnásobek statického průhybu, `2 * F * L / (E * A)`, v čase
`t = 2*L/c` a zpět na nulu v čase `t = 4*L/c`. Bez tlumení se cyklus opakuje. S
Rayleighovým tlumením odezní a tyč se ustálí na statickém průhybu. Použijte
nulové Poissonovo číslo, aby byla tyč skutečně jednorozměrná.

U kterékoli z těchto kontrol zjemněte síť alespoň jednou a potvrďte, že odpověď
konverguje, a ne že se posouvá.

---

## 6. Omezení

### Omezení modelování

- **Pouze lineární pružnost.** Malá přetvoření, malá posunutí, izotropní
  materiál, žádná plasticita, kontakt, velké deformace ani vzpěr. Výsledky mají
  smysl jen tehdy, dokud konstrukce zůstává ve své lineární oblasti.
- **Tlumení je pouze Rayleighovo.** Matice tlumení je `alpha_M*M + beta_K*K` pro
  každý materiál, takže poměrný útlum sleduje křivku z 1.7 a nelze ho nastavit
  mód po módu. Neexistují diskrétní tlumiče, frekvenčně nezávislé (hysterezní)
  tlumení ani okrajová podmínka tlumení.
- **Pouze implicitní časová integrace.** Každý krok je lineárním řešením.
  Neexistuje explicitní schéma centrálních diferencí, takže velmi krátké děje,
  jako je balistický ráz, potřebují mnoho drahých kroků.
- **Povrchové prvky jsou v rovinné deformaci.** Tenká deska zatížená ve své
  rovině bude tužší než idealizace rovinné napjatosti. Záleží-li na rozdílu,
  modelujte tenké konstrukce objemovými prvky.
- **Čárové prvky jsou pruty.** Sestavuje se pouze osová tuhost `E*A`.
  Neexistuje ohyb, smyk ani krut, ať se typ prvku jmenuje jakkoli, takže rám
  modelovaný čárovými prvky se bude chovat jako kloubová příhradová konstrukce.
- **Předpětí je pouze geometrické.** Modální analýza s předpětím znovu vyhodnotí
  lineární tuhost na posunuté síti. Nepřidává člen geometrické tuhosti, takže
  zpevnění napnutého lana ani změkčení tlačené vzpěry se nezachytí.
- **Žádné modální participační faktory ani efektivní hmotnosti.** Řešič hlásí
  pouze vlastní čísla a tvary módů, takže neexistuje přímá informace o tom,
  které módy jsou pro daný směr buzení podstatné.

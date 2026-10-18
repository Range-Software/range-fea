# Generovat čtyřstěnnou síť

Vygeneruje objemovou (čtyřstěnnou) síť.


**Objemová síť** je potřebná pro většinu **typů úloh**.


Generátor 3D sítě dokáže síť vygenerovat, pouze pokud:
1. Model obsahuje uzavřený (vodotěsný) povrch.
2. Prvky povrchu se navzájem neprotínají.


Má-li být kompozitní materiálový model reprezentován několika objemovými sítěmi, musí model obsahovat dělicí povrch.


V některých případech může být potřeba přepsat výchozí nastavení a zadat parametry TetGen přímo.

## Uživatelské rozhraní

![Dialog generování sítě](image-Generate_mesh_dialog.png)

- **Zkontrolovat celistvost povrchu** – užitečné při generování 3D sítě z povrchového modelu. Dokáže odhalit chyby povrchové sítě a možné problémy.
- **Znovu vytvořit síť (zachovat stávající uzly)** – zachová stávající uzly a nové přidá jen v případě potřeby.
- **Ponechat vypočtené výsledky** – všechny vypočtené výsledky budou namapovány/interpolovány na nové uzly a prvky.
- **Kvalitní síť** – vygeneruje 3D síť splňující kritéria kvality.
    - **Maximální objem prvku** – maximální objem každého vygenerovaného prvku. Žádný prvek by neměl mít větší objem, než je zadáno.
    - **Generovat funkci velikosti sítě** – používá se ke zjemnění sítě v místech, kde se výsledky výrazně mění.
- **Parametry TetGen** – parametry předávané generátoru sítí TetGen.

Následující obrázek ukazuje rozdíl v hustotě sítě při použití adaptivního síťování.

![Příklad adaptivní sítě](image-Adaptive_mesh-example.png)

## Syntaxe parametrů

`pYrq_Aa_miO_S_T_XMwcdzfenvgkJBNEFICQVh`


Podtržítka označují, že za některými přepínači může volitelně následovat číslo. Mezi přepínačem a jeho číselným parametrem nesmí být mezera.


Následuje přehled všech přepínačů příkazové řádky s krátkým popisem.

- **p** - Tetraedrizuje po částech lineární komplex (PLC).
- **r** - Rekonstruuje dříve vygenerovanou síť.
- **q** - Zjemní síť (pro zlepšení kvality sítě).
- **R** - Zhrubnutí sítě (pro snížení počtu prvků).
- **a** - Použije omezení maximálního objemu čtyřstěnu.
- **O** - Určuje úroveň optimalizace sítě.
- **S** - Určuje maximální počet přidaných bodů.
- **T** - Nastaví toleranci testu koplanarity (výchozí 10−8).
- **X** - Potlačí použití přesné aritmetiky.
- **M** - Neslučuje koplanární stěny ani velmi blízké vrcholy.
- **w** - Vygeneruje váženou Delaunayovu (regulární) triangulaci.
- **c** - Zachová konvexní obal PLC.
- **d** - Zjistí vzájemné průniky stěn PLC.
- **n** - Vypíše sousedy čtyřstěnů.
- **C** - Zkontroluje konzistenci výsledné sítě.
- **Q** - Tichý režim: žádný výstup na terminál kromě chyb.
- **V** - Podrobný režim: podrobné informace, více výstupu na terminál.

# Rekonstruovat povrch

Tato akce znovu vytrianguluje vybrané povrchy. Sloučí prvky ležící v jedné rovině, odstraní uzly, které neurčují tvar (uzly uvnitř rovinných oblastí a na přímých hranách), a odstraní velmi malé a protáhlé prvky.

Stávající uzly se nikdy neposouvají, nové uzly vytváří pouze volba kvalitní sítě. Uzly a hrany sdílené s jinými entitami (body, čáry, nevybrané povrchy) zůstávají zachovány, uzavřené povrchy zůstávají uzavřené a nevznikají žádná nová samoprotnutí. Zpracovávají se pouze lineární trojúhelníky a objemovou síť je nutné nejprve odstranit.

* **Úhlová tolerance** - největší úhel mezi normálami prvků ležících v jedné rovině a mezi směry hran ležících na jedné přímce.
* **Vzdálenostní tolerance** - největší vzdálenost uzlu od roviny (přímky), aby byl považován za ležící v rovině (na přímce).
* **Minimální délka hrany** - hrany kratší než tato hodnota se odstraní. Detaily menší než tato hodnota mohou zaniknout.
* **Úhel protáhlého prvku** - prvky s nejmenším úhlem pod touto hodnotou jsou považovány za protáhlé.
* **Delaunayova triangulace** - na závěr provede Delaunayovu triangulaci povrchu: hrany, u kterých je součet protilehlých úhlů větší než 180 stupňů, se překlopí, pokud se tvar zachová v rámci vzdálenostní tolerance.
* **Kvalitní síť (vkládání uzlů)** - vloží nové uzly (Delaunayovo zjemnění) tak, aby nejmenší úhel prvků byl alespoň **Úhel kvality** a hrany nebyly delší než **Maximální délka hrany** (nula znamená bez omezení). Nové uzly se umísťují na stávající povrch a na přímé hrany, takže se tvar zachová. Hrany sdílené s nevybranými entitami se nedělí. Zakřivené povrchy složené z jednotlivě skloněných plošek lze zlepšit jen částečně.

Výchozí tolerance jsou relativní k velikosti vybraných povrchů. Větší tolerance vedou k více zjednodušeným povrchům.

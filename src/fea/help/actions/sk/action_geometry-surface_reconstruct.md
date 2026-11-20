# Rekonštruovať plochu

Táto akcia znovu vytrianguluje vybrané plochy. Zlúči koplanárne prvky, odstráni uzly, ktoré neurčujú tvar (uzly vnútri rovinných oblastí a na priamych hranách), a odstráni veľmi malé a pretiahnuté prvky.

Existujúce uzly sa nikdy nepresúvajú, nové uzly vytvára iba voľba kvalitnej siete. Uzly a hrany zdieľané s inými entitami (body, čiary, nevybrané plochy) zostávajú zachované, uzavreté plochy zostávajú uzavreté a nevznikajú žiadne nové samoprieniky. Spracúvajú sa iba lineárne trojuholníky a objemovú sieť je potrebné najprv odstrániť.

* **Uhlová tolerancia** - najväčší uhol medzi normálami koplanárnych prvkov a medzi smermi kolineárnych hrán.
* **Vzdialenostná tolerancia** - najväčšia vzdialenosť uzla od roviny (priamky), aby bol považovaný za koplanárny (kolineárny).
* **Minimálna dĺžka hrany** - hrany kratšie ako táto hodnota sa odstránia. Detaily menšie ako táto hodnota môžu zaniknúť.
* **Uhol pretiahnutého prvku** - prvky s najmenším uhlom pod touto hodnotou sa považujú za pretiahnuté.
* **Delaunayova triangulácia** - na záver vykoná Delaunayovu trianguláciu plochy: hrany, pri ktorých je súčet protiľahlých uhlov väčší ako 180 stupňov, sa preklopia, ak sa tvar zachová v rámci vzdialenostnej tolerancie.
* **Kvalitná sieť (vkladanie uzlov)** - vloží nové uzly (Delaunayovo zjemnenie) tak, aby najmenší uhol prvkov bol aspoň **Uhol kvality** a hrany neboli dlhšie ako **Maximálna dĺžka hrany** (nula znamená bez obmedzenia). Nové uzly sa umiestňujú na existujúcu plochu a na priame hrany, takže sa tvar zachová. Hrany zdieľané s nevybranými entitami sa nedelia. Zakrivené plochy zložené z jednotlivo sklonených plôšok je možné zlepšiť len čiastočne.

Predvolené tolerancie sú relatívne k veľkosti vybraných plôch. Väčšie tolerancie vedú k viac zjednodušeným plochám.

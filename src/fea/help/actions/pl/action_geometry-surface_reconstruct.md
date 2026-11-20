# Odtwórz powierzchnię

Ta akcja ponownie trianguluje wybrane powierzchnie. Scala elementy współpłaszczyznowe, usuwa węzły, które nie określają kształtu (węzły wewnątrz obszarów płaskich i na prostych krawędziach), oraz usuwa bardzo małe i wydłużone elementy.

Istniejące węzły nigdy nie są przesuwane, nowe węzły tworzy tylko opcja siatki wysokiej jakości. Węzły i krawędzie współdzielone z innymi encjami (punkty, linie, niewybrane powierzchnie) są zachowywane, zamknięte powierzchnie pozostają zamknięte i nie powstają nowe samoprzecięcia. Przetwarzane są tylko trójkąty liniowe, a siatkę objętościową należy najpierw usunąć.

* **Tolerancja kątowa** - największy kąt między normalnymi elementów współpłaszczyznowych oraz między kierunkami krawędzi współliniowych.
* **Tolerancja odległości** - największa odległość węzła od płaszczyzny (prostej), przy której jest on uznawany za współpłaszczyznowy (współliniowy).
* **Minimalna długość krawędzi** - krótsze krawędzie są usuwane. Szczegóły mniejsze niż ta wartość mogą zniknąć.
* **Kąt elementu wydłużonego** - elementy o najmniejszym kącie poniżej tej wartości są uznawane za wydłużone.
* **Triangulacja Delaunaya** - na koniec wykonuje triangulację Delaunaya powierzchni: krawędzie, dla których suma przeciwległych kątów przekracza 180 stopni, są odwracane, o ile kształt zostaje zachowany w granicach tolerancji odległości.
* **Siatka wysokiej jakości (wstawianie węzłów)** - wstawia nowe węzły (zagęszczanie Delaunaya) tak, aby najmniejszy kąt elementów wynosił co najmniej **Kąt jakości**, a krawędzie nie były dłuższe niż **Maksymalna długość krawędzi** (zero oznacza brak ograniczenia). Nowe węzły są umieszczane na istniejącej powierzchni i na prostych krawędziach, więc kształt zostaje zachowany. Krawędzie współdzielone z niewybranymi encjami nie są dzielone. Powierzchnie zakrzywione złożone z indywidualnie nachylonych ścianek można poprawić tylko częściowo.

Domyślne tolerancje są względne do rozmiaru wybranych powierzchni. Większe tolerancje dają bardziej uproszczone powierzchnie.

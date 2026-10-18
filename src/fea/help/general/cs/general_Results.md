# Výsledky

**Výsledky** vytváří **řešič** a skládají se z **výsledkových proměnných**.

## Výsledková proměnná

**Výsledková proměnná** je sada hodnot vypočtených **řešičem**. **Výsledková proměnná** je určena svým **typem** (např. **teplota** nebo **posunutí**) a vektorem hodnot. Podle **typu** může **výsledková proměnná** obsahovat vektor skalárních nebo vektorových hodnot.


Například výsledková proměnná **teplota** obsahuje vektor teplot pro každý uzel, kdežto **posunutí** obsahuje vektor vektorů posunutí pro každý uzel.


**Výsledkovou proměnnou** lze počítat buď na úrovni uzlů, nebo na úrovni prvků. Například **posunutí** se počítá na úrovni uzlů, kdežto **napětí** na úrovni prvků. Každou proměnnou vypočtenou na úrovni prvků lze převést na úroveň uzlů. Převod proměnné z úrovně prvků na úroveň uzlů je jednorázová operace a nelze ji vrátit; k tomu je nutné výsledky znovu vypočítat **řešičem**.

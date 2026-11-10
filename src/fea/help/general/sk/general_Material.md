# Materiál

**Materiál** je kolekcia materiálových vlastností relevantných pre daný **typ úlohy**. Hodnoty každej materiálovej vlastnosti môžu byť teplotne závislé.

## Priradenie materiálu

1. Vyberte jednu alebo viac entít v **strome modelu**.
2. Otvorte záložku **Materiál** na pravej strane hlavného okna.
3. Zaškrtnutím políčka pred názvom materiálu ho priradíte vybraným entitám.

![Materiál](image-Material.png)

Materiály, ktoré obsahujú všetky vlastnosti vyžadované vybranými **typmi úloh**, sú uvedené **tučným** písmom, všetky ostatné materiály _kurzívou_.

_Poznámka: Všetky entity modelu musia mať priradený materiál._

## Materiálové vlastnosti

Vlastnosti vybraného materiálu sú uvedené v dolnej časti záložky **Materiál**. Hodnotu možno upraviť priamo v stĺpci **Hodnota**. Tlačidlami **Pridať** a **Odstrániť** vlastnosť pridáte alebo odstránite.

Ak má byť vlastnosť teplotne závislá, rozbaľte ju a kliknutím na **Upraviť teplotne závislé hodnoty** zadajte tabuľku hodnôt pre dané teploty. Vlastnosť s teplotne závislými hodnotami je v stĺpci **Hodnota** zašednutá a možno ju meniť iba v tejto tabuľke.

## Databáza materiálov

Zoznam materiálov je databáza zdieľaná všetkými modelmi. Každý materiál je uložený vo vlastnom súbore, ktorý je uvedený v stĺpci **Súbor**. Tlačidlom **Nový** vytvoríte nový materiál, tlačidlom **Zmazať** zmažete vybraný. Zmeny materiálových vlastností sa ukladajú do databázy.

# Model

**Model** je virtuálna geometrická reprezentácia trojrozmerného objektu. Model sa skladá z geometrických **entít**.


**Entita** je kolekcia prvkov (geometrických simplexov) rovnakého typu a delí sa na tieto typy:

- **Bod** – kolekcia 0D prvkov (uzlov).
- **Čiara** – kolekcia 1D prvkov (úsečiek).
- **Plocha** – kolekcia 2D prvkov (trojuholník, štvoruholník).
- **Objem** – kolekcia 3D prvkov (tetraeder, hexaeder).

Bez ohľadu na to, či sú prvky 0D, 1D, 2D alebo 3D, všetky sú reprezentované v 3D priestore. Napríklad bod (uzol) nemá žiadne rozmery – má iba polohu, zatiaľ čo čiara má jeden rozmer (dĺžku).


Každej **entite** okrem **Objemu** možno priradiť **geometrické** vlastnosti na doplnenie 3D reprezentácie. Sú to:

- **Bod** – objem v kubických metroch.
- **Čiara** – plocha prierezu v štvorcových metroch.
- **Plocha** – hrúbka v metroch.

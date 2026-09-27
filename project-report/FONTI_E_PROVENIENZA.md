# Fonti e provenienza

Questa sezione separa le fonti pubbliche, i dati forniti dall'utente e il repository di sviluppo.

## A. Dati retail forniti dall'utente

- `[Pc Game ITA] Zero Comico.ISO` — 506.378.240 byte.
- `zero.zip` — 503.426.381 byte.
- `zero.zip.001` … `zero.zip.005` — archivio spezzato ricostruito e testato.
- File eseguibili e risorse contenuti nel disco/archivio: `Zero Comico.exe`, `japotek3d.dll`, script, P3D, ANJ, BSP, JGF5, AVI, MP3.

## B. Repository pubblico di reverse engineering/documentazione

Repository principale:

https://github.com/vs-sr-dev/pc-zerocomico-doc

File/documenti usati come fonti tecniche principali:

- README:
  https://github.com/vs-sr-dev/pc-zerocomico-doc/blob/master/README.md
- compressione JFX1/JGF5:
  https://github.com/vs-sr-dev/pc-zerocomico-doc/blob/master/docs/07-two-magics-and-a-1989-algorithm.md
- scripting:
  https://github.com/vs-sr-dev/pc-zerocomico-doc/blob/master/docs/08-a-script-language-and-the-two-people-who-wrote-its-editor.md
- BSP/pathfinding:
  https://github.com/vs-sr-dev/pc-zerocomico-doc/blob/master/docs/09-floor-plans-and-a-two-dimensional-bsp.md
- modelli/animazioni/audio:
  https://github.com/vs-sr-dev/pc-zerocomico-doc/blob/master/docs/10-models-images-and-forty-four-minutes-of-audio.md
- domande aperte:
  https://github.com/vs-sr-dev/pc-zerocomico-doc/blob/master/docs/12-open-questions.md
- strumenti:
  https://github.com/vs-sr-dev/pc-zerocomico-doc/blob/master/docs/14-tools.md

Tool di riferimento:

- LZHUF/JFX/JGF:
  https://github.com/vs-sr-dev/pc-zerocomico-doc/tree/master/tools
- `tools/lzhuf.py`
- `tools/jgf.py`
- `tools/gsc.py`
- `tools/bsp.py`
- `tools/p3d.py`

Output/notebook di riferimento:

- `notes/gsc-grammar.txt`
- `notes/gsc-dialogue.txt`
- `notes/gsc-rooms.txt`
- `notes/gsc-speakers.txt`
- `notes/gsc-comments.txt`
- `notes/gsc-refs.txt`
- `notes/bsp-summary.txt`
- `notes/bsp-geometry.txt`
- `notes/p3d-summary.txt`
- `notes/p3d-verify.txt`
- `notes/str-japotek3d-dll.txt`
- `notes/iso-tree-joliet.txt`

Nota metodologica: il repository pubblico dichiara di essere una descrizione/strumentazione del disco e non un progetto di porting. Nel progetto ScummVM i risultati di formato sono stati usati come riferimento fattuale; il codice C++ del port è stato sviluppato come implementazione propria, evitando di copiare ciecamente codice Python in assenza di una licenza esplicita compatibile.

## C. Gioco gemello / confronto engine

https://github.com/vs-sr-dev/pc-bloodandlace-doc

Usato principalmente come conferma storica/tecnica di:

- stesso engine;
- JGF5;
- ordine BGRA;
- evoluzione dei formati/script.

## D. ScummVM ufficiale

Repository:

https://github.com/scummvm/scummvm

Usato come riferimento per:

- `AdvancedMetaEngineDetection`;
- layout engine;
- API `Common`;
- `Video::AVIDecoder`;
- Indeo 4/5;
- build e `common/libcommon.a`;
- convenzioni CI e C++ correnti.

## E. Repository di sviluppo del port

https://github.com/HGWells2001/scummvm-return-to-zork-reelmagic

Rami:

- https://github.com/HGWells2001/scummvm-return-to-zork-reelmagic/tree/scratch/zerocomico-stage6
- https://github.com/HGWells2001/scummvm-return-to-zork-reelmagic/tree/scratch/zerocomico-stage7
- https://github.com/HGWells2001/scummvm-return-to-zork-reelmagic/tree/scratch/zerocomico-stage8
- https://github.com/HGWells2001/scummvm-return-to-zork-reelmagic/tree/scratch/zerocomico-stage9
- https://github.com/HGWells2001/scummvm-return-to-zork-reelmagic/tree/scratch/zerocomico-stage10
- https://github.com/HGWells2001/scummvm-return-to-zork-reelmagic/tree/scratch/zerocomico-stage11
- https://github.com/HGWells2001/scummvm-return-to-zork-reelmagic/tree/scratch/zerocomico-stage12
- https://github.com/HGWells2001/scummvm-return-to-zork-reelmagic/tree/scratch/zerocomico-stage13

Commit verificati:

| Stage | SHA |
|---|---|
| 6 | `edc9366b5477e50fda76777e46197a9c4f02e22c` |
| 7 | `399a15229ce89ef43323f4f3be44d9139bcaff2e` |
| 8 | `a3e3a492e44f9026f82f08de187ab52571bb53cb` |
| 9 | `4b06386484cf51d2cee8a3d05c6c3cbff0819842` |
| 10 | `ae54e5eeddee7cb9943065bb7a7c7a4e0a4ecd87` |
| 11 | `9459b306f88632a0db778bfc90a732b0ea812b73` |
| 12 | `a01cc4d5ab97b87fc2bb4e9b8d29306db48b13ae` |
| 13 | `d16834d41e632dc585610a964538987ed106665c` |

CI verdi:

| Stage | Run |
|---|---:|
| 6 | 36295778988 |
| 7 | 36297322705 |
| 8 | 36298147262 |
| 9 | 36299649160 |
| 10 | 36300908171 |
| 11 | 36301237750 |
| 12 | 36301712871 |
| 13 | 36306957502 |

## F. Regola di interpretazione delle fonti

Le quantità provenienti da `pc-zerocomico-doc` sono considerate misure riproducibili del disco.

Le affermazioni su comportamento runtime introdotte dal port sono indicate come implementazioni/test del progetto.

Le questioni ancora prive di prova diretta sono lasciate esplicitamente nella sezione “Limiti e lavoro ancora aperto”.

# Relazione tecnica completa — Porting di *Zero Comico* in ScummVM

**Stato della relazione:** 27 settembre 2026  
**Versione coperta:** fino a Stage 13 incluso  
**Repository di sviluppo:** `HGWells2001/scummvm-return-to-zork-reelmagic`  
**Ramo tecnico più avanzato:** `scratch/zerocomico-stage13`  
**Commit Stage 13 verificato:** `d16834d41e632dc585610a964538987ed106665c`  
**Stima informale di avanzamento del motore:** circa **82%**. Questa percentuale è una misura progettuale indicativa, non una metrica automatica di coverage o compatibilità.

---

## 1. Scopo del progetto

L'obiettivo è creare un motore ScummVM capace di eseguire la versione italiana retail di **Zero Comico** (Windows, 2001) usando i dati originali del gioco, senza riscrivere a mano scene, puzzle o dialoghi.

L'approccio adottato è stato progressivo e data-driven:

1. identificazione e verifica dei contenitori proprietari;
2. decodifica delle risorse;
3. ricostruzione dei formati 3D e animazione;
4. interpretazione degli script originali;
5. ricostruzione del frontend/menu;
6. ingresso nel primo MainPlace di gameplay;
7. navigazione, hotspot e puzzle;
8. runtime del protagonista;
9. dialoghi;
10. integrazione di rendering, animazioni e SequenceTable.

L'obiettivo tecnico non è una semplice “conversione grafica”, ma la ricostruzione del comportamento dell'engine **Lucifer / JapoTek 3D** dentro l'architettura ScummVM.

---

## 2. Materiale originale analizzato

### 2.1 Immagine disco

File fornito:

- `[Pc Game ITA] Zero Comico.ISO`
- dimensione: **506.378.240 byte**

Questa dimensione coincide con l'ISO studiata nel repository pubblico di documentazione `vs-sr-dev/pc-zerocomico-doc`.

Il corpus pubblico indica inoltre:

- 2.862 file sul disco;
- 502.623.549 byte di contenuto;
- stesso contenuto di gioco tra ISO WinISO e copia del CD originale;
- mastering originale datato 8 ottobre 2001;
- eseguibile retail `Zero Comico.exe` versione 0.8.0.0;
- engine basato su `japotek3d.dll` e “Lucifer 3D Interface”.

### 2.2 Archivio ricostruito

È stato fornito anche:

- `zero.zip`, circa **503.426.381 byte**

e successivamente l'archivio spezzato:

- `zero.zip.001` — 104.857.600 byte
- `zero.zip.002` — 104.857.600 byte
- `zero.zip.003` — 104.857.600 byte
- `zero.zip.004` — 104.857.600 byte
- `zero.zip.005` — 83.802.989 byte

I cinque segmenti sono stati ricomposti e verificati. Il test ZIP non aveva rilevato errori e l'archivio risultava contenere **2.959 file**.

---

## 3. Metodo di lavoro e livelli di evidenza

Durante lo sviluppo sono state mantenute tre categorie separate.

### 3.1 Dato dimostrato sui file retail

Rientrano qui elementi verificati direttamente su ISO/archivio o sul corpus pubblico riproducibile, per esempio:

- magic `JFX1` e `JGF5`;
- compressione LZHUF;
- struttura del BSP;
- quantità e tipi dei record P3D/ANJ;
- speaker/dialoghi;
- nomi e path retail;
- keyword degli script;
- token della SequenceTable presenti nella DLL.

### 3.2 Implementazione verificata

È codice C++ che:

- compila contro gli header ScummVM correnti;
- passa test mirati;
- quando possibile è stato confrontato byte per byte o sull'intero corpus.

### 3.3 Frontiera ancora non dimostrata

Questi aspetti sono esplicitamente lasciati aperti invece di essere simulati:

- semantica completa della SequenceTable `.seq`;
- mappatura finale BSP 2D → assi mondo P3D;
- ruolo definitivo della matrice 3×3 interna di alcune mesh P3D;
- ownership delle mesh shared/deformer;
- semantica completa dei branch delle scelte dialogiche;
- mapping audio speech → eventi di dialogo;
- proiezione camera definitiva per il renderer runtime corrente.

Questo principio ha evitato scorciatoie che avrebbero prodotto un port apparentemente funzionante ma tecnicamente falso.

---

# 4. Reverse engineering dei formati

## 4.1 JFX1 e LZHUF

Il gioco usa `JFX1` per script, modelli, animazioni e altri dati.

Il formato contiene:

- magic `JFX1`;
- lunghezza decodificata;
- lunghezza compressa;
- stream LZHUF.

La decompressione è basata sul classico algoritmo LZHUF di Haruhiko Okumura:

- ring buffer LZSS da 4096 byte;
- adaptive Huffman;
- parametri compatibili con l'implementazione storica del 1989.

Nel primo stage il decoder C++ è stato confrontato byte per byte con i dati reali. Esempi verificati:

| Risorsa | Packed | Decoded | Esito |
|---|---:|---:|---|
| `room.isc` | 491 | 831 | identico |
| piccola JGF5 | 35 | 16 | identico |
| `c511.p3d` | 116.017 | 262.473 | identico |
| `Giovanni.anj` | 59.659 | 277.664 | identico |
| `CD.tga` | 812.228 | 1.920.000 | identico |

Il corpus pubblico misura complessivamente **2.188 file JFX1/JGF5**, 72.816.045 byte compressi che diventano 214.388.887 byte.

## 4.2 JGF5

I 1.433 file con estensione `.tga` del gioco non sono TGA standard.

Sono contenitori `JGF5`:

- larghezza e altezza;
- 32 bit per pixel;
- dati **BGRA**;
- payload LZHUF.

È stato implementato un loader tipizzato JGF5 e il colore BGRA è stato mantenuto coerente con la correzione documentata dal gioco gemello *Blood & Lace*.

## 4.3 BSP e pathfinding

I file `.bsp` non sono BSP di rendering 3D.

Sono mappe 2D del pavimento calpestabile con:

- contorno stanza;
- eventuali buchi;
- punti e spigoli;
- celle convesse;
- portali;
- albero BSP preorder;
- grafo pesato per pathfinding;
- sezione support.

Il corpus pubblico riporta:

- 82 BSP totali;
- 80 non vuoti;
- 1.948 celle convesse;
- 3.034 portali;
- 3.483 nodi del grafo;
- 53.518 archi.

Il parser C++ introdotto durante lo sviluppo ha verificato **82/82 BSP**.

Per `Mp1`, `r11_Map00.bsp` contiene 40 nodi di grafo e 630 archi.

Il pathfinding Stage 7 usa Dijkstra sui pesi originali.

## 4.4 Script Lucifer

Il gioco contiene 204 file script in sette estensioni:

| Estensione | File | Funzione prevalente |
|---|---:|---|
| `.isc` | 36 | personaggi, stanze, puzzle, dialoghi |
| `.shp` | 15 | shape/map shape |
| `.mat` | 104 | materiali |
| `.par` | 28 | particelle |
| `.gsc` | 13 | camera, eventi, sistema |
| `.scr` | 5 | camera script |
| `.seq` | 3 | sequenze JACS |

Totale documentato:

- 852.701 byte decodificati;
- 40.958 linee;
- 920 keyword iniziali distinte.

Costrutti già riconosciuti o eseguiti nel port:

- `Variable`
- `mov`
- `if_e`
- `else`
- `endif`
- `begin_thread`
- `end_thread`
- `hide`
- `unhide`
- `e3d_hide`
- `e3d_unhide`
- `setfocus`
- `ifobjselected`
- `play_cut`
- `wait_cut`
- `loop_cut`
- `if_cutisfinished`
- `start_dialog`
- `wait_last_dialog`
- `ChangeMainplace`

Gli opcode sconosciuti non vengono ignorati: la VM si blocca sulla stessa istruzione e la espone come frontiera da implementare.

## 4.5 Oggetti interattivi

Il corpus misura **469 Object**.

Campi e flag già portati:

- `entity`
- `polygon`
- `range`
- `oprange`
- `size`
- `roomscope`
- `examine_text`
- `PICKABLE`
- `EXAMINABLE`
- `OPERATED`
- `ENABLED`
- presenza di handler `examine` / `operate`

La pipeline gameplay collega la mesh selezionata all'Object retail tramite `entity:`.

---

# 5. P3D / ANJ — ricostruzione del 3D

## 5.1 Corpus

La copia retail contiene:

- **263 file P3D**
- **263 file ANJ**

Il corpus pubblico chiude correttamente **526/526 file** e 31.436 record strutturali.

## 5.2 Record principali identificati

Durante il reverse engineering e il confronto con `japotek3d.dll` sono stati identificati:

- `F000` — materiale;
- `F001` — camera;
- `F002` — luce;
- `F003` — mesh / binding mesh;
- `F007` — timeline animazione;
- `F032` — nodo trasformazione;
- `F044` — gruppo/container di animazione;
- trailer `00 ED FF FF`.

## 5.3 Camera F001

Sono stati verificati **255 record camera**.

Struttura stabilita:

- Source: 3 float;
- Target: 3 float;
- Roll;
- FOV;
- marker opzionale range `0xF0F01234`;
- near/far range.

Nel menu `Int_Camera01` la timeline porta il FOV al valore operativo di circa 15°.

## 5.4 Materiali F000

Sono stati verificati **4.585 materiali** nel corpus completo.

Il materiale contiene il riferimento alla risorsa grafica, per esempio:

- `int_MAIN → MAIN.TGA`
- `int_INTERF → INTERF.TGA`
- `int_A_HELP → HELP_ACC.TGA`

Questo ha permesso il collegamento automatico:

`triangolo → gruppo materiale → materiale → texture JGF5`.

## 5.5 Mesh F003

Sono stati classificati **12.304 record F003**:

- 2.735 mesh classiche;
- 856 mesh con vertici condivisi;
- 8.713 blocchi deformazione.

Per le mesh classiche sono stati decodificati:

- vertici;
- indici triangoli;
- gruppi materiale;
- UV;
- associazione materiale/texture.

Le UV del ramo classico usano tre coppie `(u,v)` per triangolo.

## 5.6 ANJ e F044

Il parser ANJ ricorsivo apre i gruppi F044 e separa:

- binding oggetto;
- trasformazioni;
- camera;
- target camera;
- luci;
- timeline.

Nel menu `interfaccia.anj`:

- 298 record;
- 34 gruppi F044;
- 34 binding;
- 230 timeline.

Le chiavi di trasformazione TCB sono state ricostruite come:

- traslazione;
- scala;
- rotazione axis-angle.

Un esempio verificato nel menu produce una rotazione iniziale di circa 120°.

La risoluzione per nome oggetto ha risolto **261/263** coppie P3D/ANJ. I due file residui condividono lo stesso caso speciale `0x0E3D`.

---

# 6. Ricostruzione del menu originale

## 6.1 Bootstrap Mp0

Lo script originale `Mp0/gameplay/room.isc` esegue:

- setup volume;
- disabilitazione interfaccia/3D;
- `play_CD_film Data/Intro.avi`;
- `wait_last_film`;
- riattivazione;
- `mov if_MenuIface 1`;
- ingresso in `Room Interfaccia`.

Il port riproduce questo percorso invece di saltare direttamente a un menu artificiale.

## 6.2 Video intro

È stato collegato il decoder AVI di ScummVM con supporto Indeo 4/5.

Il gioco retail usa sette AVI Indeo 5 320×240 a 25 fps.

## 6.3 Rendering del menu

Nello Stage 5 è stato prodotto un frame reale del menu usando:

- P3D;
- ANJ;
- camera originale;
- materiali;
- texture JGF5;
- z-buffer;
- UV prospettiche;
- visibilità script.

Il primo frame completo del menu usava:

- FOV 15;
- 13 mesh visibili;
- 408 triangoli.

L'anteprima `MENU_PREVIEW.png` è stata generata dai dati 3D, non da uno screenshot del gioco.

---

# 7. Stage-by-stage

## Stage 1 — risorse e scheletro engine

Risultati principali:

- detection iniziale;
- scheletro `engines/zerocomico/`;
- JFX1/LZHUF C++;
- JGF5;
- parser P3D iniziale;
- smoke test su script e modelli reali.

Avanzamento stimato all'epoca: 8–10%.

Artefatto storico: `ZeroComico_ScummVM_Stage1.zip`.

## Stage 2 — BSP, intro e record P3D

Risultati:

- terminatore P3D corretto `00 ED FF FF`;
- parser BSP/pathfinding C++;
- 82/82 BSP;
- integrazione AVI/Indeo;
- approfondimento F044 e loader DLL.

Avanzamento stimato: 14–16%.

Artefatto: `ZeroComico_ScummVM_Stage2.zip`.

## Stage 3 — VM menu, camera, mesh e materiali

Risultati:

- bootstrap Mp0 eseguito via VM;
- `if_MenuIface`;
- camera F001;
- mesh F003 classiche;
- UV;
- materiale F000;
- texture JGF5;
- 263/263 P3D verificati;
- 2.735/2.735 mesh classiche.

Avanzamento stimato: ~23%.

Artefatto: `ZeroComico_ScummVM_Stage3.zip`.

## Stage 4 — tutte le famiglie F003 e ANJ

Risultati:

- 12.304 F003 classificati;
- 2.735 classici;
- 856 shared;
- 8.713 deformer;
- ANJ con TCB;
- 263/263 P3D;
- 261/263 ANJ completamente risolti per nome.

Avanzamento stimato: ~34%.

Artefatto: `ZeroComico_ScummVM_Stage4.zip`.

## Stage 5 — primo rendering reale

Risultati:

- scene registry;
- trasformazioni P3D+ANJ;
- rasterizzazione menu;
- texture originali;
- z-buffer;
- picking;
- anteprima 640×480 del frontend.

Avanzamento stimato: ~45%.

Artefatti:

- `ZeroComico_ScummVM_Stage5.zip`
- `MENU_PREVIEW.png`

## Stage 6 — menu interattivo e cambio MainPlace

Branch:
`scratch/zerocomico-stage6`

Commit:
`edc9366b5477e50fda76777e46197a9c4f02e22c`

CI:
run `36295778988` — verde.

Risultati:

- picking pixel-accurato;
- hover materiali;
- animazioni pulsanti;
- opcode cutscene;
- `ChangeMainplace Mp1`;
- transizione transazionale;
- self-test del cambio MainPlace.

Avanzamento stimato: ~58%.

## Stage 7 — Mp1, BSP e Object

Branch:
`scratch/zerocomico-stage7`

Commit:
`399a15229ce89ef43323f4f3be44d9139bcaff2e`

CI:
run `36297322705` — verde.

Risultati:

- parser BSP runtime;
- Dijkstra;
- `PathFollower`;
- loader start room;
- registry Object;
- mesh → Object;
- avvicinamento a `oprange`;
- dispatch `operate/examine`.

Avanzamento stimato: ~65%.

## Stage 8 — runtime gameplay e prima VM puzzle

Branch:
`scratch/zerocomico-stage8`

Commit:
`a3e3a492e44f9026f82f08de187ab52571bb53cb`

CI:
run `36298147262` — verde.

Risultati:

- Giovanni come risorsa condivisa Mpx;
- `GameplayVariables`;
- `ObjectHandlerRegistry`;
- `ObjectHandlerVM`;
- condizioni e assegnazioni;
- dialog yield;
- movimento BSP → handler;
- nessuna velocità retail inventata.

Avanzamento stimato: ~69%.

## Stage 9 — actor model completo

Branch:
`scratch/zerocomico-stage9`

Commit:
`4b06386484cf51d2cee8a3d05c6c3cbff0819842`

CI:
run `36299649160` — verde.

File principali:

- `resource_decoder.*`
- `p3d_model.*`
- `anj_document.*`
- `anj_tracks.*`
- `actor_model.*`
- `actor_animation_runtime.*`
- `actor_textures.*`
- `actor_render_catalog.*`

Risultati:

- JFX1/JGF5 consolidati nel ramo GitHub;
- ActorModel Giovanni;
- ANJ/TBC runtime;
- texture attore;
- catalogo render mesh classiche.

## Stage 10 — dialoghi

Branch:
`scratch/zerocomico-stage10`

Commit:
`ae54e5eeddee7cb9943065bb7a7c7a4e0a4ecd87`

CI:
run `36300908171` — verde.

Risultati:

- parser `dialog.isc`;
- speaker;
- colore;
- typing speed;
- battute;
- opzioni;
- runtime dialogo;
- collegamento `start_dialog / wait_last_dialog`.

Il corpus documenta:

- 150 Dialog block;
- 698 battute;
- 164 righe di scelta;
- 49.163 caratteri di testo.

Per Mp1:

- 29 dialoghi;
- 123 battute;
- 12 scelte;
- 9.108 caratteri.

L'audio speech è lasciato separato perché non esiste prova di una corrispondenza semplice 1:1 tra battute e MP3.

## Stage 11 — integrazione gameplay + attore + dialoghi

Branch:
`scratch/zerocomico-stage11`

Commit:
`9459b306f88632a0db778bfc90a732b0ea812b73`

CI:
run `36301237750` — verde.

Risultati:

- `ActorMotionController`;
- mapping Idle/Walk iniettato;
- nessun nome clip hardcodedato;
- `IntegratedGameplayHost`;
- `GameplaySession`;
- sincronizzazione:
  gameplay/path → motion state → ANJ.

Avanzamento stimato: ~78%.

## Stage 12 — frame render-ready di Giovanni

Branch:
`scratch/zerocomico-stage12`

Commit:
`a01cc4d5ab97b87fc2bb4e9b8d29306db48b13ae`

CI:
run `36301712871` — verde.

Risultati:

- `ActorRenderFrameBuilder`;
- triangoli world-space;
- trasformazione ANJ;
- actor root transform;
- UV/materiale/texture;
- visibility;
- backend-neutral `ActorTriangleSink`;
- `GameplayActorRenderer`.

Self-test numerico:

- scala;
- quaternion;
- rotazione 90°;
- traslazione mesh;
- traslazione root;
- UV;
- visibilità;
- rigetto UV malformate.

## Stage 13 — SequenceTable JACS

Branch:
`scratch/zerocomico-stage13`

Commit:
`d16834d41e632dc585610a964538987ed106665c`

CI:
run `36306957502` — verde.

Risultati:

- `SequenceTableForensicParser`;
- loader retail `.seq` via JFX1/LZHUF;
- riconoscimento token provati dalla DLL:
  `fromseq`, `start`, `blending`, `table`, `endseq`, `end_seq`;
- classi di transizione `0>1`, `1>1`, `1>0`;
- conservazione linee non riconosciute;
- correlazione esatta identificatori SequenceTable ↔ clip ANJ;
- esclusione dei match per semplice substring.

Avanzamento stimato complessivo: ~82%.

---

# 8. Architettura corrente

La pipeline implementata è ormai separata in livelli.

```
Disc / ZIP retail
     │
     ├── JFX1 → LZHUF → script / P3D / ANJ / SEQ
     └── JGF5 → BGRA texture
              │
              ▼
        Resource / Model layer
              │
     ┌────────┼─────────┐
     ▼        ▼         ▼
   P3D       ANJ       BSP
 mesh       tracks    path graph
     │        │         │
     └───┬────┘         │
         ▼              ▼
   ActorModel       GameplayRuntime
         │              │
         ├──────┬───────┤
         ▼      ▼       ▼
      Motion  Object   Dialogue
      ANJ     VM       Runtime
         │      │       │
         └──────┴───┬───┘
                    ▼
              GameplaySession
                    │
                    ▼
             ActorRenderFrame
                    │
                    ▼
             Triangle backend
```

Il design consente di cambiare backend grafico senza toccare puzzle, dialoghi o navigazione.

---

# 9. Stato del gameplay Mp1

Il percorso implementato è:

```
Menu
 → Nuovo
 → ChangeMainplace Mp1
 → room.isc
 → StartPlace
 → Room iniziale
 → BSP
 → click pavimento / oggetto
 → Dijkstra
 → Giovanni cammina
 → raggiunge oprange
 → operate/examine
 → ObjectHandlerVM
 → variabili / hide / cut / dialogo
 → wait
 → continuazione script
```

Il renderer runtime finale della stanza deve ancora consumare il frame di Giovanni Stage 12 con la convenzione camera definitiva.

---

# 10. Test e CI

Gli Stage GitHub hanno CI indipendenti.

| Stage | Branch | Commit | Run verde |
|---|---|---|---:|
| 6 | `scratch/zerocomico-stage6` | `edc9366b...` | 36295778988 |
| 7 | `scratch/zerocomico-stage7` | `399a1522...` | 36297322705 |
| 8 | `scratch/zerocomico-stage8` | `a3e3a492...` | 36298147262 |
| 9 | `scratch/zerocomico-stage9` | `4b063864...` | 36299649160 |
| 10 | `scratch/zerocomico-stage10` | `ae54e5ee...` | 36300908171 |
| 11 | `scratch/zerocomico-stage11` | `9459b306...` | 36301237750 |
| 12 | `scratch/zerocomico-stage12` | `a01cc4d5...` | 36301712871 |
| 13 | `scratch/zerocomico-stage13` | `d16834d4...` | 36306957502 |

Le CI usano:

- ScummVM corrente;
- `-std=c++17`;
- `-Wall -Wextra -Werror` per il codice Zero Comico;
- build della vera `common/libcommon.a` di ScummVM per i test eseguibili recenti;
- self-test dedicati;
- produzione di artifact ZIP.

---

# 11. Problemi risolti più significativi

## Compressione proprietaria

Risolta e verificata byte per byte.

## “TGA” non standard

Riconosciuti come JGF5 e convertiti BGRA.

## BSP interpretato erroneamente come rendering tree

Corretto: è navigazione 2D.

## F044 inizialmente ambiguo

Riconosciuto come container/gruppo con lunghezza e usato nel parsing ANJ.

## Camera e mesh

Camera F001 e mesh F003 portate da blob opachi a strutture C++ tipizzate.

## UV

Il blocco F003 `0x20` è stato identificato come tre UV per triangolo.

## Materiali

F000 collega direttamente la mesh alla risorsa texture.

## Animazione

TCB traslazione/scala/rotazione axis-angle ricostruite.

## Menu ruotato

La prima prova Stage 5 ha rivelato una base camera errata di 90°. La correzione ha prodotto il menu orientato correttamente.

## Scene registry

Ha eliminato falsi conflitti materiale/mesh con lo stesso nome e ha risolto quasi tutto il corpus ANJ per nome.

## Interazione

Picking 3D e Object script ora condividono lo stesso identificatore della scena.

## VM

Gli opcode mancanti bloccano esplicitamente l'esecuzione.

## SequenceTable

Invece di inventare nomi “Walk/Idle”, Stage 13 legge il file JACS e correla gli identificatori alle clip ANJ.

---

# 12. Limiti e lavoro ancora aperto

## 12.1 SequenceTable completa

Serve ricostruire la semantica di:

- stati;
- start/stop;
- blending;
- table;
- transizioni 0→1, 1→1, 1→0;
- mapping finale a `walk`, `standby`, `special_standby`, ecc.

Stage 13 fornisce già il loader e il livello forense.

## 12.2 Renderer runtime

Stage 12 emette triangoli world-space pronti.

Manca ancora dimostrare e fissare:

- handedness P3D;
- proiezione camera runtime definitiva;
- relazione BSP x/y con assi 3D;
- matrice 3×3 P3D;
- mesh shared/deformer.

## 12.3 Caso `0x0E3D`

Due ANJ del corpus (`c476`, `c478`) condividono un record luce speciale ancora non interpretato semanticamente.

## 12.4 Dialoghi con scelte

Le opzioni vengono riconosciute e presentate.

La semantica dei branch successivi deve ancora essere ricostruita senza scegliere arbitrariamente.

## 12.5 Speech audio

558 MP3 sono presenti nel gioco, ma il mapping completo tra eventi dialogici e MP3 non è ancora provato.

## 12.6 Portali/cambio stanza/camera

L'infrastruttura Room esiste, ma la gestione completa di passaggi tra stanze e camera switching deve essere chiusa.

## 12.7 Inventario

Mancano ancora:

- UI completa;
- combine;
- take animation;
- semantica completa degli oggetti combinabili.

## 12.8 Minigiochi e combattimenti

Dovranno essere affrontati dopo aver chiuso il loop adventure base.

---

# 13. Prossima roadmap tecnica

La sequenza consigliata è:

1. estrarre e censire il vero `Giovanni.seq`;
2. promuovere solo le forme di riga provate a record semantici;
3. derivare mapping `standby/walk/stop` → clip ANJ;
4. alimentare `ActorMotionController`;
5. collegare `ActorRenderFrame` al rasterizzatore/renderer ScummVM;
6. dimostrare BSP → mondo 3D;
7. rendere Giovanni visibile e in movimento in Room1_1;
8. chiudere portali/camere;
9. espandere VM opcode-driven;
10. completare dialoghi/inventario;
11. test progressivi dei puzzle Mp1;
12. ripetere su Mp2–Mp5;
13. save/load;
14. detection e packaging engine ScummVM definitivo.

---

# 14. Conclusione

Il progetto non è più una semplice esplorazione del formato.

Alla fine dello Stage 13 sono disponibili:

- decoder proprietari;
- parser 3D;
- parser animazioni;
- materiali e texture;
- menu ricostruito;
- picking;
- transizione a Mp1;
- BSP/pathfinding;
- Object runtime;
- VM puzzle;
- Giovanni come ActorModel;
- runtime ANJ;
- dialoghi;
- frame render-ready;
- prima analisi strutturata della SequenceTable.

Il blocco principale non è più “capire come sono fatti i file”, ma completare le poche convenzioni runtime rimaste e collegare in modo definitivo i sottosistemi già implementati.

Il risultato raggiunto è una base tecnica coerente e testata per trasformare *Zero Comico* in un vero engine ScummVM data-driven.

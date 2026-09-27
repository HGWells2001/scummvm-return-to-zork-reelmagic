# Inventario dei file e artefatti creati

## 1. Artefatti storici prodotti nella conversazione

- `ZeroComico_ScummVM_Stage1.zip`
- `ZeroComico_ScummVM_Stage2.zip`
- `ZeroComico_ScummVM_Stage3.zip`
- `ZeroComico_ScummVM_Stage4.zip`
- `ZeroComico_ScummVM_Stage5.zip`
- `MENU_PREVIEW.png`
- `ZeroComico_ScummVM_Stage7_GitHubArtifact.zip`
- `ZeroComico_ScummVM_Stage8_GitHubArtifact.zip`
- `ZeroComico_ScummVM_Stage10_GitHubArtifact.zip`
- `ZeroComico_ScummVM_Stage11_GitHubArtifact.zip`
- `ZeroComico_ScummVM_Stage12_GitHubArtifact.zip`
- `ZeroComico_ScummVM_Stage13_GitHubArtifact.zip`

Nota: Stage 6 e Stage 9 sono presenti e versionati come rami GitHub; il pacchetto Stage 13 contiene cumulativamente il codice Stage 6–13.

## 2. File Stage 6

- `zerocomico-stage6/README.md`
- `animation_player.cpp/.h`
- `mainplace.cpp/.h`
- `mainplace_transition.cpp/.h`
- `menu_controller.cpp/.h`
- `menu_highlight.cpp/.h`
- `menu_input.cpp/.h`
- `menu_material_state.cpp/.h`
- `scene_picker.cpp/.h`
- `scene_runtime.cpp/.h`
- `script_bridge.cpp/.h`
- `script_opcodes.cpp/.h`
- `timeline_eval.cpp/.h`
- `stage6_selftest.cpp`

## 3. File Stage 7

- `zerocomico-stage7/README.md`
- `bsp_navigation.cpp/.h`
- `gameplay_interaction.cpp/.h`
- `gameplay_mainplace.cpp/.h`
- `gameplay_object.cpp/.h`
- `gameplay_room.cpp/.h`
- `path_follower.cpp/.h`
- `stage7_selftest.cpp`

## 4. File Stage 8

- `zerocomico-stage8/README.md`
- `gameplay_runtime.cpp/.h`
- `gameplay_variables.cpp/.h`
- `object_handler_vm.cpp/.h`
- `object_handlers.cpp/.h`
- `shared_actor.cpp/.h`
- `stage8_selftest.cpp`

## 5. File Stage 9

- `zerocomico-stage9/README.md`
- `resource_decoder.cpp/.h`
- `p3d_model.cpp/.h`
- `anj_document.cpp/.h`
- `anj_tracks.cpp/.h`
- `actor_model.cpp/.h`
- `actor_animation_runtime.cpp/.h`
- `actor_textures.cpp/.h`
- `actor_render_catalog.cpp/.h`
- `stage9_selftest.cpp`

## 6. File Stage 10

- `zerocomico-stage10/README.md`
- `dialogue_document.cpp/.h`
- `dialogue_mainplace_loader.cpp/.h`
- `dialogue_runtime.cpp/.h`
- `gameplay_dialogue_service.cpp/.h`
- `stage10_selftest.cpp`

## 7. File Stage 11

- `zerocomico-stage11/README.md`
- `actor_motion_controller.cpp/.h`
- `gameplay_session.cpp/.h`
- `integrated_gameplay_host.cpp/.h`
- `stage11_selftest.cpp`

## 8. File Stage 12

- `zerocomico-stage12/README.md`
- `actor_render_frame.cpp/.h`
- `actor_render_submitter.cpp/.h`
- `gameplay_actor_renderer.cpp/.h`
- `stage12_selftest.cpp`

## 9. File Stage 13

- `zerocomico-stage13/README.md`
- `sequence_table.cpp/.h`
- `actor_sequence_table.cpp/.h`
- `stage13_selftest.cpp`

## 10. Workflow CI

Nel repository sono stati aggiunti workflow dedicati:

- `.github/workflows/zerocomico-stage6.yml`
- `.github/workflows/zerocomico-stage7.yml`
- `.github/workflows/zerocomico-stage8.yml`
- `.github/workflows/zerocomico-stage9.yml`
- `.github/workflows/zerocomico-stage10.yml`
- `.github/workflows/zerocomico-stage11.yml`
- `.github/workflows/zerocomico-stage12.yml`
- `.github/workflows/zerocomico-stage13.yml`

## 11. Contenuto del nuovo ZIP relazione

Il nuovo ZIP contiene:

- `project-report/RELAZIONE_COMPLETA.md`
- `project-report/FONTI_E_PROVENIENZA.md`
- `project-report/INVENTARIO_FILE.md`
- `project-report/README.md`
- tutto il codice corrente di `zerocomico-stage6` … `zerocomico-stage13`;
- i workflow CI Stage 6–13.

Gli artefatti storici Stage 1–5 non vengono duplicati fisicamente nel nuovo ZIP, ma sono elencati qui e restano disponibili nella conversazione. Le funzionalità ancora rilevanti di quei primi stage sono state consolidate nei moduli successivi.

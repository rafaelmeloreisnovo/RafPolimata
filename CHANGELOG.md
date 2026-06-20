# Changelog

## v1.0.0 — 2026-06-20

Release de maturidade completa: todos os métodos implementados, todas as estratégias
evidenciadas, checklist 96/96 verificado.

### Adicionado

- **56 métodos M001–M056** implementados, todos compilando e host-runnable com selftest
  integrado (`scripts/raf_baseline_measure.sh`).
- **40 estratégias S01–S40** evidenciadas em `RAF_CHECKLIST_96_ITEMS.md`.
- Artefatos arquiteturais novos: `docs/arch/` (diagramas de fluxo do pipeline).
- Pacotes educacionais e industriais: `packages/educational/`, `packages/industrial/`.
- Script de medição de baseline: `scripts/raf_baseline_measure.sh`.
- Notas de release: `RELEASE_NOTES.md` v1.0.0.

### PRs mergeados

- **#80** — Reconciliação do checklist RAF_CHECKLIST_96_ITEMS.md.
- **#83** — 56 métodos M (M001–M056) + estratégias S16–S28.
- **#84** — Checklist 96/96 completo: todas as lacunas fechadas.

### Invariantes confirmadas

- Zero `malloc`/`calloc`/`free` em `Apkc/` (freestanding preservado).
- 56/56 métodos com `.text` < 4096 bytes (medido via `raf_binary_size_test.sh`).
- CRC-32 header AVR = `0x5A9F075B` (verificado em `ci/reports/baseline_measurements.txt`).

---

## v0.1.0-apkc-termux-arm32-proof — 2026-06-14

Primeira release funcional provada do ApkC.

### Adicionado

- Documento de release: `releases/v0.1.0-apkc-termux-arm32-proof.md`.
- Prova mínima Termux/Android ARM32 em `docs/APKC_TERMUX_ARM32_PROOF.md`.
- Documento de comandos, flags e limites em `docs/APKC_FLAGS_LIMITS_AND_COMMANDS.md`.

### Provado

- `apkc.c` compilado no Termux/Android ARM32 com `-nostartfiles -Wl,-e,_start`.
- `hello.apk` gerado pelo ApkC.
- ZIP/APK parseável por `unzip`.
- Manifest AXML parseável por `aapt dump xmltree`.
- `classes.dex` com SHA-1 interno válido.
- `lib/armeabi-v7a/libhello.so` ELF32 ARM parseável por `readelf`.
- APK assinado por `apksigner` com v1/v2/v3 true.
- Instalação confirmada por `cmd package list packages | grep com.rafael.teste`.
- Abertura manual observada no aparelho.

### Claim permitido

```text
ApkC gera no Termux/Android ARM32 um APK mínimo assinado, instalável e abrível no Android, contendo Manifest AXML válido, classes.dex com SHA-1 correto e libhello.so ARM32 válida com símbolos NativeActivity.
```

### Claim bloqueado

- Produção Play Store.
- Compatibilidade ampla de Android versions/devices.
- ARM64 instalado/rodando.
- Execução útil interna da NativeActivity medida via logcat.
- `-nostdlib` puro no Termux ARM32 sem helpers internos.

### Referência estável

Branch de release criada:

```text
release/v0.1.0-apkc-termux-arm32-proof
```

Commit de corte:

```text
9990e28aef96c4548a7f07376335f3d3be1fc15d
```

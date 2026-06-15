# Changelog

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

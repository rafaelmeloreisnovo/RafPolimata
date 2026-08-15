# PHASE C→D — Truthful Device Gates Receipt — 2026-08-15

## Base observada

- repository: `rafaelmeloreisnovo/RafPolimata`
- base: `main@10fcaa47286eec1c2e701714788862841c1d142e`
- branch: `hardening/phase-cd-truthful-device-gates-20260815`
- contract: `CAPABILITY != ARTIFACT != EXECUTION != EVIDENCE != CLAIM`
- `claim_allowed=false`

## Problema material

O roadmap Phase C→D descrevia o ambiente como praticamente pronto para device testing, mas o gate mestre ainda promovia capacidade estrutural para PASS em vários pontos:

1. L1 exigia `Apkc/apkc`, embora o caminho canônico atual use transformação/hardening de `Apkc/apkc.c` antes de compilação.
2. L2 promovia a mera existência do comando `adb` a capacidade de device testing, sem exigir `adb get-state == device`.
3. L3 validava um header ELF sintético criado no próprio script, não um artefato produzido a partir do ApkC hardened.
4. L4 promovia a existência de `validate_dex_pipeline.sh` a PASS sem exigir `javac`, `d8/dx`, `.class` e `classes.dex` observados.
5. L5 e L9 não participavam do master verifier.
6. L8 promovia presença do header de type system a “formalização”, embora implementação/teste não equivalha a prova formal.
7. L10 aceitava `jarsigner OR zipalign` como proxy de auditoria de assinatura; `zipalign` não verifica assinatura.
8. O validador L4 podia terminar `PASS` mesmo sem compilador DEX, pois indisponibilidade era tratada como warning.

## Delta aplicado

### `tools/phase2_android_preflight.sh`

Novo probe read-only que registra:

- `clang`, `python3`, `readelf`;
- `javac`;
- `d8/dx` por PATH ou `ANDROID_SDK_ROOT/ANDROID_HOME`;
- `adb` e estado real do device;
- `apksigner` e `zipalign`;
- Android local/Termux quando observável;
- estado final `READY_DEVICE_EVIDENCE_RUN`, `TOKEN_VAZIO_TOOLCHAIN` ou `TOKEN_VAZIO_DEVICE`.

A ausência de recurso externo é preservada como `TOKEN_VAZIO`, não como falha lógica nem como PASS.

### `tools/verify_all_gaps.sh`

Reescrito para três classes de resultado:

- `PASS`: propriedade realmente observada no escopo do gate;
- `FAIL`: falsificação/erro intrínseco com os pré-requisitos disponíveis;
- `TOKEN_VAZIO`: dependência, artefato ou evidência ainda ausente.

Mudanças principais:

- L1: gera e verifica source-cap hardened em arquivo temporário e grava SHA-256 + git head;
- L2: exige device adb real ou ambiente Android local;
- L3: executa `tools/raf_apkc_runtime_hardening_proof.sh` para compilar/inspecionar AArch64+ARM32 reais;
- L4: exige `javac`, compilador DEX e DEX realmente gerado;
- L5: explicitamente `TOKEN_VAZIO_IMPLEMENTATION` quando não existe falsificador FFI dedicado;
- L6: escopo reduzido a reprodutibilidade do hardened runtime já comprovada pelo receipt L3;
- L7: existência de benchmark não é promovida a benchmark real;
- L8: type-system implementado != prova formal;
- L9: implementação T^7 != teorema de convergência;
- L10: exige `apksigner verify` sobre APK real informado por `RAF_APK_UNDER_TEST`.

### `tools/validate_dex_pipeline.sh`

Contrato corrigido:

- exit `0`: DEX realmente gerado e header validado;
- exit `1`: falsificação com toolchain disponível;
- exit `2`: `TOKEN_VAZIO_TOOLCHAIN`;
- Java exige `javac`; Kotlin exige `kotlinc`;
- `d8/dx` é localizado por PATH ou Android SDK;
- removida a noção incorreta de “DEX 64-bit”; o gate valida o header/versionamento DEX real.

## Estado epistemológico após o corte

O repositório fica melhor preparado para Phase D porque o gate não confunde mais “temos script” com “temos evidência”.

Esperado em CI sem device físico:

- L1/L3/L4/framework podem chegar a PASS se a toolchain estiver disponível;
- L2 permanece `TOKEN_VAZIO_DEVICE`;
- L5 permanece `TOKEN_VAZIO_IMPLEMENTATION` até existir harness FFI;
- L7 permanece `TOKEN_VAZIO_REAL_WORKLOAD` até benchmark real;
- L8/L9 permanecem `TOKEN_VAZIO_FORMAL_PROOF`;
- L10 permanece `TOKEN_VAZIO_ARTIFACT` ou `TOKEN_VAZIO_TOOLCHAIN` até APK real + `apksigner`.

## Próximo gate verificável

1. CI do PR deve provar que os novos scripts não introduzem falsificação intrínseca.
2. Em Android/Termux ou host com device USB autorizado, executar `bash tools/phase2_android_preflight.sh`.
3. Executar `bash tools/verify_all_gaps.sh`.
4. Para L10, fornecer `RAF_APK_UNDER_TEST=/caminho/app.apk` e repetir L10.
5. Somente depois promover L2/L4/L10 de `TOKEN_VAZIO` para evidência observada.

`merge != device verified`; `framework != runtime`; `claim_allowed=false`.

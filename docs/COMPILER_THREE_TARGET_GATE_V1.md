# RAFAELIA — Gate de Compilador para Três Alvos — V1

**Data:** 2026-07-26  
**Autoridade técnica:** `rafaelmeloreisnovo/RafPolimata`  
**Estado:** `IMPLEMENTED_IN_BRANCH / EXECUTION_RECEIPTS_PENDING / claim_allowed=false`

## 1. Regra operacional

Nenhum aplicativo é instalado ou iniciado por este gate.

A liberação de execução futura só pode ocorrer depois que pelo menos um alvo de
aplicativo completo produzir:

```text
compiler core PASS
→ source checkout clean and pinned by commit
→ offline target preflight PASS
→ target build exit 0
→ APK/AAB observed
→ SHA-256 recorded
→ ZIP/Android structure PASS
→ immutable receipt written
```

Este contrato separa claramente:

- núcleo do compilador;
- APK nativo de bootstrap;
- aplicativo Android completo;
- instalação;
- runtime no aparelho.

`bootstrap != full application != installed application != observed runtime`.

## 2. Compilador-raiz

Antes de qualquer alvo, o orquestrador executa:

```sh
make -C research/APKC_RMR_RESEARCH_CORE verify
make -C research/APKC_RMR_RESEARCH_CORE test
make -C research/APKC_RMR_RESEARCH_CORE coupled-build
```

Essa cadeia verifica os artefatos selados, comentários normativos, licenças,
stubs, raiz SHA-256, geração do header e compilação freestanding.

## 3. Alvos

| Target | Nível | Resultado esperado |
|---|---|---|
| `rafpolimata-coupled-core` | `COMPILER_CORE` | objetos/receipt do núcleo selado |
| `rafpolimata-apkc-native` | `NATIVE_BOOTSTRAP_APK` | APK NativeActivity dual-ABI |
| `rafgittools-apkc-native` | `NATIVE_BOOTSTRAP_APK` | bootstrap RafGitTools; não é Compose/JGit |
| `rafgittools-full-dev-debug` | `FULL_ANDROID_APPLICATION` | APK completo `assembleDevDebug` |
| `vectras-full-debug` | `FULL_ANDROID_APPLICATION` | APK completo `:app:assembleDebug` |
| `vectras-release-unsigned` | `FULL_ANDROID_APPLICATION` | release interna unsigned |

Os alvos completos usam `--offline`. O orquestrador não baixa Gradle, SDK,
NDK, Maven, dependências ou repositórios.

## 4. Layout local

```text
work/
  RafPolimata/
  RafGitTools/
  Vectras-VM-Android/
```

Caminhos diferentes podem ser definidos:

```sh
export RAFGITTOOLS_ROOT=/caminho/RafGitTools
export VECTRAS_ROOT=/caminho/Vectras-VM-Android
```

Para os alvos completos, `ANDROID_SDK_ROOT` ou `ANDROID_HOME` deve apontar para
um SDK já materializado. O Vectras mantém seu contrato de JDK 17/21, SDK 35,
Build Tools 35.0.0, NDK 27.2.12479018 e CMake 3.22.1; o próprio wrapper do
repositório faz a verificação final.

## 5. Uso

Somente plano:

```sh
python3 scripts/compiler_orchestrator.py \
  --target vectras-full-debug \
  --mode plan
```

Preflight sem compilar:

```sh
python3 scripts/compiler_orchestrator.py \
  --target rafgittools-full-dev-debug \
  --mode preflight
```

Compilar sem instalar ou iniciar:

```sh
python3 scripts/compiler_orchestrator.py \
  --target rafpolimata-apkc-native \
  --mode build \
  --abi both
```

```sh
python3 scripts/compiler_orchestrator.py \
  --target vectras-full-debug \
  --mode build
```

Cada tentativa produz:

```text
build/compiler-gate/<target>/
  compiler-receipt.json
  logs/
```

## 6. Receipt

O receipt registra:

- commit e estado limpo/sujo do RafPolimata;
- commit e estado do repositório-alvo;
- hash do registry de targets;
- comandos exatos;
- toolchain observada;
- variáveis de ambiente exigidas, sem registrar segredos;
- exit code e hash dos logs;
- caminhos, tamanhos e SHA-256 dos artefatos;
- BLAKE3 somente quando `b3sum` local está disponível;
- validação estrutural de APK/AAB;
- `install_executed=false`;
- `launch_executed=false`;
- `claim_allowed=false`.

## 7. Estrutura Android mínima validada

Para APK:

- container ZIP válido;
- CRC integral;
- `AndroidManifest.xml`;
- `classes.dex`;
- ao menos uma biblioteca `lib/<abi>/*.so`.

Para AAB:

- container ZIP válido;
- manifesto de módulo;
- pelo menos um DEX.

A existência de um ZIP não basta.

## 8. Fronteira BLAKE3

O compilador não depende de Gemini, de sessão web ou de acesso remoto ao
repositório BLAKE3.

```text
SHA-256 = obrigatório
BLAKE3  = selo adicional local
```

Se `b3sum` não estiver materializado localmente:

```text
blake3.state = TOKEN_VAZIO_B3SUM_NOT_AVAILABLE
```

Isso não transforma o build em sucesso ou falha falsa; apenas mantém a lacuna
auditável. A procedência do BLAKE3/RMR continua governada no repositório próprio.

## 9. Bloqueios deliberados

O registry rejeita comandos de:

- instalação;
- `adb`;
- launch/start de Activity;
- testes conectados que iniciem aplicativo;
- download ou clone automático.

Worktree suja também bloqueia por padrão. Uma exceção exige
`--allow-dirty` explícito e aparece no receipt.

## 10. Critério “compilador pronto”

O compilador pode ser chamado de pronto para iniciar a próxima fase quando:

```yaml
compiler_core: PASS
unit_tests: PASS
target_preflight: PASS
target_build: PASS
artifact_count: ">=1"
android_structure: PASS
sha256: PRESENT
source_commit: PRESENT
dirty_worktree: false
receipt: PRESENT
install_executed: false
launch_executed: false
claim_allowed: false
```

Para cumprir a regra do autor, a primeira promoção operacional deve ser um dos:

1. `vectras-full-debug`;
2. `rafgittools-full-dev-debug`;
3. `rafpolimata-apkc-native`.

Os outros aplicativos permanecem fora de execução até esse receipt existir.

## 11. Estado atual

```yaml
compiler_contract: IMPLEMENTED_IN_BRANCH
rafpolimata_coupled_core: MERGED_STRUCTURE_REMOTE_EXECUTION_TOKEN_VAZIO_ZERO_STEPS
rafpolimata_native_apk_route: IMPLEMENTED_RECEIPT_PENDING
rafgittools_native_bootstrap_route: MERGED
rafgittools_full_target: REGISTERED_RECEIPT_PENDING
vectras_full_target: REGISTERED_RECEIPT_PENDING
device_install: FORBIDDEN_IN_THIS_GATE
device_runtime: TOKEN_VAZIO
claim_allowed: false
```

## 12. Retroalimentação

**F_ok:** um único compilador-raiz agora conhece os três repositórios e diferencia
bootstrap, app completo e runtime.

**F_gap:** ainda falta um receipt real de build completo ou do APK nativo
RafPolimata no ambiente local autorizado.

**F_next:** executar primeiro o target disponível com toolchain já materializada,
sem instalar; revisar o receipt e somente então liberar a fase de aparelho.

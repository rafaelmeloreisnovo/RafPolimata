# C04 — ApkC Structural Closure

**Stack base:** `c02/runtime-truth-receipt-2026-07-26@55b057e22633c4e43814b40d6684315d1911ff50`  
**Estado inicial:** `IMPLEMENTED / EXECUTION_PENDING`  
**Claim global:** `claim_allowed=false`

## Finalidade

Consolidar em um receipt único a prova estrutural do ApkC:

- source-to-binary reproduzível para AArch64 e ARM32;
- vínculo com o commit exato;
- APK como ZIP íntegro;
- DEX com magic, SHA-1 interno, Adler-32, header, map e limites válidos;
- ELF32 ARM e ELF64 AArch64 estruturalmente válidos;
- presença simultânea de `armeabi-v7a` e `arm64-v8a`.

O receipt não transforma estrutura válida em prova de assinatura, instalação, lançamento ou runtime.

## Correção da fronteira de claim

O validador estrutural histórico usa `claim_allowed=true` quando seus bytes passam. No C04, esse resultado é encapsulado com escopo explícito:

```yaml
claim_allowed: false
structural_claim_allowed: true | false
claim_scope: APK_ZIP_DEX_ELF_AND_DUAL_ABI_STRUCTURE_ONLY
```

`structural_claim_allowed=true` significa apenas que o artifact observado passou pelos gates estruturais declarados.

## Entradas obrigatórias

```text
Apkc/proofs/out/hello.apk
Apkc/proofs/out/apkc-compile.status.json
results/apkc-first-part-gate.json
results/apkc-runtime-preflight.json
```

### Source-to-binary

O documento deve usar `raf.apkc.source-to-binary-proof.v2`, apontar para o mesmo commit da execução e registrar, para AArch64 e ARM32:

- `build=PASS`;
- `identity=PASS`;
- `reproducibility=PASS`;
- SHA-256 válido.

### First-part gate

O documento deve usar `raf.apkc-first-part-gate.v1`, retornar `PASS`, preservar `claim_allowed=false` e não conter contradições.

### Runtime preflight

O preflight usa `raf.apkc-runtime-preflight.v1`. Ele pode retornar `BLOCKED` sem invalidar uma prova puramente estrutural, porém o receipt deve registrar:

- quantidade de blockers;
- blockers críticos;
- `source_contract_safe=false`;
- runtime não promovido.

## Estados

- `PASS_STRUCTURAL`: todos os gates estruturais e de cadeia de custódia passaram;
- `INCOMPLETE`: artifact obrigatório ausente ou prova ainda vazia;
- `FAIL`: contradição, commit divergente, prova de build inválida, ZIP/DEX/ELF inválido ou ABI ausente.

## Execução

```sh
python3 scripts/apkc_structural_closure_receipt.py \
  --apk Apkc/proofs/out/hello.apk \
  --source-proof Apkc/proofs/out/apkc-compile.status.json \
  --first-part-gate results/apkc-first-part-gate.json \
  --runtime-preflight results/apkc-runtime-preflight.json \
  --source-commit "$(git rev-parse HEAD)" \
  --output results/apkc-structural-closure.json
```

## Testes

`tests/test_apkc_structural_closure_receipt.py` cobre:

- APK dual ABI + DEX + proof coerente → `PASS_STRUCTURAL` limitado;
- APK ausente → `INCOMPLETE`;
- commit divergente → `FAIL`;
- ARM32 ausente → `FAIL`;
- first-part gate com contradição → `FAIL`.

O fixture também comprova que `PASS_STRUCTURAL` pode coexistir com `runtime_preflight=BLOCKED`, mantendo `source_contract_safe=false`.

## Fronteira de runtime

Mesmo após `PASS_STRUCTURAL`:

```yaml
apk_signed: TOKEN_VAZIO
apk_installed: TOKEN_VAZIO
package_launched: TOKEN_VAZIO
nativeactivity_logcat: TOKEN_VAZIO
device_arm32: TOKEN_VAZIO
device_arm64: TOKEN_VAZIO
performance_claim: FORBIDDEN_OUT_OF_SCOPE
claim_allowed: false
```

## Falsificadores

- source proof em commit diferente;
- build, identidade ou reprodutibilidade não-PASS;
- first-part gate falho ou contraditório;
- ZIP inválido;
- DEX com SHA-1/Adler-32 incorreto;
- ELF com machine/layout incorreto;
- ausência de uma ABI;
- promoção de estrutura para assinatura, instalação ou runtime.

## Gate de saída

O C04 só pode ser marcado `PASS_STRUCTURAL` após observar `results/apkc-structural-closure.json` produzido por execução concreta e vinculado ao artifact APK do mesmo commit. Até isso ocorrer, o estado permanece `IMPLEMENTED / TOKEN_VAZIO_EXECUTION_NOT_OBSERVED`.
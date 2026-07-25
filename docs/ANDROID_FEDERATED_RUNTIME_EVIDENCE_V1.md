# RafPolimata — compilador de evidência Android federada v1

O RafPolimata recebe um **recibo**, não uma narrativa de sucesso.

## Escada epistemológica

```text
DISPATCHED
→ PARTIAL

exit_code = 0
→ TESTED

exit_code = 0 + guest_boot_artifact_sha256
→ VERIFIED_LIMITED
```

Nenhum desses estados autoriza uma alegação ampla de produção, desempenho ou
compatibilidade universal.

## Uso

```bash
python3 scripts/compile_android_runtime_evidence.py \
  tests/fixtures/android-runtime-receipt.dispatched.json
python3 -m unittest tests.test_android_runtime_evidence
```

Resultado local limitado:

```yaml
unit_tests: 4/4 PASS
fixture_state: PARTIAL
fixture_receipt_sha256: d4b345021ebfad7b06780de40fcb725b7af48b993f32944c191b16ff4ffd9812
physical_receipt: TOKEN_VAZIO
claim_allowed: false
```

## Invariante

```text
dispatch != execução
execução != guest boot
guest boot único != replicação independente
```

## R3

```text
F_ok   = compilador, digest, estados e testes negativos
F_gap  = recibo real ARM32/ARM64
F_next = receber o primeiro receipt do Termux/Vectras
```

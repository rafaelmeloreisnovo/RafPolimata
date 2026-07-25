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

O compilador exige hashes de entrada e saída, status, efeitos observados,
safe state e R3. Recibos incompletos ou que exponham caminhos privados são
bloqueados.

## Uso

```bash
python3 scripts/compile_android_runtime_evidence.py \
  tests/fixtures/android-runtime-receipt.dispatched.json
python3 -m unittest tests.test_android_runtime_evidence
```

Resultado local limitado:

```yaml
unit_tests: 5/5 PASS
fixture_state: PARTIAL
fixture_receipt_sha256: 11ef4b014a19eabb210e1909a1f5cf63d68748edba73e2bb7aad36c3632736a8
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
F_ok   = compilador, envelope completo, digest, estados e testes negativos
F_gap  = recibo real ARM32/ARM64
F_next = receber o primeiro receipt do Termux/Vectras
```

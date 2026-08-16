# CI Recovery — TOKEN_VAZIO proof closure — 2026-08-16

Base: `fd33e6efda03f3f6642b10508104892d551f0690`

## Falha observada

A CI `31925681579` falhou em `Validate TOKEN_VAZIO gates (Hotfix H1)` após 15/15 testes unitários do validador passarem. O scan `--strict` reportou 17 referências novas sem closure: 9 em `Apkc/proofs/runs/*` e 8 em `docs/TRABALHO_COMPLETADO_2026-08-16.md`.

## Correção na fonte

`tools/raf_clean_proof_run.sh` passa a vincular cada ausência gerada à closure aplicável:

- `CLOSURE_L1`: proveniência/toolchain/hash ausente;
- `CLOSURE_L2`: execução/runtime Android ausente;
- `CLOSURE_L3`: validação ARM64 ausente.

A mudança preserva `TOKEN_VAZIO`; nenhum estado ausente é convertido em PASS.

## Reconciliação documental

`docs/APKC_VALUE_AND_GAPS.md` foi atualizado para não tratar L1 source→binary como pendente depois da rodada limpa de 2026-08-16. L2 runtime e o empacotamento ARM64 continuam pendentes e vinculados às closures correspondentes.

## Invariantes

- Receipts históricos não foram apagados nem reescritos.
- `TOKEN_VAZIO != PASS`.
- Runtime Android físico permanece fora do escopo desta correção.
- ARM64 estrutural não é promovido a runtime/device proof.
- A CI do commit da mudança é a autoridade para o fechamento desta regressão.

## Estado

`claim_allowed=false` para runtime/device até receipt físico do mesmo artefato/commit.

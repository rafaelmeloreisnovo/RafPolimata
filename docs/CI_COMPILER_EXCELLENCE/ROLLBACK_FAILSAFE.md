# Rollback, failsafe e mitigação

> **Entrada canônica:** docs/AGENTES.md §5 (pipeline operacional — ROLLBACK como estado explícito no ciclo VOID → VALIDATED) e §7 (CI gates — exit não-zero para falha real, TOKEN_VAZIO para ausência de ferramenta).

## Failsafe

| Evento | Ação | Exit |
|---|---|---:|
| Violação de heap/import proibido | registrar `FAIL` e parar | não-zero |
| Compilação/verificação básica falha | registrar `FAIL` e parar | não-zero |
| Ferramenta externa ausente | registrar `TOKEN_VAZIO` | zero quando não é gate obrigatório |
| Device Android ausente | registrar `TOKEN_VAZIO` | zero quando não é gate obrigatório |

## Rollback

- Não sobrescrever código-fonte do ApkC durante validação.
- Manter artefatos gerados em `Apkc/proofs/out/`.
- Não versionar APK, `.so`, keystore ou binários locais.
- Se um gate novo quebrar CI por ambiente incompleto, converter para `TOKEN_VAZIO` somente quando for ausência de ferramenta, nunca quando for erro real do código.

## Mitigação

- Criar arquivo de relatório para cada gap.
- Preferir checagens pequenas e determinísticas.
- Separar prova técnica de plano de build, assinatura, instalação e runtime.

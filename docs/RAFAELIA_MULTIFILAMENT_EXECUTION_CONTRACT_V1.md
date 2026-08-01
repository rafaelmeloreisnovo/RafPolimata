# RAFAELIA — Contrato de Execução Multifilamento V1

**Estado:** `CANONICAL_DRAFT`  
**Política:** `claim_allowed=false`  
**Escopo:** integração RafPolimata ↔ Mapa ↔ Vectras-VM-Android  
**Autoridade:** `RAFAELIA — Implementação Latentes e Papers — Drive GitHub V1`

## 1. Função

Este contrato posiciona o RafPolimata como camada de transformação formal do ecossistema RAFAELIA. Ele recebe fontes e contratos identificados, produz IR, ELF, APK/DEX/ZIP ou relatórios e entrega somente artefatos acompanhados por proveniência, teste e receipt.

```text
source
  -> identity
  -> parser / semantic normalization
  -> IR
  -> lowering
  -> artifact
  -> validator
  -> receipt
  -> runtime handoff
```

Documentação, código, compilação, execução e evidência permanecem estados distintos.

## 2. Topologia estrutural

| filamento | entrada | transformação RafPolimata | saída | falha segura |
|---|---|---|---|---|
| `F_SOURCE` | arquivo/revisão/commit | identidade e normalização | source descriptor | `TOKEN_VAZIO_SOURCE` |
| `F_BUILD` | toolchain, flags, contrato | compile/lower/link/package | artefato + log | `TOKEN_VAZIO_BUILD` |
| `F_RUNTIME_HANDOFF` | ELF/APK/configuração | envelope para Vectras | runtime manifest | `TOKEN_VAZIO_RUNTIME` |
| `F_EPISTEMIC` | claim e falsificador | validação de fronteira | decisão limitada | `claim_allowed=false` |
| `F_SAFETY` | parent, riscos e gates | rollback/fail-safe | safety receipt | circuit breaker |
| `F_MEMORY` | eventos e receipts | append-only | ledger longitudinal | quarentena |
| `F_SEMANTIC` | keywords e relações | indexação tipada | catálogo IA/humano | `TOKEN_VAZIO_RELATION` |

## 3. Contrato de entrega ao Vectras

O RafPolimata somente deve entregar um artefato como candidato de runtime quando existir:

- `source_commit` ou revisão de origem;
- `toolchain_id` e versões;
- comandos e flags;
- hashes do artefato;
- arquitetura/ABI declarada;
- resultado dos validadores estruturais;
- lista de dependências;
- estado epistemológico;
- plano de rollback.

Envelope mínimo:

```json
{
  "artifact_id": "TOKEN_VAZIO_UNTIL_BUILD",
  "producer": "RafPolimata",
  "consumer": "Vectras-VM-Android",
  "source_commit": "TOKEN_VAZIO",
  "target": "armv7|aarch64|x86_64|TOKEN_VAZIO",
  "format": "ELF|APK|DEX|ZIP|TOKEN_VAZIO",
  "hashes": {},
  "validators": [],
  "claim_allowed": false,
  "rollback": "restore_previous_verified_artifact"
}
```

## 4. Segurança

### Rollback

Toda alteração deve manter o commit-pai, artefato anterior verificável e comando de restauração. O histórico não é apagado; uma reversão gera novo receipt.

### Fail-safe

Falha de schema, hash, dependência, compilação ou validação produz bloqueio de handoff. Ausência de execução nunca vira `PASS`.

### Failover

```text
artifact from current commit
  -> previous verified artifact
  -> source rebuild with pinned toolchain
  -> TOKEN_VAZIO
```

### Watchdog

Sinais mínimos:

- build sem source commit;
- artefato sem hash;
- ABI divergente;
- dependência não resolvida;
- log ausente ou truncado;
- receipt duplicado;
- handoff sem consumidor identificado;
- claim promovido sem runtime.

### Circuit breaker

Interromper o pipeline em `hash_mismatch`, `schema_failure`, `unknown_toolchain`, `unknown_target`, `validator_failure` ou `authority_conflict`.

## 5. Gates

| gate | condição de sucesso | estado se ausente/falhar |
|---|---|---|
| identidade | fonte e revisão identificadas | `TOKEN_VAZIO_SOURCE` |
| build | comando reproduzível e exit 0 | `TOKEN_VAZIO_BUILD` ou `FAIL` |
| formato | ELF/APK/DEX/ZIP estruturalmente válido | `FAIL_FORMAT` |
| hash | digest recalculado coincide | `TOKEN_VAZIO_HASH_MISMATCH` |
| handoff | manifest completo e consumidor conhecido | `TOKEN_VAZIO_RUNTIME` |
| rollback | retorno ensaiado ou procedimento verificável | `TOKEN_VAZIO_ROLLBACK_PROOF` |
| evidência | stdout/stderr, ambiente e receipt | `claim_allowed=false` |

## 6. Relação com a memória longitudinal

Cada evento usa identidade única e append-only:

```text
SOURCE_SEEN -> BUILD_ATTEMPTED -> BUILD_RESULT
-> VALIDATION_RESULT -> HANDOFF_CREATED
-> RUNTIME_RESULT -> EPISTEMIC_DECISION
```

Eventos posteriores podem corrigir estado anterior, mas não reescrever sua existência histórica.

## 7. Classificação atual

- `PROVADO`: este contrato existe após commit Git observável.
- `EVIDENCIADO`: RafPolimata possui corpos de compilação, validação e governança documentados.
- `HIPÓTESE`: o envelope reduz incompatibilidades entre compilador e runtime.
- `MODELO_ANALÓGICO`: filamentos representam trilhas operacionais.
- `REFUTADO`: existência do contrato não prova build, APK instalado ou boot.
- `TOKEN_VAZIO`: build corrente, artefato corrente, handoff real, rollback ensaiado e runtime no Vectras.

## 8. F_next verificável

1. Materializar schema JSON do envelope.
2. Gerar um envelope a partir de build local real.
3. Validar hashes e ABI.
4. Entregar ao gate de ingestão do Vectras.
5. Registrar execução, falha ou rollback sem promover claims além da evidência.

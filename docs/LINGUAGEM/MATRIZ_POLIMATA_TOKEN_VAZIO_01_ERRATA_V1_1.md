# Errata v1.1 — escada universal e perfis de evidência

> **Documento de origem:** `MATRIZ_POLIMATA_TOKEN_VAZIO_01.md`  
> **Autoridade executável:** `configs/language-matrix.v1.json`, `contracts/language-matrix-state.schema.json` e `scripts/language_matrix.py`  
> **Estado:** `CANONICAL_CORRECTION`

## Correção

A versão inicial apresentava uma única escada conceitual:

```text
0.1 definição
...
0.5 fixture hasheado
0.6 tokenizer validado
...
1.0 auditoria de domínio
```

Essa sequência não é universal. `validated_tokenizer` é pertinente à família linguística, mas não a operadores matriciais, Fibonacci ou proveniência de commits.

A régua executável v1.1 fica:

| Score | Gate universal |
|---:|---|
| `0.1` | `definition` |
| `0.2` | `contract` |
| `0.3` | `implementation` |
| `0.4` | `local_test` |
| `0.5` | `hashed_fixture` |

Acima de `0.5`:

```text
TOKEN_VAZIO_AXIS_SPECIFIC_GATE_PROFILE
```

## Perfis candidatos não executáveis

```text
linguistic:
  validated_tokenizer → out_of_sample → independent_replication

deterministic:
  randomized_fixture → out_of_sample → independent_replication

provenance:
  clean_checkout → parent_chain_verified → independent_replication
```

Esses perfis são `REFERENCE_NOT_EXECUTABLE`. Só poderão liberar `0.6–1.0` depois de contrato, implementação e testes próprios.

## Invariante

\[
\boxed{um\ gate\ só\ promove\ o\ eixo\ para\ o\ qual\ possui\ significado}
\]

Assim, um tokenizer não valida matemática; um hash de commit não valida compreensão humana; uma reprodução matemática não comprova uma hipótese linguística.

## Estado

```text
universal_max_score = 0.5
post_0_5 = TOKEN_VAZIO_AXIS_SPECIFIC_GATE_PROFILE
claim_allowed = false
```

# Cross-Repo Bridge V1 — RafGitTools ↔ RafPolimata ↔ FlorisBoard

**Estado:** `IMPLEMENTED_LOCAL / EXTERNAL_PRODUCER_TOKEN_VAZIO`  
**Modo:** read-only  
**Issue local:** `#298`  
**Issue produtor:** `RafGitTools#357`  
**Governança:** `CLOSURE_L11`

## 1. Contrato

O bridge consome referências JSONL no schema:

`schemas/cross_repo_artifact_ref.v1.json`

Campos obrigatórios:

```text
schema
repo
path
commit_sha
content_hash
media_type
provenance_state
artifact_kind
claim_allowed
```

Campos opcionais permitidos:

```text
producer
relation_hints
```

Qualquer outro campo é rejeitado.

Isso é deliberado: o bridge é de **metadados de artefato**, não de conteúdo arbitrário.

## 2. Privacidade FlorisBoard/IME

A fronteira é estrutural:

```text
additionalProperties = false
```

O schema não contém campos como:

```text
typed_text
input_text
composing_text
clipboard
keystrokes
telemetry_payload
```

O parser também mantém uma allowlist independente do schema. Portanto adicionar um payload inesperado ao JSON gera `FAIL`, em vez de ele atravessar silenciosamente para o grafo.

Isso não prova que o FlorisBoard inteiro nunca processa texto — um IME necessariamente processa entrada durante sua função. A claim desta ponte é menor: **o protocolo cross-repo V1 não aceita conteúdo digitado como campo do artifact_ref**.

## 3. TOKEN_VAZIO e claims

`commit_sha`, `content_hash`, `provenance_state` e `artifact_kind` podem representar ausência com `TOKEN_VAZIO` onde o schema permite.

Se qualquer identidade/proveniência central estiver `TOKEN_VAZIO`:

```text
claim_allowed = false
```

Além disso, `claim_allowed=true` exige `provenance_state=VERIFIED`.

O receipt do bridge permanece sempre:

```text
claim_allowed=false
```

porque ele prova somente a transformação determinística do manifesto, não a verdade independente das afirmações do produtor.

## 4. Projeção

Cada artifact_ref vira um nó com tipo:

```text
source | code | build | runtime | receipt | claim | TOKEN_VAZIO
```

Relações aceitas:

```text
derived_from
supports
contradicts
supersedes
requires
observed_in
```

Uma relação cujo `target_content_hash` não existe no lote não é inventada. Ela entra em `unresolved_edges` com:

```text
state=TOKEN_VAZIO
claim_allowed=false
```

## 5. Determinismo

O input é normalizado e ordenado por serialização JSON canônica.

O bridge gera:

- `graph_hash`;
- `input_hash`;
- `output_hash`;
- contagem de entradas;
- versões dos schemas.

A ordem original das linhas não altera o grafo ou receipt.

## 6. Uso

Via arquivo:

```sh
python3 scripts/cross_repo_bridge_v1.py manifest.jsonl \
  --output results/cross-repo-bridge/graph.json \
  --receipt results/cross-repo-bridge/receipt.json
```

Via stdin, sem dependência do filesystem do produtor:

```sh
cat manifest.jsonl | python3 scripts/cross_repo_bridge_v1.py -
```

## 7. Evidência local

```sh
python3 -m unittest -v tests.test_cross_repo_bridge_v1
```

Os testes cobrem:

- allowlist do schema;
- rejeição de `typed_text`;
- fail-closed de `TOKEN_VAZIO`;
- claim proibida com proveniência não verificada;
- determinismo independente da ordem;
- aresta resolvida;
- aresta não resolvida preservada como `TOKEN_VAZIO`;
- parsing JSONL equivalente a stdin;
- receipt sem promoção de claim do produtor.

## 8. Limite externo

No corte de 2026-08-16, `RafGitTools#357` ainda especifica, mas não demonstra, o emissor read-only de manifesto `artifact_ref`.

Logo:

```text
RafPolimata consumer = IMPLEMENTED
privacy boundary = testable locally
real RafGitTools producer = TOKEN_VAZIO
real cross-repo receipt = TOKEN_VAZIO
```

O issue #298 não deve ser encerrado como integração completa enquanto não houver pelo menos um manifesto real produzido fora do RafPolimata, consumido sem edição manual e ligado a receipt/hash.

## 9. FlorisBoard observado

O repositório `rafaelmeloreisnovo/florisboard` existe como fork público do FlorisBoard, com código predominantemente Kotlin e licença Apache-2.0 detectada pelo GitHub. Isso é metadado de repositório; não é evidência de integração com este bridge.

## 10. F_ok / F_gap / F_next

```text
F_ok:
  schema + consumer + graph + receipt + privacy allowlist + tests

F_gap:
  emissor real RafGitTools + manifesto real + receipt cross-repo real

F_next:
  implementar o produtor read-only no RafGitTools conforme issue #357 e alimentar este consumer sem transformação manual
```

# CLOSURE_L11 — Operational Gap Topology Governance

**Estado:** `IMPLEMENTED_GOVERNANCE / MATERIAL_GAPS_REMAIN_OPEN`  
**Escopo:** `TOKEN_VAZIO`, `PENDING`, urgência, dependências e critérios de fechamento registrados pela topologia operacional  
**Fonte executável:** `configs/operational-gap-topology.v1.json`  
**Validador:** `scripts/validate_operational_gap_topology.py`

## O que esta closure significa

`CLOSURE_L11` fecha **a ausência de governança sobre as lacunas**, não as lacunas materiais em si.

Antes desta camada, vários estados estavam distribuídos entre documentos, issues, runtime, release, segurança, licença e integração. A topologia passa a exigir que cada gap material tenha:

```text
id
+ estado
+ urgência
+ impacto
+ classe de incerteza
+ claim_allowed
+ owner_role
+ required_roles
+ provenance
+ evidence
+ next_action
+ closure.condition
+ closure.required_artifacts
+ closure.required_gates
```

Portanto:

```text
CLOSURE_L11 PASS
!=
all gaps PASS
```

## Invariantes

1. `TOKEN_VAZIO != PASS`.
2. `PENDING != PASS`.
3. `FAIL != PASS`.
4. Um registro governado por `CLOSURE_L11` continua com `claim_allowed=false` enquanto seu próprio estado material não for demonstrado.
5. A closure do grafo não substitui `CLOSURE_L2` para runtime Android nem `CLOSURE_L3` para ARM64 ELF.
6. Decisões de proprietário permanecem `OWNER_DECISION` e não podem ser automatizadas.
7. Aresta `requires` é dependência operacional; não é inferência causal universal.
8. Standards e frameworks externos aparecem apenas como referências de arquitetura; nenhum mapeamento interno concede certificação.

## Critério de validade do registro

Um `TOKEN_VAZIO` nesta região está ligado validamente a `CLOSURE_L11` somente se o registro correspondente passar o validador da topologia. O validador rejeita, entre outros:

- claim permitido em `TOKEN_VAZIO`, `PENDING` ou `FAIL`;
- gap P0/P1 sem papéis e controles de referência;
- gap sem proveniência, next action ou closure;
- `PASS` sem evidência;
- aresta para endpoint inexistente;
- ciclo na relação `requires`;
- `OWNER_DECISION` sem decisão humana requerida.

## Estado material preservado

No snapshot inicial de 2026-08-16 permanecem gaps reais, incluindo:

- Android runtime físico do artefato corrente — `TOKEN_VAZIO` e governado também por `CLOSURE_L2`;
- ARM64 ELF dentro do APK corrente — `TOKEN_VAZIO` e governado também por `CLOSURE_L3`;
- decisão de licença — `TOKEN_VAZIO_OWNER_DECISION`;
- canal privado de vulnerabilidade — `PENDING` até verificação administrativa;
- SBOM/release atual — `TOKEN_VAZIO`;
- federação/receipts cross-repo — `TOKEN_VAZIO`;
- reprodução independente — `TOKEN_VAZIO`.

Nenhum desses estados é promovido por esta closure.

## Gate

```sh
python3 -m unittest -v tests.test_operational_gap_topology
python3 scripts/validate_operational_gap_topology.py
```

`CLOSURE_L11` é considerada operacional apenas quando esses gates passam no commit/PR correspondente.

## Promoção

Um gap individual só muda de estado quando seus próprios `required_artifacts`, `required_gates` e `closure.condition` forem satisfeitos e a evidência for registrada. Depois disso, revisar todas as arestas dependentes; nenhuma promoção é transitiva automaticamente.

## F_ok / F_gap

```text
F_ok  = ausência estrutural de ownership/proveniência/closure do mapa de gaps é fechada por um contrato validável
F_gap = cada ausência técnica, física, jurídica, comercial ou externa permanece no seu estado material até evidência própria
```

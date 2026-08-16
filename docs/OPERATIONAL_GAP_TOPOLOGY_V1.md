# Operational Gap Topology V1 — técnico, comercial, segurança, conformidade e governança

**Estado:** `CANONICAL`  
**Área:** governança operacional / evidence / produto  
**Responsável lógico:** `repository-maintainer` + donos de domínio  
**Fonte executável:** `configs/operational-gap-topology.v1.json`  
**Schema:** `schemas/operational-gap-topology.v1.schema.json`  
**Validador:** `scripts/validate_operational_gap_topology.py`

## 1. Objetivo

Esta camada converte lacunas, incertezas, urgências, dependências e ausências em um grafo operacional versionado.

A unidade fundamental é:

```text
Gap_i = (
  estado,
  urgência,
  impacto,
  classe_de_incerteza,
  claim_allowed,
  responsável,
  papéis,
  proveniência,
  evidência,
  próxima_ação,
  condição_de_fechamento
)
```

E a relação:

```text
Edge_j = (origem, destino, relação, força, justificativa)
```

Assim, “esquecido”, “ignorado”, “óbvio”, “pendente” e `TOKEN_VAZIO` deixam de depender da memória de uma sessão e passam a ter identidade e rota.

## 2. Invariantes

1. `TOKEN_VAZIO != PASS`.
2. `PENDING != PASS`.
3. `FAIL != PASS`.
4. `TOKEN_VAZIO`, `PENDING` e `FAIL` exigem `claim_allowed=false`.
5. Gap P0/P1 precisa de responsável, papéis necessários, proveniência, controle de referência, próxima ação e closure.
6. `PASS` exige evidência não vazia.
7. Relação `requires` não pode formar ciclo.
8. Aresta não pode apontar para gap inexistente.
9. Decisão jurídica/comercial classificada `OWNER_DECISION` exige autorização humana explícita.
10. Referência ISO/NIST/SLSA é **alinhamento arquitetural**, nunca certificação automática.
11. Evidência histórica permanece histórica; não prova o HEAD por herança.
12. Fechar um nó não fecha automaticamente os nós dependentes.

## 3. Snapshot material — 2026-08-16

O grafo V1 contém:

```text
17 gaps
17 edges
3  P0
10 P1
3  P2
1  P3

12 TOKEN_VAZIO
5  PENDING
```

O validador local da implementação inicial retornou `PASS` para a estrutura e os testes negativos rejeitaram claim indevido, endpoint inexistente, ciclo `requires`, decisão de proprietário não autorizada e `PASS` sem evidência.

Esses números descrevem o **grafo**, não a maturidade total do projeto.

## 4. P0 — fronteira imediata

| Gap | Estado | Por que bloqueia |
|---|---|---|
| `GAP-TECH-ARM64-APK-L4` | `TOKEN_VAZIO` | identidade do ELF ARM64 dentro do APK atual ainda não foi observada como cadeia contínua |
| `GAP-TECH-ANDROID-RUNTIME-L2` | `TOKEN_VAZIO` | CI verde não equivale a install/launch/logcat em device físico do mesmo artefato |
| `GAP-LEGAL-LICENSE-DECISION` | `TOKEN_VAZIO_OWNER_DECISION` | redistribuição/comercialização não deve presumir permissões jurídicas |

A ordem parcial é:

```text
ARM64 current artifact inside APK
        -> Android current-artifact runtime

license decision
        -> release assets
        -> external/commercial validation
```

## 5. P1 — risco alto e integração

Os P1 atuais cobrem:

- canal privado de vulnerabilidade verificável;
- SBOM + inventário de termos de terceiros;
- ledger de custódia em Linux CI + Termux e reconciliação do issue #121;
- assinatura federada real do issue #134;
- receipts cross-repo reais do issue #134;
- `cross_repo_bridge_v1` do issue #298;
- fronteira executável de privacidade FlorisBoard/IME;
- release atual com artefatos/checksums;
- claims históricos de `PHASES_49_50_COMPLETION_SUMMARY.md` ainda sob revisão;
- evidência das regras de branch/ruleset/merge control.

Isso transforma os três issues abertos #121, #134 e #298 em uma única região topológica, sem apagar a autoridade local de cada issue.

## 6. Domínios e responsabilidade

| Domínio | Papel principal | Evidência mínima |
|---|---|---|
| técnico | `apkc-maintainer` / `runtime-maintainer` | artifact + command + environment + receipt |
| segurança | `security-license` | policy + negative/positive gate + disclosure path |
| compliance | `compliance-owner` | escopo + obrigação + evidência + exceção |
| governança | `evidence-custodian` | identidade + cadeia + decisão + rollback |
| comercial | `commercial-owner` / humano | produto delimitado + termos + validação externa |
| release | `release-manager` | artifact + checksum + provenance + notes |
| federação | `federation-maintainer` | authority + receipt + aggregate + verifier |
| integração | `integration-maintainer` | schema + adapter + deterministic replay |
| privacidade | `privacy-review` | allowlist + negative fixtures + data boundary |
| qualidade | `quality-assurance` | regressão + reprodução + desvios |
| documentação | `documentation-governance` | claim-to-evidence + lifecycle + links |
| jurídico | `human-authorizer` + revisão | decisão explícita + inventário + compatibilidade |

Papéis são funções, não pessoas fixas.

## 7. Topologia comercial

A estrutura comercial é tratada como cinco planos independentes:

```text
P0 CORE
  código / compiler / runtime / formatos

P1 PROOF PLANE
  tests / custody / provenance / receipts / reproducibility

P2 DISTRIBUTION PLANE
  license / release / SBOM / security policy

P3 TRUST PLANE
  compliance mapping / risk / vulnerability response / governance

P4 MARKET PLANE
  support lifecycle / external reproduction / pilots / adoption
```

Regra:

```text
valor técnico alto
!=
produto distribuível
!=
produto comercialmente suportado
!=
produto externamente validado
```

Essa separação reduz risco de valuation narrativo e due diligence inconsistente.

## 8. Segurança e supply-chain

### 8.1 Disclosure

`.github/SECURITY.md` passa a existir como política versionada. A configuração administrativa de Private Vulnerability Reporting permanece `TOKEN_VAZIO` enquanto não houver evidência de que o recurso está habilitado.

### 8.2 Proveniência

O ledger interno já possui schema e workflow próprios; o gap atual não é “inventar ledger”, mas concluir o contrato cross-environment e reconciliar o issue #121.

### 8.3 Release

A release `V1.0.0`, observada em 2026-08-16, é prerelease de 2026-06-14 e não possui assets anexados. Ela permanece referência histórica, não release corrente reproduzível do HEAD.

### 8.4 Merge controls

O contrato `AGENTS.md` exige branch de trabalho e autorização humana para merge. O endpoint de branch protection não pôde ser lido pela integração atual; portanto configuração concreta de ruleset/protection permanece `TOKEN_VAZIO` até verificação administrativa.

## 9. Licença e propriedade intelectual

O metadata GitHub observado em 2026-08-16 registra `license=null` e não há `LICENSE` na raiz.

`docs/LICENSE_DECISION_RECORD.md` transforma essa ausência em decisão P0 rastreável.

Nenhum agente deve escolher silenciosamente MIT, Apache, GPL, AGPL, proprietária ou dual-license. O proprietário precisa primeiro decidir a intenção e revisar autoria/terceiros.

## 10. Privacidade e FlorisBoard

O issue #298 define que conteúdo digitado permanece fora do escopo por padrão.

A topologia torna isso executável como dependência:

```text
GAP-INTEG-CROSS-REPO-BRIDGE-298
    requires
GAP-PRIV-IME-DATA-BOUNDARY
```

Portanto o bridge não fecha apenas por conseguir ingerir `artifact_ref`; precisa também demonstrar que payload de texto digitado não entra no grafo por default.

## 11. Federação

O issue #134 já registra 11/11 autoridades locais reconciliadas, mas mantém:

```text
FEDERATED_RUNTIME = TOKEN_VAZIO_NOT_VERIFIED
REAL_SIGNATURES = TOKEN_VAZIO_PLACEHOLDER
CROSS_REPO_RECEIPTS = TOKEN_VAZIO_NOT_VERIFIED
claim_allowed = false
```

A topologia separa dois nós:

- assinatura/verificação criptográfica real;
- receipt cross-repo físico e agregação.

Isso impede que “implementou assinatura” seja confundido com “federação inteira validada”.

## 12. Qualidade e academia

CI interna é evidência relevante, mas reprodução independente é outro nó.

A closure acadêmico-técnica mais forte exige:

```text
protocolo publicado
-> ambiente independente
-> mesmos inputs
-> execução
-> receipt
-> comparação
-> contabilização de divergências
```

Resultado independente negativo também é evidência válida e não deve ser removido.

## 13. Referências normativas — snapshot de versão

Snapshot observado em 2026-08-16:

| Referência | Estado usado pela topologia |
|---|---|
| NIST Cybersecurity Framework | **2.0**, final; funções Govern, Identify, Protect, Detect, Respond, Recover |
| NIST SSDF | **1.1 final** como baseline; **1.2 Rev.1 draft** apenas monitorado |
| SLSA | **1.2**, approved/current |
| ISO/IEC 27001 | **2022**, referência de ISMS |
| ISO 37301 | **2021**, compliance management |
| ISO 31000 | **2018**, risk management |
| ISO 9001 | **2015 + Amd 1:2024** ainda publicado; edição 2026 estava em publicação e não é tratada como norma final neste corte |

O repositório **não declara certificação** por essas normas. Elas são vocabulário de arquitetura, controle e due diligence.

## 14. Mapeamento de referência

A topologia usa rótulos de alto nível:

- `NIST-CSF-2.0-GV/PR/DE/RS` — funções de governança, proteção, detecção e resposta;
- `NIST-SSDF-PO/PS/PW/RV` — preparar organização, proteger software, produzir software seguro e responder a vulnerabilidades;
- `SLSA-1.2-Source/Build/Provenance` — fonte, build e proveniência;
- `ISO-IEC-27001-2022` — gestão de risco de segurança;
- `ISO-37301` — obrigações e compliance;
- `ISO-31000` — risco e tratamento;
- `ISO-9001` — qualidade/processo.

Esses rótulos são **crosswalk interno**, não reprodução integral dos standards.

## 15. Como atualizar um gap

Nunca apenas trocar `TOKEN_VAZIO` por `PASS`.

Processo:

```text
1. identificar gap_id
2. executar next_action
3. preservar comando/ambiente/input/output
4. anexar evidence/provenance
5. executar required_gates
6. verificar closure.condition
7. atualizar state
8. revisar edges impactadas
9. executar validador e testes
10. merge por decisão humana
```

Se a evidência fechar apenas parte da condição, usar `PENDING`/`IMPLEMENTED`, não `PASS`.

## 16. Comandos

```sh
python3 -m unittest -v tests.test_operational_gap_topology
python3 scripts/validate_operational_gap_topology.py
python3 scripts/validate_operational_gap_topology.py \
  --write-report ci/reports/operational-gap-topology.json
```

## 17. Critério de excelência operacional

A excelência não é “zerar a lista” por edição textual.

```text
Excelência =
  lacuna detectável
  + dono explícito
  + proveniência
  + dependências
  + fechamento falsificável
  + evidência preservada
  + claim limitado
```

Uma lacuna que depende de device, proprietário, auditor externo ou outro repositório continua válida como `TOKEN_VAZIO`; o avanço é torná-la **localizável, limitada, roteável e verificável**.

## 18. F_ok / F_gap / F_next deste contrato

```text
F_ok:
  schema + grafo + validador + testes + política de segurança + decisão de licença modelada

F_gap:
  os gaps materiais permanecem abertos até seus próprios required_gates

F_next:
  fechar P0 em ordem de dependência e, em paralelo, P1 que não depende de device
```

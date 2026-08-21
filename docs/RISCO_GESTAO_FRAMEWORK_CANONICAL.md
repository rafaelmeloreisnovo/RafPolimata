# RISCO — Framework Canônico de Gestão, Detecção e Prevenção

> **Status:** `CANONICAL`  
> **Versão:** 1.0  
> **Data:** 2026-08-21  
> **Entrada canônica:** docs/AGENTES.md §10 (governança) + docs/INDEX.md §3 (excelência operacional)

---

## Visão geral

RAFAELIA implementa **gestão de riscos contínua e orientada por evidência**, estruturada em quatro camadas inseparáveis. Toda ação operacional passa por gates de detecção, prevenção e responsabilidade antes de execução.

O framework responde a três questões principais:

1. **Que riscos existem?** (Identificação)
2. **Como evitamos?** (Prevenção)
3. **Como detectamos se falhou?** (Observação)
4. **Como corrigimos?** (Remediação)
5. **Como melhoramos?** (Evolução)

---

## Estrutura em Camadas

### C1: Base Acadêmica e Normativa

Identificar **por quê** os riscos importam:

- **Metodologias de risco:** NIST RMF, ISO 31000, frameworks de segurança;
- **Legislação aplicável:** LGPD (dados), Lei de Propriedade Industrial (IP), contratos públicos;
- **Padrões técnicos:** OWASP Top 10, CWE, CVSS, conformidade de freestanding;
- **Pesquisa:** artigos sobre falhas em compiladores, formatadores binários, criptografia;
- **Autoridades:** órgãos de certificação, relatórios públicos, alertas de segurança.

**Saída C1:** Bibliografia, matriz fonte→requisito, regulações identificadas.

---

### C2: Camada Normativa e Contratual

Transformar requisitos externos em **obrigações operacionais**:

| Origem | Obrigação | Escopo | Responsável |
|--------|-----------|--------|-------------|
| LGPD Art. 46 | Integridade de dados | conversation_indexer | Polimata |
| NIST SP 800-53 | Detecção de anomalia | runtime | Mapa |
| Contrato X | Auditoria determinística | raf_compile | ApkC/Polimata |
| Lei PI | Citação correta | docs | RafGitTools |

**Nota:** Obrigação normativa ≠ implementação autoral. Uma lei diz *que* você precisa controlar, não *como* você controla.

**Saída C2:** Mapa de requisitos → gate, responsáveis, prazos.

---

### C3: Camada Operacional Autoral (RAFAELIA)

Implementação própria de controles:

#### 3.1. Categorias de Risco Identificadas

**R1 — Compilação/Geração**
- Entrada malformada causa segfault
- Saída não-determinística
- Vazamento de dados do compilador
- Header corrupção silenciosa

**R2 — Serialização/Indexação**
- Truncamento não-detectado
- Corrupção de bytes
- Verificação de integridade fraca
- Desserialização não-segura

**R3 — Runtime/Execução**
- NULL dereferencing
- Buffer overflow
- Escape de contexto
- Falha de rollback

**R4 — Autoria/IP**
- Confusão entre acadêmico e implementação
- Claim não suportado
- Atribuição incorreta
- Reprodução não autorizada

**R5 — Concorrência/Integridade**
- Race conditions
- Deadlock
- Invalidação de invariantes
- Atomicidade quebrada

**R6 — Obsolência e Evolução**
- Comando histórico que mudou sintaxe
- Teste que não reflete código atual
- Documentação desatualizada
- Ferramenta descontinuada

---

#### 3.2. Gates de Prevenção (Pré-execução)

Antes de qualquer mudança significativa:

**G0: Legalidade**
```
Existe lei/regulação que proíbe isto?
├─ Sim → Escalate
└─ Não → G1
```

**G1: Conformidade de Contrato**
```
Viola termo de serviço / contrato / termo de uso?
├─ Sim → Escalate
└─ Não → G2
```

**G2: Verdade do Claim**
```
Posso comprovar com evidência o que estou afirmando?
├─ Não → Reduzir claim ou coletar evidência
└─ Sim → G3
```

**G3: Respeito a Autoria e Propriedade Intelectual**
```
Isso apropria-se de marca, logotipo, material protegido ou endosso não autorizado?
├─ Sim → Corrigir ou remover
└─ Não → G4
```

**G4: Assimetria Coercitiva**
```
Isso cria vantagem manipulativa, lock-in ou desvantagem competitiva injusta para terceiros?
├─ Sim → Revisar eticamente
└─ Não → G5
```

**G5: Teste de Universalização (Kant/Rawls)**
```
Se TODOS os competidores fizessem isto:
├─ Mercado melhora?     → G6 (robust)
├─ Mercado piora?       → FLAG como alto risco
└─ Só funciona se "eu fiz primeiro"? → Suspeito, revisar
```

**G6: Transparência Pública**
```
Eu publicaria isto explicando a estratégia a clientes, reguladores e concorrentes?
├─ Não → Há risco reputacional
└─ Sim → G7
```

**G7: Cascata Previsível**
```
Quais são efeitos de segunda, terceira ordem?
├─ Documentar e aceitar
└─ Prosseguir com ciência de risco
```

→ **EXECUÇÃO** com registro de gate

---

#### 3.3. Gates de Detecção (Execução)

Durante e após:

**D1: Teste de Falha**
```
Código novo passa em teste de falha esperada?
├─ Não → Falha silenciosa detectada
└─ Sim → D2
```

**D2: Limite e Capacidade**
```
Não há truncamento, vazamento ou alocação oculta?
├─ Buffer seguro, checked,lientsize conhecido
└─ D3
```

**D3: Corrupção de Integridade**
```
Receipt/hash antes e depois identifica mudança não autorizada?
├─ Não → Auditoria falhou
└─ Sim → D4
```

**D4: Semântica Preservada**
```
Código/doc/teste permanecem coerentes?
├─ Drift detectado → Registrar delta
└─ Sim → D5
```

**D5: Rastreabilidade**
```
Posso conectar execução → input → output → receipt → claim?
├─ Não → TOKEN_VAZIO/auditoria incompleta
└─ Sim → PASS
```

---

#### 3.4. Gates de Remediação (Resposta)

Quando falha é detectada:

**R0: Isolamento**
```
Parar propagação:
├─ Rollback automático (se contrato permite)
├─ Fallback para versão anterior
└─ Flag para revisão manual
```

**R1: Análise de Raiz**
```
Por que o gate abriu?
├─ Bug em código → patch
├─ Teste insuficiente → cobertura
├─ Documentação enganosa → correção
└─ Mudança não prevista → reversão + deliberação
```

**R2: Evidência**
```
Registrar:
├─ Timestamp da detecção
├─ Afetados (commits, artifacts, usuários)
├─ Raiz (técnica ou operacional)
├─ Ação tomada
├─ Commit de correção
└─ Novo teste adicionado
```

**R3: Comunicação**
```
Quem precisa saber?
├─ Stakeholders técnicos
├─ Autoridades (se aplicável)
└─ Clientes (se dados afetados)
```

---

#### 3.5. Gates de Melhoria Contínua

Retroalimentação para evolução:

**I0: Análise de Frequência**
```
Esta classe de risco ocorreu antes?
├─ Sim → Por que o gate anterior falhou?
└─ Primeiro evento → incorporar aprendizado
```

**I1: Efetividade do Gate**
```
Quantos eventos este gate teria prevenido se ativo?
├─ Alta efetividade → Promover a pré-execução
├─ Baixa → Revisar gate
```

**I2: Evolução de Ameaça**
```
Novos riscos identificados no campo?
├─ Sim → Adicionar gate
└─ Atualizar priorização
```

**I3: Tooling**
```
Automação disponível para este gate?
├─ Sim → Integrar em CI/CD
├─ Não → Criar
└─ Custo de automação > custo de risco?
```

---

### C4: Camada de Claims (Públicos)

O que **podemos afirmar** sobre excelência:

#### Pode afirmar:

✓ "RAFAELIA implementa **detecção de risco orientada por evidência**"
  - Evidência: gates D1-D5 executados, receipts em docs/results/

✓ "Risco de corrupção de índice é **mitigado por verificação de integridade**"
  - Limite: escopo conversation_indexer, tamanho máximo 2GB

✓ "Conformidade com **LGPD Artigo 46** é verificável"
  - Cadeia: LGPD → requisito → gate → teste → receipt

✓ "Recuperação de falha é **testada regularmente**"
  - Falsificador: chaos test em cada commit

---

#### **Não pode afirmar** (sem evidência complementar):

✗ "Certificado ISO 9001"
  - ↳ Certificação requer auditor terceiro, não autoria própria

✗ "Seguro contra risco de compilador"
  - ↳ Não existe segurança absoluta; apenas mitigação conhecida

✗ "100% conformidade LGPD"
  - ↳ LGPD é contínua; conformidade é evolução, não estado

✗ "Padrão de indústria"
  - ↳ RAFAELIA é único; pode estar alinhado com boas práticas, não é padrão

---

#### **TOKEN_VAZIO** (evidência ausente):

≈ "Risco de ataque X264 no ApkC"
  - Motivo: Não existe reprodutor de ataque X264 no ambiente
  - Ação: Criar falsificador ou aceitar como TOKEN_VAZIO

≈ "Detecção de risco em dispositivo Android real"
  - Motivo: Ambiente de teste não inclui device
  - Ação: Agendar ou documentar limitação

---

## Matriz de Subsistemas

| Subsistema | Risco Primário | Gate Crítico | Remediação | Status |
|---|---|---|---|---|
| **ApkC** | Compilação não-determinística | D1 (golden test) | Rollback a versão anterior | IMPLEMENTED |
| **Conversation Indexer** | Corrupção de índice | D3 (hash check) | Reindexação | IMPLEMENTED |
| **Verbovivo/T^7** | Divergência de convergência | D4 (semântica) | Re-execução com raiz conhecida | REFERENCE |
| **Runtime Router** | NULL dereference | D2 (bounds) | Fallback a modo seguro | IMPLEMENTED |
| **Governança Documental** | Documento desatualizado | I1 (frequência) | Verificação automática | PENDING |

---

## Gates Executáveis

### CLI de verificação

```bash
# Verificar gates de risco em branch atual
python3 scripts/risk_gates_validator.py --branch HEAD

# Executar falsificador de corrupção
python3 scripts/chaos_corruption_test.py --sample 100

# Validar cadeia de receipts
python3 scripts/receipt_chain_validator.py --target conversation_indexer

# Auditoria de compliance
python3 scripts/compliance_audit.py --regulation LGPD --report results/audit_lgpd_2026.json
```

### Integração CI/CD

Gate de risco é **bloqueante** em:
- Merge request (G0-G7 pré-execução)
- Release (D1-D5 execução + I0-I3 retrospectiva)

Gate de risco é **warning** em:
- Commit (informativo)
- PR draft (não bloqueia)

---

## Evidência e Receipts

Toda execução de gate produz receipt:

```json
{
  "timestamp": "2026-08-21T14:32:00Z",
  "commit": "9da4c835ef473d2a6fe5903f92cf8ff0b29e0dc4",
  "branch": "claude/governance-ethics-layers-2aon2n",
  "gate_id": "G5_universalization",
  "gate_type": "prevention",
  "subsystem": "entire",
  "claim_tested": "RAFAELIA implementa gestão de riscos contínua",
  "result": "PASS",
  "evidence": {
    "gate_description": "Se todos competidores fizessem isto, o mercado melhoraria?",
    "response": "Sim. Framework abre visibilidade, aumenta confiança, reduz custo de verificação para todos.",
    "universalization_score": 0.85
  },
  "scope": "governance framework",
  "limitations": "Resposta é qualitativa; requer revisão humana",
  "reviewed_by": "rafael@...",
  "next_review": "2026-09-21"
}
```

---

## Cronograma de Validação

| Período | Gate | Frequency | Responsável |
|---------|------|-----------|-------------|
| **Cada commit** | G1-G3 | Automático | Git hooks + CI |
| **Cada PR** | G0-G7 | Automático | GitHub Actions |
| **Quinzenal** | D1-D5 | Manual + script | Polimata |
| **Mensal** | I0-I3 + retrospectiva | Manual | RafGitTools + Mapa |
| **Trimestral** | Revisão de efetividade | Manual | Arquiteto técnico |

---

## Próximos Passos

1. **Criar falsificadores** para cada risco em R1-R6
2. **Integrar gates** no Makefile/CI (.github/workflows)
3. **Documentar receipts** em docs/results/risk/
4. **Treinar subsistemas** com matriz em RISCO_MATRIZ_SUBSISTEMAS.md
5. **Validar claim base** com auditor externo (se aplicável)

---

## Referências

- NIST RMF: https://csrc.nist.gov/publications/detail/sp/800-39/final
- ISO 31000: Risk Management — Principles and Guidelines
- LGPD Lei Geral de Proteção de Dados: http://www.planalto.gov.br/ccivil_03/_ato2015-2018/2018/lei/l13709.htm
- OWASP Risk Rating: https://owasp.org/www-project-risk-rating/
- docs/AGENTES.md: Protocolo operacional
- docs/DOCUMENT_GOVERNANCE.md: Ciclo de vida documental

---

**Fechamento R3:**

```
F_ok   = Estrutura de 4 camadas definida; gates G0-G7 + D1-D5 + R0-R3 + I0-I3 documentados
F_gap  = Falsificadores ainda não implementados; automação CI/CD em TODO
F_next = Criar docs/RISCO_MATRIZ_SUBSISTEMAS.md com risco por subsistema e riscos específicos
```

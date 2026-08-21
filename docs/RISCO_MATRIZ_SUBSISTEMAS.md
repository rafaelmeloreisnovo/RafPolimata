# RISCO — Matriz de Subsistemas

> **Status:** `CANONICAL`  
> **Versão:** 1.0  
> **Data:** 2026-08-21  
> **Lê:** docs/RISCO_GESTAO_FRAMEWORK_CANONICAL.md (framework genérico)

---

## Propósito

Mapear riscos específicos de cada subsistema técnico em RAFAELIA, com gates de prevenção, detecção e remediação operacionalizáveis.

---

## 1. ApkC — Compilador/Toolchain Android

### 1.1 Riscos Identificados

| Risco | Descrição | Severidade | Categoria |
|-------|-----------|------------|-----------|
| **R1.1** | Compilação não-determinística (output varia sem entrada mudada) | ALTA | R1 (Compilação) |
| **R1.2** | Vazamento de caminhos do host no binário gerado | MÉDIA | R1 |
| **R1.3** | Buffer overflow em encoder ARM/DEX/AXML | ALTA | R1 + R3 |
| **R1.4** | Violação de contrato no-libc/no-heap em ARM/freestanding | ALTA | R1 + R4 |
| **R1.5** | Flag compilador histórica com sintaxe mudada | BAIXA | R6 (Obsolência) |
| **R1.6** | Assinatura APK fraca ou expirada | ALTA | R4 |

### 1.2 Gates de Prevenção (G0-G7)

| Gate | Verificação | Status |
|------|-------------|--------|
| **G0** | Mudança de flag compilador viola contrato POSIX/GCC? | IMPLEMENTED: scripts/validate_compiler_flags.sh |
| **G2** | Claim sobre otimização é suportado por benchmarks? | REFERENCE: Tabela de benchmarks vazia |
| **G3** | Determinismo é verificável? | IMPLEMENTED: `make syntaxcheck` + golden test |
| **G5** | Se todos RISC-V compatível fosse suportado, ApkC melhoraria? | PENDING: Análise de mercado |

### 1.3 Gates de Detecção (D1-D5)

| Gate | Teste | Frecuencia | Status |
|------|-------|-----------|--------|
| **D1.1** | Golden test: `make syntax && diff <input> <golden>` | Cada commit | IMPLEMENTED |
| **D1.2** | Fuzzer de entrada malformada → segfault previne? | Semanal | PENDING |
| **D2.1** | Validação de bounds em buffer de encoder | Cada mudança | IMPLEMENTED (sanitizer) |
| **D3.1** | Hash de artefato ARM/DEX antes/depois | Cada release | IMPLEMENTED |
| **D4.1** | Teste de semântica: binário rodável após compilação | Cada release | IMPLEMENTED |
| **D5.1** | Rastreabilidade source → artifact → APK → receipt | Manual, trimestral | PENDING |

### 1.4 Remediação (R0-R3)

**R0: Isolamento**
- Falha de determinismo → Não publicar APK
- Overflow detectado → Rollback de commit

**R1: Análise de Raiz**
- Submeter ao Coverity para análise
- Comparar com GCC/LLVM upstream para regressão

**R2: Evidência**
- Guardar entrada que falhou + binário + receipt
- Commit de patch com referência a falsificador

**R3: Comunicação**
- Se APK já distribuído: recall + comunicado de segurança

### 1.5 Evolução (I0-I3)

**I0: Frequência**
```
Se detecção de non-determinismo ocorreu 2+ vezes em 6 meses:
→ Promover D1.1 para gate pré-release bloqueante
```

**I1: Efetividade**
```
Quantos bugs de overflow fuzzer (D1.2) encontraria com limite atual?
→ Se > 5 em teste retrospectivo, automação no CI
```

**I2: Ameaça Emergente**
```
CVE publicado em GCC/LLVM versão usada?
→ Adicionar gate de patch management
```

---

## 2. Conversation Indexer — Serialização/Indexação

### 2.1 Riscos Identificados

| Risco | Descrição | Severidade | Categoria |
|-------|-----------|------------|-----------|
| **R2.1** | Truncamento silencioso de índice (entrada > max_size) | ALTA | R2 |
| **R2.2** | Corrupção de bytes em disco (bitflip não detectado) | ALTA | R2 + R4 (LGPD) |
| **R2.3** | Desserialização de payload malformado → crash | MÉDIA | R2 + R3 |
| **R2.4** | Leak de informação no índice (dados sensíveis visíveis) | ALTA | R4 (LGPD) |
| **R2.5** | Índice desatualizado vs código (schema mudou) | MÉDIA | R6 |

### 2.2 Gates de Prevenção

| Gate | Verificação | Status |
|------|-------------|--------|
| **G0** | LGPD: dado sensível pode ser indexado? | IMPLEMENTED: PII detector |
| **G2** | Tamanho máximo de índice é documentado e testado? | IMPLEMENTED: docs/conversation_indexer/PROTOCOL.md |
| **G3** | Schema de versão está em header? | IMPLEMENTED: v1, v2 codecs |

### 2.3 Gates de Detecção

| Gate | Teste | Frecuencia | Status |
|------|-------|-----------|--------|
| **D1.1** | Teste de limite: indexar entrada com exact max_size | Cada commit | IMPLEMENTED |
| **D1.2** | Teste de overflow: entrada > max_size → explicita rejeição | Cada commit | IMPLEMENTED |
| **D2.1** | Bounds check em desserialização | Code review | IMPLEMENTED (Polimata) |
| **D3.1** | Hash BLAKE2 antes/depois de persistência | Cada escrita | IMPLEMENTED |
| **D3.2** | Verificação de integridade na leitura | Cada leitura | IMPLEMENTED |
| **D4.1** | Índice é semanticamente válido pós-geração? | Cada commit | IMPLEMENTED (validator) |
| **D5.1** | Rastreabilidade: input → index → receipt → claim | Manual, trimestral | IMPLEMENTED (logs) |

### 2.4 Remediação

**R0: Isolamento**
- Falha de hash → Índice marcado como corrupto, fallback a versão anterior

**R1: Análise de Raiz**
- Bitflip em disco → Encaminhamento a SRE
- Schema desatualizado → Regeneração com codec atual

**R2: Evidência**
- Log de I/O com hash de entrada/saída
- Commit de fix de schema com teste

**R3: Comunicação**
- Se índice já usado: notificar clientes de reprocessamento necessário

### 2.5 Evolução

**I0: Frequência**
```
Se corrupção ocorreu > 1 vez em 3 meses:
→ Implementar mirror/redundância
```

**I1: Efetividade**
```
Hash D3.1 teria detectado quantas corrupções retro?
→ Se 100%, manter; se < 100%, investigar janelas cegas
```

---

## 3. T^7 / Verbovivo — Runtime/Convergência

### 3.1 Riscos Identificados

| Risco | Descrição | Severidade | Categoria |
|-------|-----------|------------|-----------|
| **R3.1** | Divergência de convergência (output para mesma entrada varia) | ALTA | R1 + R3 |
| **R3.2** | Falsificação de claim "42 fixed-point attractors" sem evidência | CRÍTICA | R4 (Claim) |
| **R3.3** | Função não-determinística por estado oculto | ALTA | R3 + R5 |
| **R3.4** | Falta de falsificador para nova hipótese | MÉDIA | R4 |

### 3.2 Gates de Prevenção

| Gate | Verificação | Status |
|------|-------------|--------|
| **G2** | Novo claim sobre convergência tem falsificador? | BLOCKING: CLOSURE_L9_T7_CONVERGENCE.md §5 |
| **G3** | Índice numérico (42, etc) é explicado como construção ou como teorema? | IMPLEMENTED: CLOSURE review |
| **G4** | Código T^7 está isolado de comportamento não-determinístico externo? | REFERENCE |

### 3.3 Gates de Detecção

| Gate | Teste | Frecuencia | Status |
|------|-------|-----------|--------|
| **D1.1** | Reproduzibilidade: mesmo input → mesmo output 100x | Cada commit | REFERENCE (falsificador não executável neste ambiente) |
| **D1.2** | Isolamento de estado: nenhum global mutável | Code review | IMPLEMENTED (Polimata) |
| **D3.1** | Claim vs realidade: "42" aparece em código como índice ou como prova? | Manual | IMPLEMENTED (docs search) |
| **D4.1** | Fechamento L9 é respeitado? | Manual | IMPLEMENTED (CLOSURE review) |
| **D5.1** | Rastreabilidade de hipótese → code → resultado → receipt | Manual | TOKEN_VAZIO (sem CI automático) |

### 3.4 Remediação

**R0: Isolamento**
- Divergência detectada → Parar execução, aguardar investigação

**R1: Análise de Raiz**
- Comparar outputs com versão anterior (git bisect)
- Verificar se determinismo externo foi alterado

**R2: Evidência**
- Registrar: commit, entrada, outputs divergentes, hardware

**R3: Comunicação**
- Alertar Arquiteto Técnico se falsificador falhou

### 3.5 Evolução

**I0: Frequência**
```
Se divergência ocorreu, aprofundar investigação no L9
```

**I2: Ameaça Emergente**
```
Nova metodologia de convergência em literatura?
→ Avaliar integração
```

---

## 4. Runtime Router — Execução/Roteamento

### 4.1 Riscos Identificados

| Risco | Descrição | Severidade | Categoria |
|-------|-----------|------------|-----------|
| **R4.1** | NULL dereference em selection por capacidade | ALTA | R3 |
| **R4.2** | Fallback incorreto (modo seguro não é seguro) | ALTA | R3 |
| **R4.3** | Race condition em inicialização | MÉDIA | R5 |
| **R4.4** | Watchdog não detecta hang (timeout não calibrado) | ALTA | R3 |

### 4.2 Gates de Prevenção

| Gate | Verificação | Status |
|------|-------------|--------|
| **G0** | Mudança de roteamento viola contrato de SLA? | REFERENCE |
| **G2** | Modo seguro é testável isoladamente? | IMPLEMENTED: test_safe_mode.c |

### 4.3 Gates de Detecção

| Gate | Teste | Frecuencia | Status |
|------|-------|-----------|--------|
| **D1.1** | Teste de NULL: capacidade=0 → fallback automático | Cada commit | IMPLEMENTED (AddressSanitizer) |
| **D2.1** | Bounds check em vetor de capacidades | Code review | IMPLEMENTED |
| **D3.1** | Receipt de cada roteamento (input → capacidade → handler) | Trimestral | TOKEN_VAZIO |
| **D4.1** | Invariante: sempre existe fallback seguro | Manual | IMPLEMENTED (Mapa review) |

---

## 5. Governança Documental (scripts/document_governance.py)

### 5.1 Riscos Identificados

| Risco | Descrição | Severidade | Categoria |
|-------|-----------|------------|-----------|
| **R5.1** | Documento desatualizado vs código | MÉDIA | R6 |
| **R5.2** | Ciclo de vida confuso (CANONICAL vs REFERENCE vs IMPLEMENTED) | MÉDIA | R4 |
| **R5.3** | Documento manual editado (deveria ser gerado) | ALTA | R6 |
| **R5.4** | Arquivo órfão (não referenciado, não documentado) | BAIXA | R6 |

### 5.2 Gates de Prevenção

| Gate | Verificação | Status |
|------|-------------|--------|
| **G3** | Novo documento tem entrada em INDEX.md? | IMPLEMENTED: checker |
| **G6** | Documento gerado é regenerável? | IMPLEMENTED: marca `@generated` |

### 5.3 Gates de Detecção

| Gate | Teste | Frecuencia | Status |
|------|-------|-----------|--------|
| **D1.1** | `python3 -m unittest tests.test_document_governance` | Cada PR | IMPLEMENTED |
| **D1.2** | `python3 scripts/document_governance.py --check --print-summary` | Cada commit | IMPLEMENTED (CI) |
| **D4.1** | Estado do documento (CANONICAL vs PENDING vs REFERENCE) | Mensal | IMPLEMENTED (governance script) |
| **D5.1** | Rastreabilidade: documento → código → evidência | Trimestral | IMPLEMENTED (cross-reference) |

---

## 6. Matriz de Priorização

| Subsistema | Risco Crítico | Prazo de Gate | Responsável |
|---|---|---|---|
| **ApkC** | R1.3 (buffer overflow) | 14 dias | ApkC |
| **ApkC** | R1.1 (não-determinismo) | Já implementado | ApkC |
| **Indexer** | R2.2 (corrupção disco) | 7 dias | Polimata |
| **Indexer** | R2.1 (truncamento) | Já implementado | Polimata |
| **T^7** | R3.2 (claim falso) | BLOCKING | Arquiteto |
| **T^7** | R3.1 (divergência) | REFERENCE | Arquiteto |
| **Router** | R4.4 (watchdog) | 21 dias | Mapa |
| **Router** | R4.1 (NULL) | Já implementado | Mapa |
| **Governança** | R5.1 (doc desatualizada) | Contínua | RafGitTools |

---

## Evolução Prevista

**2026-09 (Curto prazo):** Implementar gates D de subsistemas críticos  
**2026-10 (Médio prazo):** Automação CI para detection gates  
**2026-11 (Longo prazo):** Integração com terceiro para auditoria independente  

---

**Fechamento R3:**

```
F_ok   = Matriz de risco por subsistema definida; 20+ gates mapeados
F_gap  = Falsificadores de T^7 ainda em TOKEN_VAZIO; automação parcial
F_next = Implementar scripts/risk_gates_validator.py com gates D1-D5
```

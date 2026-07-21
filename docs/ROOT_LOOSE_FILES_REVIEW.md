# Revisão Técnica dos Arquivos Soltos da Raiz

> **Entrada canônica:** docs/AGENTES.md §5 (pipeline operacional VOID→VALIDATED) e §6 (escalação e conflito). Revisão técnica e rota de cada arquivo solto da raiz — matriz de decisão, diagnóstico por arquivo e ordem operacional P0–P3.

**Estado:** `REVIEW_REQUIRED`  
**Decisões máquina-legíveis:** `configs/root-file-decisions.v1.json`  
**Validador:** `scripts/validate_root_file_decisions.py`  
**Regra:** nenhum arquivo é apagado ou movido automaticamente.

## 1. Diagnóstico executivo

A raiz acumulou quatro classes diferentes de conteúdo:

1. **pontos de entrada legítimos**, que precisam permanecer na raiz;
2. **documentação de componente**, relacionada a código ativo;
3. **transcrições e experimentos**, misturando código, comandos e resultados;
4. **artefatos históricos/binários**, que exigem inventário, licença e cadeia de custódia.

O problema não é apenas estético. Arquivos heterogêneos na raiz aumentam:

- ambiguidade de fonte de verdade;
- risco de sobreclaim;
- dificuldade de encontrar dependências;
- possibilidade de executar conteúdo experimental como produção;
- duplicidade documental;
- perda de licença e proveniência durante reorganizações manuais.

## 2. Matriz de decisão

| Arquivo | Natureza | Evidência atual | Risco | Rota | Destino proposto |
|---|---|---|---|---|---|
| `safe-extended` | wrapper CLI | implementação ligada; runtime Termux pendente | baixo | `KEEP_AT_ROOT` | raiz |
| `README_RAFAELIA_ROOT_OPTIMIZER.md` | documentação de componente | implementação e exemplo localizados | baixo | `MOVE_PROPOSED` | `docs/components/RAFAELIA_ROOT_OPTIMIZER.md` |
| `RAFAELIA_MASTER_DOC.txt` | documento master histórico | claims mistos sem reconciliação integral | alto | `ARCHIVE_PROPOSED` | `docs/archive/RAFAELIA_MASTER_DOC_2024_2025.txt` |
| `RAFAELIA_COMPLETE_v4.zip` | arquivo binário | inventário interno ainda pendente | alto | `BINARY_ARTIFACT_REVIEW` | release/LFS/evidence store após auditoria |
| `Arduíno.txt` | dosiê AVR/registradores | referência técnica sem datasheet+bench completo | médio | `MOVE_AND_REFACTOR_PROPOSED` | `docs/hardware/AVR_ATMEGA328P_REGISTER_LAB.md` |
| `Arm64 Mixer leve criptografia.md` | transcrição com código SIMD | não compilado/validado nesta cadeia | alto | `SPLIT_AND_REFACTOR_PROPOSED` | `docs/experiments/ARM64_NEON_MIXER_REVIEW.md` |
| `L1.md` | heredoc C + comandos + resultados | benchmark sem proveniência suficiente | alto | `SPLIT_REQUIRED` | `tests/experiments/vectra_kernel_arm32/` |
| `RASBERY.MD` | brainstorm/roadmap de 50 itens | planejado, não matriz de implementação | médio | `CONVERT_TO_TYPED_BACKLOG` | `docs/roadmaps/RAFAELIA_BARE_METAL_LAB.md` |
| `big_test.sh` | orquestrador de testes | corrigido; execução integral pendente | médio | `FIX_THEN_MOVE_PROPOSED` | `scripts/big_test.sh` em PR posterior |

## 3. Análise por arquivo

### 3.1 `safe-extended`

É um ponto de entrada legítimo:

- usa `set -eu`;
- localiza a raiz pelo próprio caminho;
- verifica `python3`;
- delega diretamente para `scripts/safe_extended.py`;
- não usa `eval` nem carrega código dinâmico.

**Decisão:** permanecer na raiz para ergonomia CLI. Recebe decisão explícita porque exceção documentada é melhor que allow-list invisível.

### 3.2 `README_RAFAELIA_ROOT_OPTIMIZER.md`

O documento corresponde a:

- `raf_c_to_asm_root_optimizer.py`;
- `raiz_example.c`;
- backends ARM64 e x86-64;
- parser, AST mínimo, IR e relatório de auditoria.

O escopo é honestamente limitado a subconjunto C-like. Não é arquivo órfão; é documentação de componente no local errado.

**Decisão:** mover em PR dedicado, preservando histórico e atualizando links. Antes da movimentação, adicionar testes de parser, IR e assembly emitido.

### 3.3 `RAFAELIA_MASTER_DOC.txt`

É um documento histórico amplo, datado de 2024–2025, com:

- índice de 18 seções;
- declaração SPDX GPL-3.0-only;
- arquitetura, fórmulas, hardware e roadmap;
- métricas e estados apresentados como validados.

Ele tem valor como memória e fonte de hipóteses, mas não deve funcionar como estado material atual porque:

- hardware e ambiente podem ter mudado;
- métricas não estão todas ligadas a artefatos do commit atual;
- algumas fórmulas são modelos/analogias e precisam de classificação;
- licença/autoria deve ser confirmada por seção se houver conteúdo incorporado de terceiros.

**Decisão:** arquivar preservando integralidade e criar matriz `claim → fonte → implementação → prova → estado` antes de extrair conteúdo para documentos canônicos.

### 3.4 `RAFAELIA_COMPLETE_v4.zip`

É um binário versionado na raiz. A existência do ZIP não demonstra:

- origem de cada entrada;
- correspondência com os fontes atuais;
- ausência de executáveis/segredos;
- licença de cada componente;
- segurança de paths e links;
- adequação como release.

Foi criado `scripts/audit_zip_artifact.py`, que não extrai no filesystem e verifica:

- SHA-256 do arquivo e entradas seguras;
- CRC;
- path traversal e paths absolutos;
- nomes duplicados;
- symlinks;
- criptografia;
- limite por entrada e total;
- razão de compressão;
- extensões executáveis ou sensíveis.

**Decisão:** manter até materializar o inventário. Depois decidir entre GitHub Release, Git LFS ou armazenamento de evidência. Não usar como fonte canônica.

### 3.5 `Arduíno.txt`

Contém conteúdo técnico útil sobre ATmega328P:

- GPIO;
- Timer/PWM;
- UART;
- SPI;
- ADC;
- TWI/I²C;
- watchdog;
- EEPROM;
- interrupções;
- power modes.

Entretanto, o mesmo arquivo mistura:

- endereços e máscaras;
- código demonstrativo;
- números de ciclos/latência;
- frases de novidade;
- propostas experimentais sem bancada.

Há trechos que exigem revisão especialmente cuidadosa, como técnicas baseadas em EEPROM, ADC virtual e afirmações de frequência/consumo.

**Decisão:** preservar o original, extrair exemplos compiláveis e construir laboratório verificável. Cada técnica deve ter:

```text
claim → datasheet → código → flags → binário → medição → limite
```

### 3.6 `Arm64 Mixer leve criptografia.md`

É uma transcrição de orientação com um kernel AArch64 NEON. Pontos positivos:

- distingue ARMv7/A32 de AArch64;
- tenta manter dados em registradores;
- discute ILP, prefetch e afinidade.

Gaps concretos:

- o registrador usado como contador é alterado por `subs`, mas aparece como input-only no inline ASM;
- `posix_memalign` não é verificado;
- memória é consumida sem inicialização explícita;
- não há resultado persistido ou teste de correção;
- benchmark não registra distribuição, ambiente ou baseline;
- “criptográfico” não é justificável sem análise de segurança.

**Decisão:** renomear conceitualmente para kernel vetorial experimental, corrigir constraints e separar código, teste e relatório.

### 3.7 `L1.md`

Mistura três artefatos distintos:

1. heredoc que cria `vectra_kernel_arm32.c`;
2. comandos de compilação/execução;
3. números de bandwidth copiados para o mesmo documento.

Os resultados não registram:

- commit;
- ABI/CPU;
- compilador e versão;
- flags completas;
- número de repetições;
- warmup;
- mediana/p95;
- hash do binário;
- validação de saída.

**Decisão:** dividir em fixture C, runner shell e JSON de benchmark. O original permanece como audit trail até a migração estar provada.

### 3.8 `RASBERY.MD`

É um backlog conceitual com 50 itens distribuídos entre:

- MCU/registradores;
- Raspberry Pi/ARM;
- QEMU/Android/Termux;
- edge AI;
- produto, CI e benchmark.

O próprio documento reconhece a falta de compilação, medição e separação entre hipótese e técnica provada.

**Decisão:** converter para backlog tipado com campos:

```text
id | domínio | estado | prioridade | responsável | dependências
   | repositório | critério de aceite | evidência | rollback
```

Nenhum item deve aparecer como feature implementada apenas por estar listado.

### 3.9 `big_test.sh`

É uma automação útil porque preserva:

- ambiente;
- ferramentas;
- comandos;
- logs por gate;
- artefatos;
- resumo agregado.

Foi corrigido nesta tranche:

- removido mascaramento do build obrigatório;
- `FAIL required` produz exit 1;
- `TOKEN_VAZIO required` produz exit 2;
- adicionados testes de governança;
- adicionado catálogo documental;
- adicionada cobertura das decisões de raiz.

**Decisão:** manter temporariamente na raiz para compatibilidade; mover para `scripts/big_test.sh` somente após atualizar todas as chamadas.

## 4. Ordem operacional

### P0 — segurança e integridade

1. executar auditor do ZIP;
2. validar cobertura das decisões de raiz;
3. corrigir qualquer `QUARANTINE_REVIEW`;
4. garantir que nenhum segredo apareça nos relatórios.

### P1 — falso verde e prova

1. executar `big_test.sh` no Termux;
2. resolver `FAIL` antes de `TOKEN_VAZIO`;
3. materializar catálogo e grafo;
4. anexar hashes e logs ao PR.

### P2 — decomposição

1. `L1.md` → fixture + runner + resultado;
2. mixer ARM64 → código corrigido + teste + benchmark;
3. Arduino → módulos de registradores + datasheet + bancada.

### P3 — documentação e produto

1. mover README do otimizador;
2. arquivar documento master com matriz de claims;
3. converter roadmap de 50 tópicos;
4. decidir o armazenamento do ZIP.

## 5. Comandos

```sh
python3 -m unittest \
  tests.test_document_governance \
  tests.test_audit_repository_structure \
  tests.test_validate_root_file_decisions \
  tests.test_audit_zip_artifact

python3 scripts/validate_root_file_decisions.py \
  --write results/root-file-decisions-validation.json

python3 scripts/audit_zip_artifact.py RAFAELIA_COMPLETE_v4.zip \
  --write results/document-governance/rafaelia-complete-v4-zip.json

python3 scripts/document_governance.py --write --print-summary
python3 scripts/document_governance.py --check --print-summary
```

## 6. Critério para movimentar

Um arquivo só deve ser movido quando:

```text
hash atual confere
∧ decisão cobre o blob atual
∧ destino está definido
∧ links/dependências foram encontrados
∧ testes aplicáveis passam
∧ rollback está descrito
∧ aprovação humana existe
```

Até lá, a raiz continua imperfeita, porém governada e auditável.

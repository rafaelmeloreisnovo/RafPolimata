# Governança Documental e Indexação Operacional — RafPolimata

> **Entrada canônica:** docs/AGENTES.md §3 (estados canônicos) e §7 (CI gates — document governance). Motor de governança documental — arquitetura em 6 níveis L0–L5, estados de ciclo de vida, graus de evidência E0–E4 e rotas operacionais por prioridade.

**Estado:** `IMPLEMENTED` no código; catálogo material depende da execução no checkout.  
**Política:** `configs/document-governance.v1.json`  
**Executor:** `scripts/document_governance.py`  
**Contrato de registro:** `schemas/document-record.v1.schema.json`

## 1. Objetivo

Esta camada transforma arquivos dispersos em um sistema governável sem confundir:

```text
arquivo existente ≠ arquivo compreendido ≠ arquivo indexado
                  ≠ evidência ≠ documento canônico ≠ verdade runtime
```

O sistema não move, apaga, consolida nem promove conteúdo automaticamente. Ele produz identidade, relações, risco, fila de decisão e evidência revisável para que cada mudança estrutural seja deliberada e reversível.

## 2. Invariantes

1. **Proveniência antes de reorganização:** nenhum arquivo é movido sem caminho original, hash e decisão registrada.
2. **Existência não é PASS:** presença física concede no máximo evidência `E0`.
3. **Índice não é prova:** uma referência melhora encontrabilidade, mas não demonstra execução.
4. **Saída determinística:** o mesmo commit e política devem produzir o mesmo catálogo.
5. **Segredo não é relatório:** o detector registra apenas o identificador do risco, nunca o valor encontrado.
6. **Revisão humana para destruição:** exclusão, fusão ou arquivamento exigem PR dedicado.
7. **Separação de funções:** política, executor, catálogo, fila e índice são artefatos distintos.
8. **Falha fechada no crítico:** chave privada ou keystore potencial bloqueia o gate.
9. **Revisão graduada no não crítico:** conteúdo órfão, antigo ou duplicado entra em fila; não é apagado.
10. **TOKEN_VAZIO preservado:** ausência de histórico, dono ou prova não é preenchida por inferência.

## 3. Arquitetura em seis níveis

| Nível | Função | Entrada | Saída |
|---|---|---|---|
| `L0` | Estrutura física | árvore versionada | caminhos, tamanho, tipo |
| `L1` | Identidade | bytes do arquivo | SHA-256 e hash normalizado de texto |
| `L2` | Relações | links Markdown e caminhos inline | grafo dirigido e referências quebradas |
| `L3` | Governança | área, dono lógico, classificação e histórico Git | ciclo de vida e revisão temporal |
| `L4` | Qualidade e risco | evidência, duplicidade, sensibilidade e conectividade | notas 0–100 e rota |
| `L5` | Operação | catálogo classificado | índice, fila de revisão e decisão de promoção |

A invariante é:

```text
Documento governável
= identidade × contexto × relação × temporalidade × responsabilidade × evidência
```

## 4. Registro por arquivo

Cada arquivo recebe um registro `raf.document-record.v1` com, no mínimo:

- caminho relativo estável;
- SHA-256 dos bytes;
- hash normalizado para candidato de duplicidade textual;
- tamanho e contagem de linhas quando textual;
- classe de mídia;
- área e responsável lógico;
- classificação de dados;
- ciclo de vida;
- grau de evidência;
- referências de entrada e saída;
- último commit e data observável;
- idade e intervalo de revisão;
- flags de sensibilidade e política;
- grupos de duplicidade;
- referências quebradas;
- notas de qualidade e risco;
- rota operacional e justificativa.

O registro é descritivo. `quality_score=100` não transforma hipótese em prova científica e `risk_score=0` não certifica segurança.

## 5. Ciclo de vida

| Estado | Uso | Promoção ou saída |
|---|---|---|
| `CANONICAL` | entrada oficial, política ou índice | revisão curta e mudança controlada |
| `ACTIVE` | documento ou código em uso | manter relações, dono e revisão |
| `SUPPORTING` | ativo auxiliar | provar dependência ou arquivar |
| `AUDIT` | contrato, manifesto ou trilha | manter integridade e reproducibilidade |
| `EVIDENCE` | teste, prova ou resultado | ligar ao comando e ao commit |
| `GENERATED` | derivado por ferramenta | regenerar, não editar manualmente |
| `ARCHIVE_CANDIDATE` | sem uso atual comprovado | decisão humana e preservação |
| `QUARANTINE` | risco de exposição ou integridade | remover da distribuição e investigar |

## 6. Graus de evidência

| Grau | Critério mínimo | Limite |
|---|---|---|
| `E0` | arquivo existe e possui identidade | nenhuma relação comprovada |
| `E1` | arquivo aparece em índice ou recebe referência | encontrabilidade, não validade |
| `E2` | contém estado explícito ou contrato documental | sem execução obrigatória |
| `E3` | relacionado a teste, prova, resultado ou workflow | execução ainda pode estar pendente |
| `E4` | evidência contém comando e hash no corpo auditado | runtime precisa ser validado conforme domínio |

A classificação é conservadora: o algoritmo só sobe quando encontra condições observáveis.

## 7. Rotas operacionais

A prioridade evita que um problema de baixa urgência esconda risco maior:

| Prioridade | Rota | Ação |
|---:|---|---|
| 1 | `QUARANTINE_REVIEW` | bloquear; investigar possível chave privada/keystore |
| 2 | `SENSITIVITY_REVIEW` | revisar possível dado sensível sem expor valor |
| 3 | `CANONICAL` | manter sob revisão curta |
| 4 | `DUPLICATE_REVIEW` | comparar origem, licença, autoria e dependências |
| 5 | `ROOT_REVIEW` | decidir índice ou destino de arquivo solto na raiz |
| 6 | `OWNER_REQUIRED` | atribuir área e papel responsável |
| 7 | `REFERENCE_REPAIR` | corrigir link local quebrado |
| 8 | `LINK_REQUIRED` | conectar a índice ou documento justificando uso |
| 9 | `REVIEW_STALE` | revisar conteúdo cujo prazo venceu |
| 10 | `INDEXED` | manter e acompanhar drift |

Nenhuma dessas rotas autoriza exclusão automática.

## 8. Qualidade e risco

### 8.1 Qualidade

A nota de qualidade combina sinais verificáveis:

- indexação;
- referências de entrada e saída;
- área e responsável lógico;
- histórico Git disponível;
- grau de evidência;
- revisão temporal em dia;
- penalidades por link quebrado, sensibilidade e violação de política.

### 8.2 Risco

A nota de risco aumenta com:

- ausência de índice;
- ausência de referência de entrada;
- revisão vencida;
- referências quebradas;
- sensibilidade;
- arquivo de raiz fora da política;
- duplicidade exata ou normalizada;
- ausência de área.

As notas servem para ordenar a fila. Não são certificação ISO, NIST, LGPD ou segurança formal.

## 9. Governança temporal

A data de geração é obtida do commit atual. Isso garante que o mesmo commit produza a mesma referência temporal.

A política define intervalos por área:

| Área | Revisão padrão |
|---|---:|
| entradas canônicas | 30 dias |
| workflows | 60 dias |
| ApkC, automação, runtime e testes | 90 dias |
| documentação | 180 dias |
| métodos de baixo nível | 180 dias |
| resultados e dados | 365 dias |
| assets | 365 dias |

`review_due=true` significa que a data do último commit ultrapassou o intervalo configurado. Não significa que o conteúdo está errado; significa que precisa de nova decisão explícita.

## 10. Governança de dados

### 10.1 Classificações

| Classe | Uso |
|---|---|
| `PUBLIC` | documentação e ativos destinados a publicação |
| `INTERNAL` | código, configuração, teste e evidência operacional |
| `RESTRICTED` | informação que exige autorização e controles adicionais |

A classificação padrão é por área e pode ser refinada na política.

### 10.2 Sensibilidade

O scanner procura nomes e padrões compatíveis com:

- chave privada;
- keystore/JKS;
- material PEM;
- arquivos `.env`;
- nomes relacionados a credenciais ou segredos;
- atribuições textuais candidatas a segredo.

O relatório contém somente flags como `private_key_block`; nunca copia o segredo ou trecho correspondente.

## 11. Grafo documental

O grafo é dirigido:

```text
source --references--> target
```

São reconhecidos:

- links Markdown locais;
- caminhos entre crases quando parecem arquivos;
- caminhos relativos ao documento ou à raiz.

Links externos são ignorados pelo grafo local. Referências a diretórios existentes não são marcadas como quebradas.

`inbound_references=0` identifica arquivos que nenhum outro corpo versionado alcança. Isso é sinal de possível isolamento, não prova de inutilidade.

## 12. Duplicidade

### Exata

Mesmo SHA-256 byte a byte. Recebe `DUPLICATE_REVIEW`, exceto arquivos vazios, que não são agrupados automaticamente.

### Normalizada

Textos com pelo menos 200 caracteres recebem hash após normalização de espaços e linhas. O resultado é apenas candidato: diferenças de autoria, licença, contexto e semântica ainda precisam de revisão humana.

## 13. Saídas

```text
results/document-governance/summary.json
results/document-governance/catalog.jsonl
results/document-governance/relations.jsonl
results/document-governance/duplicates.json
results/document-governance/review-queue.json
docs/generated/DOCUMENT_GOVERNANCE_INDEX.md
docs/generated/DOCUMENT_REVIEW_QUEUE.md
```

Os arquivos em `docs/generated/` e `results/document-governance/` são derivados. Sua fonte é o script + política + commit.

## 14. Operação

### Gerar catálogo

```sh
python3 scripts/document_governance.py --write --print-summary
```

### Verificar drift

```sh
python3 scripts/document_governance.py --check --print-summary
```

### Gate estrito

```sh
python3 scripts/document_governance.py --check --strict
```

O modo normal falha apenas em bloqueadores críticos ou índice canônico ausente. O modo `--strict` também falha enquanto existir qualquer item na fila de revisão.

### Testes

```sh
python3 -m unittest tests.test_document_governance
```

### Safe Extended local

```sh
sh safe-extended run .github/workflows/document-governance.yml
```

## 15. Eficiência operacional

O executor evita uma chamada `git log` por arquivo. Ele lê até o limite configurado de commits em uma única passagem e guarda a primeira ocorrência de cada caminho.

Complexidade dominante:

```text
hash dos arquivos: O(total de bytes)
relações textuais: O(total de texto lido)
histórico Git: O(commits + nomes observados)
ordenatção: O(n log n)
```

O limite padrão de leitura textual é 2 MB por arquivo; arquivos maiores continuam recebendo SHA-256 e metadados, mas não são analisados semanticamente.

## 16. Responsabilidades lógicas

| Papel | Responsabilidade |
|---|---|
| `repository-maintainer` | decisões de raiz e promoção canônica |
| `documentation-governance` | índices, relações e revisão editorial |
| `schema-governance` | política, contratos e compatibilidade de versão |
| `evidence-custodian` | resultados, hashes, origem e retenção |
| `ci-governance` | gate e drift automatizado |
| mantenedores de área | validade técnica do conteúdo |
| `quality-assurance` | testes positivos, negativos e regressivos |
| humano autorizador | mover, apagar, consolidar, restringir ou publicar |

Papéis são responsabilidades funcionais, não identidades pessoais fixas.

## 17. Processo de incorporação de arquivo solto

```text
1. detectar
2. calcular identidade
3. classificar área e dados
4. localizar referências
5. verificar duplicidade
6. medir temporalidade e evidência
7. atribuir rota
8. revisar conteúdo e proveniência
9. atualizar índice e relações
10. somente então mover, consolidar ou arquivar em PR dedicado
```

Critérios mínimos para promoção a `INDEXED`:

- área definida;
- responsável lógico definido;
- ao menos uma referência de entrada;
- nenhum bloqueador de sensibilidade;
- referências locais íntegras;
- decisão de ciclo de vida explícita.

## 18. Integração com auditorias existentes

`scripts/audit_repository_structure.py` continua como auditoria estrutural `L0`: diretórios, raiz, links e RAF index.

`scripts/document_governance.py` cobre `L1–L5`: identidade, grafo, responsabilidade, temporalidade, evidência, risco e fila.

```text
estrutura física correta
∧ catálogo governado
∧ evidência honesta
= condição necessária, não suficiente, para promoção
```

## 19. Critério de conclusão

A governança documental alcança `PASS` somente quando:

- todas as entradas canônicas existem;
- não há material sensível bloqueante;
- não há fila de revisão;
- saídas geradas correspondem ao commit;
- testes do motor passam;
- mudança estrutural possui PR e rollback possível.

Até lá, o estado adequado é `REVIEW_REQUIRED` ou `FAIL`, nunca um PASS narrativo.

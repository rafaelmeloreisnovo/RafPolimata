---
name: Copilot — Internal Custody Ledger
about: Implementar ledger hash-encadeado para a cadeia de custódia informacional
labels: ''
assignees: 'rafaelmeloreisnovo'
---

# [Copilot] Implementar ledger hash-encadeado da cadeia de custódia informacional

## Ordem de leitura

Leia integralmente antes de editar:

1. `.github/copilot-instructions.md`
2. `docs/INFO_DYNAMICS_INTERNAL_TRACEABILITY.md`
3. `docs/AGENTES.md`
4. `scripts/apkc_validate.sh`
5. `scripts/validate_coherence_protocol.py`
6. `tools/raf_codegen_select_test.c`
7. `rafaelia/verbovivo.c`
8. `.github/workflows/ci.yml`

Este issue é ordem de execução. Não finalize com plano, pseudocódigo, TODO, stubs ou falsa conclusão.

## Objetivo

Implementar um ledger determinístico e tamper-evident que registre cada transformação relevante do RafPolimata e ligue:

- entrada;
- saída;
- código executado;
- versão do repositório;
- toolchain;
- parâmetros e seed;
- ambiente;
- resultado;
- evento anterior.

O ledger fecha `L0` — proveniência — e `L1` — reprodutibilidade. Não deve alegar fechamento de `L2`, validade empírica externa.

## Entregáveis obrigatórios

### 1. Schema canônico

Criar `schemas/internal_custody_event.schema.json` com:

- [ ] `event_version`
- [ ] `event_id`
- [ ] `parent_event_id`
- [ ] `repository`
- [ ] `commit_sha`
- [ ] `path`
- [ ] `blob_sha`
- [ ] `symbol`
- [ ] `toolchain`
- [ ] `parameters`
- [ ] `seed`
- [ ] `environment`
- [ ] `input_sha256`
- [ ] `output_sha256`
- [ ] `stdout_hash`
- [ ] `stderr_hash`
- [ ] `exit_code`
- [ ] `timestamp_utc`
- [ ] `result`

Campos desconhecidos devem usar `TOKEN_VAZIO`. Nenhum dado pode ser inferido silenciosamente.

### 2. Serialização determinística

A representação canônica deve definir e testar:

- [ ] UTF-8;
- [ ] chaves em ordem lexical;
- [ ] ausência de espaços não significativos;
- [ ] representação estável de números;
- [ ] tratamento explícito de `null` e `TOKEN_VAZIO`;
- [ ] regra de newline final;
- [ ] mesmos campos lógicos geram exatamente os mesmos bytes.

### 3. Escritor hosted

Criar `tools/internal_custody_event.py`, somente com Python stdlib, capaz de:

- [ ] calcular hashes SHA-256 de entradas e saídas;
- [ ] obter `commit_sha` por Git;
- [ ] obter `blob_sha` quando o artefato estiver versionado;
- [ ] carregar o evento anterior;
- [ ] construir `parent_event_id`;
- [ ] calcular `event_id` sobre bytes canônicos;
- [ ] anexar evento em JSONL;
- [ ] usar escrita atômica ou lock para evitar corrupção concorrente;
- [ ] retornar código de saída determinístico por classe de erro.

### 4. Verificador independente

Criar `tools/verify_internal_custody.py` para validar:

- [ ] schema;
- [ ] hash de cada evento;
- [ ] encadeamento `parent_event_id`;
- [ ] hashes dos artefatos presentes;
- [ ] campos obrigatórios;
- [ ] ordem da cadeia;
- [ ] adulteração de um byte;
- [ ] evento removido;
- [ ] evento duplicado;
- [ ] parent incorreto;
- [ ] truncamento do JSONL.

O verificador não pode confiar no escritor.

### 5. Relação de custódia

Implementar a relação:

```text
E_n = SHA256(canonical(E_n sem event_id) || E_(n-1).event_id)
```

O evento gênese deve usar valor explícito e documentado para `parent_event_id`.

### 6. Integração com gates existentes

Integrar emissão de eventos em:

- [ ] `scripts/apkc_validate.sh`;
- [ ] `scripts/validate_coherence_protocol.py`;
- [ ] teste compilado de `tools/raf_codegen_select_test.c`;
- [ ] build e recall de `rafaelia/verbovivo.c`.

Cada gate deve produzir `PASS`, `FAIL` ou `TOKEN_VAZIO`.

### 7. Testes obrigatórios

- [ ] replay determinístico;
- [ ] alteração de um byte invalida `event_id`;
- [ ] troca da ordem de eventos quebra a cadeia;
- [ ] `parent_event_id` incorreto é rejeitado;
- [ ] campo ausente falha ou vira `TOKEN_VAZIO` conforme schema;
- [ ] ledger vazio cria evento gênese;
- [ ] duas escritas concorrentes não corrompem o arquivo;
- [ ] caminho com UTF-8;
- [ ] arquivo vazio;
- [ ] artefato maior que a memória disponível, com hashing streaming;
- [ ] execução em Termux;
- [ ] execução em Linux CI.

### 8. CI

Adicionar job:

```text
validate-internal-custody-ledger
```

O job deve:

1. gerar uma cadeia íntegra de fixture;
2. validar a cadeia;
3. adulterar uma cópia;
4. confirmar falha com exit code diferente de zero;
5. publicar ledger, manifesto e relatório como artefatos;
6. não depender de rede nem de corpus privado.

## Critérios de aceite

- [ ] schema versionado;
- [ ] escritor determinístico;
- [ ] verificador independente;
- [ ] hash chain completa;
- [ ] detecção real de adulteração;
- [ ] gates F0–F6 integrados;
- [ ] CI demonstra caso positivo e negativo;
- [ ] `TOKEN_VAZIO` preserva lacunas reais;
- [ ] documentação não confunde proveniência com verdade externa;
- [ ] nenhuma dependência de rede;
- [ ] compatível com Termux/Android e Linux CI;
- [ ] documentação registra comandos realmente executados e hashes obtidos.

## Fora de escopo

- assinatura digital com chave privada;
- timestamp por autoridade externa;
- prova de validade científica `L2`;
- substituição de bibliografia externa.

Esses itens pertencem a fase posterior, após fechamento íntegro de `L0` e `L1`.

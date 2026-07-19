# Dinâmica Informacional e Rastreabilidade Interna

**Estado**: `REFERENCE`
**Autoria**: Rafael Melo Reis
**Origem documental**: Sessão Claude `claude/info-dynamics-internal-traceability-nxbznc`
**Data**: 2026-07-11
**Escopo de referências**: cadeia interna do código para provar transformação e linhagem; afirmações sobre o mundo externo continuam exigindo evidência externa verificável

---

## Parte I — Framework Epistemológico

> A extração de uma dinâmica evolutiva a partir de um sistema informacional, sem o
> suporte de uma cadeia de custódia bibliográfica, exige a substituição da autoridade
> externa, no domínio da transformação interna, por um modelo de rastreabilidade — um
> protocolo que demonstre como a informação se transforma e se autentica dentro do
> próprio sistema.

### O Paradoxo da Cadeia de Custódia

Em ciência, referências bibliográficas documentam a origem e a linhagem pública de
ideias, métodos e resultados. Cada citação pode funcionar como um elo verificável entre
o conhecimento atual e registros anteriores. Sem essa cadeia, uma afirmação externa se
torna um vestígio órfão: pode até ser verdadeira, mas sua origem, contexto e integridade
não podem ser auditados.

Este documento não usa episódios históricos sem fonte como demonstração do próprio
argumento. Quando uma afirmação depender de fatos externos e a fonte não estiver
registrada, o estado correto é `TOKEN_VAZIO`, e não certeza retórica.

### A Dinâmica Evolutiva Sem Cadeia de Custódia Externa

Mesmo sem referências externas, um sistema de informação pode exibir dinâmica
evolutiva operando sob três princípios:

**1. A Informação como Sistema Dinâmico**

A dinâmica evolutiva é a mudança no estado de um sistema ao longo do tempo. A
informação não é tratada como objeto estático, mas como estado que evolui por meio de
interações. Essas interações podem ser determinísticas ou estocásticas; quando forem
estocásticas, sementes, parâmetros e ambiente devem fazer parte do registro de custódia.

**2. Seleção Informacional**

Variantes mais consistentes com os invariantes, restrições e objetivos do sistema podem
persistir, ser reutilizadas ou receber maior peso; variantes incompatíveis podem ser
rejeitadas. A analogia com seleção natural é operacional, não uma alegação de identidade
com evolução biológica.

**3. Emergência de Nichos Informacionais**

A interação entre entidades processadoras pode produzir regiões estáveis do espaço de
estados. Esses nichos podem sustentar padrões recorrentes e uma proto-autopoiese
computacional: manutenção e regeneração de estado segundo regras internas explícitas.

### Axiomas de Validade Epistêmica Interna

| Axioma | Descrição |
|--------|-----------|
| **Consistência interna** | A transformação deve respeitar invariantes, tipos, limites e contratos do sistema |
| **Poder preditivo interno** | O sistema deve produzir padrões ou resultados testáveis contra entradas e estados posteriores |
| **Rastreabilidade procedimental** | Cada transformação deve registrar entrada, saída, agente, versão, parâmetros e vínculo com o evento anterior |
| **Reprodutibilidade** | Mesma entrada, mesma versão e mesmos parâmetros devem reproduzir o resultado ou uma distribuição declarada |
| **Falsificabilidade operacional** | Deve existir condição explícita que faça um gate falhar e rejeite a transformação |

### Limite Epistemológico — Proveniência não é Verdade Externa

A cadeia interna prova **o que o sistema fez**, **com quais dados**, **em qual versão** e
**com qual resultado**. Ela não prova, sozinha, que uma afirmação corresponde ao mundo
externo.

| Nível | O que pode ser demonstrado | Evidência mínima |
|------:|-----------------------------|-----------------|
| `L0` | Integridade e linhagem do artefato | hashes, commit, parent hash, manifesto |
| `L1` | Reprodutibilidade e coerência operacional | testes, seeds, parâmetros, ambiente, gates |
| `L2` | Validade empírica externa | observações, datasets independentes, bibliografia e comparação externa |

O RafPolimata pode fechar `L0` e `L1` internamente. `L2` permanece `TOKEN_VAZIO` até
que evidência externa adequada seja anexada.

---

## Parte II — Implementação no RafPolimata

Esta seção mapeia princípios epistemológicos para arquivos, símbolos e comportamentos
do repositório. O código e os registros internos implementam a cadeia de transformação;
não substituem bibliografia quando a proposição depende do mundo externo.

### Regra de Âncora Estável

Referências `arquivo:linha` são pistas de navegação e podem mudar após commits. A âncora
canônica de custódia deve ser composta por:

```text
repository + commit_sha + path + blob_sha + symbol + input_hash + output_hash
```

Para execuções estocásticas, acrescentar:

```text
seed + parameters + toolchain + environment_manifest
```

### Mapeamento de Princípios → Código

| Princípio | Implementação | Localização |
|-----------|--------------|-------------|
| Consistência interna | `phi_fst()` — métrica Q16 de coerência KAM-7 | `Apkc/coherence.h:19` |
| Seleção informacional | `codegen_select()` — seleção determinística de variante de instrução baseada nos bytes já emitidos | `Apkc/codegen_select.h:15` |
| Nichos informacionais (42 atratores) | `t7_step()` — evolução do estado do toroid T^7 com `omega_inv[3]` | `Benchmark/raf_toroid.h:78` |
| Proto-autopoiese computacional | `T7State.omega_inv[3]` — acumuladores de trajetória do estado | `Benchmark/raf_toroid.h:13` |
| Rastreabilidade procedimental | `_codegen_variant_log[256]` + printout `[phi=... attractor=...]` | `Apkc/apkc.c:277` |
| Poder preditivo interno | `vv_recall()` — recuperação dos N engrams mais ressonantes com o contexto | `rafaelia/verbovivo.c:316` |
| Identidade estrutural | `FiberHash` 256 bits — assinatura estrutural de quatro lanes djb2 | `rafaelia/fiber_h.h` |
| Diversidade informacional | `fiber_hash_distance()` — distância de Hamming `[0, 256]` | `rafaelia/fiber_h.h:74` |
| Custódia procedimental | `instruction_trace[]` + `audit_hash` em JSON | `raiz_audit_arm64.json` |
| Métricas Ω da fonte | `RafCtx.omega_phi_q16`, `omega_attractor`, `omega_path` | `raf_compile.h:85–91` |
| Auditoria de compliance | `vv_audit()` — emissão de métricas e verificações declaradas | `rafaelia/verbovivo.c:229` |
| Provas por sessão | `Apkc/proofs/CHAIN_OF_CUSTODY_*.md` — gates F0–F6 com `TOKEN_VAZIO` | `Apkc/proofs/` |

### As Cinco Fases da Cadeia de Custódia Interna

O pipeline RafPolimata produz rastreabilidade procedimental em cinco fases. Cada fase
transforma a informação e deve registrar a assinatura do estado resultante.

```text
Fonte (bytes) → [Fase 1] → [Fase 2] → [Fase 3] → [Fase 4] → [Fase 5] → Engram
```

#### Fase 1 — Classificação Ω da Fonte

**Arquivo**: `raf_compile.h:85–91` · `Apkc/omega_classifier.h:81`

A fonte é classificada segundo a taxonomia Ω de cinco caminhos:
`PROCESSUAL`, `VOID`, `FORGOTTEN`, `MENOSPREZADO`, `URGENT`.

Métricas extraídas: `entropy_milli` `[0, 8000]`, `phi_q16`, `attractor`
`[0, 41]`, `flags` e `path`.

A classificação Ω determina a rota de dispatch antes da transformação posterior.

#### Fase 2 — Emissão ARM64 com Seleção de Variante

**Arquivo**: `Apkc/apkc.c:376` · `Apkc/codegen_select.h:15`

Para famílias de equivalência, como `MOV Xd,Xm`, `ORR Xd,XZR,Xm`, `ADD Xd,Xm,#0`
e `SUB Xd,Xm,#0`, `codegen_select()` escolhe deterministicamente a variante usando
o estado classificado dos bytes já emitidos.

Mesma fonte, mesmos bytes anteriores e mesma versão devem produzir a mesma escolha. A
seleção é auditável via `_codegen_variant_log[256]` e impressa como:

```text
[codegen mov_family: total=N orr=N add0=N sub0=N]
```

Esta é a implementação computacional da seleção informacional entre variantes já
declaradas semanticamente equivalentes.

#### Fase 3 — Invariante de Coerência Geométrica

**Arquivo**: `Apkc/coherence.h:19` · `Apkc/apkc.c:1754–1774`

Após `build_apk()`, `phi_fst()` computa uma métrica interna do APK produzido:

```text
phi_fst = (1 − H_norm) × C_norm   em Q16 [0, 65536]

H_norm = unique_byte_count / 256
C_norm = KAM-7 dot product / norm(freq)
```

O resultado é impresso como:

```text
[phi=0.3142 attractor=17]
```

Essa linha é uma assinatura métrica interna, rastreável e reprodutível. Ela não deve ser
interpretada, isoladamente, como prova de validade externa ou qualidade universal.

#### Fase 4 — Motor de Convergência T^7

**Arquivo**: `Benchmark/raf_toroid.h:46,78` · `rafaelia/verbovivo.c:439`

O toroid T^7 evolui em sete dimensões: entropia, coerência, intenção, observação,
ruído, transmutação e memória. A cada passo:

- `omega_inv[3]` acumula checksums XOR da trajetória;
- `attractor` evolui por `(attractor + omega + u_t) % 42`;
- `perm_class` representa setor hexagonal e nível Fibonacci.

Os 42 valores de `attractor` formam nichos discretos do espaço de estados. A
propriedade de estabilidade deve ser demonstrada por testes de convergência; não deve
ser presumida apenas pela existência do operador módulo 42.

Checksums XOR fornecem detecção limitada de alteração, mas não equivalem a assinatura
criptográfica. Para resistência forte contra fabricação retroativa, o ledger deve usar
hash encadeado e, quando necessário, assinatura digital.

#### Fase 5 — Memória de Engrams e Recall

**Arquivo**: `rafaelia/verbovivo.h:48` · `rafaelia/verbovivo.c:316`

`VerbVivoState` mantém um ring buffer de 64 engrams (`memory[VV_MEM_SIZE]`), cada
um com `FiberHash` e `VVHyperVec`.

`vv_recall()` recupera os N engrams mais ressonantes com o contexto acumulado:

```c
recall_score = phi_weight * cos(query, e.vec)
             + (1 - phi_weight) * (1 - hamming / 256)
```

Esse mecanismo implementa recuperação contextual interna. Seu poder preditivo só fica
demonstrado quando existe um teste prospectivo com alvo, métrica e baseline definidos.

---

## Parte III — Verificação da Cadeia

A validade operacional do framework depende de comandos executáveis e resultados
registrados.

```bash
# Fase 3 — verificar compilação freestanding do ApkC:
clang -target aarch64-linux-gnu -fsyntax-only -nostdlib -nostdinc \
  -ffreestanding -I Apkc Apkc/apkc.c

# Fase 2 — compilar e executar a prova hosted do seletor:
cc -std=c11 -O2 -Wall -Wextra tools/raf_codegen_select_test.c \
  -o /tmp/raf_codegen_select_test
/tmp/raf_codegen_select_test

# Fase 5 — compilar o VerbVivo e testar recall sobre um artefato existente:
gcc -std=c11 -O2 -I. -IBenchmark -DVERBOVIVO_MAIN \
  rafaelia/verbovivo.c -lm -o /tmp/verbovivo
/tmp/verbovivo -r 3 < out.apk

# Gates F0–F6 da cadeia procedimental:
sh scripts/apkc_validate.sh

# Contrato canônico de coerência:
python3 scripts/validate_coherence_protocol.py
```

Cada execução deve registrar pelo menos:

```text
commit_sha, blob_sha, command, toolchain, environment,
input_sha256, output_sha256, exit_code, stdout_hash, stderr_hash
```

Os gates da CI são evidência computacional de reprodutibilidade no ambiente declarado.
Eles não constituem, por si sós, prova de validade científica externa.

### Contrato de Evento Encadeado — alvo de implementação

Estado atual: `PROPOSED` até existir schema, escritor, verificador e gate CI dedicados.

```json
{
  "event_version": 1,
  "event_id": "sha256(canonical_event_without_event_id)",
  "parent_event_id": "sha256(previous_event)",
  "repository": "rafaelmeloreisnovo/RafPolimata",
  "commit_sha": "<40-hex>",
  "path": "<artifact-path>",
  "blob_sha": "<git-blob-sha>",
  "symbol": "<function-or-stage>",
  "toolchain": "<compiler-and-version>",
  "parameters": {},
  "seed": null,
  "input_sha256": "<64-hex>",
  "output_sha256": "<64-hex>",
  "exit_code": 0,
  "timestamp_utc": "<RFC3339>",
  "result": "PASS"
}
```

A relação de custódia é:

```text
E_n = SHA256(canonical(E_n sem event_id) || E_(n-1).event_id)
```

Qualquer campo ausente deve ser representado explicitamente como `TOKEN_VAZIO`; não
deve ser inventado.

---

## Diagrama de Rastreabilidade

```text
Fonte (arquivo ou stream)
      │
      ▼ [raf_compile.h — omega_classify()]
Classificação Ω: path, phi_q16, entropy_milli, attractor
      │
      ▼ [Apkc/apkc.c — codegen_select()]
Emissão ARM64: seleção determinística de variante
Registro: _codegen_variant_log[256]
      │
      ▼ [Apkc/coherence.h — phi_fst()]
Métrica interna: phi ∈ [0, 65536], attractor ∈ [0, 41]
      │
      ▼ [Benchmark/raf_toroid.h — t7_step()]
T^7: atualização de estado e acumuladores omega_inv[3]
      │
      ▼ [rafaelia/verbovivo.c — vv_recall()]
Ring buffer de engrams: FiberHash + VVHyperVec
      │
      ▼
Artefato + evento de custódia + hashes + resultado dos gates
```

---

## Ver também

- `Apkc/coherence.h` — implementação de `phi_fst()` e `phi_attractor()`
- `Apkc/codegen_select.h` + `Apkc/omega_classifier.h` — seleção e classificação Ω
- `Benchmark/raf_toroid.h` — T^7, atratores e `omega_inv`
- `rafaelia/verbovivo.h` — API pública do motor de convergência
- `raiz_audit_arm64.json` — manifesto de auditoria com `instruction_trace[]`
- `Apkc/proofs/` — provas de cadeia de custódia por sessão CI
- `docs/AGENTES.md` — protocolo de sessão para agentes
- `docs/MAPA_ESTRUTURAL_REPOSITORIO.md` — mapa estrutural do repositório

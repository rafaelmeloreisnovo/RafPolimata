# Dinâmica Informacional e Rastreabilidade Interna

**Estado**: `REFERENCE`
**Autoria**: Rafael Melo Reis / Sessão Claude `claude/info-dynamics-internal-traceability-nxbznc`
**Data**: 2026-07-11
**Referências**: internas ao código (arquivo:linha) — sem cadeia bibliográfica externa

---

## Parte I — Framework Epistemológico

> A extração de uma dinâmica evolutiva a partir de um sistema informacional, sem o
> suporte de uma cadeia de custódia bibliográfica, exige a substituição da autoridade
> externa (a citação) por um modelo de rastreabilidade interna — um protocolo que
> demonstre como a informação se transforma e se autentica dentro do próprio sistema.

### O Paradoxo da Cadeia de Custódia

Em ciência, a cadeia de custódia da informação é tradicionalmente mantida por
referências bibliográficas. Cada citação é um elo que documenta a origem e a linhagem
de uma ideia, assegurando que o conhecimento atual é uma evolução documentada do
conhecimento passado. Sem essa cadeia, a informação se torna um vestígio órfão, cuja
integridade e evolução não podem ser atestadas por um histórico cronológico de fontes
verificáveis.

O paradoxo histórico é instrutivo: o primeiro cirurgião a realizar uma cesariana não
conseguiu nomear a teoria que fundamentava o procedimento; Gregor Mendel completou
10.000 cálculos sobre ervilhas verdes e amarelas que só foram reconhecidos 34 anos
após sua morte — o pai da genética não estava estudando DNA, e ninguém nomeou sua
atitude. Informação sem rastreabilidade de origem é alucinação: pode ser verdadeira,
mas não pode ser verificada.

### A Dinâmica Evolutiva Sem Cadeia de Custódia Externa

Mesmo na ausência de referências externas, um sistema de informação pode exibir
dinâmica evolutiva operando sob três princípios:

**1. A Informação como Sistema Dinâmico**

A dinâmica evolutiva é a mudança no estado de um sistema ao longo do tempo. A
"informação" não é um objeto estático, mas um estado que evolui por meio de
interações. A ausência de cadeia de custódia externa não impede a evolução — ela a
torna autônoma, regida por leis internas e interações determinísticas.

**2. Seleção Natural da Informação**

Assim como na evolução biológica a seleção natural atua sobre variação genética, a
informação em um sistema sem cadeia de custódia está sujeita a seleção informacional.
Ideias ou dados mais "aptos" (mais consistentes com outras informações no sistema, ou
mais úteis para resolução de problemas) persistem e se replicam; outros são
descartados. Este processo gera dinâmica evolutiva sem referências externas.

**3. Emergência de Nichos Informacionais**

Em sistemas complexos, a interação entre entidades processadoras de informação pode
levar à emergência de nichos informacionais. Nestes nichos, a informação se
auto-organiza em redes estáveis, criando uma proto-autopoiese — um sistema que se
mantém e se regenera a si mesmo. Essa auto-organização é uma forma de evolução que
emerge naturalmente das leis físicas e processos determinísticos.

### Axiomas de Validade Epistêmica

A validade da informação em um sistema de rastreabilidade interna depende
inteiramente de:

| Axioma | Descrição |
|--------|-----------|
| **Consistência interna** | A informação deve ser logicamente coerente com o restante do sistema |
| **Poder preditivo** | A dinâmica evolutiva deve gerar padrões testáveis dentro do próprio sistema |
| **Rastreabilidade procedimental** | Deve existir registro do processo de transformação da informação, uma "cadeia de custódia" interna que documente sua história cronológica |

---

## Parte II — Implementação no RafPolimata

Esta seção é a "cadeia de custódia" do framework acima: cada princípio filosófico é
mapeado para um arquivo, função e linha de código verificável. As implementações
substituem as citações bibliográficas.

### Mapeamento de Princípios → Código

| Princípio | Implementação | Localização |
|-----------|--------------|-------------|
| Consistência interna | `phi_fst()` — métrica Q16 de coerência KAM-7 | `Apkc/coherence.h:19` |
| Seleção natural informacional | `codegen_select()` — seleção determinística de variante de instrução baseada nos bytes já emitidos | `Apkc/codegen_select.h:15` |
| Nichos informacionais (42 atratores) | `t7_step()` — evolução do estado do toroid T^7 com `omega_inv[3]` | `Benchmark/raf_toroid.h:78` |
| Proto-autopoiese (estado autossustentado) | `T7State.omega_inv[3]` — checksums XOR de trajetória que se acumulam e nunca resetam | `Benchmark/raf_toroid.h:13` |
| Rastreabilidade procedimental | `_codegen_variant_log[256]` + printout `[phi=... attractor=...]` | `Apkc/apkc.c:277` |
| Poder preditivo (recall de engrams) | `vv_recall()` — recuperação dos N engrams mais ressonantes com o contexto | `rafaelia/verbovivo.c:316` |
| Identidade estrutural (assinatura) | `FiberHash` 256 bits Hamming — hash estrutural de 4 lanes djb2 | `rafaelia/fiber_h.h` |
| Diversidade informacional (Hamming) | `fiber_hash_distance()` — distância de Hamming [0, 256] entre assinaturas | `rafaelia/fiber_h.h:74` |
| Cadeia de custódia procedimental | `instruction_trace[]` + `audit_hash` em JSON | `raiz_audit_arm64.json` |
| Métricas Ω da fonte | `RafCtx.omega_phi_q16`, `omega_attractor`, `omega_path` | `raf_compile.h:85–91` |
| Auditoria de compliance | `vv_audit()` — ISO 27001/25010, NIST 800-53, IEEE 12207 | `rafaelia/verbovivo.c:229` |
| Provas de custódia por sessão | `Apkc/proofs/CHAIN_OF_CUSTODY_*.md` — F0–F6 gates com TOKEN_VAZIO | `Apkc/proofs/` |

### As Cinco Fases da Cadeia de Custódia Interna

O pipeline RafPolimata produz rastreabilidade procedimental em cinco fases
documentadas. Cada fase transforma a informação e registra a assinatura do estado
resultante — análogo a um elo de cadeia bibliográfica, mas interno.

```
Fonte (bytes) → [Fase 1] → [Fase 2] → [Fase 3] → [Fase 4] → [Fase 5] → Engram (SVG)
```

#### Fase 1 — Classificação Ω da Fonte

**Arquivo**: `raf_compile.h:85–91` · `Apkc/omega_classifier.h:81`

A fonte é classificada segundo a taxonomia Ω de cinco caminhos:
`PROCESSUAL`, `VOID`, `FORGOTTEN`, `MENOSPREZADO`, `URGENT`.

Métricas extraídas: `entropy_milli` [0, 8000], `phi_q16` (coerência KAM-7),
`attractor` [0, 41], `flags`, `path`.

A classificação Ω determina a rota de dispatch antes de qualquer transformação.

#### Fase 2 — Emissão ARM64 com Seleção de Variante

**Arquivo**: `Apkc/apkc.c:376` · `Apkc/codegen_select.h:15`

Para famílias de equivalência (ex: `MOV Xd,Xm` ≡ `ORR Xd,XZR,Xm` ≡ `ADD Xd,Xm,#0`
≡ `SUB Xd,Xm,#0`), `codegen_select()` escolhe deterministicamente a variante usando
`phi_fst` + `phi_attractor` sobre os bytes já emitidos.

Mesmo source → mesmos bytes anteriores → mesma escolha em todo build. A seleção é
auditável via `_codegen_variant_log[256]` e impressa como:

```
[codegen mov_family: total=N orr=N add0=N sub0=N]
```

Esta é a implementação computacional da **seleção natural informacional**: variantes
mais coerentes com o estado atual do buffer são escolhidas deterministicamente.

#### Fase 3 — Invariante de Coerência Geométrica

**Arquivo**: `Apkc/coherence.h:19` · `Apkc/apkc.c:1754–1774`

Após cada `build_apk()`, `phi_fst()` computa a coerência do APK produzido:

```
phi_fst = (1 − H_norm) × C_norm   em Q16 [0, 65536]

H_norm = unique_byte_count / 256        (proxy de entropia)
C_norm = KAM-7 dot product / ‖freq‖   (coerência vs semente {40503…})
```

O resultado é impresso:
```
[phi=0.3142 attractor=17]
```

Esta linha é a **assinatura de consistência interna** de cada APK gerado —
rastreável, reprodutível, sem referência externa.

#### Fase 4 — Motor de Convergência T^7

**Arquivo**: `Benchmark/raf_toroid.h:46,78` · `rafaelia/verbovivo.c:439`

O toroid T^7 evolui em 7 dimensões (entropia, coerência, intenção, observação,
ruído, transmutação, memória). A cada passo:

- `omega_inv[3]` acumula checksums XOR de trajetória — invariante de tamper-evidence
- `attractor` evolui via `(attractor + omega + u_t) % 42` — os **nichos informacionais**
- `perm_class` = S₆×C₇: setor hexagonal (alto nibble) + nível Fibonacci (baixo nibble)

O T^7 implementa a **emergência de nichos informacionais**: 42 atratores estáveis
que o estado converge e que não podem ser fabricados retroativamente (os
`omega_inv` acumulam toda a trajetória desde `t7_init()`).

#### Fase 5 — Memória de Engrams e Recall

**Arquivo**: `rafaelia/verbovivo.h:48` · `rafaelia/verbovivo.c:316`

`VerbVivoState` mantém um ring buffer de 64 engrams (`memory[VV_MEM_SIZE]`), cada
um com `FiberHash` (identidade estrutural) e `VVHyperVec` (identidade semântica).

`vv_recall()` recupera os N engrams mais ressonantes com o contexto acumulado:

```c
recall_score = phi_weight × cos(query, e.vec)
             + (1 − phi_weight) × (1 − hamming/256)
```

`phi_weight = (float)t7.phi / 65536.0f` lido diretamente do T^7 vivo —
regime coerente quando phi alto, regime caótico quando phi baixo.

Este é o **poder preditivo interno**: o sistema pode recuperar, de sua própria
memória, os estados mais relevantes para o contexto presente, sem consulta externa.

---

## Parte III — Verificação da Cadeia

A validade do framework não depende de citações externas. Depende dos seguintes
comandos reprodutíveis:

```bash
# Verificar consistência interna (Fase 3):
clang -target aarch64-linux-gnu -fsyntax-only -nostdlib -nostdinc \
  -ffreestanding -I Apkc Apkc/apkc.c
# Deve retornar 0 erros

# Verificar rastreabilidade procedimental (Fase 2):
python3 tools/raf_codegen_select_test.c  # ou compilar e rodar
# Mesma entrada → mesma sequência de variantes

# Verificar poder preditivo (Fase 5):
gcc -std=c11 -O2 -I. -IBenchmark -DVERBOVIVO_MAIN rafaelia/verbovivo.c -lm -o verbovivo
./verbovivo -r 3 < out.apk
# Imprime top-3 engrams por recall score

# Verificar cadeia de custódia procedimental:
python3 scripts/apkc_validate.sh   # gates F0–F6
python3 scripts/validate_coherence_protocol.py  # 50 fórmulas canônicas
```

Todos os gates CI em `.github/workflows/ci.yml` são a prova computacional de que
as cinco fases da cadeia interna são reprodutíveis e consistentes — sem dependência
de autoridade externa.

---

## Diagrama de Rastreabilidade

```
Fonte (arquivo ou stream)
      │
      ▼ [raf_compile.h:85 — omega_classify()]
Classificação Ω: path, phi_q16, entropy_milli, attractor
      │
      ▼ [Apkc/apkc.c:376 — codegen_select()]
Emissão ARM64: seleção determinística de variante
Registro: _codegen_variant_log[256]
      │
      ▼ [Apkc/coherence.h:19 — phi_fst()]
Coerência do APK: phi ∈ [0, 65536], attractor ∈ [0, 41]
Printout: "[phi=X attractor=N]"
      │
      ▼ [Benchmark/raf_toroid.h:78 — t7_step()]
T^7 toroid: 42 atratores, omega_inv[3] acumula
Nicho informacional: convergência em attractor estável
      │
      ▼ [rafaelia/verbovivo.c:316 — vv_recall()]
Ring buffer de engrams: FiberHash + VVHyperVec
Recall: top-N engrams por ressonância com contexto
      │
      ▼
Engram SVG — assinatura visual da dinâmica evolutiva
     rastreável, reprodutível, sem referência externa
```

---

## Ver também

- `Apkc/coherence.h` — implementação de `phi_fst()` e `phi_attractor()`
- `Apkc/codegen_select.h` + `Apkc/omega_classifier.h` — seleção e classificação Ω
- `Benchmark/raf_toroid.h` — T^7 toroid, atratores, `omega_inv`
- `rafaelia/verbovivo.h` — API pública do motor de convergência
- `raiz_audit_arm64.json` — manifesto de auditoria com `instruction_trace[]`
- `Apkc/proofs/` — provas de cadeia de custódia por sessão CI
- `docs/AGENTES.md` — protocolo de sessão para agentes
- `docs/MAPA_ESTRUTURAL_REPOSITORIO.md` — mapa em 5 níveis de profundidade

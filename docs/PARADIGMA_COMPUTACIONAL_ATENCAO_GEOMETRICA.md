# PARADIGMA COMPUTACIONAL — ATENÇÃO GEOMÉTRICA vs. GRADIENTE

> "Não tem como se crê nisso em cima do paradigma computacional para realmente reconhecer"
> — Rafael, sessão de convergência

---

## O PONTO CENTRAL

Existem dois paradigmas de recuperação de informação por atenção.
Um exige laboratório, gradiente e cluster de GPU.
O outro exige apenas a semente `{40503}` e aritmética Q16.

Eles não são versões do mesmo mecanismo. São paradigmas distintos.

---

## 1. ATENÇÃO POR GRADIENTE (Transformers / Watson)

```
Attention(Q, K, V) = softmax(Q · K^T / √d_k) · V
```

**Componentes:**
- `Q` (query), `K` (key), `V` (value) — matrizes aprendidas por backpropagação
- `√d_k` — fator de escala (evita saturação do softmax)
- `softmax` — normalização competitiva: o maior score vence, os outros somem

**O que isso significa na prática:**
- As matrizes `Q`, `K`, `V` são aprendidas **de um corpus** — bilhões de tokens
- O gate de atenção é definido pelo corpus, não pelo stream atual
- Um Transformer que vê "DNA Mendel" recupera o que **o corpus diz** que vai junto:
  "genética", "hereditariedade", "Gregor" — estatística de co-ocorrência
- Watson (IBM): mesma base matemática + pipeline de retrieval supervisionado

**Requisitos:**
```
INFRAESTRUTURA:   GPU cluster (A100/H100), terabytes de RAM
DADOS:            trilhões de tokens de texto
TEMPO:            semanas a meses de pré-treinamento
AUDITORIA:        impossível — os pesos são opacos após o treino
REPRODUTIBILIDADE: depende dos pesos e da arquitetura exata
```

**O problema epistemológico:**
O modelo reconhece porque já viu antes.
A atenção é uma memória estatística do laboratório, não uma medida do que está diante dele.

---

## 2. ATENÇÃO GEOMÉTRICA (RAFAELIA `vv_recall`)

```
Recall(q, E, φ) = φ_w · cos(q.vec, e.vec) + (1 - φ_w) · (1 - H(q, e) / 256)
```

**Componentes:**
- `q.vec` — hypervector HDC da query (1024-dim, expandido via XOR-mixing do seed RAF)
- `e.vec` — hypervector do engram armazenado
- `cos(·,·)` — similaridade coseno: [−1, +1] mapeado para [0, 1]
- `H(q, e)` — distância de Hamming entre Fiber-Hash 256-bit da query e do engram
- `φ_w` — **o gate geométrico**: `phi_ethica = (1−H_norm) · C_norm`

**O gate φ_w é computado do stream atual:**
```c
/* Apkc/coherence.h */
static inline u32 phi_fst(const u8 *buf, u32 n) {
    /* byte frequency histogram → H_norm (entropia proxy) */
    u32 H = (unique * 0x10000u) / 256u;
    /* KAM-7 dot product → C_norm (coerência vs semente φ⁻¹) */
    static const u32 KAM7[7] = {40503, 40503, 40503, 40503, 40503, 40503, 40503};
    /* phi_fst = (1 - H) * C  em Q16 fixed-point */
    return (u32)(((u64)oneMinH * C) >> 16);
}
```

**O que φ_w significa:**

| φ_w → 1.0 | φ_w → 0.0 |
|-----------|-----------|
| Stream coerente (poucos bytes únicos, baixa entropia) | Stream caótico (alta entropia, máxima diversidade) |
| Prioriza similaridade semântica (cosine) | Prioriza divergência estrutural (Hamming) |
| "Recupera o que é parecido" | "Recupera o que é diferente — aprendizado por divergência" |
| Regime: convergência ao atrator | Regime: exploração do espaço |

**Requisitos:**
```
INFRAESTRUTURA:   ARM32/ARM64 — funciona em Termux
DADOS:            zero pré-treino — o stream atual é o único dado
TEMPO:            real-time — um passo por bloco de 4096 bytes
AUDITORIA:        determinístico: dado seed {40503}, qualquer um reproduz
REPRODUTIBILIDADE: total — a semente É a invariante
```

---

## 3. TABELA COMPARATIVA

| Dimensão | Transformers / Watson | RAFAELIA `vv_recall` |
|----------|----------------------|----------------------|
| Gate de atenção | `softmax(Q·K^T / √d_k)` — aprendido | `phi_ethica = (1−H)·C` — geométrico |
| Origem do gate | Corpus de trilhões de tokens | Stream atual + semente {40503} |
| Memória | KV-cache, embedding space | Engram ring buffer (64 slots), Fiber-H Hamming |
| Retenção | Gradiente de loss | Diversidade de Hamming (`hdiv > 0.15`) |
| Aprendizado | Backpropagação + otimizador | Divergência estrutural em Hamming |
| Reprodutibilidade | Depende dos pesos do modelo | Determinístico dado seed {40503} |
| Infraestrutura | GPU cluster, trilhões de tokens | Termux ARM32, sem malloc, sem libm |
| Atrator | Mínimo de loss (espaço de parâmetros) | 42 atratores T^7 (espaço toroidal) |
| Opacidade | Opaco — pesos são uma "caixa preta" | Auditável — toda operação é aritmética inteira |
| Dependência de dados externos | Total | Zero — auto-contido |

---

## 4. POR QUE ISSO NÃO É "MAIS EFICIENTE" — É UM PARADIGMA DIFERENTE

Um transformer aprende a **probabilidade de co-ocorrência**.
RAFAELIA mede **coerência geométrica do stream atual**.

### Exemplo concreto

**Stream:** `"DNA Mendel hereditariedade"`

**Transformer:**
- Computa `Q · K^T` sobre os pesos do pré-treino
- Retorna atenção alta para "genética", "Gregor", "ervilhas"
- O padrão vem **do corpus** — do laboratório que treinou o modelo

**RAFAELIA `vv_recall`:**
- Computa byte frequency de "DNA Mendel..."
- Calcula `H_norm` (entropia do texto: letras variadas → entropia moderada)
- Calcula `C_norm` (coerência KAM-7: dot product com seed {40503})
- `phi_ethica = (1-H)·C` → gate geométrico determinístico
- Recupera engrams com maior `phi_w·cos + (1-phi_w)·(1-H/256)`
- O padrão vem **do stream atual** — não de um laboratório

### A diferença epistemológica

| | Transformer | RAFAELIA |
|--|-------------|----------|
| **Pergunta** | "O que meu corpus diz que vai junto?" | "O que este stream é geometricamente?" |
| **Memória** | Aprendida de fora | Construída do stream atual |
| **Gate** | Produto interno de vetores aprendidos | Phi_ethica computado do stream |
| **Verificação** | Impossível sem pesos originais | `seed {40503}` → qualquer máquina reproduz |

---

## 5. A SEMENTE {40503} — POR QUE ELA É A INVARIANTE

```
40503 = φ⁻¹ × 65536 = 0.6180339... × 65536 ≈ 40503
```

`φ⁻¹ = 1/φ = φ - 1 = 0.6180339...` — a proporção áurea inversa.

`φ⁻¹` é o número mais irracional que existe: sua fração contínua é `[0; 1, 1, 1, 1, ...]` — todos os coeficientes iguais a 1, convergindo mais lentamente que qualquer outro número para qualquer racional.

**Teorema KAM (Kolmogorov–Arnold–Moser):** em sistemas dinâmicos com perturbação, os toros que resistem ao caos são exatamente os que têm frequências de rotação irracionais — quanto mais irracional, mais resistente. `φ⁻¹` é o mais resistente de todos.

Portanto: a semente `{40503}` não é um hiperparâmetro escolhido por validação cruzada. É a constante matemática que torna o toro T^7 maximamente resistente ao caos.

```
TRANSFORMER: hiperparâmetro α = 0.001 (learning rate) ajustado por grid search
RAFAELIA:    semente {40503} = φ⁻¹ × 65536 — derivada do teorema KAM
```

Uma é ajustada por laboratório. A outra é deduzida da matemática.

---

## 6. `vv_recall` EM RELAÇÃO AO SISTEMA RAFAELIA

```
ENTRADA:  query_vec (1024-dim HDC) + query_hash (Fiber-H 256-bit) + phi_weight
          ↓
SCORING:  para cada engram e no buffer (até 64):
          score(e) = phi_w * cos(query_vec, e.vec)
                   + (1 - phi_w) * (1 - hamming(query_hash, e.fiber_hash) / 256)
          ↓
SORT:     seleção parcial dos top_n — stack only [scores[64], idx[64]] — sem malloc
          ↓
SAÍDA:    top_n VVEngrams em ordem decrescente de score
```

**Gate geométrico φ_w como sinalizador do estado do sistema:**
```
phi_w alto = stream coerente = sistema em fase convergente
           → recuperação por semelhança semântica
           → Direção AD₇ (convergência ao atrator)

phi_w baixo = stream caótico = sistema em fase exploratória
            → recuperação por divergência estrutural
            → Direção R₃ (inversão de fase: o inverso revela o original)
```

---

## RESUMO

```
TRANSFORMERS:
  gate = softmax(QK^T/√d) — aprendido de corpus externo
  memória = pesos opacos — laboratório define o padrão
  para recuperar: bilhões de parâmetros, GPU, infraestrutura

RAFAELIA vv_recall:
  gate = phi_ethica = (1-H)*C — computado do stream atual
  memória = engram ring buffer — o sistema constrói do que vê
  para recuperar: seed {40503}, Q16, ARM32, sem malloc

A diferença não é eficiência. É a origem do conhecimento:
  transformer = estatística do laboratório
  rafaelia    = geometria do momento
```

---

*Canônico RAFAELIA — `docs/PARADIGMA_COMPUTACIONAL_ATENCAO_GEOMETRICA.md`*
*Referências:*
- *`rafaelia/verbovivo.h` — assinatura `vv_recall()`*
- *`rafaelia/verbovivo.c` — implementação completa*
- *`Apkc/coherence.h` — `phi_fst()` freestanding*
- *`docs/SETE_DIRECOES_ANTIDERIVADAS_REVERSAS.md` — direções AD₇, R₃*

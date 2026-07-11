# ESTRUTURA DE APRENDIZADO — TODAS AS FORMAS DO SISTEMA RAFAELIA

> **Entrada canônica:** `docs/AGENTES.md` §1 (leitura rápida — o que este repo faz e onde estão
> os invariantes) e §3 (ciclo de sessão — startup/execução/shutdown). Este documento descreve a
> estrutura de aprendizado progressivo do sistema RAFAELIA em todas as suas formas.

> "Não comece pelas folhas. Comece pelos mapas."
> — Ordem da Biblioteca, imagem da sessão

---

## O QUE É ESTE DOCUMENTO

Este documento é a **forma mais densa**: texto puro, computacionalmente legível,
que carrega as relações entre todos os arquivos, formas e modos do sistema RAFAELIA.
Imagens transmitem gestalt. Texto carrega estrutura.

---

## MAPA DAS FORMAS — 6 CAMADAS DE LEITURA

Cada conteúdo do sistema existe em até 6 formas simultâneas:

| Camada | Nome         | Forma no Repositório                    | Densidade |
|--------|--------------|-----------------------------------------|-----------|
| 1      | FACTUAL      | `.c`, `.h`, `.csv`, `.json`             | Máxima    |
| 2      | SIMBÓLICA    | `.md` com símbolos (Σ, Ω, Δ, Φ)        | Alta      |
| 3      | MATEMÁTICA   | `RAFAELIA_MASTER_DOC.txt` (50 fórmulas) | Alta      |
| 4      | COMPUTACIONAL| `scripts/`, `Benchmark/`, CI workflow   | Máxima    |
| 5      | ÉTICA        | `docs/IA_AGENTE_HUMANOS*.md`            | Média     |
| 6      | ESPIRITUAL   | Parábolas (ver `TOKEN_VAZIO_PARABOLAS_MESTRES.md`) | Viva |

---

## FORMA MAIS DENSA: O ARQUIVO DE TEXTO

**Por que texto supera imagem em densidade operacional:**

```
IMAGEM:
  - Lida por visão (1 canal)
  - Não é indexável por grep/sed/awk
  - Não é versionada semanticamente
  - Não é executável
  - Não é compressível com semântica preservada

TEXTO (.md, .txt, .c):
  - Lido por todos os mecanismos: grep, embeddings, AST, compilador
  - Versionado linha a linha no git
  - Indexável: cada conceito é um token endereçável
  - Parcialmente executável (blocos de código)
  - Compressível com semântica preservada (ZIPRAF, deflate)
```

O arquivo `RAFAELIA_MASTER_DOC.txt` (74KB) carrega mais estrutura operacional
que todas as imagens juntas. As imagens são **portais de entrada**, não
**recipientes de conteúdo**.

---

## ESTRUTURA DE APRENDIZADO — TODAS AS FORMAS

### FORMA 0 · TOKEN VAZIO (base de honestidade)
Antes de aprender qualquer forma: reconhecer quando não há conteúdo.
Ver: `TOKEN_VAZIO_PARABOLAS_MESTRES.md`

### FORMA 1 · REGISTROS (começo obrigatório)
```
Arquivos-chave de entrada:
  RAF_INDEX.md              → índice principal de 56 métodos
  RAFAELIA_MASTER_DOC.txt   → núcleo matemático completo (18 seções)
  RAF_40_STRATEGIES.md      → 40 estratégias operacionais
  RAF_CHECKLIST_96_ITEMS.md → 96 itens de verificação
```

### FORMA 2 · SELOS / HASHES (verificação de integridade)
```
Cada transição no sistema RAFAELIA é certificada:
  CRC32C(L) = 0x6E5DD6D2    → hash reprodutível da sessão
  BLAKE3                    → hash de arquivos e commits
  rafaelia.sync_hash.log    → trilha de integridade temporal
```

### FORMA 3 · MATEMÁTICA (o esqueleto)
```
50 fórmulas canônicas em RAFAELIA_MASTER_DOC.txt SEC 02:
  T^7 = (R/Z)^7             → espaço toroidal 7D
  F_{n+1} = F_n·√3/2 - π·sin(279°) → sequência Rafaeliana
  C_{t+1} = (1-α)·C_t + α·C_in → EMA (α=0.25)
  dx/dt = -Lx + α·M(c)     → grafo espectral ΣΩ
  λ_max = 31.0268           → velocidade de convergência
```

### FORMA 4 · CÓDIGO C (a matéria)
```
56 arquivos RAF_001 a RAF_056:
  001-020: AVR/ATmega (GPIO, Timer, ADC, UART, SPI, I2C, Watchdog, Sleep)
  021-035: RPi/ARM Linux (mmap, GPIO, contadores ARM64)
  036-050: Android/Termux (threads, JNI, CMake, ABI, syscall)
  051-056: QEMU/Benchmark (probe, medição, batching, cache, comparação)
```

### FORMA 5 · OCTOGONAL (8 direções de atenção)
```
FOCO → PERCEPÇÃO → MEMÓRIA → INTUIÇÃO
EMOÇÃO → VONTADE → IMAGINAÇÃO → EXECUÇÃO

Cada vetor é uma direção de fluxo informacional.
Virtualização: cada vetor se projeta em camadas de realidade via BLENDs.
```

### FORMA 6 · TORO T^7 (a geometria que une)
```
s = (u, v, ψ, χ, ρ, δ, σ) ∈ [0,1)^7
42 atratores determinísticos
Período: x_{n+42} = x_n
Stride toroidal: 7, gcd(7,1000)=1 → cobertura total
```

### FORMA 7 · PARÁBOLAS (a tradição viva)
```
21 tradições humanas → 1 jardim de sementes
Cada tradição ensina a MESMA operação por um ângulo diferente.
Ver: TOKEN_VAZIO_PARABOLAS_MESTRES.md
```

---

## CAMINHO DE APRENDIZADO — PROGRESSÃO CANÔNICA

```
NÍVEL 0: Ler RAF_INDEX.md (mapa)
NÍVEL 1: Ler RAFAELIA_MASTER_DOC.txt SEC 01-02 (resumo + matemática)
NÍVEL 2: Compilar e executar RAF_001 a RAF_010 (AVR base)
NÍVEL 3: Executar Benchmark/raf_main.c no Termux (validar hardware)
NÍVEL 4: Ler docs/ARQUITETURA_21_NIVEIS.md + docs/DEZ_DIMENSOES_SEMANTICAS.md
NÍVEL 5: Ler docs/PROTOCOLO_CANONICO_COHERENCIA.md + aplicar gates
NÍVEL 6: Ler docs/SETE_DIRECOES_ANTIDERIVADAS_REVERSAS.md (este nível exige domínio dos anteriores)
NÍVEL 7: CICLO VIVO: LER → RETROALIMENTAR → EXPANDIR → VALIDAR → EXECUTAR → ÉTICA
```

---

## NÚCLEO FAÇA (operação mínima de cada sessão)

```
1. LER       → coletar fatos, dados e contexto sem distorção
2. RETROALIMENTAR → identificar F_id, f_next no grafo
3. EXPANDIR  → integrar novos padrões e conexões
4. VALIDAR   → testar hipóteses, evidências e fronteiras
5. EXECUTAR  → converter entendimento em ação observável
6. ÉTICA     → verdade, responsabilidade, coerência e integridade
```

Fórmula operacional: `Φ = ⟨LER → RETROALIMENTAR → EXPANDIR → VALIDAR → EXECUTAR⟩ + Φ_ética`

---

## INVARIANTES DO SISTEMA (o que nunca muda)

```
Σ  Σ   Simetria      Relações preservadas sob transformação
Ω  Ω   Totalidade    Informação, energia e recursos rentáveis
∞  ∞   Causalidade   Causas precedem efeitos observáveis
Δ  Δ   Escala        Coerência entre níveis micro e macro
Φ  Φ   Geometria     Proporções preservadas com transformação
S  S   Entropia      Ruído monitorado e organização mantida
⟳  C   Ciclos        Retroalimentações identificadas e estáveis
▣  L   Limites       Restrições físicas, lógicas e operacionais
✦  P   Composição    Partes integradas sem contradições
✓  V   Verificação   Resultados reprodutíveis e auditáveis
```

`R(t+1) = R(t) · Φ_ética · E_prova · (√3/2)^n` → Recorrência ética por ressonância.

---

## RELAÇÃO ENTRE AS FORMAS

```
TOKEN_VAZIO  ←→  FATO BRUTO  ←→  MATEMÁTICA  ←→  CÓDIGO
     ↕                ↕               ↕             ↕
  HONESTIDADE    OBSERVAÇÃO      ESTRUTURA      EXECUÇÃO
     ↕                ↕               ↕             ↕
  PARÁBOLA       SÍMBOLO         TORO T^7      BENCHMARK
```

Todas as formas são projeções do mesmo objeto em dimensões diferentes.
Nenhuma é completa sozinha. Juntas, cobrem os 360° do espaço de compreensão.

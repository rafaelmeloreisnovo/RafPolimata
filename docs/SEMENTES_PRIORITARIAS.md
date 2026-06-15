# SEMENTES PRIORITÁRIAS — RAFAELIA / RafPolimata

Data: 2026-06-15  
Branch alvo: `main-1st`  
Status: `AUDITABLE_SEED_PRIORITY_MAP`  
Origem cruzada: `ChipQuantum/Outro/RAFAELIA_SEMENTES.txt`, `ChipQuantum/Outro/sementes_v2.txt`, `Vectras-VM-Android/Incluir/sessao_completa_possibilidades_e_matematica.md`, `RafPolimata/docs/CONVERGENCIA_UNICA_METODOLOGICA.md`.

> Regra soberana: semente não é prova. Semente é objeto germinável. Só vira claim forte quando existe definição, implementação, teste, ambiente, medição, hash e repetição.

---

## 0. Régua de status

| Status | Significado |
|---|---|
| `HIPOTESE_FORMAL` | objeto matemático definido parcialmente, ainda sem prova/benchmark suficiente |
| `BLUEPRINT_EXECUTAVEL` | há roteiro ou script/protótipo, mas sem validação canônica no RafPolimata |
| `REFERENCE_MODEL` | existe modelo de referência em outro repositório/linguagem |
| `PENDING_TEST` | teste mínimo definido, ainda não executado nesta árvore |
| `PENDING_LITERATURE_REVIEW` | precisa busca bibliográfica antes de claim de novidade |
| `PENDING_PORT` | precisa portar para C/ASM/APKc/RafPolimata |
| `TOKEN_VAZIO` | ausência honesta de origem/prova suficiente |
| `PROMOTABLE` | pode virar claim técnico após passar gates mínimos |

---

## 1. Visão das 7 sementes-mãe

| ID | Semente | Tipo | Status atual | Melhor destino |
|---|---|---|---|---|
| S1 | `√3/2 / 0xDDB4` | operador de contração | `HIPOTESE_FORMAL` | `rafaelia/math_constants.h` |
| S2 | `F* = 23.158` | ponto fixo Fibonacci-Rafael | `HIPOTESE_FORMAL` | `tests/test_fibonacci_rafael.c` |
| S3 | `42 atratores em T^7` | dinâmica/topologia | `REFERENCE_MODEL` | `rafaelia/t7_q16.*` |
| S4 | `D2 / Hausdorff / Grassberger-Procaccia` | medição fractal | `PENDING_TEST` | `tools/measure_attractor_d2.py` + C runner |
| S5 | `CRC32C state hash` | cadeia de custódia de estado | `BLUEPRINT_EXECUTAVEL` | `rafaelia/state_crc32c.h` |
| S6 | `NEON attractor engine` | hardware como máquina de estado | `BLUEPRINT_EXECUTAVEL` | `Apkc/` + `rafaelia/neon_attractor.c` |
| S7 | `n_crítico(L)` | semântica/informação | `HIPOTESE_FORMAL` | `research/n_critico_linguas.md` |

---

# S1 — `√3/2 / 0xDDB4`

## Objeto

```text
Q16_SQRT3_2 = 0xDDB4 = 56756 / 65536 ≈ 0.86603
```

Interpretação operacional:

```text
√3/2 é tratado como operador de contração geométrica:
- polo IIR estável;
- expoente de Lyapunov negativo;
- altura do triângulo equilátero;
- fator de decaimento em recorrências;
- ponte simbólica para estabilidade toroidal.
```

## Status

```text
HIPOTESE_FORMAL
PENDING_LITERATURE_REVIEW
PENDING_CANONICAL_TEST
```

## Teste mínimo

Criar teste C que confirme:

```text
abs(Q16_SQRT3_2 / 65536.0 - sqrt(3)/2) < tolerância
lambda = ln(sqrt(3)/2) < 0
s[n+1] = s[n] * Q16_SQRT3_2 converge para 0
```

## Arquivo proposto

```text
rafaelia/math_constants.h
```

## Gate de promoção

```text
PROMOTABLE quando houver:
- constante Q16 definida;
- teste numérico;
- relatório com tolerância;
- separação clara entre fato matemático e hipótese multi-domínio.
```

---

# S2 — `F* = 23.158` / ponto fixo Fibonacci-Rafael

## Objeto

Recorrência proposta:

```text
F[n+1] = F[n] · (√3/2) − π · sin(279°)
```

Como `sin(279°) = -sin(81°)`, a forma positiva do termo fixo fica:

```text
c = π · sin(81°)
F* = c / (1 - √3/2) ≈ 23.158
```

## Status

```text
HIPOTESE_FORMAL
PENDING_LITERATURE_REVIEW
PENDING_BIFURCATION_MAP
```

## Teste mínimo

Criar teste determinístico:

```text
1. iniciar F0 = 0, F0 = 1, F0 = -100, F0 = 100;
2. iterar N passos;
3. verificar convergência para F* dentro de tolerância;
4. variar θ e c para gerar mapa F*(θ,c).
```

## Arquivo proposto

```text
tests/test_fibonacci_rafael.c
research/fibonacci_rafael_fixed_point.md
```

## Gate de promoção

```text
PROMOTABLE como resultado matemático interno se a convergência for demonstrada por teste.
Não promover como novidade acadêmica sem busca bibliográfica.
```

---

# S3 — 42 atratores em `T^7`

## Objeto

Modelo base:

```text
T^7 = toro 7-dimensional
estado Q16: s[7] ∈ [0, 65535]^7
attractor_count = 42
phi_ethica = (1 - H) * C
```

Interpretação:

```text
42 atratores funcionam como roteadores de estado.
A dinâmica T^7 pode ser usada como núcleo de seleção, compressão, memória e navegação.
```

## Status

```text
REFERENCE_MODEL em ChipQuantum
PENDING_PORT para C/Q16 em RafPolimata
PENDING_BIT_EXACT_TEST
```

## Teste mínimo

Portar para C:

```text
q16_wrap(x)
ema_quarter(prev,new)
phi_gate_q16(C,H)
step_state_branchless_q16(state,phi,bias)
xor_acc(data)
fnv1a64(data)
coprime_stride(...)
rolling_attractor42(history)
```

Comparar C contra referência Python com vetores fixos.

## Arquivo proposto

```text
rafaelia/t7_q16.h
rafaelia/t7_q16.c
tests/test_t7_q16.c
```

## Gate de promoção

```text
PROMOTABLE quando C == Python reference para vetores fixos e relatório de equivalência existir.
```

---

# S4 — `D2 / Hausdorff / Grassberger-Procaccia`

## Objeto

Medir dimensão de correlação do atrator:

```text
C(epsilon) = #{pares (i,j): |x_i - x_j| < epsilon} / N^2
D2 = lim_{epsilon→0} log(C(epsilon)) / log(epsilon)
```

A hipótese registrada é que a órbita/fractal RAFAELIA poderia apresentar dimensão não-inteira.

## Status

```text
PENDING_TEST
PENDING_DATA
PENDING_RUNTIME_MEASUREMENT
```

## Teste mínimo

```text
1. gerar trajetória T^7 por N passos;
2. coletar vetores reduzidos: lambda_q16, phi_q16, acc_weight_q16 ou s[7];
3. calcular distâncias por amostra;
4. varrer epsilon = {1,2,4,8,...};
5. ajustar log(C(epsilon)) vs log(epsilon);
6. emitir D2, R² do ajuste e faixa usada.
```

## Arquivo proposto

```text
tools/measure_attractor_d2.py
rafaelia/t7_trace_runner.c
proofs/d2_attractor/README.md
```

## Gate de promoção

```text
PROMOTABLE apenas como medição experimental quando houver:
- dataset de trajetória;
- comando reproduzível;
- commit;
- ambiente;
- gráfico/log;
- mediana ou repetição por seed.
```

---

# S5 — `CRC32C state hash`

## Objeto

Cadeia de custódia de estado:

```text
state_step_i → CRC32C_i
H_i = crc32c(H_{i-1}, state_i)
```

Interpretação:

```text
Cada transição do atrator gera um selo barato.
O objetivo não é criptografia forte isolada; é detecção rápida de divergência e trilha de execução.
```

## Status

```text
BLUEPRINT_EXECUTAVEL
PENDING_PORT
PENDING_TEST_ON_ARM64
```

## Teste mínimo

```text
1. gerar sequência fixa de estados;
2. computar CRC32C por fallback C;
3. computar CRC32C por instrução ARM64, quando disponível;
4. comparar outputs;
5. registrar se hardware path e fallback convergem.
```

## Arquivo proposto

```text
rafaelia/state_crc32c.h
tests/test_state_crc32c.c
```

## Gate de promoção

```text
PROMOTABLE quando fallback e hardware path produzirem o mesmo hash para vetores fixos.
```

---

# S6 — `NEON attractor engine`

## Objeto

Usar SIMD como substrato de máquina de estado, não apenas otimização de throughput:

```text
estado vetorial int32x4_t
s[n+1] = s[n] - alpha * s[n]
convergência por norma
```

## Status

```text
BLUEPRINT_EXECUTAVEL
PENDING_INTEGRATION_APKC
PENDING_ARM64_TEST
```

## Teste mínimo

```text
1. compilar kernel NEON;
2. rodar mesma seed 31 vezes;
3. verificar mesma contagem de passos;
4. comparar contra versão scalar;
5. registrar cycles/op ou tempo mediano.
```

## Arquivo proposto

```text
rafaelia/neon_attractor.c
tests/test_neon_attractor.c
proofs/neon_attractor/manifest.json
```

## Gate de promoção

```text
PROMOTABLE quando:
- NEON e scalar convergirem para mesma classe de resultado;
- determinismo for repetível;
- houver medição mediana no hardware alvo.
```

---

# S7 — `n_crítico(L)` / dimensão toroidal de línguas

## Objeto

Hipótese:

```text
cada língua L teria uma dimensão toroidal mínima n_crítico(L)
capaz de representar sua estrutura sem perda semântica excessiva.
```

Formulações candidatas:

```text
I_Tn = n · 32 bits  // Q16 em duas coordenadas por dimensão
I_sem(L,k) = k · H(L)
n_crítico(L) = argmin_n |V_n(1) - H(L)/H_max|
```

## Status

```text
HIPOTESE_FORMAL
PENDING_CORPUS
PENDING_INFORMATION_THEORY_REVIEW
PENDING_EXPERIMENT
```

## Teste mínimo

```text
1. escolher corpus pequeno por língua;
2. medir entropia por caractere, token e palavra;
3. calcular candidatos n_crítico;
4. testar compressão/reconstrução simples em T^n Q16;
5. comparar contra baseline: PCA, autoencoder simples, hashing vetorial.
```

## Arquivo proposto

```text
research/n_critico_linguas.md
tools/measure_language_entropy.py
```

## Gate de promoção

```text
PROMOTABLE como hipótese computacional se houver:
- corpus;
- fórmula fixa;
- baseline;
- resultado negativo/positivo registrado;
- nenhuma alegação universal sem validação ampla.
```

---

## 2. Ordem recomendada de execução

```text
1. S1 — math_constants.h
2. S2 — teste do ponto fixo
3. S3 — T7 Q16 em C
4. S5 — CRC32C state hash
5. S6 — NEON attractor engine
6. S4 — D2 / Grassberger-Procaccia
7. S7 — n_crítico(L)
```

Motivo:

```text
Primeiro fixar constantes e recorrência.
Depois portar T^7.
Depois encadear prova de estado.
Depois acelerar/validar em NEON.
Depois medir fractalidade.
Por último atacar linguagem natural, que exige corpus e revisão teórica maior.
```

---

## 3. Mini-roadmap de artefatos

| Sprint | Artefato | Resultado esperado |
|---|---|---|
| R1 | `rafaelia/math_constants.h` | constantes Q16 canônicas |
| R2 | `tests/test_fibonacci_rafael.c` | prova local de convergência para F* |
| R3 | `rafaelia/t7_q16.*` | T^7 C reference |
| R4 | `rafaelia/state_crc32c.h` | cadeia de estado verificável |
| R5 | `rafaelia/neon_attractor.c` | kernel SIMD determinístico |
| R6 | `tools/measure_attractor_d2.py` | medição fractal experimental |
| R7 | `research/n_critico_linguas.md` | hipótese linguística formalizada |

---

## 4. Parágrafo canônico

As sementes prioritárias da RAFAELIA não são apenas ideias soltas: elas formam uma escada de germinação. A base é `√3/2`, que atua como constante de contração e estabilidade. Dela nasce o ponto fixo `F*`, que transforma recorrência em objeto medível. O T^7 com 42 atratores transforma essa recorrência em espaço de estados. O CRC32C cria trilha de custódia sobre cada transição. O NEON transforma hardware em substrato determinístico de evolução. A dimensão D2 testa se a trajetória tem estrutura fractal real. E `n_crítico(L)` tenta levar essa geometria para linguagem natural. O caminho correto é manter cada semente sob a luz da falsificabilidade: se mede, sobe; se falha, ensina; se não há prova, permanece `TOKEN_VAZIO` fértil, não mentira.

---

## 5. Retroalimentação

```text
F_ok:
- 7 sementes-mãe priorizadas;
- cada uma tem status, teste mínimo, arquivo proposto e gate de promoção;
- separação entre hipótese matemática, blueprint executável e prova.

F_gap:
- nenhum teste foi executado por este documento;
- novidade acadêmica continua pendente de revisão bibliográfica;
- portas C/ASM ainda precisam implementação.

F_next:
- iniciar por S1/S2 com math_constants.h + teste de F*;
- depois portar T7 Q16;
- manter proof manifests por commit e ambiente.
```

ΣΩΔΦBITRAF · FIAT LUX

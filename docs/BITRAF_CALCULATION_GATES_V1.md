# BITRAF — Gates de Cálculo e Localização da Primeira Divergência V1

Status: `CANONICAL_EXECUTION_CONTRACT`  
Causa física: `TOKEN_VAZIO` até intervenção replicada

## 1. Pergunta operacional

O cálculo deve ocorrer **nas fronteiras onde o estado pode mudar ou deixar de ser observado**, não apenas no final da matriz.

```text
S0 golden/reference
→ S1 transformação Bitraf lógica
→ S2 buffer/cache
→ S3 CPU escalar ou NEON/SIMD
→ S4 GPU/DMA
→ S5 serialização/transporte/storage
→ S6 releitura
```

Cada estágio produz um recibo imutável.

## 2. Recibo por estágio

Para cada `stage_j` registrar:

```yaml
stage_id
run_id
matrix_id
pattern_id
seed
input_sha3_256
output_sha3_256
byte_length
bit_length
expected_available_mask
observed_available_mask
xor_diff_count
flip_0_to_1
flip_1_to_0
erasure_count
omission_count
syndrome
corrected_count
uncorrected_count
temperature_c
voltage_v
frequency_hz
latency_ns
cache_policy
owner: CPU | NEON | GPU | DMA | STORAGE
monotonic_timestamp_ns
```

## 3. Primeira divergência

Se `x_j` é o vetor observado após o estágio `j` e `x_0` a referência:

\[
D_j=x_j\oplus x_0
\]

nos bits conhecidos. Defina:

\[
j^*=\min\{j:\|D_j\|_0>0\ \lor\ E_j\ne\varnothing\ \lor\ O_j>0\},
\]

onde `E_j` é o conjunto de erasures e `O_j` a contagem de omissions.

`j*` localiza a primeira fronteira observável de divergência. Ele **não prova** a causa física dentro do estágio.

## 4. Ordem dos cálculos

### Gate A — Integridade

Antes e depois de cada estágio:

\[
h_j=SHA3\_256(x_j).
\]

Hash diferente indica divergência, não localização bit a bit.

### Gate B — Máscara de disponibilidade

\[
a_{j,i}=1\quad\text{se o bit }i\text{ foi observado};
\qquad a_{j,i}=0\quad\text{caso contrário}.
\]

Isso separa `0` de ausência.

### Gate C — Diferença bit a bit

Somente onde `a_{0,i}=a_{j,i}=1`:

\[
d_{j,i}=x_{0,i}\oplus x_{j,i}.
\]

### Gate D — Síndrome

Para cada stripe/código:

\[
s_j=H x_j^\top.
\]

A síndrome deve ser calculada **antes do banco vetorial**.

### Gate E — Recuperação exata

Para erasures conhecidos `E`:

\[
H_E x_E^\top=H_Kx_K^\top\pmod2.
\]

Recuperar somente se:

\[
\operatorname{rank}(H_E)=|E|
\]

e a solução passar por hash/proveniência. Caso contrário, `TOKEN_VAZIO`.

### Gate F — Índice vetorial

Somente após A–E, indexar o residual:

```text
posição + tempo + classe + ambiente + síndrome + geometria
```

O vetor serve para localizar recorrência e escolher o próximo ensaio; não autoriza preencher bits.

## 5. Régua adaptativa e salto

Se `R(p)` mede densidade de divergência em uma região/parâmetro `p`:

\[
\Delta R_k=|R(p_{k+1})-R(p_k)|.
\]

Refinar onde `ΔR_k` ultrapassa o limiar. Janelas Fibonacci podem definir tamanhos de inspeção, mas devem ser comparadas contra janelas uniformes e potências de dois.

O próximo ponto de cálculo é:

\[
p^*=\arg\max_p
[\alpha I(p)+\beta G(p)+\gamma U(p)-\lambda C(p)],
\]

onde:

- `I`: informação esperada;
- `G`: gradiente de erro;
- `U`: incerteza;
- `C`: custo de captura.

## 6. Interpretação geométrica

- hexágono: vizinhança de seis direções;
- octógono: projeção de oito direções;
- D dimensões: features adicionais de tempo, estágio, temperatura, tensão, shard e síndrome;
- geometria não altera a distância mínima do código;
- ganho geométrico precisa superar baseline fora da amostra.

## 7. Ordem prática recomendada

```text
1. gerar golden e paridades
2. capturar S0
3. executar somente um estágio
4. capturar Sj
5. calcular disponibilidade, XOR e síndrome
6. parar na primeira divergência j*
7. repetir o estágio isolado
8. variar uma única condição
9. tentar recuperação exata
10. indexar o residual no banco vetorial
```

## 8. Estado

```yaml
F_ok:
  - fronteiras de cálculo definidas
  - primeira divergência formalizada
  - ordem ECC antes de vetor preservada
F_gap:
  - hooks de captura em cada backend real
  - endereço físico e contadores ECC
  - experimento térmico controlado
F_next:
  - instrumentar S0/S1 no comparador Python/C existente
  - depois separar CPU escalar de NEON
```

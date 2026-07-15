# Anexo matemático — ciclo de 60 minutos, bases, 12² = 144 e grade de 0,1 Hz

> Integração complementar ao [`MAPA_MULTIPLEX_RLL_RAFAELIA_ECOSSISTEMA.md`](MAPA_MULTIPLEX_RLL_RAFAELIA_ECOSSISTEMA.md).
>
> Estado: `VERIFIED` para identidades aritméticas; `HYPOTHESIS / CLAIM_BLOCKED` para qualquer interpretação física ainda sem sinal, instrumento, unidade e ensaio.
>
> Regra: coincidência numérica organiza a investigação, mas não prova causalidade, ressonância física ou propriedade cosmológica.

## 1. O número 60 e a escrita do tempo

A quantidade “sessenta minutos” é o inteiro decimal:

```text
60₁₀ minutos = 3600₁₀ segundos
```

A escrita de horas, minutos e segundos é melhor descrita como sistema **sexagesimal misto**, porque cada campo reinicia em 60:

```text
01:00:00 = 1 hora = 60 minutos = 3600 segundos
```

Como inteiro em base 60:

```text
60₁₀ = 10₆₀
```

Portanto, “60 minutos” não significa que o número esteja originalmente escrito em base 60; significa que a unidade temporal usa uma fronteira de 60 para mudar de campo.

## 2. Conversão dos pontos 7, 35, 50, 60, 70 e 144

Convenções:

- em base 20, `A = 10` e `F = 15`;
- em base 70, algarismos maiores que 35 são escritos entre colchetes, pois o alfabeto comum não possui símbolos suficientes;
- a quantidade não muda quando a base muda: muda somente sua representação.

| Valor decimal | Base 10 | Base 2 | Base 20 | Base 7 | Base 70 |
|---:|---:|---:|---:|---:|---:|
| 7 | `7₁₀` | `111₂` | `7₂₀` | `10₇` | `[7]₇₀` |
| 35 | `35₁₀` | `100011₂` | `1F₂₀` | `50₇` | `[35]₇₀` |
| 50 | `50₁₀` | `110010₂` | `2A₂₀` | `101₇` | `[50]₇₀` |
| 60 | `60₁₀` | `111100₂` | `30₂₀` | `114₇` | `[60]₇₀` |
| 70 | `70₁₀` | `1000110₂` | `3A₂₀` | `130₇` | `10₇₀` |
| 144 | `144₁₀` | `10010000₂` | `74₂₀` | `264₇` | `24₇₀` |

Relações exatas relevantes:

```text
35 = 5 × 7
70 = 10 × 7
50 = 5 × 10
60 = 6 × 10
12 = 2 × 6
144 = 12²
```

## 3. Os pontos no círculo de 60 minutos

Em um ciclo completo de uma hora:

```text
60 minutos ↔ 360°
1 minuto   ↔ 6°
```

A transformação é:

\[
\theta(m)=360^\circ\frac{m}{60}=6m^\circ.
\]

| Marcador | Fração do ciclo | Ângulo acumulado | Posição modular |
|---:|---:|---:|---:|
| 7 min | `7/60` | `42°` | `42°` |
| 35 min | `7/12` | `210° = 5 × 42°` | `210°` |
| 50 min | `5/6` | `300°` | `300°` |
| 60 min | `1` | `360°` | `0°` |
| 70 min | `7/6` | `420°` | `60°` |

Assim, há uma relação geométrica limpa:

```text
7 minutos  → 42 graus
35 minutos → 5 × 7 minutos → 5 × 42 graus → 210 graus
70 minutos → 10 × 7 minutos → 10 × 42 graus → 420 graus
```

O marcador de 70 minutos já pertence ao segundo giro; reduzido módulo 60 minutos, ele coincide com 10 minutos. Reduzido módulo 360°, coincide com 60°.

## 4. Divisões por 6, 12 e 144

### Divisão por 6

```text
60 / 6 = 10 minutos por setor
360° / 6 = 60° por setor
```

### Divisão por 12

```text
60 / 12 = 5 minutos por setor
360° / 12 = 30° por setor
```

### Divisão por 144 = 12²

Como uma hora possui 3600 segundos:

\[
\Delta t_{144}=\frac{3600}{144}=25\ \text{segundos}.
\]

Logo:

```text
144 células × 25 segundos = 3600 segundos = 60 minutos
```

Também é possível ler a estrutura como uma malha `12 × 12`:

```text
12 setores principais × 5 minutos
cada setor principal dividido em 12 células
5 minutos / 12 = 25 segundos por célula
```

Essa é uma identidade exata, não uma aproximação.

## 5. As leituras 3, 6 e 9

É necessário distinguir valor, raiz quadrada, soma de dígitos e raiz digital:

```text
144 = 12²
√144 = 12
1 + 4 + 4 = 9
raiz digital decimal de 144 = 9
raiz digital decimal de 12 = 1 + 2 = 3
12 = 2 × 6
```

Portanto, a sequência ordenada abaixo produz a tríade decimal:

| Objeto | Operação | Resultado reduzido |
|---|---|---:|
| `12` | soma decimal dos dígitos | `3` |
| `6` | identidade | `6` |
| `144` | soma decimal dos dígitos | `9` |

```text
12 → 3
6  → 6
144 → 9
```

Isso sustenta um **índice aritmético 3–6–9**. A expressão popularmente ligada a Tesla pode servir como rótulo histórico-cultural, mas não é necessária para demonstrar essas identidades e não constitui prova física.

Importante: a soma de dígitos depende da base. Por exemplo, `144₁₀ = 264₇`; portanto, “1+4+4=9” é uma propriedade da escrita decimal de 144, não uma propriedade independente de representação.

## 6. Grade discreta em passos de 0,1 Hz

Defina a grade:

\[
f_k=0{,}1k\ \text{Hz},\qquad k\in\mathbb{Z}_{\ge 0}.
\]

Para `k > 0`, o período é:

\[
T_k=\frac{1}{f_k}=\frac{10}{k}\ \text{segundos}.
\]

Em 60 minutos, o número de ciclos é:

\[
N_k=f_k\cdot3600=360k.
\]

Para o primeiro passo:

```text
0,1 Hz = 1 ciclo a cada 10 segundos
       = 6 ciclos por minuto
       = 360 ciclos por hora
```

### Índices 3, 6, 9 e 12 na grade de 0,1 Hz

Há duas leituras diferentes, que não devem ser misturadas:

1. **Índices da grade** `k = 3, 6, 9, 12`:

| k | Frequência | Período | Ciclos/minuto | Ciclos/60 min |
|---:|---:|---:|---:|---:|
| 3 | `0,3 Hz` | `10/3 s` | `18` | `1080` |
| 6 | `0,6 Hz` | `5/3 s` | `36` | `2160` |
| 9 | `0,9 Hz` | `10/9 s` | `54` | `3240` |
| 12 | `1,2 Hz` | `5/6 s` | `72` | `4320` |

2. **Frequências literais** `3, 6, 9, 12 Hz`, correspondentes aos índices `30, 60, 90, 120` da mesma grade.

Essa separação impede confundir o número do passo com o valor físico em hertz.

## 7. Encaixe entre 144 células e a grade de 0,1 Hz

Cada uma das 144 células dura 25 segundos. Para `f_k = 0,1k Hz`, a quantidade de ciclos por célula é:

\[
C_k=f_k\cdot25=\frac{5k}{2}=2{,}5k.
\]

Consequência:

```text
k par   → número inteiro de ciclos em cada célula de 25 s
k ímpar → meia unidade; fechamento inteiro após duas células = 50 s
```

Formalmente:

\[
C_k\in\mathbb{Z}\iff k\equiv0\pmod2.
\]

Para `k` ímpar:

\[
2C_k=5k\in\mathbb{Z}.
\]

Exemplos:

| Frequência | k | Ciclos/25 s | Encaixe mínimo |
|---:|---:|---:|---:|
| `0,1 Hz` | 1 | `2,5` | `50 s → 5 ciclos` |
| `0,2 Hz` | 2 | `5` | `25 s` |
| `0,3 Hz` | 3 | `7,5` | `50 s → 15 ciclos` |
| `0,6 Hz` | 6 | `15` | `25 s` |
| `0,9 Hz` | 9 | `22,5` | `50 s → 45 ciclos` |
| `1,2 Hz` | 12 | `30` | `25 s` |

Para o passo fundamental de 0,1 Hz:

```text
144 células × 2,5 ciclos/célula = 360 ciclos por hora
72 pares de células × 5 ciclos/par = 360 ciclos por hora
```

O período comum mínimo entre a célula de 25 segundos e o período de 10 segundos é:

```text
MMC(25 s, 10 s) = 50 s
```

Essa é a costura discreta exata entre `144`, `60 minutos` e `0,1 Hz`.

## 8. Marcadores temporais não são automaticamente frequências

Os valores `7`, `35`, `50`, `60` e `70` foram tratados acima como **minutos/posições de fase**. Eles não se tornam automaticamente `7 Hz`, `35 Hz`, `50 Hz`, `60 Hz` ou `70 Hz`.

Para promover um marcador temporal a frequência física, é obrigatório declarar:

```text
fenômeno periódico
+ sinal medido
+ taxa de amostragem
+ unidade
+ janela temporal
+ método espectral
+ incerteza
```

Sem isso:

```text
estado = HYPOTHESIS ou TOKEN_VAZIO
```

## 9. Registro operacional candidato

```json
{
  "model_id": "RAF-CYCLE60-BASES-144-HZ01-v1",
  "cycle_seconds": 3600,
  "time_notation": "mixed-sexagesimal",
  "minute_to_degree": 6,
  "markers_minutes": [7, 35, 50, 60, 70],
  "subdivisions": 144,
  "cell_seconds": 25,
  "frequency_step_hz": 0.1,
  "frequency_indices": [3, 6, 9, 12],
  "frequency_values_hz": [0.3, 0.6, 0.9, 1.2],
  "odd_index_alignment_seconds": 50,
  "evidence_state": "MATHEMATICAL_IDENTITY",
  "physical_claim_state": "CLAIM_BLOCKED"
}
```

## 10. Invariante do anexo

\[
\boxed{
60\ \text{min}
=3600\ \text{s}
=144\times25\ \text{s}
}
\]

\[
\boxed{
f_k=0{,}1k\ \text{Hz}
\Longrightarrow
N_{60\text{min}}=360k
\quad\text{e}\quad
C_{25\text{s}}=\frac{5k}{2}
}
\]

Leitura segura:

```text
base organiza representação
ciclo organiza fase
144 organiza resolução temporal
0,1 Hz organiza a grade de frequência
3–6–9 organiza um índice decimal
experimento decide qualquer alegação física
```

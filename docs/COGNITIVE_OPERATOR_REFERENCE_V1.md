# Operadores Cognitivos — Implementação de Referência V1

**Estado:** `VERIFIED_LIMITED_LOCAL`  
**Claim gate:** `claim_allowed=false`  
**Registro canônico:** `rafaelmeloreisnovo/Mapa`, PR #161  
**Produtor de evidência:** `rafaelmeloreisnovo/RafPolimata`

## 1. Escopo

Esta implementação cobre cinco objetos matemáticos normalizados do registro 15–70:

| ID | Nome normalizado | Método |
|---|---|---|
| `COG-015` | segunda derivada ordinária | diferença central de segunda ordem |
| `COG-016` | segunda diferença regressiva | stencil uniforme regressivo |
| `COG-018` | derivada por função implícita | `-F_Φ/F_Ψ`, com bloqueio singular |
| `COG-019` | derivada de produto complexo | regra do produto com denominador constante |
| `COG-024` | segunda derivada de `ln Φ` | identidade analítica para `Φ>0` |

O código implementa os objetos normalizados, não transforma rótulos simbólicos em novas classes matemáticas.

## 2. Invariantes

```text
implementation != evidence of generality
analytic fixture PASS != production PASS
finite difference != exact derivative
constant denominator != varying denominator
physical analogy != mechanism
claim_allowed=false
```

Todos os resultados retornam:

```text
operator_id
value_real
value_imag
state=VERIFIED_LIMITED_LOCAL
claim_allowed=false
```

## 3. Fronteiras operacionais

### COG-015

Usa:

```text
[f(τ+h) - 2f(τ) + f(τ-h)] / h²
```

Requer `h>0`. A aproximação possui erro de truncamento e deve receber estudo de convergência antes de uso científico.

### COG-016

Usa três amostras uniformemente espaçadas:

```text
[Ψ(τ) - 2Ψ(τ-Δ) + Ψ(τ-2Δ)] / Δ²
```

Isto é uma diferença finita regressiva. Não implementa retropropagação de rede neural automaticamente.

### COG-018

Implementa:

```text
-dF/dΦ ÷ dF/dΨ
```

A operação é rejeitada quando `|dF/dΨ|` está abaixo da tolerância singular.

### COG-019

Implementa a derivada de:

```text
A'(τ) exp(i phase(τ)) / denominator
```

sob a hipótese explícita de que o denominador é constante em `τ`. Se `ΔΣχ` variar, esta implementação não é aplicável porque faltam termos da regra do quociente.

### COG-024

Implementa:

```text
(Φ Φ'' - Φ'²) / Φ²
```

somente para `Φ` real, finito e estritamente positivo.

## 4. Validação

```bash
python3 -m py_compile \
  scripts/cognitive_operator_reference.py \
  tests/test_cognitive_operator_reference.py

python3 -m unittest -v tests/test_cognitive_operator_reference.py

python3 scripts/cognitive_operator_reference.py \
  --output build/cognitive-operator-reference/report.json
```

Fixtures analíticas:

- derivada de polinômio quadrático;
- segunda diferença de sequência quadrática;
- derivada implícita regular e denominador singular;
- produto complexo comparado à solução analítica;
- `d² ln(exp τ)/dτ² = 0`;
- passos nulos/negativos e domínio não positivo rejeitados.

## 5. Estados honestos

| Corpo | Estado |
|---|---|
| Código de referência | `IMPLEMENTED` |
| Testes locais analíticos | `PASS_LOCAL_ANALYTIC_FIXTURES` |
| Generalidade numérica | `TOKEN_VAZIO_CONVERGENCE` |
| CI remoto deste commit | `TOKEN_VAZIO_RUNNER` até execução observável |
| Benchmark p50/p95/p99 | `TOKEN_VAZIO_BENCHMARK` |
| Execução Termux ARM32/ARM64 | `TOKEN_VAZIO_DEVICE` |
| Claim científico/cognitivo | `claim_allowed=false` |

## 6. Próximos falsificadores

1. executar estudo de convergência para `COG-015` e `COG-016` em múltiplos passos;
2. comparar `COG-018` com diferenças finitas de uma curva implícita conhecida;
3. adicionar variante completa de `COG-019` com denominador variável;
4. executar em Termux ARM32/ARM64 e registrar ambiente, comando, stdout, stderr e hashes;
5. comparar os resultados com diferenciação simbólica ou automática independente.

## 7. Retroalimentação

- **F_ok:** cinco operadores saíram do estado apenas formal e receberam código limitado e testes analíticos.
- **F_gap:** convergência, desempenho, device e generalidade permanecem `TOKEN_VAZIO`.
- **F_next:** executar o gate remoto e acrescentar estudo de refinamento de malha sem ampliar o claim.

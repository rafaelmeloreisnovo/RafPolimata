# Quatro eixos, Venturi de base e retorno vortex — ponte de evidência

**Estado:** `AUDIT | exact_geometry=PASS | operator_state=PASS`  
**Interpretação fluida/material:** `NOT_APPLICABLE`  
**Autor:** Rafael Melo Reis (∆RafaelVerboΩ)

## 1. Correção epistemológica

A classificação anterior estava errada ao tratar `Venturi` e `vortex` como hipóteses físicas. Neste modelo eles são nomes de operadores matemático-computacionais:

- `Venturi`: dobra uma abscissa linear numa abertura finita de base, preservando carry;
- `vortex`: executa retorno cíclico por soma, preservando fase, ciclo e winding.

Não existe claim de pressão, fluido, viscosidade ou vórtice material a promover. Por isso:

```text
operator_state = PASS
empirical_fluid_interpretation = NOT_APPLICABLE
```

## 2. Quatro eixos e oito portas

```text
vertical:          upper <-> lower
horizontal:        right <-> left
diagonal_rising:   upper_right <-> lower_left
diagonal_falling:  upper_left <-> lower_right
```

Para `C=(R,0)` e raio `r`:

\[
P(\theta)=C+r(\cos\theta,\sin\theta).
\]

Cada par satisfaz:

\[
\frac{P_++P_-}{2}=C,
\qquad
\|P_+-P_-\|=2r.
\]

A direção tangente satisfaz:

\[
\hat n=(\cos\theta,\sin\theta),
\quad
\hat t=(-\sin\theta,\cos\theta),
\quad
\hat n\cdot\hat t=0.
\]

As diagonais usam:

\[
\frac1{\sqrt2}(\pm1,\pm1).
\]

## 3. Operador Venturi de base

Para `n>=0` e `b>=2`:

\[
n=bq+r,
\qquad
0\le r<b.
\]

Define-se:

\[
\boxed{\mathcal V_b(n)=(q,r)}.
\]

A abscissa é projetada numa abertura finita `r`, mas o carry `q` não é descartado. A inversa é:

\[
n=bq+r.
\]

Logo, o operador não extingue número.

## 4. Sete decimal em base sete

Na notação posicional ordinária:

\[
\boxed{7_{10}=10_7}.
\]

Pela divisão euclidiana:

\[
7=1\cdot7+0
\Rightarrow
(q,r)=(1,0).
\]

Portanto:

\[
7\bmod7=0
\]

é somente a projeção do resto. Não representa o estado completo.

## 5. Numeração cíclica sem fase zero

Para números positivos:

\[
\boxed{p_b(n)=1+((n-1)\bmod b)}.
\]

Assim:

\[
p_7(7)=7,
\qquad
p_7(8)=1.
\]

O símbolo `7` aqui é rótulo de fase one-based, não dígito posicional da base sete.

## 6. Curvatura da abscissa

\[
\Theta_b(n)=\frac{2\pi n}{b}.
\]

A projeção circular é:

\[
\gamma_b(n)=(R\cos\Theta_b(n),R\sin\Theta_b(n)).
\]

`n=0` e `n=b` chegam à mesma costura circular. Para preservar a volta, utiliza-se o levantamento:

\[
\Gamma_b(n)=
\left(
R\cos\Theta_b(n),
R\sin\Theta_b(n),
\lambda\frac{\Theta_b(n)}{2\pi}
\right).
\]

A coordenada pode retornar; o winding não desaparece.

## 7. Operador vortex aditivo

Para `delta>=0`:

\[
\boxed{\mathcal W_b(n,\delta)=E_b(n+\delta)}.
\]

O sistema avança apenas por soma:

```text
6 + 1 -> fase 7
7 + 1 -> fase 1, com novo carry/ciclo
```

O retorno visual à primeira fase não é apagamento histórico.

## 8. Zero válido

```text
zero como valor        != informação ausente
zero como resto        != estado total
zero como costura      != extinção da volta
bloco desligado        != bloco inexistente
```

## 9. Assembly unsigned

Uma subtração pode ser executada por soma do complemento de dois:

\[
a-b\equiv a+(\sim b+1)\pmod{2^w}.
\]

No operador RAFAELIA, o domínio é não negativo e as transições são soma, carry e wrap. Isso é uma convenção computacional explícita; não é a afirmação de que todas as ISAs deixem de possuir instruções de subtração.

## 10. Claims governados

| Claim | Classe | Estado |
|---|---|---|
| quatro eixos geram oito portas | `[E]` | `PASS` |
| pares opostos preservam centro e diâmetro | `[E]` | `PASS` |
| tangente é perpendicular à radial | `[E]` | `PASS` |
| diagonais normalizadas usam `1/sqrt(2)` | `[E]` | `PASS` |
| `Venturi = V_b(n)=(q,r)` reversível | `[E]` | `PASS` |
| `vortex = W_b(n,delta)` aditivo | `[E]` | `PASS` |
| `7_10=10_7` | `[E]` | `PASS` |
| `(q,r)=(1,0)` preserva o carry | `[E]` | `PASS` |
| fase one-based de 7 é 7 | `[E]` | `PASS` |
| zero é estado válido | `[E]` | `PASS` |

## 11. Roteamento

- `ChipQuantum`: runtime e testes;
- `RafPolimata`: claims e promoção;
- `RLL/PapersPub`: formalização acadêmica.

## 12. Validação

```bash
python3 scripts/validate_flow_port_axial_diagonal_governance.py
python3 -m unittest -v tests/test_flow_port_axial_diagonal_governance.py
```

Resultado esperado e registrado:

```text
16/16 gates PASS
claim_allowed = true
operator_state = PASS
empirical_fluid_interpretation = NOT_APPLICABLE
```

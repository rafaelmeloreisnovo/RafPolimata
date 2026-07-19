# Quatro eixos de portas de fluxo — ponte de evidência

**Estado:** `AUDIT | exact_geometry=PASS | physical_state=TOKEN_VAZIO`  
**Autor:** Rafael Melo Reis (∆RafaelVerboΩ)

## 1. Objeto governado

A extensão formaliza quatro eixos opostos atravessando um mesmo centro geométrico:

```text
vertical:          upper <-> lower
horizontal:        right <-> left
diagonal_rising:   upper_right <-> lower_left
diagonal_falling:  upper_left <-> lower_right
```

São quatro diâmetros e oito bocas, não quatro pontos independentes.

## 2. Fórmulas aceitas

Para centro `C=(R,0)` e raio `r`:

\[
P(\theta)=C+r(\cos\theta,\sin\theta).
\]

Cada par oposto satisfaz:

\[
\frac{P_++P_-}{2}=C,
\qquad
\|P_+-P_-\|=2r.
\]

A direção tangente é perpendicular à radial:

\[
\hat n=(\cos\theta,\sin\theta),
\qquad
\hat t=(-\sin\theta,\cos\theta),
\qquad
\hat n\cdot\hat t=0.
\]

As projeções diagonais usam:

\[
\frac1{\sqrt2}(\pm1,\pm1).
\]

## 3. Relação matricial

A vizinhança governada é:

```text
(0,+1)  upper
(0,-1)  lower
(+1,0)  right
(-1,0)  left
(+1,+1) upper_right
(-1,-1) lower_left
(-1,+1) upper_left
(+1,-1) lower_right
```

A escolha da vizinhança axial + diagonal é `[C]`, enquanto as posições, distâncias, diâmetros e perpendicularidades são `[E]`.

## 4. Projeção toroidal

Cada boca meridiana `(rho_P,z_P)` pode ser varrida pelo ângulo `u`:

\[
X_P(u)=(\rho_P\cos u,\rho_P\sin u,z_P).
\]

Isso cria oito anéis projetivos no toro. A varredura é uma transformação geométrica; não comprova um fluxo físico.

## 5. Claims

| Claim | Classe | Estado |
|---|---|---|
| quatro eixos geram oito bocas | `[E]` | `PASS` |
| pares opostos têm centro comum e diâmetro `2r` | `[E]` | `PASS` |
| tangente é perpendicular à radial | `[E]` | `PASS` |
| oito direções cobrem axial + diagonal | `[E/C]` | `PASS/REFERENCE` |
| diagonais normalizadas usam `1/sqrt(2)` | `[E]` | `PASS` |
| bocas são gates computacionais da matriz | `[C]` | `REFERENCE` |
| bocas produzem Venturi físico | `[H]` | `TOKEN_VAZIO` |
| bocas produzem vórtice físico | `[H]` | `TOKEN_VAZIO` |

## 6. Gates físicos

A promoção de Venturi ou vórtice exige, no mínimo:

- área e perfil das bocas;
- pressão;
- densidade;
- viscosidade;
- vazão;
- circulação;
- equações governantes;
- condições iniciais e de contorno;
- dataset;
- incerteza;
- falsificador;
- reprodução independente.

Sem esses elementos:

```text
physical_state = TOKEN_VAZIO
claim_allowed = false
```

## 7. Roteamento

- `ChipQuantum`: implementação e testes numéricos;
- `RafPolimata`: claims, proveniência e promoção;
- `RLL/PapersPub`: formalização acadêmica.

## 8. Validação

```bash
python3 scripts/validate_flow_port_axial_diagonal_governance.py
python3 -m unittest -v tests/test_flow_port_axial_diagonal_governance.py
```

Resultado registrado:

```text
16/16 gates PASS
exact_geometry_state = PASS
physical_state = TOKEN_VAZIO
```

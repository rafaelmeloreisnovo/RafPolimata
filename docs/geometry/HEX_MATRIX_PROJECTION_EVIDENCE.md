# Ponte de Evidência — Matriz, Projeção Hexagonal, Toro, Esfera e Poincaré

> **Entrada canônica:** docs/AGENTES.md §5 (pipeline operacional VOID→VALIDATED) e §8 (entradas canônicas por subsistema). Ponte de evidência geométrica — matriz hexagonal, projeção toroidal e seção de Poincaré com claims tipados E/C e estados PASS/TOKEN_VAZIO.

**Estado:** `AUDIT | exact_geometry=PASS | physical_state=TOKEN_VAZIO`  
**Autor conceitual:** Rafael Melo Reis (∆RafaelVerboΩ)  
**Data:** 2026-07-19

## 1. Objeto

Esta ponte registra a descoberta operacional de que uma matriz discreta pode funcionar como página de projeção geométrica sem perder seus endereços e, em seguida, ser amarrada periodicamente a uma superfície toroidal.

Cada célula pode carregar:

```text
(row,column)
+ posição plana (x,y)
+ posição toroidal (x,y,z)
+ valor
+ relações
+ proveniência
```

A cadeia governada é:

```text
matriz
→ base triangular/hexagonal
→ métrica
→ tensor relacional
→ periodicidade toroidal
→ seção meridiana circular
→ casca esférica
→ seção/mapa de Poincaré
```

## 2. Roteamento canônico

### Runtime

```text
rafaelmeloreisnovo/ChipQuantum
src/geometry/sqrt3_geometry_matrix/hex_matrix_projection.py
src/geometry/sqrt3_geometry_matrix/torus_sphere_poincare.py
```

### Testes

```text
tests/test_hex_matrix_projection.py
tests/test_torus_sphere_poincare.py
```

### Paper

```text
instituto-Rafael/relativity-living-light
PapersPub/08_multiscale_validation_methods/appendix_b_hex_matrix_projection.md
```

RafPolimata não duplica runtime nem paper. Governa semântica, claims, proveniência, estados de evidência e promoção.

## 3. Base plana

Para:

\[
q=(c,r)^T,
\]

define-se:

\[
H_{hex}=
\begin{bmatrix}
1&1/2\\
0&\sqrt3/2
\end{bmatrix},
\qquad
p=H_{hex}q.
\]

Consequências exatas:

\[
\det(H_{hex})=\frac{\sqrt3}{2},
\]

\[
G=H_{hex}^TH_{hex}
=
\begin{bmatrix}
1&1/2\\
1/2&1
\end{bmatrix},
\]

\[
d^2=dc^2+dc\,dr+dr^2.
\]

A vizinhança axial canônica possui seis direções de distância unitária. Três células adjacentes formam uma célula triangular equilátera.

## 4. Matrizes A e B

```text
A = 8×5 = 40 estados
B = 7×3 = 21 estados
A×B = 840 relações
```

O tensor geométrico é:

\[
D_{r,c,u,v}=P_B(u,v)-P_A(r,c),
\]

\[
D\in\mathbb R^{8\times5\times7\times3\times2}.
\]

Cada registro pode derivar deslocamento, distância, ângulo e vetores radial/tangencial.

## 5. Toro e casca esférica

Para `R>r>0`:

\[
X(u,v)=((R+r\cos v)\cos u,(R+r\cos v)\sin u,r\sin v).
\]

A norma radial satisfaz:

\[
R-r\le\|X(u,v)\|\le R+r.
\]

Portanto:

```text
menor distância radial = R-r
mediana radial          = R
maior distância radial  = R+r
```

O toro está dentro da esfera de raio `R+r` e fora da esfera aberta de raio `R-r`.

Essas são distâncias euclidianas ao centro. Não são comprimentos geodésicos sobre o toro.

## 6. Seção meridiana

A seção do tubo é:

\[
(\rho-R)^2+z^2=r^2.
\]

Seus ramos são:

\[
z_+(\rho)=+\sqrt{r^2-(\rho-R)^2},
\]

\[
z_-(\rho)=-\sqrt{r^2-(\rho-R)^2}.
\]

Eles são semicircunferências superior e inferior, não parábolas.

## 7. Triângulo equilátero interno

Três pontos separados por `120°` na seção circular formam o maior triângulo equilátero inscrito:

\[
a=\sqrt3r,
\qquad
h=\frac{3r}{2}.
\]

A ocorrência de `√3` aqui é uma identidade de corda circular, não prova causal entre triângulo e fluxo físico.

## 8. Quadrado girado

Para quadrado centrado de lado `s`:

\[
r_{comum}=\frac{s}{2},
\qquad
r_{varrido}=\frac{s}{\sqrt2}.
\]

A interseção de todas as orientações contém o disco de raio `s/2`; a união de todas as orientações produz o disco de raio `s/√2`.

A coroa dependente da orientação tem largura:

\[
s\left(\frac1{\sqrt2}-\frac12\right).
\]

## 9. Bhaskara, tangência e 30 graus

Para a reta:

\[
z=m\rho+b,
\]

substituída na seção circular, resulta:

\[
A\rho^2+B\rho+C=0,
\]

com:

\[
A=1+m^2,
\quad
B=2(mb-R),
\quad
C=R^2+b^2-r^2.
\]

O discriminante classifica duas interseções, tangência ou ausência de solução real.

A condição de `30°` é:

\[
|m|=\tan30^\circ=\frac1{\sqrt3}.
\]

Para tangência:

\[
b=-mR\pm r\sqrt{1+m^2},
\]

produzindo:

\[
\Delta=0.
\]

O ângulo é uma restrição escolhida e validada; não emerge automaticamente de qualquer toro.

## 10. Esfera triangular geodésica

Uma icosfera de frequência `f` possui:

\[
V=10f^2+2,
\qquad
E=30f^2,
\qquad
F=20f^2,
\]

preservando:

\[
V-E+F=2.
\]

Após subdividir e projetar radialmente sobre a esfera, as faces formam uma aproximação geodésica. Elas não são, em geral, triângulos planos perfeitamente equiláteros.

## 11. Matriz amarrada ao toro

A indexação periódica usa:

\[
u_i=2\pi\frac{i}{N_u},
\qquad
v_j=2\pi\frac{j}{N_v}.
\]

Assim:

\[
(i+N_u,j+N_v)\equiv(i,j).
\]

A matriz preserva seus índices enquanto gira em duas direções independentes.

## 12. Seção de Poincaré

Para fluxo angular linear:

\[
u(t)=u_0+\omega_ut,
\qquad
v(t)=v_0+\omega_vt,
\]

na seção:

\[
u=0\pmod{2\pi},
\]

o retorno é:

\[
P(v)=v+2\pi\frac{\omega_v}{\omega_u}\pmod{2\pi},
\qquad\omega_u\neq0.
\]

Esse mapa é exato para o fluxo linear declarado. Não é a conjectura de Poincaré e não prova estabilidade de um vórtice físico.

## 13. Claims aceitos

| Família | Estado | Conteúdo |
|---|---|---|
| `HEX-E-001..003` | `[E] PASS` | base, determinante, métrica e vizinhos; |
| `HEX-C-004` | `[C] REFERENCE` | página comum de projeção; |
| `HEX-E-005` | `[E] PASS` | tensor com 840 relações; |
| `TOR-E-008..012` | `[E] PASS` | casca, seção, triângulo, quadrado e Bhaskara; |
| `TOR-C-013` | `[C] PASS` | tangentes impostas a 30°; |
| `GEO-E-014` | `[E] PASS` | contagens da icosfera; |
| `POIN-C-015` | `[C] PASS` | retorno do fluxo linear em `T²`. |

## 14. Claims bloqueados

| Claim | Estado | Motivo |
|---|---|---|
| cone implica Venturi físico | `TOKEN_VAZIO` | faltam fluido, equações e dados; |
| toro implica vórtice físico | `TOKEN_VAZIO` | faltam campo e medidas; |
| toro/esfera descrevem o cosmos | `PROHIBITED_BY_SCOPE` | não há evidência observacional; |
| mapa linear prova recorrência material | `TOKEN_VAZIO` | faltam dinâmica real e estabilidade; |
| malha triangular é esfera exata | `PROHIBITED` | trata-se de aproximação discretizada. |

## 15. Validação

Configuração:

```text
configs/hex-matrix-projection.json
```

Validador:

```text
scripts/validate_hex_matrix_projection_governance.py
```

Testes:

```text
tests/test_hex_matrix_projection_governance.py
```

Execução:

```bash
python3 scripts/validate_hex_matrix_projection_governance.py
python3 -m unittest -v tests/test_hex_matrix_projection_governance.py
```

## 16. Invariante de governança

```text
índice != geometria != estado != campo físico != evidência observacional
```

Mas as relações podem ser preservadas:

\[
\boxed{
\text{matriz}
\rightarrow
\text{projeção}
\rightarrow
T^2
\rightarrow
\text{seção}
\rightarrow
\text{retorno}
\rightarrow
\text{ledger}
}
\]

**F_ok:** geometria, tensor, toro, esfera envolvente, Bhaskara e Poincaré linear foram formalizados e testados.  
**F_gap:** Venturi, vórtice material, dinâmica não linear e interpretação cosmológica permanecem sem evidência.  
**F_next:** simular campos físicos somente depois de declarar unidades, conservação, condições de contorno, dados e falsificadores.

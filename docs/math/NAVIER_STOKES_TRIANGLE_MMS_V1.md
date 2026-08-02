# PRM-08 — Gate manufaturado incompressível 2D em triângulos — V1

**Estado:** `G3_IMPLEMENTATION_LOCAL_PASS`  
**Claim permitido:** conservação discreta e residual analítico para o caso manufaturado definido abaixo.  
**Claim bloqueado:** solução do problema de regularidade de Navier–Stokes em 3D.

## 1. Equação e campo manufaturado

Usa-se a equação estacionária

\[
(\mathbf u\cdot\nabla)\mathbf u
=
-\nabla p+\nu\Delta\mathbf u+\mathbf f.
\]

A função corrente é

\[
\psi(x,y)=x^2y-xy^2.
\]

O campo derivado é

\[
\mathbf u=
\left(
\frac{\partial\psi}{\partial y},
-\frac{\partial\psi}{\partial x}
\right)
=
(x^2-2xy,\;y^2-2xy).
\]

Logo,

\[
\nabla\cdot\mathbf u
=
(2x-2y)+(2y-2x)=0.
\]

Escolhe-se

\[
p(x,y)=x+y,\qquad \nabla p=(1,1),
\]

e o forcing é calculado, não ajustado narrativamente:

\[
\mathbf f
=
(\mathbf u\cdot\nabla)\mathbf u+\nabla p-\nu\Delta\mathbf u.
\]

Assim, o residual da equação é zero até erro de ponto flutuante.

## 2. Fluxo orientado por aresta

Para uma aresta orientada \(P\to Q\), a implementação usa

\[
F_{P\to Q}
=
\int_{P}^{Q}\mathbf u\cdot\mathbf n\,ds
=
\psi(Q)-\psi(P).
\]

Em uma face triangular orientada \(A\to B\to C\to A\),

\[
F_{AB}+F_{BC}+F_{CA}=0.
\]

A conservação é telescópica e independe da forma do triângulo.

## 3. Famílias testadas

- \(45^\circ\!-\!45^\circ\!-\!90^\circ\);
- \(30^\circ\!-\!60^\circ\!-\!90^\circ\);
- equilátero;
- isósceles \(10^\circ\!-\!85^\circ\!-\!85^\circ\).

## 4. O que foi provado pelo teste

- divergência analítica nula nos pontos amostrados;
- residual de momento nulo para viscosidades \(0\), \(0.1\) e \(1\);
- balanço de fluxo nulo nas quatro famílias;
- orientação reversa muda a área assinada, mas preserva o fechamento do fluxo;
- triângulos degenerados e viscosidade negativa são rejeitados;
- `claim_allowed=false`.

## 5. O que permanece aberto

```yaml
solver_temporal: TOKEN_VAZIO
convergencia_h: TOKEN_VAZIO
erro_L2_H1: TOKEN_VAZIO
comparacao_FEM_FVM: TOKEN_VAZIO
estabilidade_CFL: TOKEN_VAZIO
navier_stokes_3d_regularity_solution: NOT_CLAIMED
independent_replication: TOKEN_VAZIO
```

## 6. Próximo gate

Construir uma malha com faces compartilhadas, montar operadores discretos e executar método de solução manufaturada sob refinamento:

\[
h\downarrow
\quad\Rightarrow\quad
\|u_h-u\|_{L^2},\;
\|p_h-p\|_{L^2}
\]

com ordem observada e baseline FEM/FVM congelados.

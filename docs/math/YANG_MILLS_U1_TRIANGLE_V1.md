# PRM-09 — Transporte de gauge \(U(1)\) em complexos triangulares — V1

**Estado:** `G3_IMPLEMENTATION_LOCAL_PASS`  
**Claim permitido:** invariância de holonomia no modelo clássico discreto \(U(1)\) definido abaixo.  
**Claims bloqueados:** construção quântica de Yang–Mills 4D e mass gap.

## 1. Variáveis de aresta

Cada aresta orientada carrega

\[
U_{AB}\in U(1),\qquad |U_{AB}|=1,
\]

com inversão

\[
U_{BA}=U_{AB}^{-1}=\overline{U_{AB}}.
\]

Para campo magnético constante \(B\), usa-se o gauge simétrico

\[
\mathbf A=(-By/2,\;Bx/2).
\]

A integral exata em uma aresta reta \(P\to Q\) fornece

\[
U_{PQ}
=
\exp\left[
\frac{iB}{2}
(x_Py_Q-y_Px_Q)
\right].
\]

## 2. Holonomia da face

Para a face orientada \(A\to B\to C\to A\),

\[
U_\triangle=U_{AB}U_{BC}U_{CA}.
\]

No campo constante,

\[
\boxed{
U_\triangle
=
e^{\,iB\,K_{\rm signed}}
}
\]

onde \(K_{\rm signed}\) é a área orientada.

A ação clássica mínima é

\[
S_\triangle=w_\triangle\left(1-\Re U_\triangle\right),
\qquad w_\triangle\ge 0.
\]

## 3. Transformação de gauge

Com \(g_A,g_B,g_C\in U(1)\),

\[
U_{AB}'
=
g_AU_{AB}g_B^{-1}.
\]

Então,

\[
U_\triangle'=U_\triangle.
\]

O cancelamento é testado numericamente.

## 4. Refinamento

A face \(ABC\) é dividida no ponto médio \(M\) de \(BC\):

\[
ABC\rightarrow ABM+AMC.
\]

Para o modelo abeliano,

\[
U_{ABM}\,U_{AMC}=U_{ABC}.
\]

Os links internos se cancelam e os segmentos \(B\to M\to C\) recompõem \(B\to C\).

## 5. O que foi provado pelo teste

- links possuem módulo unitário;
- holonomia coincide com \(e^{iBK_{\rm signed}}\);
- transformação de gauge preserva a holonomia;
- inversão da orientação produz o conjugado;
- produto das subfaces coincide com a face original;
- campo nulo produz identidade e ação zero;
- links não unitários e faces degeneradas são rejeitados;
- `claim_allowed=false`.

## 6. O que permanece aberto

```yaml
non_abelian_SU2: TOKEN_VAZIO
continuum_limit: TOKEN_VAZIO
renormalization: TOKEN_VAZIO
quantum_measure: TOKEN_VAZIO
yang_mills_4d_construction: NOT_CLAIMED
mass_gap: NOT_CLAIMED
independent_replication: TOKEN_VAZIO
```

## 7. Próximo gate

Evoluir de uma face para um complexo orientado com:

- tabela canônica de vértices, arestas e faces;
- cada aresta armazenada uma única vez;
- loops de Wilson em múltiplas escalas;
- testes de refinamento;
- primeiro baseline \(U(1)\), depois agenda separada para \(SU(2)\).

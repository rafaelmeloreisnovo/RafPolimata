# Triangle Geometry Atlas V1

## Scope

A deterministic reference implementation for triangles represented by vertices, side lengths, angles, Gram matrices, areas, altitudes, and geometric classes.

The package validates four canonical families:

1. equilateral triangles and the factor \(\sqrt3/2\);
2. isosceles triangles with a \(10^\circ\) vertex angle;
3. right triangles with a \(10^\circ\) acute angle;
4. right isosceles triangles.

## Core identities

For \(u=B-A\) and \(v=C-A\):

\[
G=\begin{bmatrix}u\cdot u&u\cdot v\\v\cdot u&v\cdot v\end{bmatrix},
\qquad
K=\frac12\sqrt{\det G}.
\]

For sides \(a,b,c\):

\[
h_a=\frac{2K}{a},\qquad h_b=\frac{2K}{b},\qquad h_c=\frac{2K}{c}.
\]

For an equilateral triangle of side \(l\):

\[
h=l\frac{\sqrt3}{2},\qquad K=\frac{\sqrt3}{4}l^2.
\]

For an isosceles triangle with equal sides \(l\) and vertex angle \(10^\circ\):

\[
b=2l\sin5^\circ,\qquad h=l\cos5^\circ,\qquad K=\frac12l^2\sin10^\circ.
\]

For a right triangle with hypotenuse \(H\) and acute angle \(10^\circ\):

\[
x=H\cos10^\circ,\quad y=H\sin10^\circ,\quad x^2+y^2=H^2.
\]

The algebraic bridge is:

\[
8\cos^3(10^\circ)-6\cos(10^\circ)=\sqrt3.
\]

## Spiral operator

\[
z_{n+1}=\frac{\sqrt3}{2}e^{i\pi/3}z_n.
\]

This is a contraction plus rotation. It is not a universal physical law.

## Scientific boundary

The atlas may be used as a geometric substrate for numerical experiments on triangular meshes. It does **not** solve Navier–Stokes and does **not** establish the Yang–Mills mass gap.

```yaml
claim_allowed: false
local_tests: 10/10 PASS
physical_runtime: TOKEN_VAZIO
independent_replication: TOKEN_VAZIO
```

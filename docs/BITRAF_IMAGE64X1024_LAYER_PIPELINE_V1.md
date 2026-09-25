# BITRAF IMAGE64x1024 - Layer / Vector / Sequence Pipeline V1

Date: 2026-09-25
State: IMPLEMENTED_REFERENCE
claim_allowed: false

Purpose: convert raster images into deterministic derived layers before semantic shape labels.

Pipeline:
source bytes -> RGB/CMYK -> luminance Q10 -> 8x8 spatial lanes -> 64x1024 histogram carrier -> gradient magnitude/orientation -> directional sine sweep -> rotated-copy similarity -> Fibonacci/Rafael/Tribonacci traversal probes -> exploratory morphology -> JSON/matrix/SVG.

Carrier:
lane l in 0..63.
Y = 0.2126 R + 0.7152 G + 0.0722 B.
s = round(1023 Y / 255).
M[l,s] = count of pixels in lane l with Q10 state s.
M is 64x1024 and has 65536 cells.
Address a = 1024*l + s, 0 <= a < 65536.
Since 20^3=8000 < 65536 < 20^4=160000, four base-20 digits suffice.

This is a typed image carrier. It does not redefine BITRAF Matrix V1 with 20^3=8000 states and 4096 binary core states.

Color layers:
RGB is retained as additive source color.
For normalized r,g,b:
K=1-max(r,g,b).
If K<1:
C=(1-r-K)/(1-K), M=(1-g-K)/(1-K), Yc=(1-b-K)/(1-K).

Vector field:
grad I = (Ix,Iy)
G=sqrt(Ix^2+Iy^2)
theta=atan2(Iy,Ix)
Orientation energy is grouped in 22.5 degree sectors.

Directional sine sweep:
S(phi,lambda,phase)=sin(2*pi/lambda*(x*cos(phi)+y*sin(phi))+phase).
The implementation measures sine/cosine correlation over frozen angle/wavelength sets. This is a directional texture response, not a field measurement.

Rotated copies:
45, 90, 135 and 180 degree centered comparisons are descriptive similarities, not proof of exact symmetry.

Canonical sequence authority:
rafaelmeloreisnovo/Matem-tica-/docs/formal/FIBONACCI_INVERSE_REVERSE_JUMP_RULER_V1.md

Classical Fibonacci:
F0=0, F1=1, F(n+1)=F(n)+F(n-1).

Reverse recurrence:
F(n-1)=F(n+1)-F(n).

Exact inverse:
F^-1(x)={n:F_n=x}.

Quantized inverse ruler:
n*(D)=argmin_n |F_n-D|.

Rafael affine:
R1=2, R2=4, R(n+1)=R(n)+R(n-1)+1.
R_n=F_(n+3)-1.

Multiscale ruler:
R_RAF(n)=(F_n,F_(n+1),F_(n+3)-1).

Tribonacci Rafael:
T_n=T_(n-1)+T_(n-2)+T_(n-3)
with documented prefix 0,0,0,1,1,2,4,7,13,24,44,...

Sequence probes are traversal diagnostics only. Matrix optimality and workload advantage remain TOKEN_VAZIO.

Exploratory morphology namespace:
The user-specified DMT geometry label is preserved only as a visual/morphological namespace. V1 measures circular-arc alignment, radial-line alignment, signed spiral bias and absolute spiral bias. No domain interpretation is inferred from those image metrics.

ZIPRAF boundary:
The adapter can serialize 64*1024 uint32 little-endian counts. This prepares a deterministic carrier. It does not establish compression superiority, entropy violation, traversal optimality, or cryptographic security.

Usage:
python3 scripts/bitraf_image64x1024_layers.py input.ppm --json report.json --matrix-bin matrix.bin --svg report.svg

PNG/JPEG may be read when Pillow is available; PPM is the dependency-free route.

Every report carries source SHA-256 and original dimensions. Derived layers never replace the source.

F_ok: typed 64x1024 carrier, color layers, vector orientations, sine sweep, rotated copies, canonical sequence probes, exploratory morphology, base20 addressing and deterministic outputs.
F_gap: benchmark against Gray/Morton/Hilbert/random traversal, physical-device replay, ZIPRAF codec comparison and independent corpus.
F_next: batch-run frozen session images, publish aggregate receipt, then let Papers retain all interpretations while RLL consumes only predeclared formal diagnostics.

# BITRAF Matrix 8000/4096 V1

## 1. Invariante de contagem

A matriz total é definida por cinco coordenadas:

\[
S=(x,y,z,v,p)
\]

com:

- \(x,y,z\in\{0,\ldots,9\}\): três eixos visíveis `10×10×10`;
- \(v\in\{0,1,2,3\}\): quatro vértices tetraédricos ocultos;
- \(p\in\{0,1\}\): dois estados de paridade.

Logo:

\[
\boxed{10^3\times4\times2=8000=20^3}
\]

Todo estado recebe exatamente três dígitos de base 20:

```text
000 ... JJJ
```

O alfabeto adotado é:

```text
0123456789ABCDEFGHIJ
```

Não há módulo nem colisão: `000..JJJ` cobre exatamente os índices `0..7999`.

## 2. Núcleo binário 4096 dentro da matriz vigesimal

O cubo interno usa as coordenadas `1..8` em cada eixo:

\[
8^3=512\ \text{células}
\]

Com quatro vértices ocultos e duas paridades:

\[
\boxed{8^3\times4\times2=4096=2^{12}}
\]

A casca externa é:

\[
(10^3-8^3)\times4\times2
=(1000-512)\times8
=3904
\]

Portanto:

```text
MATRIX8000 = CORE4096 ⊕ SHELL3904
```

O núcleo possui um endereço binário reversível de 12 bits. A casca não é descartada: ela permanece como expansão vigesimal, borda, interface e `TOKEN_VAZIO` potencial.

## 3. Quatro vértices ocultos

Os quatro valores de `v` são projetados como os vértices de um tetraedro:

```text
v0 = (+,+,+)
v1 = (+,-,-)
v2 = (-,+,-)
v3 = (-,-,+)
```

A paridade inverte somente o deslocamento tetraédrico. Assim, número, posição e letra podem compartilhar o mesmo endereço visível sem perder a camada oculta.

## 4. Relações tipadas

O núcleo implementa relações separadas:

| Relação | Regra |
|---|---|
| `SAME` | todos os cinco campos iguais |
| `ORTHO_NEIGHBOR` | distância Manhattan 1 nos eixos visíveis |
| `PARITY_TWIN` | mesma célula e vértice, paridade oposta |
| `VERTEX_SIBLING` | mesma célula e paridade, vértice diferente |
| `OPPOSITE` | `(x,y,z,v,p) → (9-x,9-y,9-z,3-v,p⊕1)` |
| `ROTATE_Z90` | `(x,y) → (9-y,x)` |

As propriedades verificadas são:

\[
Opposite(Opposite(S))=S
\]

\[
Rotate_{90}^{4}(S)=S
\]

## 5. Oito medianas e projeções

As oito direções centrais são tratadas como oito octantes, identificados pelos três testes:

```text
x < 5 / x ≥ 5
y < 5 / y ≥ 5
z < 5 / z ≥ 5
```

Isso gera `0..7`, equivalentes aos oito raios do centro para os cantos do cubo.

Também existem duas vistas derivadas:

- projeção quadrada: `u=10x+y`, `v=z`;
- projeção hexagonal axial: `q=x-y`, `r=z-floor((x+y)/2)`.

A projeção hexagonal é uma vista determinística; não é declarada isometria.

## 6. Bola aberta B³ em Q16

Cada estado recebe uma coordenada inteira Q16 dentro da bola aberta unitária. A parte `10³` define o corpo do cubo; `v` acrescenta o deslocamento tetraédrico; `p` inverte esse deslocamento.

O teste exaustivo verifica:

\[
\|E(S)\|^2 < 1
\quad\forall S\in MATRIX8000
\]

Isso é uma geometria portadora para grafos e visualizações. Não é uma afirmação física, cosmológica ou uma isometria automática do modelo de Poincaré.

## 7. Primos, ruído e lacuna

Cada índice pode ser classificado como primo por teste determinístico. Isso permite criar subgrafos sem confundir filtro `6k±1` com prova de primalidade.

```text
Ruído classificado → dado lateral
Erro reproduzido   → engenharia
Lacuna sem fonte   → TOKEN_VAZIO
```

## 8. Linha histórica dos alfabetos e codificações

`data/encoding_timeline.v1.json` separa:

- ITA2;
- ASCII;
- ECMA-6;
- EBCDIC CP037;
- Unicode;
- UTF-8;
- UTF-16;
- UTF-32.

A tabela é extensível, mas não afirma enumerar toda code page já criada. Quando não existe uma única versão terminal — como numa família de code pages — o estado permanece `FAMILY_NO_SINGLE_LATEST_VERSION`. Datas de primeira edição não confirmadas permanecem `TOKEN_VAZIO_FIRST_EDITION`.

## 9. Arquivos

```text
rafaelia/bitraf_matrix.h
scripts/bitraf_matrix_manifest.py
data/encoding_timeline.v1.json
tests/bitraf_matrix_test.c
tests/test_bitraf_matrix.py
```

## 10. Execução local

```bash
python3 -m unittest tests/test_bitraf_matrix.py -v
python3 scripts/bitraf_matrix_manifest.py --out output/bitraf_matrix_v1 --csv core
```

O núcleo C pode ser compilado separadamente:

```bash
cc -std=c11 -O2 -Wall -Wextra -Werror \
  tests/bitraf_matrix_test.c -o /tmp/bitraf_matrix_test
/tmp/bitraf_matrix_test
```

## 11. Estado epistemológico

```text
computational_model = IMPLEMENTED
exhaustive_local_test = REQUIRED_BEFORE_PROMOTION
physical_claim = NOT_CLAIMED
historical_registry = PRIMARY_SOURCE_ANCHORED_WITH_EXPLICIT_GAPS
claim_allowed = false
```

### R₃

- `F_ok`: bijeção `20³ ↔ 8000`; núcleo reversível `2¹² ↔ 4096`; casca preservada; relações e projeções tipadas.
- `F_gap`: calibração semântica entre caracteres, fonemas, ideogramas e estados ainda depende de corpus e autoridade por domínio.
- `F_next`: ligar o carrier ao AllStar Matrix existente sem substituir a fonte original nem misturar símbolo com prova.

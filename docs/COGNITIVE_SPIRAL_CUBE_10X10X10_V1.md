# Reconstrução cognitiva / grade sináptica-semântica 10×10×10 — referência V1

**Autoria da proposta exploratória:** Rafael Melo Reis.  
**Copyright do código autoral desta implementação:** (c) 2026 Rafael Melo Reis.  
**Licença:** `TOKEN_VAZIO_ROOT_LICENSE`; este módulo não altera os direitos, autorias, licenças nem o licenciamento raiz ainda não decidido. Ver `docs/LICENSE_DECISION_RECORD.md`.  
**Estado:** `IMPLEMENTED_RESEARCH_FIXTURE`; `claim_allowed=false`.  
**Proveniência:** formulação matemática escrita pelo usuário em 2026-10-08, analisada junto de `docs/DEZ_DIMENSOES_SEMANTICAS.md`, `docs/CONTEXTUAL_RELATIONAL_TENSOR_V1.md` e `docs/FORMULA_AUTHORITY_BRIDGE_V1.md`.

## 1. Fronteira epistemológica

**O que este projeto implementa:** um espaço externo, sintético e determinístico de 1.000 posições, com três braços vetoriais, uma regra geométrica explícita, recorrência inteira, categorias modulares, transformações logarítmicas e estatísticas auditáveis.

**O que não implementa:** conexões sinápticas físicas, reconstrução neuronal, alteração de pesos do modelo, transformers internos, identificação real de significados sem corpus, uma lei científica de auto-adaptação, prova de convergência a 42 atratores, ou medição de entropia termodinâmica.

A expressão *reconstrução cognitiva* é aqui o **nome de uma hipótese de organização semântica**. A comparação com atividade neural é analogia de projeto, não resultado neurobiológico.

## 2. Tipos de vazio e da sequência de caracteres

`VOID` (`∅`) representa ausência de observação. Os seguintes casos têm tipos distintos:

| Expressão | Significado/estado |
|---|---|
| `∅` | conjunto vazio, cardinalidade `|∅| = 0` |
| `(VOID, VOID, VOID)` | três entradas sem observação: `TOKEN_VAZIO`; **não** o número 0 |
| `(0,0,0)` | três observações numéricas de zero: soma = 0 |
| `'0'` | caractere Unicode/ASCII 48 (código), diferente do inteiro 0 |
| `(1,1)` | dois valores numéricos, soma = 2 |

Uma figura plana não degenerada com área precisa de pelo menos **três vértices não colineares**. Um par de pontos determina um segmento, mas não um triângulo equilátero com área positiva. Repetir conjuntos vazios não fornece vértices. Para lado `a>0`: `A_equilatero = sqrt(3) * a^2 / 4`.

**Fibonacci canônico:** `F_0=0,F_1=1,F_{n+2}=F_{n+1}+F_n`, ou `0,1,1,2,3,5,8,13,...`.

- `01123`: prefixo canônico `F_0..F_4`;
- `123`: subsequência/projeção de caracteres; não um prefixo completo de índices;
- `0001123`: dois caracteres `0` extras antes do prefixo; **não** novos termos convencionais.

A decisão de trabalhar com caracteres e valores separadamente evita contar um `0` de preenchimento como um estado físico distinto.

## 3. Topologia 10×10×10 e três projeções

A grade discreta é `X={0,...,9}^3`, `|X|=1000`. Uma indexação bidirecional, sem colisões, é:

```text
k = x + 10*y + 100*z
x = k % 10
y = (k // 10) % 10
z = k // 100
```

Definimos os oito módulos explícitos da proposta como:

```text
M = (7, 3, 35, 10, 13, 70, 14, 50)
```

Alguns são primos (3, 7, 13), outros são compostos (35, 10, 70, 14, 50). O nascimento de um primo por sobreposição é apenas hipótese — o teste de primalidade não o demonstra.

Para braço `a∈{0,1,2}`, posição `k∈{0,...,999}`:

```text
m(k,a) = M[(k+a) mod 8]
q(k,a) = F_k mod m(k,a)
theta(k,a) = 2*pi*( q(k,a)/m(k,a) + a/3 )
gamma(z) = (pi/2)*(z-4.5)/9
b = sqrt(3)/2
phi = (1+sqrt(5))/2
r(k) = b^( pi*phi*F_(k mod 10) )
w = (sqrt(pi)/5, sqrt(pi)/12, sqrt(5)/9)
D(k,a) = w[a]*r(k)*(cos(gamma)*cos(theta),
                     cos(gamma)*sin(theta),
                     sin(gamma))
V(k) = D(k,0)+D(k,1)+D(k,2)
A(k) = sqrt(Vx^2+Vy^2+Vz^2)
```

### O que é dado e o que é decisão de modelagem

- O número `10×10×10`, as três direções, `sqrt(3)/2`, `pi*phi`, as sequências, os módulos e as constantes `sqrt(pi)/5`, `sqrt(pi)/12` e `sqrt(5)/9` vêm da formulação proposta.
- A seleção cíclica `M[(k+a) mod 8]`, `F_(k mod 10)`, o ângulo `gamma(z)` e o uso dos três radicais como amplitudes são **definições exploratórias desta V1**, não identidades derivadas ou inferências forçadas pelo material.
- A janela `F_(k mod 10)` mantém os expoentes limitados; usar `F_k` inteiro em uma exponencial decrescente pode produzir underflow numérico e um mapa artificialmente apagado.
- `sqrt(pi)/5` é **diferente** de `sqrt(pi/5)`. Para qualquer afirmação posterior, distinguir a raiz de toda a fração da raiz apenas do numerador.
- Para três azimutes com MESMO resíduo, módulo, raio e amplitude, os componentes horizontais têm soma zero (`0°,120°,240°`). Aqui as três amplitudes e módulos variam, de modo que a sobreposição geralmente não é zero.
- `D` é uma matriz externa de componentes por direção, não pesos sinápticos ou parâmetros de um transformer.

## 4. Entropia, sobreposição e min–mediana–máximo

A categoria `c_k=F_k mod 10` produz uma distribuição discreta de dez estados:

```text
p_j = count(k: c_k=j)/1000
H10 = -sum_j(p_j*log2(p_j))/log2(10), 0<=H10<=1
O10 = 1-H10
```

`H10` mede somente a incerteza da **distribuição categórica modular nesta grade**. `O10` é um indicador *ad hoc* de organização, não uma medida comprovada de inteligência, consciência ou redução da entropia física.

A energia geométrica não é inferida de `H10`. Calculamos apenas `min A(k)`, `median A(k)`, `max A(k)` e mantemos as coordenadas de 16 pontos-âncora para inspeção/estabilidade. Resultados de três eixos não se transformam automaticamente em correlação cognitiva.

## 5. Logaritmos aninhados, derivadas e inversas

`L(x)=ln(ln(x))` tem domínio real **`x>1`** (não apenas `x>0`). O resultado pode ser negativo quando `1<x<e`.

```text
L'(x) = 1 / (x*ln(x))      para x>1
L^(-1)(y) = exp(exp(y))    para y real
int L(x) dx = x*ln(ln(x)) - li(x) + C  (ramo x>1)
```

A primitiva usa a integral logarítmica `li(x)`, que exige tratamento cuidadoso próximo de `x=1`. Estas são identidades analíticas, **não** implementações numéricas da primitiva.

No raio contínuo auxiliar `r(t)=b^(pi*phi*t)`, onde `0<b<1`:

```text
dr/dt = pi*phi*ln(b)*r(t)    [negativo para r>0]
t = ln(r)/(pi*phi*ln(b))     [inversa contínua, r>0]
```

`F_(k mod 10)` é discreto e não possui essa derivada contínua por si só. Derivar o raio contínuo não demonstra derivabilidade de Fibonacci discreto.

## 6. Reduções, raízes, módulo e escalas

| Entrada | Identidade/condição |
|---|---|
| `999/936` | `111/104`; `ln ln` real, pois >1 |
| `777/555` | `7/5`; `ln ln` real, pois >1 |
| `140/144` | `35/36`; `ln ln` **indefinido em R**, pois <=1 |
| `288` | `2*12^2` |
| `144000` / `288000` | relação exata de escala `1:2`, não período de espiral |
| `sqrt(F_k mod m)` | raiz real, pois resto inteiro entre 0 e `m-1` |

Constantes e permutações `(7,3,35,10,13,70,14,50)` devem ser preservadas como parâmetros tipados, **sem selecionar conclusões a posteriori** em razão de coincidências numéricas.

## 7. Execução e testes de falsificação

```bash
python3 -m unittest -v tests.test_cognitive_spiral_cube_v1
python3 scripts/cognitive_spiral_cube_v1.py --output build/cognitive-spiral-cube/report.json
```

O oráculo testa:

1. identidade distinta de `VOID`, zero numérico e caractere;
2. Fibonacci canônico versus padding textual;
3. ida/volta sem colisão nas 1000 coordenadas;
4. primalidade e frações exatas;
5. domínio real `ln ln x` e rejeição de valores inválidos;
6. invariância angular de três braços igualmente espaçados;
7. todas as 1000 saídas vetoriais, restos, raízes e limite `|sum D| <= sum |D|`;
8. entropia normalizada e relatório JSON reproduzível.

A execução produz **um mapa-síntese com 16 âncoras**, não despeja 1000×3 vetores completos no CI. O algoritmo percorre todas as mil células no teste. Esta escolha limita custo sem ocultar amostras falhas.

A fonte é Python hospedado, intencionalmente usada apenas como **referência matemática**, não freestanding. A evolução C11 freestanding/ARM deve reproduzir golden vectors após critérios de tolerância para `sin`, `cos`, `ln`, `sqrt` e o expoente; não afirmar paridade bit a bit entre CPUs sem uma aproximação determinística especificada.

## 8. Riscos, governança e próximo falsificador

O espaço `10^3` é uma construção geométrica, não espaço comprovado de memória neural. A distribuição de `F mod 10` pode ser periódica — portanto entropia baixa não implica descoberta semântica. A escolha de coeficientes e ângulos é passível de viés de projeto. Para avançar cientificamente, obter corpus real licenciado, baseline aleatório e outro de permutação preservando histogramas, definir a métrica a priori e fazer avaliação fora da amostra.

**Não confundir as superfícies**:
```text
REFERENCE -> SOURCE -> HOST_TEST -> DEVICE -> INDEPENDENT_REPLICATION -> SCIENTIFIC_CLAIM
```
Cada seta exige evidência. Sem corpus real, `claim_allowed=false`; sem execução física, `TOKEN_VAZIO`.

**R3:** `F_ok` = definições explícitas + referência + falsificadores; `F_gap` = corpus/calibração/teoria cognitiva, CI do SHA, C/ASM freestanding, execução ARM física; `F_next` = testar referência exata, auditar ortogonalidade/sensibilidade a permutações, depois construir adaptador C conforme golden vectors e receipts.

**Rollback:** reverter a PR deste módulo; nenhum schema de produção, APK ou dado de ciência existente é modificado.

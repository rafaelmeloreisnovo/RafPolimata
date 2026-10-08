# Perfil de base 14 — grade 14³, três direções e zeros tipados

**Autoria da proposta:** Rafael Melo Reis. **Implementação autoral (2026-10-08):** complemento matemático independente em RafPolimata. **Licenciamento da raiz:** TOKEN_VAZIO_ROOT_LICENSE, escopo sujeito ao titular e revisão em `docs/LICENSE_DECISION_RECORD.md`. Não sublicencia fontes nem altera o módulo precedente.

**Rota de contexto:** `docs/COGNITIVE_SPIRAL_CUBE_10X10X10_V1.md` → `scripts/cognitive_spiral_cube_v1.py` → `scripts/cognitive_spiral_base14_v1.py` → `tests/test_cognitive_spiral_base14_v1.py`. Gaps de evidência: **CLOSURE_L11**, sem confundir fonte com medição física.

## 1. O número fica redondo em qual sentido?

Em base 14, os dígitos são `0123456789ABCD`; `A=10, B=11, C=12, D=13` (valores em decimal). O numeral `10₁₄` significa quatorze, não dez.

Há **duas operações diferentes**:

1. **Reexpressar os mesmos 1000 voxels decimais:** `1000₁₀ = 516₁₄`. Isto muda somente a notação, sem alterar topologia, coordenadas ou prova.
2. **Trocar o lado para 14:** `14³ = 2744₁₀ = 1000₁₄`. Agora a grade é `{0,...,13}³` e de fato o número de células se escreve `1000` em base 14. Esta é a nova alternativa testável.

Logo, `1000₁₄` **não representa** a grade anterior `10×10×10`; representa o volume da nova `14×14×14`.

## 2. Exemplo de conversões (valores preservados)

| decimal | base 14 |
|---:|---:|
| 7 | `7` |
| 10 | `A` |
| 13 | `D` |
| 14 | `10` |
| 35 | `27` |
| 50 | `38` |
| 70 | `50` |
| 140 | `A0` |
| 144 | `A4` |
| 288 | `168` |
| 999 | `515` |
| 1000 | `516` |
| 2744 | `1000` |

O ordenamento modular original `(7,3,35,10,13,70,14,50)` se escreve `(7,3,27,A,D,50,10,38)₁₄`. Converter **a aparência** do módulo não muda a operação `F_k mod m`, e dígito `'0'` segue diferente do inteiro 0 e do tipo `VOID`.

## 2A. Relação estrutural 14³, 12², 1,5 — dois níveis de base

**Convenção indispensável:** sem subscrito, os números desta primeira identidade são **decimais**:

```text
(12+2)^3
 = 12^3 + 3*(12^2)*2 + 3*12*(2^2) + 2^3
 = 1728 + 864 + 144 + 8
 = (3/2)*12^3 + 12^2 + 2^3
 = 2592 + 144 + 8
 = 2744 = 14^3
```

O fator exato `3/2` surge porque `3*12^2*2 = 864 = 12^3/2`; o termo seguinte é também exatamente `12²` porque `3*12*2² = 144`. Portanto:

```text
(14³ - 12² - 2³)/12³ = 3/2            [igualdade exata]
(14³ - 12²)/12³ = 3/2 + 1/216          [quase 1,5; NÃO exatamente]
```

A mesma decomposição, **sem trocar os valores**, em escrita de base 14 é:

```text
1000₁₄ = D32₁₄ + A4₁₄ + 8₁₄
2744₁₀ = 2592₁₀ + 144₁₀ + 8₁₀
```

Um segundo particionamento exato aproxima explicitamente os números `10³` e `100` da matriz:

```text
14³ = 10³ + 12³ + 4² = 1000 + 1728 + 16
14³ = 12² + (14+12)*10² = 144 + 26*100
14³ - 10³ = 1744 = 12³ + 16
```

**Identidade de dígitos, separada da anterior:** se a expressão `12²=144` for escrita **inteiramente na base b**, ela é verdadeira em toda base `b>=5`:

```text
(12)_b = b+2
(144)_b = b²+4b+4
(12)_b² = (144)_b
```

Em particular, `12₁₄=16₁₀`, então `(12₁₄)²=144₁₄=256₁₀`. Este `144₁₄` **não é** o `144₁₀=A4₁₄` do particionamento decimal anterior. Igualmente `10₁₄=14₁₀` e `1000₁₄=2744₁₀`.

Caso `1050` seja um numeral da matriz, o contrato precisa anotar sua base: `1050₁₄=2814₁₀`, enquanto `1050₁₀=550₁₄`; `100₁₄=196₁₀`. Nenhum destes números é intercambiável apenas pelo desenho dos caracteres.

**Fração 1,5 e base 14:** no sistema decimal, `1,5₁₀=3/2`; na escrita posicional da base 14, esse mesmo valor é `1,7₁₄` porque `7/14=1/2`. O numeral `1,5₁₄` vale `1+5/14=19/14` em decimal, portanto **não representa um e meio**. O fator `3/2` nas equações foi definido como razão racional exata, independente de notação.

**Hipótese vs prova:** as identidades acima são exatas em aritmética. Que elas determinem causalmente entropia, semântica ou dinâmica física continua `TOKEN_VAZIO`; o bloco matemático não altera pesos nem resultados do corpus.

## 3. Geometria e espirais comparáveis

Agora as coordenadas `(x,y,z)` estão em `[0,13]^3`. A indexação é `k=x+14y+196z`, `0<=k<2744`. Na ordem dos índices `0..2743`, os três braços permanecem deslocados por `2πa/3`, `a=0,1,2`:

```text
m(k,a)=M[(k+a) mod 8]
q(k,a)=F_k mod m(k,a)
theta(k,a)=2π*(q(k,a)/m(k,a)+a/3)
gamma(z)=(z-6.5)*π/26
r(k)=(sqrt(3)/2)^(π*phi*F_(k mod 10))
w=(sqrt(π)/5, sqrt(π)/12, sqrt(5)/9)
D(k,a)=r(k)*w[a]*(cos(gamma)*cos(theta),
                   cos(gamma)*sin(theta), sin(gamma))
V(k)=D(k,0)+D(k,1)+D(k,2)
```

**Controle de variáveis:** o período 10 da janela radial não foi trocado por 14, intencionalmente: manter o expoente da espiral constante entre perfis permite atribuir efeitos **à geometria e aos resíduos**. Trocar também a janela pelo período 14 exigiria um experimento separado; de outro modo não haveria ablação clara. Nenhuma das decisões de ângulo, amplitude ou janela constitui uma lei neural.

## 4. Organização e sensibilidade direcional

Há 14 camadas com 196 células cada. Em cada camada o modelo calcula:

- entropia discreta normalizada do histograma `F_k mod 14`;
- coerência geométrica como norma dos três vetores unitários ponderados, dividida pela soma de pesos;
- atualização externa `C'=0.75*C+0.25*C_obs`, `H'=0.75*H+0.25*H_obs`;
- contraste de permutação, repetindo a mesma observação em ordem `z=13..0`.

Também calcula mínimo, mediana e máximo das normas de sobreposição e exemplos de coordenadas nos pontos-âncora escolhidos. A palavra *entropia* aqui refere-se à distribuição modular, **não à entropia termodinâmica ou atividade cerebral**. A ordem `z` não é um eixo temporal medido.

## 5. Comandos, falsificadores e contrato de evidência

```bash
python3 -m unittest -v tests.test_cognitive_spiral_base14_v1
python3 scripts/cognitive_spiral_base14_v1.py --output build/cognitive-spiral-cube/base14.json
```

O teste varre **todos os 2744 índices**, confirma `encode14(decode14(s))=s` para formas canônicas, inversão de coordenadas, três direções, restos válidos, limites de norma, histograma com 2744 observações, `alpha=.25`, ordem reversa e saída reprodutível. Caracteres inválidos e zeros de preenchimento são rejeitados no codec canônico. Não é usado modelo externo, tokenizer de IA, ferramenta de treino, bibliotecas de terceiros ou plataforma Android.

**Retenção do modelo anterior:** `scripts/cognitive_spiral_cube_v1.py` e `tests/test_cognitive_spiral_cube_v1.py` continuam inalterados; a comparação A/B precisa preservar esta distinção.

**Estado honesto:** `SOURCE_IMPLEMENTED` para o perfil base14, `HOST_TEST` depende do SHA exato, `PHYSICAL_ANDROID=TOKEN_VAZIO`, `neural_validation=NOT_RUN`, `claim_allowed=false`.

**Rollback:** reverter apenas os novos arquivos e suas entradas no roteador/CI; não substituir ou sobrescrever receipts prévios.

**R3:** `F_ok=codec canônico + grade 14³ + vetores + falsificadores`; `F_gap=corpus semântico licenciado, benchmark comparativo, C freestanding, paridade ARM32/ARM64`; `F_next=CI rápido -> revisão -> controles por permutação aleatória -> execução física vinculada a fonte`.

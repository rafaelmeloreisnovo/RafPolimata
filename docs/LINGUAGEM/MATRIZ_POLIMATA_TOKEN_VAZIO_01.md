# Matriz Polímata de Linguagem — escada 0,1 e `TOKEN_VAZIO`

> **Área:** linguagem, semântica, matrizes e evidência  
> **Responsável lógico:** arquitetura semântica + governança de evidência  
> **Ciclo de vida:** `AUDIT / CANDIDATE`  
> **Entrada técnica:** `scripts/language_matrix.py`  
> **Estado global:** `claim_allowed=false`

## 1. Finalidade

Este subsistema organiza os materiais de linguagem do RafPolimata sem inventar:

- pesos;
- camadas específicas;
- blending;
- tokenização linguística validada;
- inventário total de tokens;
- dependência temporal;
- acurácia;
- equivalência entre línguas;
- geometria comum entre todos os repositórios.

A regra central é:

\[
\boxed{\text{ausente}\neq 0}
\]

E também:

\[
\boxed{\text{definido}\neq\text{medido}\neq\text{validado}}
\]

Um valor só entra na escada decimal depois de possuir o gate correspondente. Até lá:

```text
score = null
status = TOKEN_VAZIO
```

O número `0.0` fica reservado para **ausência medida** ou efeito nulo demonstrado. Nunca representa dado faltante.

---

## 2. Escada de sustentação em décimos

| Valor | Gate mínimo | Significado |
|---:|---|---|
| `0.1` | `definition` | objeto, domínio, unidade e limite definidos |
| `0.2` | `contract` | configuração ou schema legível por máquina |
| `0.3` | `implementation` | implementação determinística presente |
| `0.4` | `local_test` | teste local executado e registrado |
| `0.5` | `hashed_fixture` | corpus/fixture congelado e hasheado |
| `0.6` | `validated_tokenizer` | tokenização específica por língua validada |
| `0.7` | `out_of_sample` | validação fora da amostra |
| `0.8` | `independent_replication` | reprodução independente |
| `0.9` | `multi_repository` | sustentação em matrizes de repositórios independentes |
| `1.0` | `domain_audit` | fechamento no domínio declarado, com auditoria independente |

A promoção é monotônica e unitária:

\[
s_{n+1}=s_n+0.1
\]

Somente o gate imediatamente seguinte pode promover o eixo. Saltos são inválidos.

---

## 3. Eixos que devem permanecer vazios

No corte inicial:

```text
pesos                      = TOKEN_VAZIO_CALIBRACAO
camadas_de_linguagem       = TOKEN_VAZIO_MAPEAMENTO
blending                   = TOKEN_VAZIO_MODELO
tokenizacao_linguistica    = TOKEN_VAZIO_CORPUS_VALIDADO
tokens_canonicos           = TOKEN_VAZIO_CORPUS
tempo                      = TOKEN_VAZIO_SERIE_TEMPORAL
acuracia                   = TOKEN_VAZIO_GROUND_TRUTH
alinhamento_multirepos     = TOKEN_VAZIO_INVENTARIO
coeficientes_fibonacci     = TOKEN_VAZIO_CALIBRACAO
dimensao_fractal           = TOKEN_VAZIO_ESTIMADOR
```

O repositório já possui uma arquitetura de 21 camadas e um espaço T⁷. Isso não autoriza atribuir automaticamente cada camada ou coordenada ao novo subsistema de linguagem.

---

## 4. Matriz relacional mínima

Considere uma matriz de relações não negativas:

\[
A\in\mathbb{R}_{\ge 0}^{n\times n}
\]

### Direta

\[
D(A)=A
\]

### Inversa relacional

\[
I(A)=A^\mathsf{T}
\]

Ela troca origem e destino. Não afirma causalidade reversa.

### Indireta

\[
N(A)=A^2
\]

Ela conta ou pondera caminhos de dois passos. Não transforma correlação em causa.

### Recíproca

\[
R_{ij}(A)=\min(A_{ij},A_{ji})
\]

Ela preserva apenas a parte sustentada nos dois sentidos.

### Logarítmica

\[
L(A)=\log(1+A)
\]

A transformação comprime amplitude preservando zero e exige \(A_{ij}\ge0\).

### Inversa logarítmica

\[
L^{-1}(X)=e^X-1
\]

O roundtrip é verificável numericamente:

\[
L^{-1}(L(A))\approx A
\]

---

## 5. Fibonacci e inversa

A sequência de Fibonacci é usada somente como régua de janelas:

\[
F_0=0,\quad F_1=1,\quad F_n=F_{n-1}+F_{n-2}
\]

Aplicações permitidas:

- tamanhos progressivos de janela;
- níveis de amostragem;
- blocos de contexto;
- comparação multiescalar.

A “inversa de Fibonacci” é definida apenas como busca de índice exato:

\[
F^{-1}(x)=n\quad\text{se }F_n=x
\]

Se \(x\) não pertence à sequência, o resultado é:

```text
TOKEN_VAZIO_NOT_FIBONACCI
```

Fibonacci não é usado como prova de significado, causalidade, qualidade ou verdade.

---

## 6. Fractalização permitida

O termo “fractalização” recebe aqui uma definição operacional restrita:

> decomposição hierárquica reproduzível de uma sequência ou matriz em blocos aninhados.

São implementados dois esquemas:

1. **partição diádica:** divisão recursiva em metades;
2. **janelas Fibonacci:** observação em escalas pertencentes à sequência.

Isso não implica dimensão fractal física ou autossimilaridade natural. A dimensão fractal permanece `TOKEN_VAZIO` até haver estimador, domínio e dados.

---

## 7. Línguas, scripts e fonética

O inventário inicial distingue:

- língua;
- script;
- direção de escrita;
- caracteres de exemplo;
- caracteres fonéticos IPA;
- tokens computacionais;
- proveniência.

Inclui somente amostras de:

- grego;
- hebraico;
- aramaico em script siríaco;
- português;
- IPA;
- símbolos computacionais.

O inventário não é corpus e não representa cobertura linguística completa.

### Direção

```text
grego       = LTR
português   = LTR
hebraico    = RTL
siríaco     = RTL
números     = direção dependente do script/contexto
código      = majoritariamente LTR
```

A direção gráfica pode modular tarefas espaciais; não determina sozinha pensamento, tempo ou inteligência.

---

## 8. Tokenização

Dois baselines determinísticos são implementados:

1. `unicode_codepoint_nonspace`;
2. `unicode_whitespace`.

Eles servem para:

- integridade;
- contagem;
- fixtures;
- testes Unicode;
- comparação inicial.

Eles **não** são tokenizadores linguísticos validados para grego, hebraico ou aramaico.

Logo:

```text
tokenizer_baseline = IMPLEMENTED
tokenizer_linguistico = TOKEN_VAZIO
```

---

## 9. Estado inicial sustentável

| Eixo | Último valor sustentável | Estado |
|---|---:|---|
| contrato da escada | `0.4` | implementação e testes locais |
| operadores matriciais | `0.4` | implementação e testes locais |
| Fibonacci como janela | `0.4` | implementação e testes locais |
| partição hierárquica | `0.4` | implementação e testes locais |
| inventário de scripts | `0.2` | contrato legível por máquina |
| tokenização baseline | `0.4` | implementação e testes locais |
| tokenização linguística | `null` | `TOKEN_VAZIO` |
| pesos | `null` | `TOKEN_VAZIO` |
| blending | `null` | `TOKEN_VAZIO` |
| mapeamento das 21 camadas | `null` | `TOKEN_VAZIO` |
| tempo | `null` | `TOKEN_VAZIO` |
| acurácia | `null` | `TOKEN_VAZIO` |
| cobertura multirrepositório | `null` | `TOKEN_VAZIO` |

O valor `0.4` não significa 40% de verdade ou 40% de acurácia. Significa apenas que quatro gates de engenharia foram satisfeitos.

---

## 10. Relação com a arquitetura existente

A matriz se conecta às 21 camadas sem preenchê-las artificialmente:

| Camada existente | Relação atual |
|---|---|
| sinal bruto | caracteres, bytes e tokens baseline |
| normalização | Unicode NFC/NFKC, quando declarado |
| estrutura | matriz relacional e schema |
| integridade | SHA-256 de fixtures futuros |
| entropia local | calculável sobre distribuições observadas |
| estado topológico | hipótese de projeção; `TOKEN_VAZIO` |
| dinâmica temporal | `TOKEN_VAZIO` |
| semântica lexical | corpus e anotação pendentes |
| semântica sintática | parser específico pendente |
| pragmática | estudo humano pendente |
| prosódia | IPA e áudio real pendentes |
| interlíngua | alinhamento paralelo pendente |
| cognição incorporada | fora do cálculo automático |
| estabilidade interpretativa | estudo longitudinal pendente |
| jurídico-institucional | dignidade, privacidade e proveniência como gates |

---

## 11. Firewalls

1. Ausência nunca vira zero.
2. Score de sustentação nunca vira acurácia.
3. Tokenização baseline nunca vira análise linguística validada.
4. Analogia matricial nunca vira mecanismo físico.
5. Fibonacci nunca vira causalidade.
6. Fractalização operacional nunca vira dimensão fractal demonstrada.
7. Peso sem calibração permanece `TOKEN_VAZIO`.
8. Tempo sem série permanece `TOKEN_VAZIO`.
9. Criança ou participante humano nunca é reduzido a vetor de valor.
10. Fé, tradição e ciência permanecem relacionadas sem equivalência forçada.

---

## 12. Execução

```sh
python3 scripts/language_matrix.py \
  --state data/language/language-matrix-state.v1.json

python3 -m unittest tests.test_language_matrix
```

O validador rejeita:

- score fora de `[0,1]`;
- score que não seja múltiplo de `0.1`;
- `null` sem justificativa `TOKEN_VAZIO`;
- `0.0` sem medição explícita;
- promoção sem todos os gates anteriores;
- matriz não quadrada;
- logaritmo de peso negativo;
- salto de maturidade.

---

## 13. Próximos gates

1. inventariar matrizes reais de cada repositório;
2. registrar origem, dimensão, unidade, hash e licença;
3. congelar corpus multilíngue autorizado;
4. validar tokenizer por língua;
5. definir ground truth antes de calcular acurácia;
6. calibrar pesos fora da amostra;
7. testar blending contra baselines;
8. só então mapear temporalidade e camadas.

\[
\boxed{\text{o sistema começa em }0.1\text{ somente quando existe definição}}
\]

\[
\boxed{\text{sem sustentação, permanece }TOKEN\_VAZIO}
\]

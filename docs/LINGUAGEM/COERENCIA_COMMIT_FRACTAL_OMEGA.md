# Coerência por commits — Fractal Ω da linguagem

> **Área:** linguagem, evidência, proveniência e fechamento Ω  
> **Ciclo:** `AUDIT / CANDIDATE`  
> **Branch:** `research/language-matrix-token-vazio-01-20260721`  
> **Claim:** `claim_allowed=false`

## 1. Regra

O commit é evidência documental de que um estado do repositório existiu. Ele não prova sozinho execução, correção científica, acurácia, compreensão humana, validade linguística ou causalidade.

\[
\boxed{commit \neq resultado\ experimental}
\]

A unidade mínima de evidência é:

```text
commit
→ artefato
→ contrato
→ teste
→ fixture/hash
→ limite
→ próximo gate
```

## 2. Cadeia registrada

A branch foi observada com `ahead_by=7` e `behind_by=0` sobre a base `7924259d02c99720b073da28f705c7346faf529b`.

| Ordem | Commit | Corpo |
|---:|---|---|
| 1 | `d210459b...` | configuração e escada 0,1 |
| 2 | `c267cb43...` | contrato de estado |
| 3 | `5873af3b...` | estado sem pesos inventados |
| 4 | `db6e4ae1...` | operadores e gates executáveis |
| 5 | `33775a64...` | testes positivos e negativos |
| 6 | `38fc09e2...` | formalização documental |
| 7 | `bef9ba54...` | entrada canônica e comandos |

Existência e conteúdo foram observados pela API do GitHub. A cadeia completa de pais não foi exportada pelo conector:

```text
TOKEN_VAZIO_PARENT_CHAIN_NOT_EXPORTED
```

## 3. Fractalização permitida

“Fractal Ω” significa a repetição auditável da mesma célula em escalas diferentes:

\[
\mathcal C=\langle origem,artefato,contrato,teste,hash,limite,F_{next}\rangle
\]

A célula aparece em função, arquivo, commit, pull request, repositório, família e ecossistema. Isso é governança multiescalar, não dimensão fractal física.

## 4. Sete direções

| Direção | Aplicação ao commit |
|---|---|
| direta | ler exatamente o diff |
| inversa | identificar dependências e estado anterior |
| recíproca | ligar implementação a teste |
| contrária | procurar teste negativo e falsificador |
| antiderivada | recuperar base, origem e motivação |
| derivada | observar artefatos e claims produzidos |
| retroalimentação | registrar limite e próximo gate |

## 5. Fixture congelado

```text
data/language/fixtures/language-matrix-fixture.v1.json
SHA-256: 206bddd5bec7fd9f97d09793ad83adc4900a3ec98e24a38d1d6a4eef356fad83
```

Ele cobre matrizes direta, inversa, indireta e recíproca; roundtrip `log1p/expm1`; Fibonacci; partição diádica; e preservação Unicode grego–hebraico–siríaco.

Ele não é corpus linguístico, dataset humano ou ground truth.

## 6. Promoção obtida

Os eixos determinísticos podem alcançar `0.5`:

```text
support_contract
matrix_operators
fibonacci_windows
hierarchical_partition
script_inventory
baseline_tokenization
commit_provenance
```

\[
0.5=definition+contract+implementation+local\_test+hashed\_fixture
\]

Isso não significa 50% de verdade ou acurácia.

## 7. Vazios preservados

```text
linguistic_tokenization = TOKEN_VAZIO_CORPUS_VALIDATION
canonical_tokens        = TOKEN_VAZIO_CORPUS
weights                 = TOKEN_VAZIO_CALIBRATION
language_layers         = TOKEN_VAZIO_LAYER_MAPPING
blending                = TOKEN_VAZIO_MODEL
time                    = TOKEN_VAZIO_TIME_SERIES
accuracy                = TOKEN_VAZIO_GROUND_TRUTH
multi_repository        = TOKEN_VAZIO_REPOSITORY_INVENTORY
fractal_dimension       = TOKEN_VAZIO_ESTIMATOR
human_comprehension     = TOKEN_VAZIO_ETHICS_STUDY
```

## 8. Fechamento Ω

O fechamento é interseção, não média:

\[
\Omega_E=E_{documental}\cap E_{estrutural}\cap E_{executável}\cap E_{reprodutível}\cap E_{externa}
\]

```text
documental        = PRESENTE
estrutural        = PRESENTE
executável        = PRESENTE
fixture_hash      = PRESENTE
clean_checkout    = TOKEN_VAZIO
replicação        = TOKEN_VAZIO
validação externa = TOKEN_VAZIO
```

O subsistema continua `CANDIDATE`, em draft e com `claim_allowed=false`.

## 9. Comandos

```sh
python3 -m unittest \
  tests.test_language_matrix \
  tests.test_language_commit_evidence

python3 scripts/language_matrix.py \
  --state data/language/language-matrix-state.v1.json

python3 scripts/language_commit_evidence.py
```

## 10. R₃

- **F_ok:** commits, artefatos e fixture possuem ligação auditável.
- **F_gap:** falta checkout limpo, cadeia completa de pais, corpus licenciado e validação externa.
- **F_next:** executar validadores em ambiente registrado e anexar stdout, stderr, versão, commit e hashes.

\[
\boxed{limpar=remover\ afirmação\ sem\ sustentação,\ não\ remover\ o\ vazio\ honesto}
\]

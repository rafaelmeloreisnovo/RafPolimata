# CÂNONE DO COSMOS 7D — Ponte de Evidência, Semântica e Governança

> **Entrada canônica:** docs/AGENTES.md §2 (taxonomia de agentes e responsabilidades inter-repositório) e §6 (escalação e conflito). Ponte semântica entre RafPolimata e o paper 7D — contrato TAIL, matriz de roteamento e gates de promoção claim_allowed=false.

**Estado:** `AUDIT | REFERENCE | claim_allowed=false`  
**Data:** 2026-07-19  
**Autor:** Rafael Melo Reis (∆RafaelVerboΩ)  
**Assinatura:** `RAFCODE-Φ-∆RafaelVerboΩ-𓀀ΔΦΩ`

## 1. Fonte canônica

O paper acadêmico canônico permanece em:

```text
instituto-Rafael/relativity-living-light
PapersPub/08_multiscale_validation_methods/
```

Título acadêmico:

> *From the Observed Void to Recurrence: A Seven-Dimensional Epistemic-Computational Formalism with 42 Operator-State Hyperforms*

Título autoral em português:

> *Cânone do Cosmos RAFAELIA — Do ∅ observado à recorrência em sete dimensões*

RafPolimata não recebe uma cópia integral. Esta ponte registra a relação semântica, epistemológica, jurídica e operacional com o paper.

## 2. Papel de RafPolimata

RafPolimata é responsável pela camada transversal:

- tipagem de claims;
- proveniência e autoria;
- separação entre hipótese, analogia, prova parcial e runtime;
- estado `TOKEN_VAZIO`;
- rastreabilidade entre conceitos, artefatos e repositórios;
- não colisão de licenças;
- auditoria IA↔humano;
- fail-safe, failover e rollback;
- preservação da parábola sem promovê-la a evidência física.

## 3. Taxonomia incorporada

O paper utiliza:

| Marca | Estado RafPolimata | Interpretação |
|---|---|---|
| `[E]` | `AUDIT/PASS` | Resultado exato reproduzível. |
| `[C]` | `REFERENCE` | Convenção/modelagem declarada. |
| `[H]` | `PENDING` ou `TOKEN_VAZIO` | Hipótese com teste ainda incompleto. |
| `[P]` | `REFERENCE` | Parábola, ética ou interpretação espiritual. |

Invariante:

```text
[P] não promove [H]
[H] não promove [E]
ausência não vira PASS
```

## 4. TAIL de autoria e evidência

Cada artefato derivado deve registrar:

| Campo | Requisito |
|---|---|
| **T — Trace/Traço** | caminho da fonte, versão, commit e transformação aplicada; |
| **A — Authorship/Autoria** | Rafael Melo Reis como fonte dos conceitos, símbolos e parábolas; |
| **I — Intent/Intenção** | formalização técnico-acadêmica sem apagar a camada espiritual; |
| **L — License/Licença** | licença do paper separada das licenças dos repositórios irmãos. |

Também deve responder:

```text
Quem percebeu?
Quem registrou?
Quem curou?
Quem testou?
Quem revisou?
Quem publicou?
```

A assistência de IA deve aparecer como ferramenta editorial/técnica, não como autoria humana ou titularidade independente.

## 5. Resultados atualmente aceitos

O relatório canônico registra 44/44 verificações exatas para:

- comprimentos, multiplicidades, permutações e entropia das cadeias;
- contagens das matrizes A/B e tensor 8×5×7×3;
- 42 pares dimensão–operador;
- conversões em base sete;
- comprimento, alfabeto, frequências e entropia do BITRAF64;
- distinção entre `sqrt(3)/2` e `sqrt(3/2)` com parênteses formais.

A notação textual deve evitar ambiguidade:

```text
sqrt(3)/2   = 0.866...  (contração)
sqrt(3/2)   = 1.224...  (expansão)
```

## 6. Correções semânticas obrigatórias

1. A redução `0001123 -> 01123 -> 0123` não é compressão sem perdas sem mapa de multiplicidades.
2. `4! = 24` conta permutações de posições; valores repetidos podem gerar menos arranjos distintos.
3. Sete dimensões são uma convenção de estado computacional, não dimensões físicas demonstradas.
4. Poincaré exige medida finita e dinâmica que preserve medida.
5. Memória monotônica pode impedir recorrência do estado aumentado `(X,M)`.
6. Correlação e informação mútua não provam causalidade.
7. Vetores com unidades heterogêneas exigem adimensionalização.
8. Sessenta e quatro caracteres não implicam segurança de 64 bits.
9. Hash truncado não é prova de integridade.
10. O termo “DNA molecular magnético” permanece parábola/hipótese até existir molécula, unidades e experimento.

## 7. Matriz de roteamento entre repositórios

| Corpo | Responsabilidade | O que não deve reivindicar |
|---|---|---|
| `relativity-living-light/PapersPub` | paper canônico, bibliografia, claim ledger e validação | implementação de codec ou hardware não executada ali |
| `Bitraf-Bit-quantum` | alfabeto, codec, serialização e vetores de teste | segurança quântica sem prova |
| `ChipQuantum` | runtime T⁷/42 estágios, geometria e benchmarks | equivalência automática entre runtime e lei física |
| `RafPolimata` | semântica, TAIL, licença, estados de evidência e governança | apropriação de licenças ou substituição do paper |
| `llamaRafaelia` | futura ingestão semântica e recuperação | transformar parábola em ground truth de treinamento |

## 8. Contrato de ingestão semântica

Cada fórmula deve possuir, no mínimo:

```yaml
id: string
source_path: string
author: Rafael Melo Reis
evidence_class: E|C|H|P
state: PASS|REFERENCE|PENDING|TOKEN_VAZIO|PROHIBITED
domains: []
variables: []
units: {}
assumptions: []
falsifier: string
license_scope: string
provenance_commit: string
```

Uma expressão sem domínio, unidades ou critério de interpretação não pode entrar como claim científico executável.

## 9. Governança jurídica e autoral

- A autoria do corpus e das parábolas deve ser preservada.
- Citações externas fundamentam definições; não transferem autoria sobre a composição original.
- Licenças MIT de código não devem ser aplicadas automaticamente ao texto autoral.
- A licença do paper deve ser escolhida explicitamente no pacote de release.
- Nenhum nome, símbolo, marca ou assinatura deve ser apresentado como registro oficial `®` sem comprovação documental.
- DOI, copyright, licença e revisão por pares são estados distintos.

## 10. Gates para promoção

### `AUDIT -> REVIEW_READY`

- reexecução independente do validador;
- revisão matemática;
- revisão de referências e DOI;
- revisão humana da tradução inglesa;
- confirmação do nome autoral e licença;
- hashes completos do pacote de release.

### `REVIEW_READY -> SUBMITTED`

- pacote fechado por versão;
- receipt de arXiv, Zenodo publicado ou periódico;
- declaração de IA conforme política editorial;
- sem claims proibidos no resumo, título ou conclusão.

## 11. R3

```text
F_ok   = corpus tipado, paper criado, bibliografia integrada e 44 invariantes exatos reproduzidos.
F_gap  = revisão humana, licença do paper, medida recorrente, causalidade, unidades, codec e segurança.
F_next = revisar PR canônico -> validar bridges -> fechar release citável sem sobreclaim.
```

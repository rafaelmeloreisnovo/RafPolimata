# Evidência, estados e promoção de claims

**Base:** main@f22efc099ac530d946ff2ec34954455f75632e92

## 1. Modelo principal

~~~text
SOURCE -> ARTIFACT -> EXECUTION -> EVIDENCE -> CLAIM
~~~

Cada termo representa um objeto diferente.

### SOURCE
Código, documento, dataset, schema, configuração ou entrada versionada.

### ARTIFACT
Objeto produzido: binário, ELF, DEX, APK, JSON, relatório, gráfico, receipt ou outro derivado.

### EXECUTION
Evento em que um processo ou teste foi realmente executado em ambiente identificável.

### EVIDENCE
Registro suficiente para ligar source + artifact + execution + resultado.

### CLAIM
Afirmação que só pode ser promovida quando o gate aplicável está satisfeito.

## 2. Estados mínimos

| Estado | Significado |
|---|---|
| DECLARED | intenção/contrato existe |
| SOURCE_PRESENT | fonte observada |
| STATIC_PASS | validação sem runtime passou |
| BUILD_PASS | artefato foi produzido |
| EXECUTION_PASS | execução correspondente passou |
| DEVICE_PASS | hardware/device foi observado no escopo |
| REPRODUCED | reprodução independente definida passou |
| FAIL | falsificador/gate falhou |
| NOT_RUN | gate conhecido não foi executado |
| TOKEN_VAZIO | evidência necessária ausente ou insuficiente |
| PENDING | trabalho/decisão ainda aberto |
| AUDIT | registro de custódia/decisão |
| SUPERSEDED | estado posterior substitui o roteamento, sem apagar história |

## 3. TOKEN_VAZIO

TOKEN_VAZIO é informativo. Exemplos:

- TOKEN_VAZIO_RUNTIME;
- TOKEN_VAZIO_DEVICE;
- TOKEN_VAZIO_PROVIDER;
- TOKEN_VAZIO_LICENSE_SCOPE;
- TOKEN_VAZIO_REGEN_REQUIRED;
- TOKEN_VAZIO_DOC_ROUTE.

Não converter ausência em:
- zero;
- sucesso;
- falha;
- impossibilidade;
- autorização.

## 4. Evidence grades documentais existentes

docs/DOCUMENT_GOVERNANCE.md define E0–E4:

- E0: arquivo possui identidade;
- E1: está indexado/referenciado;
- E2: contrato/estado explícito;
- E3: ligado a teste/prova/workflow;
- E4: corpo auditado contém comando e hash.

Mesmo E4 documental não implica automaticamente runtime físico ou validade científica; o domínio continua governando o gate final.

## 5. Receipt mínimo recomendado

~~~yaml
receipt_id: stable-id
timestamp: ISO-8601
source:
  repository: owner/repo
  commit: full-sha
inputs:
  - path: ...
    sha256: ...
executor:
  provider: ...
  environment: ...
commands:
  - ...
artifacts:
  - path: ...
    sha256: ...
result:
  state: PASS|FAIL|TOKEN_VAZIO|NOT_RUN
  observations: []
claims:
  allowed: false
  scope: []
gaps: []
next: []
supersedes: null
~~~

O exemplo é um formato recomendado; não substitui schemas já existentes no repositório.

## 6. Regra para documentação

Uma página técnica deve indicar:
- base/revisão;
- status;
- fonte autoritativa;
- o que foi observado;
- o que é interpretação;
- o que está faltando;
- próximo gate.

Evitar palavras como completo, provado, seguro, independente ou produção sem o respectivo objeto de evidência.

## 7. Claims científicos

Além da cadeia de engenharia, claims científicos precisam, conforme o domínio:
- definição/hipótese;
- unidades;
- observáveis;
- dataset identificável;
- covariância/erro aplicável;
- falsificador;
- baseline/controle;
- método reproduzível;
- resultado;
- incerteza;
- comparação;
- reprodução independente quando alegada.

Engineering PASS ≠ scientific novelty.

## 8. Segurança e jurídico

CI verde ≠ certificação de segurança.  
Hash ≠ veracidade.  
Arquivo público ≠ licença universal.  
Documento jurídico interno ≠ parecer jurídico profissional.

## 9. Regra de supersessão

Correção cria um novo estado ligado ao anterior. Receipts históricos não são reescritos para refletir conhecimento posterior.

R3 = ⟨F_ok: estados e promoção definidos; F_gap: cada subsistema continua exigindo seus falsificadores; F_next: vincular toda nova alegação material a source/commit/run/receipt antes de promoção⟩.

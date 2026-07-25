# Contrato de Execução Limitada

Todo job do RafPolimata deve declarar, antes da execução:

```yaml
job_id: string
operation: string
source:
  identity: string
  location: opaque-reference
  read_only: true
policy:
  privacy_class: P0_PUBLIC|P1_INTERNAL|P2_PRIVATE|P3_RESTRICTED
  max_memory_bytes: integer
  max_input_bytes: integer
  max_output_bytes: integer
  timeout_seconds: integer
  allow_network: false
  allow_original_mutation: false
expected_outputs: []
acceptance_criteria: []
evidence_required: []
safe_state: string
rollback: string
```

## Gates obrigatórios

1. **Preflight** — formato, tamanho, permissões, espaço e política.
2. **Bounded resources** — memória, tempo e saída finitos.
3. **Determinism** — mesma entrada e configuração produzem a mesma identidade de saída, exceto metadados externos explicitamente separados.
4. **Custody** — hash do original, versão do executor e transformação registrada.
5. **Privacy** — dados privados não atravessam o domínio autorizado.
6. **Evidence** — `PASS` exige artefatos verificáveis.
7. **Rollback** — falha não altera o original e deixa checkpoint ou limpeza comprovável.

## Eventos mínimos

```text
JOB_ACCEPTED
PREFLIGHT_PASS|PREFLIGHT_FAIL
STAGE_STARTED
CHECKPOINT_WRITTEN
STAGE_COMPLETED|STAGE_FAILED
EVIDENCE_EMITTED
JOB_COMPLETED|JOB_BLOCKED|JOB_FAILED
```

Cada evento deve possuir `event_id`, `job_id`, `run_id`, instante, estágio, estado, bytes lidos, registros emitidos, erro tipado e referências de evidência.

## Política de cache

Cache só pode ser reutilizado quando coincidem:

```text
content_id
+ operation_version
+ configuration_id
+ schema_version
+ privacy_policy_id
```

Hit de cache sem essa composição é inválido.

## Política de erro

- erro de entrada: rejeitar sem alteração;
- corrupção: quarentena;
- recurso insuficiente: checkpoint e `BLOCKED`;
- evidência incompleta: `PASS_LIMITED` ou `TOKEN_VAZIO`;
- divergência entre fontes: preservar ambas e registrar contradição.

## Critério de eficácia

Um job é eficaz quando produz a saída necessária para o objetivo declarado. É eficiente quando faz isso com menor custo sem reduzir rastreabilidade, segurança ou evidência. Eficiência nunca substitui eficácia.

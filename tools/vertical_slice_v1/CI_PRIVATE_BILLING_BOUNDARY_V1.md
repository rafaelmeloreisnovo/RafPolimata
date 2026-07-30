# CI Private Billing Boundary V1 — Evidence Adapter

**Evento:** `RAFAELIA-CI-PRIVATE-BILLING-BOUNDARY-V1-20260730T054800Z`  
**Predecessor:** RafPolimata `01f22dc75de6b1d7f5ac28f203b51e338af07d58` (PR #189)  
**Tempo:** 2026-07-30 05:48 UTC / 02:48 BRT  
**Política:** `APPEND_ONLY · NON_DESTRUCTIVE · CLAIM_ALLOWED=false · NO_AUTO_MERGE`

## Fronteira de evidência

Para este repositório privado, a ausência de execução GitHub Actions por cobertura de pagamento indisponível é uma condição de infraestrutura informada pelo titular:

```text
CI_UNAVAILABLE_PRIVATE_BILLING
```

O estado não pode ser transformado em erro do laboratório, falha de algoritmo, regressão estatística ou resultado experimental negativo.

## Ordem de evidência

```text
fonte + commit + comando + ambiente
  → execução local/Termux
  → stdout/stderr + exit code
  → receipt hasheado
  → revisão independente
```

Sem esses elementos, o estado é `TOKEN_VAZIO`. Com eles, o resultado é limitado ao método e ambiente registrados; não libera claim científico, médico, jurídico, de segurança ou produção.

## F_next

Executar o adapter em ambiente local disponível e emitir receipt separado. A indisponibilidade de CI privada não bloqueia o protocolo de evidência local; apenas impede usar Actions como fonte de observação remota.

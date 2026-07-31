# RafPolimata — Foundation computational gate V1

**Estado:** `IMPLEMENTED_TARGET_ADAPTER_PENDING_TERMUX_RECEIPT`  
**Origem da Foundation:** `rafaelmeloreisnovo/Mapa`, PR #102, head `d713d884c7174d37aceb642f80149e037426686e`  
**Executor:** checkout RafPolimata + Termux local  
**Claim permitido:** `false`

## Contrato instalado

Este checkout já contém a Foundation e o adapter
`rafpolimata-compiler-gate/v1`. Não exige bootstrap de rede, instalação
automática, Gradle, CI remoto, SDK Android ou substituição do compilador por
um comando genérico.

```text
.rafaelia/foundation.yaml
  -> scripts/rafpolimata_foundation_compiler_gate.py
  -> scripts/validate_runtime_truth_local.sh
  -> COMPILA/<run-id>/receipt.json
  -> gate.computational.v1-<UTC>.json
```

O adapter chama o teste local já versionado com argumentos diretos. Ele registra
os nove blocos conhecidos, inclusive os falsificadores de extensão inválida,
limite de fonte e rollback. Uma falha precoce permanece `FAIL` ou
`NOT_EXECUTED`; não é completada por inferência.

## Execução Termux

Antes da execução, o checkout deve estar no commit que contém estes arquivos e
sem alterações não rastreadas:

```sh
git status --short
git rev-parse HEAD
```

Quando a primeira linha não produzir saída:

```sh
bash termux/autoexec-rafaelia.sh plan --profile compiler-local-gate
bash termux/autoexec-rafaelia.sh verify --profile compiler-local-gate
bash termux/autoexec-rafaelia.sh run compiler-local-gate
```

Copie o `RECEIPT=...` emitido pelo último comando:

```sh
bash termux/autoexec-rafaelia.sh gate \
  --receipt COMPILA/<run-id>/receipt.json \
  --test-summary COMPILA/<run-id>/test-summary.json \
  --expected-profile compiler-local-gate
```

## Limite de decisão

`READY_FOR_DOMAIN_SPECIFIC_REVIEW` só significa que este receipt computacional
está íntegro, ligado ao HEAD limpo e contém testes/falsificadores contabilizados.
Não prova correção geral do compilador, ELF, DEX, APK, instalação Android,
comportamento de dispositivo, segurança, conformidade ou qualquer claim
científico. Todos continuam `claim_allowed=false` até seus gates próprios.

## R3

`F_ok`: alvo de compilação recebeu uma rota Termux autoexecutável e auditável.  
`F_gap`: nenhum run Android/Termux do commit exato foi observado ainda.  
`F_next`: executar este profile em Termux e preservar receipt + gate sem
sobrescrever os artefatos.

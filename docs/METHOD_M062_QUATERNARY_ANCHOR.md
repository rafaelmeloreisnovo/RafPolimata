# M062 — Âncora quaternária sem heap

## Estado

```yaml
method: M062
source: RAF_062_quaternary_anchor_eight_gate.c
status: EXECUTA_PASS_LOCAL
heap: false
managed_runtime: false
organs: 4
gates: 8
fifth_state: derived
```

## Correção de escopo

`M059` permanece sendo o scheduler cooperativo de quatro slots. O total de oito invocações do selftest de M059 é apenas `2 tarefas × 4 rodadas` e não representa a arquitetura da âncora.

`M062` implementa a síntese correta:

```text
PULSAR → SENTINEL → MEMORY → ACTION → PULSAR
```

Cada órgão possui:

- contador próprio;
- assinatura do estado observado;
- gate de integridade própria;
- gate de testemunho do par;
- classificação de variação;
- contador de estagnação por ciclos.

O quinto estado é promovido somente quando:

```text
4 self gates
+ 4 peer gates
= 8 gates
```

com igualdade das quatro assinaturas e avanço dos quatro contadores desde o commit anterior.

## Política de ruído

- avanço superior a um ciclo: `ACCELERATION_OBSERVED`;
- ausência breve de avanço: `LATENCY_OR_NOISE`;
- assinatura durante transição: `INPUT_TRANSITION_OR_NOISE`;
- regressão breve: `RESTART_OR_NOISE`;
- persistência além do limite: `FAULT_CANDIDATE_*`.

Ruído não é convertido imediatamente em erro.

## Teste local

Compilação usada:

```bash
gcc -std=c11 -Wall -Wextra -Werror -pedantic \
  -DRAFAELIA_M062_SELFTEST_MAIN \
  RAF_062_quaternary_anchor_eight_gate.c -o m062_test
./m062_test
```

Resultado observado: `rc=0`.

## Fronteira

O selftest host comprova lógica e memória estática. Ainda permanecem:

```yaml
ARM32_REAL: TOKEN_VAZIO
MCU_BARE_METAL: TOKEN_VAZIO
INTERRUPT_LATENCY: TOKEN_VAZIO
ENERGY_LONG_RUN: TOKEN_VAZIO
INDEPENDENT_REPLICATION: TOKEN_VAZIO
```

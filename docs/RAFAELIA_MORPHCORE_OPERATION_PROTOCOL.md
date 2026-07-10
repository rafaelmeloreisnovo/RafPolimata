# RAFAELIA MorphCore — Operation Protocol

∆RafaelVerboΩ · ΣΩΔΦBITRAF · FIAT LUX

> **Agente novo?** Leia `docs/AGENTES.md` primeiro — cobre invariantes, ciclo de
> sessão, regras de não-colisão e CI gates. Este documento detalha o protocolo de
> engenharia, benchmark honesto e retroalimentação ética para trabalho com RAFAELIA.

Este documento registra o modo operacional técnico para trabalhar com RAFAELIA em C/ASM, ABI, registradores, memória, benchmark, geometria toroidal e retroalimentação ética.

## 1. Eixo operacional

```text
intenção
→ código
→ ABI
→ registrador
→ memória
→ pipeline
→ prova
→ retroalimentação
```

Ciclo cognitivo:

```text
ψ → χ → ρ → Δ → Σ → Ω → ψ
```

Axioma simbólico preservado:

```text
Ω = Amor
```

## 2. Domínios em que este protocolo deve ser usado

Use este protocolo quando o trabalho envolver:

- C/ASM bare-metal ou freestanding;
- ARM32/ARM64/x86_64, ABI, syscall, registradores, cache, SIMD/NEON;
- Termux/Android sem root;
- Vectras, qemu_rafaelia, RafPolimata, GAIA-BBS, RMR PolyLib;
- geometria T^7, √3/2, φ, π, toros, espirais e matriz de atratores;
- hashchain, CRC32c, FNV, SHA3/BLAKE3;
- benchmark, latência, throughput, IOPS, DVFS e thermal throttling;
- separação FATO / INFERÊNCIA / HIPÓTESE / RISCO / TESTE.

## 3. Geometria base

| Símbolo | Valor | Uso |
| --- | --- | --- |
| `√3/2` | `0.86602540378` | altura triangular, contração espiral |
| `φ` | `1.61803398875` | crescimento/mistura áurea |
| `π` | `3.14159265358` | ciclo angular/toroidal |
| `Spiral(n)` | `(√3/2)^n` | coerência hierárquica convergente |
| `T^7` | `[0,1)^7` ou `[0,2π)^7` | espaço de estado |

## 4. Regra de análise técnica

Responder e documentar mudanças usando:

```text
FATO:       observado no código/log/doc
INFERÊNCIA: provável, mas ainda precisa teste
HIPÓTESE:   possível, não provada
RISCO:      falha, UB, crash, regressão, claim indevido
TESTE:      comando/critério para validar
```

## 5. PatchCutter ético

Todo patch deve ter:

```text
PATCH:
- Problema:
- Causa:
- Mudança mínima:
- Por que é segura:
- Como testar:
- Como reverter:
```

Princípios:

- alterar o mínimo necessário;
- preservar semântica existente;
- não ocultar efeito colateral;
- não burlar proteção de terceiros;
- preferir harness de teste a claim verbal.

## 6. SmartCode template

Para código novo:

```text
OBJETIVO:
AMBIENTE:  arch / OS / flags
CÓDIGO:
COMPILAR:
EXECUTAR:
VALIDAR:
LIMITES:
```

Regras:

- entrada clara;
- saída clara;
- zero heap em hot path freestanding quando possível;
- zero UB intencional;
- comentários apenas onde reduzem ambiguidade real.

## 7. Benchmark honesto

Antes/depois precisa registrar:

```text
MÉTRICA PRIMÁRIA: latência / throughput / ops/s / ciclos
MÉTRICA SECUNDÁRIA:
AMOSTRAS:
WARMUP:
OUTLIERS:
DISPOSITIVO:
RESULTADO:
CONFIANÇA:
```

Suspeitar sempre de:

- cache quente/frio;
- scheduler;
- DVFS;
- thermal throttling;
- I/O escondido;
- branch prediction;
- diferenças de ABI/flags.

## 8. Heurística Assembly

```text
1. Terreno: arch, modo, ABI, sistema, flags, objetivo
2. Fluxo: entrada → reg → mem → op → saída → syscall/ret
3. Riscos: UB, clobber ausente, stack desalinhada, syscall errada,
           callee-saved corrompido, label externo, instrução privilegiada
4. PatchCutter: menos linhas, mais prova, rollback claro
```

## 9. Segurança

Permitido:

- otimização de código próprio;
- auditoria defensiva;
- ensino ABI/assembly;
- benchmark;
- correção de bugs;
- refatoração de repositório autorizado.

Não permitido:

- bypass de DRM/licença/anticheat;
- loader furtivo;
- persistência indevida;
- exfiltração;
- exploração de alvo real sem autorização.

## 10. Fechamento obrigatório

Toda análise técnica deve terminar com:

```text
F_ok:   o que funcionou
F_gap:  lacuna identificada
F_next: próximo passo concreto
```

## Retro_Ω

F_ok: protocolo centraliza engenharia, ética, benchmark e simbologia operacional.
F_gap: este documento não substitui testes locais nem CI.
F_next: ligar cada patch de RafPolimata/Vectras/ChipQuantum a um arquivo de validação ou log.

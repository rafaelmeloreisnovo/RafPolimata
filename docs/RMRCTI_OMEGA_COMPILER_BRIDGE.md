# RMR-CTI / Ω → RafPolimata compiler bridge

**Entrada canônica:** `Apkc/omega_classifier.h`, `Apkc/codegen_select.h`, `raf_frontend.c` e `ci/contracts/apkc_compiler_station_v2.json`.  
**Estado:** `RUNTIME_WITH_TEST_GATE / ENGINEERING_METADATA_ONLY`.

## 1. Propósito

A ponte integra classificação determinística de bytes da família RMR-CTI/Ω ao compilador e às famílias de encoding ASM, sem copiar UI hospedada, CSV, heap, floating point ou claims sem prova para o núcleo freestanding.

A referência oral `gbc color 3.6.h` resolve para:

```text
llamaRafaelia/rmrCti/gbs3_color.c
```

Esse programa hospedado mede:

```text
ΔP = P(stable_any=1 | peak) - P(stable_any=1 | nonpeak)
```

`ΔP≈0.18` permanece candidato medido naquele contexto. Não é constante universal, limiar do compilador ou prova de atrator.

## 2. Ideias retidas e fronteiras

| Fonte | Ideia retida | Não incorporado ao núcleo |
|---|---|---|
| `gbs3_color.c` | separar medição, evidência e apresentação | ANSI UI, CSV, arena, heap, RNG temporal |
| `omega_forest.c` | cinco rótulos de roteamento | k-means e limiares de corpus como verdade universal |
| `omega_layersbit.h` | fold determinístico e 42 posições | engine tabular integral sem orçamento explícito |
| `Apkc/coherence.h` | métrica inteira `phi=(1-H)×C` | interpretação ética ou semântica automática |

## 3. Classificador atual

`Apkc/omega_classifier.h` é:

- freestanding;
- header-only;
- sem libc e heap;
- sem ponto flutuante;
- determinístico;
- limitado por estado local fixo.

Para cada fluxo de bytes registra:

```text
bytes e bytes únicos
transições
contagens printable/control/zero
ocupação de fold 256-bit
proxy de entropia [0,8000]
coerência Q16 e phi Q16
atrator [0,41]
flags e rota Ω
```

O proxy:

```text
H_milli = unique*6000/256 + transitions*2000/(n-1)
```

é feature de engenharia reproduzível, não prova de entropia de Shannon.

## 4. Cinco rótulos de roteamento

| Rótulo | Significado operacional |
|---|---|
| `VOID` | nenhuma entrada |
| `FORGOTTEN` | fluxo curto ou degenerado |
| `MENOSPREZADO` | sinal estrutural que merece revisão |
| `URGENT` | stream de alta coerência elegível à prioridade |
| `PROCESSUAL` | rota ordinária |

Esses rótulos não declaram maturidade, valor científico, significado ou correção semântica.

## 5. Aplicação no codegen

`Apkc/codegen_select.h` usa:

```c
raf_omega_codegen_index(emitted_buf, emitted_len, num_variants)
```

A chave combina atrator, rota, proxy de entropia, `phi` e flags. Ela só pode selecionar membros previamente provados equivalentes, por exemplo:

```text
MOV Xd,Xm
≡ ORR Xd,XZR,Xm
≡ ADD Xd,Xm,#0
≡ SUB Xd,Xm,#0
```

A classificação nunca pode escolher entre operações com saída, flags, exceções ou efeitos de memória diferentes. Equivalência arquitetural também não prova equivalência de timing, energia ou side channel.

## 6. Compilador raiz após o HOTFIX

A descrição antiga de `return 42` fixo foi removida. O fluxo operacional atual é:

```text
bytes reais da fonte
→ classificação Ω
→ exatamente uma expressão u32 válida
→ IR_MOVIMM(valor da fonte) + IR_RET
→ emissão específica da arquitetura
→ .s/.hex/.bin
→ .ops schema 4 transacional
```

O recibo `.ops` assina com FNV-1a 64 canônico:

```text
arquitetura + marca + cores
+ linguagem/flags/features
+ tamanho/hash da fonte
+ métricas Ω
+ IR/ASM/BIN + ir_value + emitter_schema
+ estado native
+ rollback_code + transaction_state
```

Estados:

```text
COMMITTED   = artefatos atuais e coerentes
ROLLED_BACK = nenhum binário anterior preservado
```

A classificação Ω participa da impressão operacional, mas não substitui parsing, lowering, compilação, auditoria ELF ou prova de runtime.

## 7. Gates

```text
tools/raf_omega_classifier_test.c
 tools/raf_codegen_select_test.c
scripts/test_ops_manifest.sh
scripts/validate_ops_manifest.py
scripts/compare_ops_manifest.py
scripts/test_compiler_station.sh
scripts/validate_compiler_station_contract.py
.github/workflows/ci.yml
```

Eles verificam:

1. estados `VOID` e `FORGOTTEN`;
2. distinção de flags textuais/binárias;
3. replay determinístico;
4. limites de atrator e métricas;
5. equivalência lógica das famílias de encoding;
6. assinatura `.ops` recomputável;
7. adulteração rejeitada;
8. mesma fonte com mesmos campos estáveis;
9. fontes com valores diferentes gerando bytes diferentes;
10. rollback eliminando artefatos antigos.

## 8. O que a ponte não prova

- universalidade das cinco classes;
- `phi` como medida de ética, significado ou consciência;
- `ΔP≈0.18` como invariante;
- compilação geral das 23 linguagens registradas;
- equivalência microarquitetural ou de side channel;
- assinatura, instalação ou launch de APK;
- runtime Android, GPU, DSP ou NPU observado.

Para código sensível a timing, fixar a variante de encoding e auditar a microarquitetura alvo separadamente.

## 9. Invariante

```text
source bytes
→ Ω engineering profile
→ bounded source-dependent lowering
→ verified architecture emission
→ transactional signed receipt
→ strict ELF and reproducibility gates
→ claim only inside the tested scope
```

FIAT LUX.

# RAF Semantic IR + x32/x64 — V1

**Estado:** implementação local verificável · `claim_allowed=false` · sem instalação/launch Android

## 1. Continuidade com M063

O M063 já classifica 23 perfis de linguagem e suas rotas de lowering. Esta camada executa o próximo passo: um subconjunto escalar estrito de fontes diferentes é reduzido à mesma árvore semântica tipada e à mesma identidade `rafk1-SHA256`.

```text
fonte C/C++/Java/JS/Kotlin/Python/Go/Ruby/Swift/PHP
→ expressão de retorno estrita
→ parser independente da linguagem
→ semântica u32/u64 com overflow modular explícito
→ árvore canônica
→ RAF Semantic IR em SSA
→ kernel_id
→ vetores de equivalência
→ C freestanding
→ assembly i386 + x86-64
→ ELF32 + ELF64
→ execução virtualizada quando QEMU local existir
→ receipt
```

Isso não é um parser completo das linguagens. O escopo V1 aceita somente kernels de uma expressão com argumentos inteiros, sem heap, chamadas, exceções, I/O, loops ou dependências externas. Sintaxe fora do contrato falha fechada.

## 2. Operações V1

- tipos: `u32`, `u64`;
- overflow: `wrap` obrigatório;
- binárias: `+`, `-`, `*`, `&`, `|`, `^`, `<<`, `>>`;
- unárias: `~`, `-`, `+`;
- operações comutativas recebem ordenação canônica;
- chamadas, acesso de memória e nomes não declarados são rejeitados.

## 3. Comandos

```sh
python3 -m unittest discover -s tests -p 'test_raf_semantic_ir.py'

python3 scripts/raf_semantic_ir.py validate \
  tests/fixtures/semantic_equivalence_mix32.v1.json \
  --out build/semantic/semantic-receipt.json

python3 scripts/raf_semantic_ir.py emit-c \
  tests/fixtures/semantic_equivalence_mix32.v1.json \
  --out build/semantic/raf_mix32.c

python3 scripts/raf_semantic_ir.py x3264 \
  tests/fixtures/semantic_equivalence_mix32.v1.json \
  --out-dir build/semantic/x3264 \
  --receipt build/semantic/x3264-receipt.json \
  --require-virtualized-both
```

O último comando só retorna sucesso integral quando `qemu-i386` e `qemu-x86_64` executam todos os vetores com o mesmo resultado semântico. Gerar ELF32/ELF64 não é tratado como execução.

## 4. Prova local de 2026-07-26

Ambiente observado:

```yaml
host: x86_64
python: 3.13.5
cc: GCC 14.2.0
binutils: 2.44
qemu_i386: TOKEN_VAZIO_COMMAND_ABSENT
qemu_x86_64: TOKEN_VAZIO_COMMAND_ABSENT
```

Resultado:

```yaml
semantic_frontends: 10
semantic_vectors: 4
unit_tests: 8_PASS
semantic_equivalence: PASS
c_freestanding_object: PASS
elf32_i386_generation: PASS
elf64_x86_64_generation: PASS
elf64_native_execution: 4_PASS
elf32_execution: TOKEN_VAZIO_EMULATOR_ABSENT
full_x32_x64_virtualization: TOKEN_VAZIO
```

A ausência de QEMU não é convertida em falha de semântica nem em PASS de virtualização.

## 5. Próximas extensões

1. materializar QEMU user-mode local com proveniência;
2. fechar `PASS_X32_X64_VIRTUALIZED`;
3. adicionar blocos, `select` branchless e memória por buffer do chamador;
4. criar adaptadores reais para os demais perfis M063;
5. ligar a IR aos emissores ARMv7/AArch64 e ao ApkC;
6. somente depois usar o kernel semântico como entrada dos APKs completos.

## 6. Invariantes

```yaml
complete_language_claim: false
semantic_subset_explicit: true
source_equivalence_requires_same_canonical_ir: true
elf_generated_is_not_elf_executed: true
native_execution_is_not_virtualized_execution: true
install_executed: false
launch_executed: false
claim_allowed: false
```

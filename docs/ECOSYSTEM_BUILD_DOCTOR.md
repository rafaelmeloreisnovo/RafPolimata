# Ecosystem Build Doctor — CMake, flags, linker, zumbis e proveniência

## Estado

```yaml
schema: raf.ecosystem-build-doctor-report.v1
implementation: IMPLEMENTED
unit_tests: IMPLEMENTED
ecosystem_execution: TOKEN_VAZIO
build_execution: TOKEN_VAZIO
automatic_deletion: false
automatic_rewrite: false
```

O `Ecosystem Build Doctor` é um scanner **read-only** para transformar fricção difusa de compilação em uma fila verificável.

Ele não decide sozinho que um arquivo deve ser apagado, que uma warning é irrelevante ou que um binário está correto. Ele responde:

```text
onde está
→ qual contrato pode estar quebrado
→ qual evidência foi observada
→ qual ação mínima deve ser executada
```

## Invariante


a warning não é tratada como ruído descartável:

```text
warning
→ target/ABI afetado
→ hipótese de contrato
→ teste positivo/negativo
→ artefato
→ decisão
```

Erros e warnings de linker recebem prioridade porque expõem fronteiras reais entre:

- fonte e target;
- símbolo e biblioteca;
- ABI e objeto;
- C e C++;
- hosted e freestanding;
- arquivo existente e arquivo efetivamente ligado ao binário.

## Achados detectados

| Código | Interpretação |
|---|---|
| `conflicting_optimization_flags` | mais de um nível `-O*` no mesmo arquivo de build |
| `multiple_architecture_flags` | múltiplos `-march` exigindo roteamento explícito por ABI |
| `cxx_only_flag_in_c_project` | `-fno-rtti`/`-fno-exceptions` num projeto C/ASM |
| `global_cmake_flags` | flags globais em vez de contrato por target |
| `cmake_glob_source_membership` | membership escondido por `file(GLOB)` |
| `all_warnings_disabled` | uso de `-w` |
| `warnings_not_blocking` | uso amplo de `-Wno-error` |
| `ci_continue_on_error` | CI permite falha sem bloquear |
| `shell_failure_masked` | `|| true` ou equivalente mascara exit code |
| `link_options_on_static_library` | opções de link atribuídas a archive `STATIC` |
| `mixed_freestanding_hosted_link_contract` | `-nostdlib` misturado a bibliotecas hosted |
| `dead_cmake_variable_candidate` | variável CMake definida sem consumidor localizado |
| `zombie_source_candidate` | fonte C/C++/ASM sem referência localizada em build |
| `executable_source_marker` | `TODO/FIXME/STUB/PLACEHOLDER/TOKEN_VAZIO` em fonte executável |
| `linker_diagnostic_recorded` | erro de linker encontrado em log/texto versionado |
| `binary_without_provenance` | binário sem hash/manifesto/proveniência localizada |

## Uso local no ecossistema

Com os repositórios clonados lado a lado:

```sh
python3 scripts/ecosystem_build_doctor.py \
  --repo blake3=../BLAKE3 \
  --repo vectras=../Vectras-VM-Android \
  --repo qemu=../qemu_rafaelia \
  --repo termux=../termux-app-rafacodephi \
  --repo llama=../llamaRafaelia \
  --repo rafpolimata=. \
  --repo rafgittools=../RafGitTools \
  --json-out results/build-doctor/report.json \
  --markdown-out results/build-doctor/report.md
```

Para usar como gate:

```sh
python3 scripts/ecosystem_build_doctor.py \
  --repo rafpolimata=. \
  --fail-on critical
```

O `--fail-on` aceita:

- `none`: inventário sem bloqueio;
- `medium`;
- `high`;
- `critical`.

A severidade não é uma sentença sobre o código. É uma prioridade de investigação.

## Interpretação de zumbis

`zombie_source_candidate` significa apenas:

> não foi localizada uma ligação explícita entre a fonte e os manifestos de build examinados.

A fonte pode ser:

1. canônica e esquecida no build;
2. teste/demo;
3. backend opcional;
4. material experimental;
5. legado necessário;
6. duplicação;
7. código realmente morto.

A promoção correta é:

```text
candidato
→ classificar
→ localizar consumidores
→ compilar/testar
→ manter, ligar, arquivar ou remover em PR dedicado
```

Nenhum arquivo é removido automaticamente.

## Binários e reescrita

Binário versionado precisa de, no mínimo:

```yaml
artifact:
  content_hash: sha256-or-blake3
  source_commit: commit
  build_command: command
  toolchain: compiler-linker-version
  abi: architecture-and-api
  license_boundary: declared
  reproducibility: PASS | FAIL | TOKEN_VAZIO
```

O doctor procura sidecars e arquivos de manifesto/proveniência próximos. A ausência gera fila; não prova que o binário é inválido.

## Fronteira de claims

A execução do scanner permite afirmar:

```yaml
static_analysis: VERIFIED_BY_EXECUTION
```

Ela não permite afirmar:

```yaml
build_execution: TOKEN_VAZIO
runtime_correctness: TOKEN_VAZIO
performance_superiority: TOKEN_VAZIO
```

Um achado estático só muda de estado depois do gate específico.

## Ordem de fechamento recomendada

1. linker errors e símbolos ausentes;
2. ABI/arquitetura e flags conflitantes;
3. warnings mascaradas;
4. fonte canônica fora do build;
5. contrato hosted/freestanding;
6. binários sem proveniência;
7. marcadores de lacuna em fonte;
8. material experimental/legado e duplicidades.

## Retroalimentação

```text
F_ok   = scanner, relatório, schema e testes implementados
F_gap  = execução nos clones completos do ecossistema
F_next = gerar baseline e abrir correções por repositório, sem PR monolítica
```

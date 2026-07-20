# ApkC — primeira parte executável do fechamento de lacunas

> **Entrada canônica:** docs/AGENTES.md §8 (entradas canônicas por subsistema — apkc_main) e §5 (pipeline operacional VOID→VALIDATED). Primeira parte congelada do fechamento de lacunas ApkC — toolchain Termux, validação binária ELF/DEX/APK e claim_allowed=false até execução real.

**Repositório:** `rafaelmeloreisnovo/RafPolimata`  
**Branch:** `feat/safe-extended-local-ci-20260719`  
**PR:** `#147`  
**Escopo:** primeira parte congelada antes da execução real no Termux.

## Cadeia implementada

```text
verdade do artefato
→ build fail-closed
→ tabela de linguagens validada antes de cada lookup
→ toolchain resolvida no Termux
→ Java/Groovy em JAR antes do D8
→ validação binária independente ELF/DEX/APK
→ busca objetiva do navegador ASM/TLS
→ mapa dos arquivos soltos
```

## Correções centrais

- `apkc-compile.txt` deixou de poder sustentar falso PASS quando contém erro;
- AArch64 e ARM32 precisam compilar duas vezes, identificar ABI e reproduzir bytes;
- `readelf` e `llvm-readelf` são aceitos;
- extensão desconhecida retorna erro, não ASM silencioso;
- tabela inválida bloqueia `lang_profile_find()` e `lang_profile_from_path()`;
- `execve` ganhou resolução determinística Android/Termux;
- Java e Groovy geram JAR antes do D8;
- ELF, DEX e APK ganharam parser independente dos geradores C;
- documentação e artefatos são reconciliados por estado;
- arquivos soltos recebem hash, categoria e rota sem exclusão automática.

## Navegador ASM/TLS

`raf_shell/raf_shell.c` é navegador local TUI. Não foi localizado, neste repositório, um corpo que reúna HTTP, sockets, TLS 1.2/1.3, validação de hostname e cadeia X.509.

```text
TUI file browser = IMPLEMENTED
ASM web browser = TOKEN_VAZIO
TLS 1.2/1.3 + X.509 = TOKEN_VAZIO
```

## Execução obrigatória

```sh
python3 scripts/apkc_first_part_gate.py \
  --write results/apkc-first-part-gate.json \
  --write-map docs/generated/REPOSITORY_LOOSE_FILES_MAP.md

python3 -m unittest \
  tests.test_apkc_first_part_gate \
  tests.test_validate_apkc_formats

bash tools/raf_source_to_binary_proof.sh
sh safe-extended run .github/workflows/apkc-first-part.yml
```

## Estado honesto

Os workflows GitHub recuperados tiveram `failure`, porém o job inspecionado não apresentou steps. Assim, não existe prova de que estes gates foram executados pelo runner.

```text
código/gates = IMPLEMENTED
execução de testes nesta sessão = TOKEN_VAZIO
execução Termux = TOKEN_VAZIO
APK com ELF32+ELF64 = TOKEN_VAZIO
DEX funcional Android = TOKEN_VAZIO
navegador ASM/TLS = TOKEN_VAZIO
claim_allowed = false
```

A segunda parte somente deve abrir novas funções depois que esta cadeia produzir relatórios e hashes reais.

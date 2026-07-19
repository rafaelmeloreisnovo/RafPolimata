# ApkC — primeira parte executável do fechamento de lacunas

**Repositório:** `rafaelmeloreisnovo/RafPolimata`  
**Branch:** `feat/safe-extended-local-ci-20260719`  
**PR:** `#147`  
**Estado:** código implementado; execução Termux e runtime Android ainda precisam produzir evidência própria.

## Cadeia desta tranche

```text
verdade do artefato
→ build fail-closed
→ linguagem validada
→ toolchain resolvida no Termux
→ Java/Groovy em JAR antes do D8
→ validação binária independente ELF/DEX/APK
→ busca objetiva do navegador ASM/TLS
→ mapa dos arquivos soltos
```

## Falha concreta corrigida

`Apkc/proofs/out/apkc-compile.txt` contém falhas reais de compilação, enquanto documentos posteriores promoviam build, ELF e DEX a `PASS`. Os artefatos atuais de geração, ELF32, ELF64 e DEX continuam `TOKEN_VAZIO`.

`tools/raf_source_to_binary_proof.sh` agora falha fechado. Ele cria um run isolado, registra commit/toolchain, aceita `readelf` ou `llvm-readelf`, compila AArch64 e ARM32 duas vezes, valida classe/máquina, compara SHA-256 e somente promove o transcript canônico quando todos os gates passam.

## Compilador multilíngua

`Apkc/lang_profile.h` agora rejeita extensão desconhecida em vez de tratá-la como ASM. A tabela valida família de execução, nomes/extensões duplicados, compilador obrigatório e coerência D8/DEX.

`Apkc/sys.h` resolve ferramentas por caminhos determinísticos:

```text
/data/data/com.termux/files/usr/bin/
/system/bin/
/usr/bin/
/bin/
```

Um caminho convencional ausente, como `/usr/bin/python3`, recai para o basename no Termux. O ambiente mínimo contém somente `PATH`, `HOME`, `TMPDIR` e `LANG=C`.

Java e Groovy agora seguem:

```text
source → classes temporárias → JAR → D8 → classes.dex
```

Arquivos:

```text
scripts/apkc_java_to_jar.sh
scripts/apkc_groovy_to_jar.sh
```

## ELF/DEX/APK

`scripts/validate_apkc_formats.py` verifica bytes de maneira independente dos geradores C:

- DEX: magic, versão, tamanho, endian tag, SHA-1, Adler-32, seção de dados e map list;
- ELF: classe, little-endian, `ET_DYN`, máquina ARM/AArch64, tabelas, `PT_LOAD` e limites dos segmentos;
- APK: CRC ZIP, `classes.dex`, bibliotecas por ABI e exigência opcional das duas ABIs.

Testes positivos e negativos estão em `tests/test_validate_apkc_formats.py`.

```sh
python3 scripts/validate_apkc_formats.py \
  --apk Apkc/proofs/out/hello.apk \
  --require-both \
  --write results/apkc-format-validation.json
```

## Navegador ASM/TLS

`raf_shell/raf_shell.c` é navegador de arquivos TUI. Até este corte, não foi localizado no repositório um corpo que reúna HTTP, sockets, handshake TLS 1.2/1.3, hostname e cadeia X.509. Portanto:

```text
TUI file browser = IMPLEMENTED
ASM web browser = TOKEN_VAZIO
TLS 1.2/1.3 + X.509 = TOKEN_VAZIO
```

## Arquivos soltos

`scripts/apkc_first_part_gate.py` gera mapa versionado por caminho, SHA-256, tamanho, categoria, presença no índice e rota:

- `INDEXED`;
- `ADD_TO_CANONICAL_INDEX`;
- `MOVE_OR_INDEX`.

Nenhum arquivo é apagado automaticamente.

## Execução local

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

```text
código/gates desta tranche = IMPLEMENTED
execução Termux = TOKEN_VAZIO
APK atual com ELF32+ELF64 = TOKEN_VAZIO
DEX funcional em Android = TOKEN_VAZIO
navegador ASM/TLS localizado = TOKEN_VAZIO
claim_allowed = false
```

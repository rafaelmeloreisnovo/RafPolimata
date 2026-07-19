# ApkC — primeira parte executável do fechamento de lacunas

**Repositório:** `rafaelmeloreisnovo/RafPolimata`  
**Branch:** `feat/safe-extended-local-ci-20260719`  
**PR:** `#147`  
**Estado:** código implementado; execução Termux e runtime Android ainda devem produzir evidência própria.

## 1. Objetivo desta tranche

Esta etapa não tenta declarar o repositório inteiro concluído por documentação. Ela fecha primeiro as fronteiras que permitem confiar nas próximas execuções:

```text
verdade do artefato
→ build fail-closed
→ seleção correta da linguagem
→ resolução real da toolchain
→ JAR/DEX coerente
→ distinção ELF estrutural/runtime
→ localização objetiva do navegador/TLS
→ mapa dos arquivos soltos
```

## 2. Falha concreta encontrada

O transcript canônico `Apkc/proofs/out/apkc-compile.txt` contém falhas de compilação, incluindo opção `-target` enviada ao GCC e registradores ARM usados em host incompatível. Ao mesmo tempo, documentos posteriores promoviam L1/ELF/DEX a `PASS`.

Também permanecem canonicamente vazios:

```text
Apkc/proofs/out/apkc-generate.txt
Apkc/proofs/out/readelf-arm32.txt
Apkc/proofs/out/readelf-arm64.txt
Apkc/proofs/out/dex-sha1.txt
```

A correção foi registrar a contradição e impedir nova promoção parcial.

## 3. Prova source→binary v2

`tools/raf_source_to_binary_proof.sh` agora:

- cria um diretório isolado por execução;
- registra commit, data, host e toolchain;
- compila AArch64 duas vezes;
- compila ARM32 duas vezes;
- valida `Class` e `Machine` com `readelf`;
- compara SHA-256 e bytes das duas compilações;
- escreve `status.json` tipado;
- somente promove o transcript canônico quando todos os gates passam;
- sai com código diferente de zero em resultado incompleto;
- preserva o transcript anterior em `Apkc/proofs/archive/`.

```text
A64 build
∧ A64 identity
∧ A64 reproducibility
∧ A32 build
∧ A32 identity
∧ A32 reproducibility
= PASS necessário
```

## 4. Compilador multilíngua

### 4.1 Extensão desconhecida

Antes, arquivo sem extensão ou extensão desconhecida era enviado silenciosamente ao assembler.

Agora:

```text
unknown extension → NULL → erro de CLI → nenhum APK
```

A tabela de 23 perfis também possui verificador de:

- uma única família de execução por perfil;
- nomes e extensões sem duplicidade;
- compilador obrigatório quando aplicável;
- coerência `use_d8 + dex_output`;
- coerência JSX.

### 4.2 `execve` e Termux

`execve("clang", ...)` não pesquisa `PATH`. Caminhos como `/usr/bin/python3` normalmente também não existem no Termux.

`Apkc/sys.h` agora:

1. tenta o caminho explícito;
2. extrai o basename quando o caminho convencional não existe;
3. tenta prefixos determinísticos:

```text
/data/data/com.termux/files/usr/bin/
/system/bin/
/usr/bin/
/bin/
```

4. fornece ambiente mínimo sem segredo:

```text
PATH
HOME
TMPDIR
LANG=C
```

Não existe interpolação por shell nessa resolução.

### 4.3 Java e Groovy

`javac -d` e `groovyc -d` produzem diretórios de classes, não um arquivo JAR. O pipeline anterior tentava ler o diretório como se fosse `/tmp/apkc_out.so`.

Agora:

```text
Java/Groovy source
→ classes em diretório temporário
→ jar --create
→ D8
→ classes.dex
```

Arquivos:

```text
scripts/apkc_java_to_jar.sh
scripts/apkc_groovy_to_jar.sh
```

Kotlin continua emitindo JAR diretamente com `kotlinc -include-runtime -d`.

## 5. ELF e DEX

### ELF

`Apkc/fmt_elf.h` possui geradores estruturais ELF32 e ELF64 com seções dinâmicas e símbolos. Isso é implementação de formato.

Não prova ainda:

- `.so` presente no APK atual;
- `readelf` sobre o mesmo run;
- `dlopen` Android;
- `ANativeActivity_onCreate` executado;
- ausência de crash.

### DEX

`Apkc/fmt_dex.h` gera DEX 035 mínimo de 140 bytes com:

- limite de buffer;
- SHA-1 do DEX;
- Adler-32;
- header e map list.

Esse DEX mínimo é estrutural. Java/Kotlin/Groovy funcional exige:

```text
JAR real → D8 → dexdump → instalação → execução Android
```

## 6. Navegador ASM e TLS

A árvore contém `raf_shell/raf_shell.c`, que é um navegador de arquivos TUI e orquestrador. Isso não é automaticamente um navegador web.

Até este corte, a busca correlacionada não localizou no repositório:

```text
transporte HTTP
+ sockets/connect
+ handshake TLS 1.2/1.3
+ validação de hostname
+ cadeia X.509
+ testes negativos de certificado
```

Por isso:

```text
TUI file browser = IMPLEMENTED
ASM web browser = TOKEN_VAZIO
TLS 1.2/1.3 + X.509 = TOKEN_VAZIO
```

Caso o navegador esteja em outro repositório ou diretório ainda não indexado, o mapa gerado deve revelar o caminho. Quando localizado, ele entra como componente separado e recebe testes de handshake e certificado; não será promovido apenas pelo nome do arquivo.

## 7. Arquivos soltos e documentação completável

`scripts/apkc_first_part_gate.py` percorre arquivos versionados e produz:

```text
path
sha256
size
category
canonical-index reference
route
```

Rotas:

- `INDEXED` — já aparece em índice canônico;
- `ADD_TO_CANONICAL_INDEX` — arquivo em diretório conhecido, mas sem referência;
- `MOVE_OR_INDEX` — arquivo na raiz sem rota documental clara.

Nenhum arquivo é apagado automaticamente. O mapa preserva proveniência e permite completar documentos a partir dos artefatos realmente existentes.

## 8. Execução pelo Safe Extended no Termux

```sh
cd ~/RafPolimata

git checkout feat/safe-extended-local-ci-20260719

python3 scripts/apkc_first_part_gate.py \
  --write results/apkc-first-part-gate.json \
  --write-map docs/generated/REPOSITORY_LOOSE_FILES_MAP.md

python3 -m unittest tests.test_apkc_first_part_gate

bash tools/raf_source_to_binary_proof.sh

sh safe-extended run .github/workflows/apkc-first-part.yml
```

O workflow não contém rede, instalação de pacote ou root. Ele usa somente checkout local, comandos locais e preservação local de artefatos.

## 9. Critério de fechamento desta primeira parte

```text
code gates = PASS
proof contradiction = 0
source-to-binary = PASS
report hash = registrado
loose-file map = gerado
```

ELF dentro do APK, DEX funcional, assinatura, instalação e runtime continuam na próxima cadeia até que um único run os comprove.

## R₃

- **F_ok:** falso verde bloqueado; linguagem e toolchain corrigidas; mapa e gate implementados.
- **F_gap:** execução Termux, APK atual com ELF32/64, DEX funcional e fonte real do navegador ASM/TLS.
- **F_next:** executar `.github/workflows/apkc-first-part.yml` localmente e anexar os hashes ao PR #147.

# Lacunas Profundas — RafPolimata/ApkC: de MVP Técnico a Produto Forte

> **Cadeia de custódia documental — 2026-06-15**
> Autoria original: Rafael Melo Reis Novo (análise conservadora, modo auditável).
> Integração técnica: Claude Code (auditoria de código, CI e sessão de engenharia).
> Este documento não é avaliação financeira. É mapa de estado honesto: o que PASS,
> o que TOKEN_VAZIO, o que o próximo passo concreto requer.

---

## Princípio de leitura

O projeto já sabe dizer a verdade: `PASS`, `TOKEN_VAZIO`, `SKIP`, `PASS limitado`.
O próximo salto não é mudar de paradigma — é reduzir o `TOKEN_VAZIO` nos pontos que
mais aumentam valor de produto. Este documento mapeia todos eles em ordem de impacto.

```
source → build reproduzível → APK → assinatura → install → launch → logcat
       → relatório → CI artifact → release navegável
```

---

## Σ Síntese executiva

| Dimensão | Estado atual | Próximo fechamento |
|----------|-------------|-------------------|
| Código/arquitetura | ✅ Sólido | Manter invariantes |
| Prova source→binary | TOKEN_VAZIO | Transcript compile em CI |
| Runtime NativeActivity | TOKEN_VAZIO | logcat sem crash |
| ARM64 ELF validado | TOKEN_VAZIO | APK com lib/arm64-v8a/ |
| ARM32 assembler completo | GAP ABERTO | 39 mnemonics desconhecidos |
| Matriz de linguagens provadas | 1/12 (ASM) | Pipeline por linguagem |
| CI artifact verde | Parcial | Run completo com logs |
| Coerência de artefatos | Inconsistente | Rodada única limpa |

---

## Parte I — Lacunas de Prova e Cadeia de Custódia

### L1 — Prova source→binary do executável apkc

**O gap:** existe evidência de um binário `apkc` com SHA-256 registrado, mas falta
o transcript completo de compilação `source → binary`. Impossível afirmar reprodutibilidade
plena: dá para dizer "há um APK gerado", mas não "este APK foi gerado bit-a-bit por
este código versionado".

| Elemento | Estado |
|----------|--------|
| Código-fonte em commit | PASS — `Apkc/apkc.c` no repositório |
| Hash do binário apkc | PASS — SHA-256 em `CHAIN_OF_CUSTODY` |
| Transcript de compilação | TOKEN_VAZIO — `apkc-compile.txt` ausente/vazio |
| Ambiente/toolchain registrado | TOKEN_VAZIO |
| Reprodução por terceiro | TOKEN_VAZIO |

**Próximo passo:**
```sh
# No mesmo run, no mesmo commit:
gcc -std=c11 -Wall -Wextra -Wno-unused-function \
    -nostdlib -Wl,-e,_start Apkc/apkc.c -o /tmp/apkc
sha256sum /tmp/apkc
echo "commit: $(git rev-parse HEAD)"
echo "date: $(date -u)"
```
Registrar `comando + commit + date + sha256 + toolchain` em `Apkc/proofs/out/apkc-compile.txt`.

---

### L2 — Runtime NativeActivity com logcat limpo

**O gap:** visibilidade de pacote (`package:com.rafael.teste`) não é execução.
Falta evidência de lançamento sem crash, `dlopen` correto, entrada nativa executando
e ausência de `AndroidRuntime/FATAL EXCEPTION`.

| Gate | Estado |
|------|--------|
| Pacote instalado/visível | PASS limitado |
| Lançamento sem crash | TOKEN_VAZIO |
| dlopen libhello.so | TOKEN_VAZIO |
| ANativeActivity_onCreate executou | TOKEN_VAZIO |
| logcat limpo (sem FATAL) | TOKEN_VAZIO |

**Próximo passo:**
```sh
adb shell monkey -p com.rafael.teste -c android.intent.category.LAUNCHER 1
adb logcat -d | grep -i -E 'NativeActivity|AndroidRuntime|dlopen|fatal|crash'
# Registrar stdout integral em Apkc/proofs/out/logcat-nativeactivity.txt
```

---

### L3 — stdout completo de `adb install -r`

**O gap:** `adb-install.txt` registra `package:com.rafael.teste` (package visibility PASS),
mas não contém o stdout exato do `adb install -r` com: device serial, data UTC,
resultado `Success`/`Failure`, warnings de SDK ou ABI.

**Próximo passo:** capturar e arquivar `adb install -r hello-signed.apk 2>&1` na
íntegra, com `date -u` e `adb devices` no mesmo arquivo.

---

### L4 — Prova ARM64 real

**O gap:** a prova forte atual é ARM32: `libhello.so` como ELF32, little-endian,
Machine: ARM, EABI5. ARM64 está TOKEN_VAZIO: `readelf-arm64.txt` confirma ausência
de `lib/arm64-v8a/*.so` no APK enviado.

Android moderno é majoritariamente ARM64. Sem ARM64 validado, o MVP parece experimental.

| Gate | Estado |
|------|--------|
| ELF ARM32 readelf | PASS — `readelf-arm32.txt` |
| ELF ARM64 readelf | TOKEN_VAZIO — APK não contém lib/arm64-v8a/ |

**Próximo passo:**
```sh
./apkc Apkc/hello.s.txt -o /tmp/hello-arm64.apk -both
unzip -p /tmp/hello-arm64.apk 'lib/arm64-v8a/*.so' > /tmp/libhello-arm64.so
readelf -h /tmp/libhello-arm64.so | grep -E 'Class|Machine'
# Esperado: ELF64, AArch64
```

---

### L5 — 39 mnemonics ARM32 desconhecidos

**O gap:** o transcript de geração registra 39 ocorrências de `apkc: unknown ARM32 mnemonic`
antes de escrever o APK. Produto comercial não pode emitir APK com NOP placeholders
silenciosos para instruções desconhecidas.

```
F2 generate: 39× "apkc: unknown ARM32 mnemonic" → NOP placeholder
```

**Estado por instrução:** não catalogadas. A lista completa está no `apkc-generate.txt`.

**Próximo passo:** 
1. Extrair as 39 mnemonics do log
2. Para cada uma: implementar, ou emitir erro bloqueante, ou marcar `UNSUPPORTED`
3. Atualizar `Apkc/arch_arm32.h` ou `asm_insn32()` em `Apkc/apkc.c`

---

### L6 — Artefatos `out/` inconsistentes entre runs

**O gap:** `validation-summary.md` declara que DEX SHA-1, AXML e outros passaram,
mas os arquivos atuais `dex-sha1.txt` e `aapt-xmltree.txt` dizem `TOKEN_VAZIO:
hello.apk ausente`. Isso é explicável por runs parciais, mas para auditoria externa
vira gap de cadeia de custódia.

**Regra de fechamento:** uma rodada única e limpa que gere todos os arquivos no mesmo:
- Commit SHA
- Data UTC  
- Hash do APK
- Diretório `Apkc/proofs/out/`

---

## Parte II — Lacunas de CI e Automação

### L7 — CI configurado mas sem run verde com artifacts

**O gap:** o workflow existe e é arquiteturalmente sólido (15+ gates), mas a existência
do YAML não substitui um run verde com logs e artifacts. PRs recentes têm checkboxes
vazios nos gate lists dos templates.

**Estado atual (pós-sessão 2026-06-15):**

| Gate CI | Estado |
|---------|--------|
| Validate coherence protocol | Executado |
| Host C syntax check | Executado |
| Build raf_compile (strict) | Executado |
| Smoke test compiler | Executado |
| Validate ops manifest | Executado |
| Audit repository structure | Executado |
| Android ABI plan | Executado (--plan mode) |
| ApkC freestanding audit | Executado |
| ApkC validate + generate | TOKEN_VAZIO (x86_64, sem ARM) |
| **ApkC lang coverage (6/12)** | **NOVO — PR #48** |
| Debug signing | TOKEN_VAZIO |
| **P(k) gate bloqueante** | **NOVO — PR #50** |

**Próximo passo:** associar cada release/tag a um CI run concreto com
`run_id`, `artifact_url` e `commit_sha` em `CHANGELOG.md`.

---

### L8 — android_build_matrix.sh em modo --plan, não --build

**O gap:** o script roda em `--plan` mode; `--build` real exige
`ANDROID_NDK_HOME`/`ANDROID_NDK_ROOT`, ausentes no runner padrão.
A matriz declara `armeabi-v7a` + `arm64-v8a` com API configurável,
mas não prova compatibilidade por API.

**Próximo passo:** adicionar job opcional `runs-on: ubuntu-24.04-arm`
(GitHub ARM64 runners) com NDK instalado via `android-sdk-tools` action.

---

## Parte III — Lacunas de Cobertura de Linguagens

### L9 — Matriz multilíngua: 12 declaradas, 1 provada, 6 com framework

**O gap:** 12 linguagens declaradas em `lang_profile.h`. Mercado não paga por
"declara 12"; paga por "12 provadas".

| Lang | Pipeline | CI x86_64 | ARM64 Termux | Prova runtime |
|------|----------|-----------|--------------|---------------|
| ASM  | use_asm | TOKEN_VAZIO¹ | PASS | PASS limitado |
| py   | use_script | TOKEN_VAZIO¹ | Framework OK | TOKEN_VAZIO |
| sh   | use_script | TOKEN_VAZIO¹ | Framework OK | TOKEN_VAZIO |
| pl   | use_script | TOKEN_VAZIO¹ | Framework OK | TOKEN_VAZIO |
| js   | use_script | TOKEN_VAZIO¹ | Framework OK | TOKEN_VAZIO |
| php  | use_script | TOKEN_VAZIO¹ | Framework OK | TOKEN_VAZIO |
| c    | use_fork | TOKEN_VAZIO² | TOKEN_VAZIO² | TOKEN_VAZIO |
| cpp  | use_fork | TOKEN_VAZIO² | TOKEN_VAZIO² | TOKEN_VAZIO |
| rs   | use_fork | TOKEN_VAZIO² | TOKEN_VAZIO² | TOKEN_VAZIO |
| kt   | use_fork | TOKEN_VAZIO² | TOKEN_VAZIO² | TOKEN_VAZIO |
| java | use_fork | TOKEN_VAZIO² | TOKEN_VAZIO² | TOKEN_VAZIO |
| jsx  | use_fork | TOKEN_VAZIO² | TOKEN_VAZIO² | TOKEN_VAZIO |

¹ `apkc` não linka como binário nativo em x86_64 (sem `_start` para host)  
² `fork_exec_wait()` é `#ifdef __aarch64__` apenas

**Matriz ideal de evidência por linguagem:**
```
| profile | toolchain | artifact | sign | install | runtime | status | stdout |
```

---

### L10 — Caminhos de intérprete em APKs Android reais

**O gap:** o bootstrap `use_script` embute paths como `/usr/bin/python3`,
`/bin/sh`, `/usr/bin/node`, `/usr/bin/php`. Em Android padrão (fora de
Termux/proot), esses paths não existem e SELinux/sandbox pode bloquear execve.

**Definição pendente:** qual é o alvo real?
- APK para Android padrão  
- APK para Termux-ambiente  
- APK para proot  
- Device dev/laboratório  

Sem essa definição, multilíngua é potência conceitual com risco de runtime alto.

---

### L11 — Java/Kotlin/DEX funcional

**O gap:** pipeline declara `kotlinc/javac → d8 → classes.dex`. DEX mínimo
de 140 bytes não prova lógica Java útil. Falta: `.jar → d8 → classes.dex` com
DEX real, classes válidas, método de entrada coerente e runtime.

---

## Parte IV — Lacunas de Código e Módulos

### L12 — Catálogo dos 56 métodos RAF desalinhado

**O gap:** o índice aponta para `methods/001_...c`, arquivos estão na raiz como
`RAF_001_...c`. `RAF_001_...c` inclui `../include/RAF_rafaelia_common.h` mas o
header real está na raiz como `RAF_rafaelia_common.h`. 56 templates stubs
retornam 0 imediatamente.

Conceitualmente forte (GPIO, timers, DMA, JNI, QEMU, benchmark, cache).
Estruturalmente: PENDING — não é pacote compilável confiável.

---

### L13 — raf_compile hardcoded em IR abstrato

**O gap:** `raf_precompile()` gera apenas duas instruções abstratas (`IR_MOVIMM 42`
+ `IR_RET`) e emite assembly/hex fixo que retorna 42. É esqueleto de pipeline
excelente, mas não é compilador funcional.

**Decisão pendente:** implementar parser/lowering real, ou reposicionar como
"gerador de manifesto/assinatura operacional" explicitamente.

---

### L14 — verbovivo: contradição heap vs sem-heap

**O gap:** `verbovivo.h` afirma "No heap: all state is caller-allocated".
`verbovivo_main()` usa `malloc()` para a trajetória e depois `free()`.

Não invalida o ApkC, mas cria contradição interna no módulo. Para produto:
ou a promessa muda para "sem heap nos hot paths / ApkC", ou `verbovivo_main()`
migra para buffer estático.

---

### L15 — verbovivo sem CI automatizado

**O gap:** CLI documentada em `CLAUDE.md` mas não testada em CI:
modo T^7 (APK/ELF→SVG), modo Fiber-H (stdin→audit+SVG), recall top-N.

**Próximo passo:**
```yaml
- name: Build and smoke-test verbovivo
  run: |
    gcc -std=c11 -O2 -I. -IBenchmark -DVERBOVIVO_MAIN \
        rafaelia/verbovivo.c -lm -o /tmp/verbovivo
    echo "test" | /tmp/verbovivo -s > /tmp/graph.svg
    grep -q '<svg' /tmp/graph.svg
    echo "verbovivo: OK"
```

---

## Parte V — Lacunas de Política e Produto

### L16 — Política: "erro bloqueante" vs "degradação permitida"

**O gap:** 39 ARM32 mnemonics desconhecidos permitiram gerar APK (NOP placeholder).
Em produto, isso deve ser `FAIL` ou `PASS_LIMITED` explícito no exit code.

**Regra proposta:**
```
--allow-nop-placeholder   → modo experimental, aviso, exit 0
(sem flag)                → mnemonic desconhecido → error + exit 1
```

---

### L17 — Corpus regressivo de formatos

**O gap:** há prova em um APK mínimo. Não garante robustez para:
- Manifest com múltiplas permissões
- APKs com DEX maior
- Nomes de libs diferentes de `libhello.so`  
- Alinhamento ZIP não-padrão
- Seções ELF extras
- Diferenças de Android build-tools versão

**Próximo passo:** criar `tests/fixtures/` com:
```
tests/fixtures/minimal.apk          ← APK gerado + hash esperado
tests/fixtures/multi-abi.apk        ← ARM32 + ARM64
tests/fixtures/with-permissions.apk ← INTERNET + CAMERA no manifest
tests/fixtures/negative/bad-zip.apk ← ZIP corrompido → deve falhar
```

**Sub-gap fechado:** a dimensão "matriz API/ABI ampla" (minSdkVersion ×
targetSdkVersion × arquitetura) está coberta por
`scripts/apkc_api_abi_matrix.sh` (gate de CI), que gera um APK para cada
combinação de minSdkVersion ∈ {21,24,28,29,30,31,33,34} × as 6 linguagens
sem toolchain externa, e prova — lendo os bytes binários do
`AndroidManifest.xml` dentro do APK — que o valor de minSdkVersion pedido
foi de fato codificado no AXML (não apenas que o build não falhou). Sem
toolchain ARM no host, todas as células ficam `TOKEN_VAZIO` honestamente
(mesmo padrão de `apkc_lang_coverage.sh`). As demais dimensões de L17
(permissões múltiplas, DEX maior, libs com nomes diferentes, alinhamento
ZIP não-padrão, seções ELF extras, testes negativos) permanecem abertas.

---

### L18 — Assinatura de release, não só debug

**O gap:** assinatura atual é positiva (v1/v2/v3), signer RSA 2048,
`CN=ApkC Debug, O=Rafael, C=BR`. Mas v3.1, v4 e SourceStamp são `false`.

**Política necessária:**
```
debug keystore   → laboratório/CI  → NUNCA commitar
release keystore → distribuição    → CI secrets / HSM
SourceStamp      → proveniência    → cadeia de custódia comercial
```

---

### L19 — Release navegável para terceiros

**O gap:** documentação forte, cadeia de prova parcial, mas "primeiro caminho
feliz" em 5 minutos ainda não existe como release técnica com assets.

**Meta:**
```sh
git clone .../RafPolimata
make proof       # → Apkc/proofs/out/*.txt com todos os gates
make apkc-demo   # → out/hello.apk assinado + hash
make verbovivo-demo  # → engram.svg + audit
make report      # → PDF/HTML com matriz completa
```

---

### L20 — Valuation como métrica, não narrativa

**O gap:** `APKC_VALUE_AND_GAPS.md` tem faixas heurísticas corretas, mas
valor ainda é narrativa. Para valuation defensável: vincular cada faixa a gates.

| Faixa heurística | Gates necessários |
|-----------------|-------------------|
| US$ 25k–75k (atual) | Arquitetura + prova parcial ARM32 |
| US$ 100k–300k | + ARM64 + runtime logcat + source→binary |
| US$ 300k–750k | + CI artifact verde + 3+ linguagens provadas |
| US$ 1M+ | + adoção externa + corpus regressivo + release navegável |

Cada avanço técnico aumenta a tese de mercado de forma mensurável.

---

## Parte VI — Lacunas resolvidas nesta sessão (2026-06-15)

### ✅ ADR/ADRP: opcode incorreto e backpatch errado — PR #45

**Problema:** `asm_insn64()` emitia opcode `ADR` mesmo para instrução `adrp`.
O backpatch aplicava delta em bytes; ADRP exige delta em páginas (>>12).

**Correção:** `Apkc/apkc.c` — emit seleciona opcode por mnemônico;
backpatch detecta ADRP via bit 31 (`insn & 0x80000000u`) e aplica `delta>>12`.

---

### ✅ Cobertura de linguagens no CI: 1/12 → 6/12 — PR #48

**Problema:** CI cobria apenas ASM via `hello.s.txt`.

**Correção:** `scripts/apkc_lang_coverage.sh` + novo step no CI.
ASM + py + sh + pl + js + php testados. use_fork TOKEN_VAZIO (ARM64-only).

---

### ✅ P(k) gate bloqueante — PR #50

**Problema:** `first_test_pk.py` imprimia veredicto mas nunca falhava CI.

**Correção:** `.github/workflows/ci.yml` — `raise SystemExit('P(k) gate FAIL')`
quando `verdict == 'FAIL'`. Veredicto atual: `PASS` (rrmse=0.119, coverage=1.0).

---

## FNext — Próximos comandos para fechar os 5 gaps de maior valor

```sh
# 1. source→binary (L1)
gcc -std=c11 -nostdlib -Wl,-e,_start Apkc/apkc.c -o /tmp/apkc
echo "$(date -u) commit=$(git rev-parse HEAD) sha=$(sha256sum /tmp/apkc)" \
  >> Apkc/proofs/out/apkc-compile.txt

# 2. ARM64 APK (L4)
/tmp/apkc Apkc/hello.s.txt -o /tmp/hello-arm64.apk -both
unzip -p /tmp/hello-arm64.apk 'lib/arm64-v8a/*.so' | readelf -h -

# 3. ARM32 mnemonic audit (L5)
grep "unknown ARM32 mnemonic" Apkc/proofs/out/apkc-generate.txt | sort -u

# 4. Runtime logcat (L2)
adb shell monkey -p com.rafael.teste -c android.intent.category.LAUNCHER 1
adb logcat -d | grep -i -E 'NativeActivity|dlopen|fatal'

# 5. verbovivo CI (L15)
gcc -std=c11 -O2 -I. -IBenchmark -DVERBOVIVO_MAIN rafaelia/verbovivo.c -lm -o /tmp/verbovivo
echo "test" | /tmp/verbovivo -s | grep -q '<svg' && echo "verbovivo: OK"
```

---

## Invariantes a preservar durante o fechamento

- Sem `malloc`/`calloc`/`free` em `Apkc/` ou hot paths (freestanding)
- Sem includes libc em `Apkc/` (syscalls via `sys.h`)
- Nenhum gate marcado PASS sem artefato verificável
- TOKEN_VAZIO é estado legítimo; nunca converter em PASS por omissão
- Toda rodada de prova: `commit_sha + date_utc + environment + stdout_integral`

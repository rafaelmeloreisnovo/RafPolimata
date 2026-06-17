# ApkC — Ambientes-alvo e caminhos de intérprete (L10)

> **Cadeia de custódia documental — 2026-06-17**
> Fecha a lacuna L10 de `docs/LACUNAS_PROFUNDAS_MVP_PRODUTO.md`
> ("Caminhos de intérprete em APKs Android reais").
> Toda afirmação sobre estado do repositório está ancorada num arquivo lido
> e citado por caminho. Este documento não é avaliação financeira; é definição
> honesta de escopo: o que cada pipeline cobre, e onde fica `TOKEN_VAZIO`.

---

## Decisão canônica de escopo

O bootstrap `use_script` (gerado por `gen_script_code64()` em
`Apkc/lang_script.h`) embute **caminhos absolutos e fixos de intérprete**
no código nativo (`/usr/bin/python3`, `/bin/sh`, `/usr/bin/perl`,
`/usr/bin/node`, `/usr/bin/php`) — esses literais vêm do campo `compiler`
de `Apkc/lang_profile.h → _lang_table[]`.

> **Alvo primário honesto do pipeline `use_script`: Termux / proot / dev-lab.
> NÃO é o Android padrão (stock Android).**

Razão: em Android padrão (stock) esses caminhos **não existem** no
sistema de arquivos do app, e mesmo que existissem o SELinux/sandbox bloqueia
`execve` a partir do contexto de um `NativeActivity`. Portanto a execução de
script em stock Android via este bootstrap é declarada **TOKEN_VAZIO / fora de
escopo** (ver seção "Stock Android" abaixo). O alvo real e auditável do
`use_script` é o ambiente onde esses intérpretes existem nesses caminhos:
Termux (com prefixo de FS apontando para `/data/data/com.termux/...` ou
ajustado), proot, ou um device de desenvolvimento / laboratório com rootfs
Linux convencional.

Os outros dois pipelines têm alvos distintos e são honestos sobre isso:

- **`use_asm`** (linguagem `asm`/`.s`): gera código ARM nativo puro via o
  assembler interno de duas passagens — **não depende de intérprete externo**.
  É o único pipeline cujo artefato roda em stock Android sem pré-requisitos de
  intérprete. (Cobertura ISA ~65% — ver `Apkc/arch_arm64.h`; estado de prova
  runtime ainda `PASS limitado`, ver `docs/LACUNAS_PROFUNDAS_MVP_PRODUTO.md`
  L2/L4.)
- **`use_fork`** (c, cpp, rs, kt, java, jsx): `fork_exec_wait()` invoca um
  compilador externo (clang, rustc, kotlinc, javac, npx/babel) **no host de
  build**. É um pipeline de **tempo de build apenas** — `fork_exec_wait()` é
  `#ifdef __aarch64__` (ver nota ² em `docs/LACUNAS_PROFUNDAS_MVP_PRODUTO.md`
  L9). O artefato resultante (`.so`/`classes.dex`) é o que vai para o APK; o
  compilador não roda no device.

---

## Matriz: ambiente-alvo × pipeline ApkC

| Ambiente-alvo | `use_asm` (asm) | `use_script` (py/sh/pl/js/php) | `use_fork` (c/cpp/rs/kt/java/jsx) |
|---|---|---|---|
| **Stock Android (NativeActivity)** | OK em escopo (nativo puro; runtime `PASS limitado`/`TOKEN_VAZIO` por L2/L4) | **TOKEN_VAZIO** — caminhos de intérprete inexistentes + SELinux bloqueia `execve` | N/A — pipeline é só de build; artefato (.so/.dex) roda, o compilador não |
| **Termux** | OK | OK *se* o intérprete existir no caminho embutido (ver tabela de caminhos) | Build host apenas (Termux pode ser o host se tiver o toolchain) |
| **proot (rootfs Linux)** | OK | OK *se* o rootfs prover o intérprete no caminho absoluto | Build host apenas |
| **Dev-lab / device de desenvolvimento** | OK | OK (rootfs Linux convencional tem os caminhos) | OK como host de build |
| **Host de build (x86_64/CI)** | Gera bytes ARM, não executa localmente¹ | Gera bytes ARM, não executa localmente¹ | É exatamente onde `use_fork` roda o compilador externo |

¹ Em x86_64 o `apkc` não linka como binário nativo executável (sem `_start`
para host) — ver nota ¹ em `docs/LACUNAS_PROFUNDAS_MVP_PRODUTO.md` L9. No CI o
que roda são os gates de geração/validação de bytes, não a execução do APK.

Legenda: **OK** = em escopo e arquiteturalmente suportado;
**TOKEN_VAZIO** = fora de escopo / sem prova / bloqueado; **N/A** = a pergunta
não se aplica a este pipeline.

---

## Caminhos de intérprete realmente embutidos (fonte: `Apkc/lang_profile.h`)

Os valores abaixo são citados literalmente do campo `compiler` e `arg1` de
cada linha de `_lang_table[]` em `Apkc/lang_profile.h` (linhas 47–107). O
`gen_script_code64(interp, arg1, script, ...)` em `Apkc/lang_script.h` monta
`execve(interp, {interp, arg1, script, NULL}, NULL)` — ou seja, esses são
exatamente os bytes ASCII que vão para o string pool do `.text` da `.so`.

| Linguagem | `name` | `ext` | `compiler` (interp embutido) | `arg1` | Requisito em runtime |
|---|---|---|---|---|---|
| Python | `py` | `.py` | `/usr/bin/python3` | `-c` | binário Python 3 presente nesse caminho absoluto |
| Shell | `sh` | `.sh` | `/bin/sh` | `-c` | shell POSIX em `/bin/sh` |
| Perl | `pl` | `.pl` | `/usr/bin/perl` | `-e` | interpretador Perl nesse caminho |
| JavaScript | `js` | `.js` | `/usr/bin/node` | `-e` | runtime Node.js nesse caminho |
| PHP | `php` | `.php` | `/usr/bin/php` | `-r` | CLI PHP nesse caminho |

Todas as cinco linhas têm `use_script=1`, `arm64_only=1` e `use_fork=0`
(`Apkc/lang_profile.h:83-100`). O bootstrap é ARM64-only por construção
(`gen_script_code64` emite 18 instruções AArch64 — ver
`Apkc/lang_script.h:43-85`).

### Observação sobre `jsx`

A linha `jsx` (`Apkc/lang_profile.h:103-106`) **não** é `use_script` puro:
tem `use_fork=1` + `jsx_node=1`. O estágio 1 roda `npx babel` no host
(`fork_exec_wait`), e o estágio 2 embute o JS resultante via
`gen_script_code64("/usr/bin/node", "-e", ...)`. Portanto o JSX herda o mesmo
requisito de runtime do `js` (Node em `/usr/bin/node`) para o artefato final,
e o requisito de build do `use_fork` (npx/babel no host) para o estágio 1.

---

## Stock Android: por que é TOKEN_VAZIO

**Execução de script via `NativeActivity` em stock Android: TOKEN_VAZIO.**

Motivos concretos, não negociáveis:

1. **Caminhos inexistentes.** Os literais `/usr/bin/python3`, `/bin/sh`,
   `/usr/bin/perl`, `/usr/bin/node`, `/usr/bin/php`
   (`Apkc/lang_profile.h:83-100`) apontam para uma hierarquia FHS de Linux
   convencional. O Android padrão não tem `/usr/bin` populado com esses
   intérpretes; um app não traz esses binários nesses caminhos.
2. **SELinux / sandbox.** Mesmo que um intérprete existisse, o domínio
   SELinux de um app (`untrusted_app`) e a política de `execve` a partir de
   um `NativeActivity` bloqueiam a invocação direta de binários externos do
   sistema. O `execve(interp, ...)` montado em `Apkc/lang_script.h:76-81`
   simplesmente retorna falha — e o bootstrap, nesse caso, restaura a pilha e
   retorna 0 (`Apkc/lang_script.h:82-85`), sem executar o script.

Consequência de escopo: para stock Android, **apenas `use_asm`** é o caminho
honesto (código nativo puro, sem `execve` externo). Multilíngua via script é
potência conceitual cujo alvo real é Termux/proot/dev-lab — exatamente como
fixado na decisão canônica no topo deste documento.

---

## Referências de arquivo (ancoragem da cadeia de custódia)

| Afirmação | Arquivo : linha |
|---|---|
| Bootstrap `execve` ARM64 de 18 instruções | `Apkc/lang_script.h:43-85` |
| `execve(interp, {interp,arg1,script,NULL}, NULL)` | `Apkc/lang_script.h:68-81` |
| Fallback retorna 0 quando `execve` falha | `Apkc/lang_script.h:82-85` |
| Tabela declarativa de linguagens (single source of truth) | `Apkc/lang_profile.h:47-107` |
| `compiler="/usr/bin/python3"`, `arg1="-c"` (py) | `Apkc/lang_profile.h:83-84` |
| `compiler="/bin/sh"`, `arg1="-c"` (sh) | `Apkc/lang_profile.h:87-88` |
| `compiler="/usr/bin/perl"`, `arg1="-e"` (pl) | `Apkc/lang_profile.h:91-92` |
| `compiler="/usr/bin/node"`, `arg1="-e"` (js) | `Apkc/lang_profile.h:95-96` |
| `compiler="/usr/bin/php"`, `arg1="-r"` (php) | `Apkc/lang_profile.h:99-100` |
| `jsx` = `use_fork` + `jsx_node` (não script puro) | `Apkc/lang_profile.h:103-106` |
| Matriz de prova por linguagem (1/12, framework 6/12) | `docs/LACUNAS_PROFUNDAS_MVP_PRODUTO.md` L9 |
| Lacuna original deste documento | `docs/LACUNAS_PROFUNDAS_MVP_PRODUTO.md` L10 |

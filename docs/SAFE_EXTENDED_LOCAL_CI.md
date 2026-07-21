# Safe Extended v2 — executar GitHub Actions localmente no Termux

> **Entrada canônica:** docs/AGENTES.md §7 (CI gates) e §3 (ciclo de sessão — startup/shutdown). Executor local de GitHub Actions no Termux — política de segurança, estados LOCAL_CI_PASS/TOKEN_VAZIO e distinção entre execução real e action não suportada.

## Correção arquitetural

O Safe Extended não transforma a ARM em executor de CI e não cria um runtime freestanding para substituir o Termux.

A arquitetura correta é:

```text
Android/ARM
└── Termux + Bionic + shell + ferramentas nativas
    └── Safe Extended
        ├── lê o workflow
        ├── compila um plano local
        ├── aplica política de segurança
        ├── adapta ferramentas de host
        ├── executa cada etapa
        └── grava prova local
```

ARM é a arquitetura nativa do aparelho. O executor é um processo hospedado pelo Termux. O Safe Extended não usa `_start`, syscall crua, `-nostdlib` ou estado artificial na pilha para executar uma CI.

## Dependências

```sh
pkg install python bash clang make
```

Nenhuma dependência é instalada automaticamente. O runner não usa rede durante `plan` ou `run`.

## Compilar o workflow sem executar

```sh
sh safe-extended plan .github/workflows/ci.yml
```

Resultado:

```text
build/safe-extended/plans/ci.plan.json
```

O plano contém:

- SHA-256 do workflow;
- jobs e steps reconhecidos;
- scripts `run:`;
- actions suportadas;
- achados da política;
- itens ainda não executáveis localmente.

## Executar a CI principal

```sh
sh safe-extended run .github/workflows/ci.yml
```

Sem argumento, o workflow padrão é `.github/workflows/ci.yml`:

```sh
sh safe-extended run
```

## Examinar todos os workflows

```sh
sh safe-extended all
```

Esse modo compila e tenta executar cada `.yml`/`.yaml`. Uma action não suportada não é fingida: recebe estado `UNSUPPORTED` e interrompe o workflow correspondente.

## Estado e limpeza

```sh
sh safe-extended status
sh safe-extended clean
```

## Actions locais suportadas

### `actions/checkout@v4`

É convertido em `checkout_local`: o repositório já está presente no aparelho, portanto não existe clone ou acesso à rede.

### `actions/upload-artifact@v4`

É convertido em cópia local para:

```text
build/safe-extended/<run_id>/artifacts/
```

Nenhum artefato é enviado ao GitHub.

## Adaptação Termux

O Safe Extended cria shims privados dentro da execução:

```text
gcc    → clang
g++    → clang++
cc     → clang
python → python3
```

Isso adapta nomes esperados por runners Ubuntu sem falsificar a arquitetura. O compilador continua usando seu alvo nativo, que é registrado com:

```sh
clang -dumpmachine
getprop ro.product.cpu.abi
uname -m
getconf LONG_BIT
```

Nenhum `--target` ARM é inventado pelo executor. Um workflow que realmente deseja cross-compilar precisa declarar explicitamente sua toolchain e seu sysroot.

## Política de segurança

Bloqueios prévios:

- `curl`, `wget`, SSH e comandos de rede;
- `git clone/fetch/pull/push`;
- instalação de pacotes;
- `sudo`, `su` e equivalentes;
- montagem, reboot, `setprop`, fastboot e adb;
- `rm -rf /`;
- `eval` e `source` dinâmicos;
- expressões `${{ ... }}` ainda não resolvidas;
- diretório de trabalho fora do repositório.

Avisos bloqueados por padrão:

- `|| true`, porque pode transformar falha em falso PASS;
- processos em background.

Para auditoria de um workflow legado com `|| true`, existe opção explícita:

```sh
sh safe-extended run .github/workflows/ci.yml --allow-masked-failures
```

O relatório preserva o aviso. A opção não muda um código de saída diferente de zero em PASS.

## Execução

Cada `run:` é executado como:

```text
bash --noprofile --norc -euo pipefail -c <script>
```

Cada step possui timeout, log independente e código de saída real.

## Evidência

```text
build/safe-extended/<run_id>/
├── artifacts/
├── home/
├── logs/
├── shims/
├── tmp/
└── report.json
```

O relatório registra:

- hash do workflow;
- arquitetura observada;
- target nativo do Clang;
- ABI Android observada;
- cada step executado;
- política aplicada;
- código de saída;
- logs;
- resultado final.

## Estados

```text
COMPILED_PLAN  = workflow analisado e convertido em plano
POLICY_DENY    = etapa bloqueada antes de executar
UNSUPPORTED    = action ainda sem adaptador local
LOCAL_STEP_PASS = etapa realmente executada com rc=0
LOCAL_CI_PASS   = todas as etapas selecionadas executadas com rc=0
TOKEN_VAZIO     = etapa não executada ou prova ausente
```

## Limite atual

O parser implementa deliberadamente um subconjunto de GitHub Actions:

- jobs simples;
- steps;
- `name`;
- `uses`;
- `run: |`;
- `env`;
- `with`;
- `working-directory`.

Matrizes, containers, services, secrets, caches e expressões complexas não são simulados. Eles são lacunas explícitas, não comportamentos inventados.

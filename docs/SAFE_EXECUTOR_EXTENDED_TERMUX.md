# Safe Executor Extended — CI local no Termux/Android

## Objetivo

O Safe Executor Extended é o executor local e offline do RafPolimata. Ele não dispara GitHub Actions, não chama a API do GitHub e não interpreta YAML como código.

```text
fonte local
→ preflight
→ auditoria
→ compilação freestanding
→ execução ARM no próprio Android
→ compilação do RafPolimata
→ smoke test
→ relatório local
```

O lançador resolve a raiz pela própria localização:

```sh
sh safe-extended plan
```

Não depende do diretório atual, de variável `ROOT` ou de checkout remoto.

## Perfis

### Planejamento

```sh
sh safe-extended plan
```

Mostra o grafo fixo sem compilar.

### Compilar para a ABI do aparelho

```sh
sh safe-extended build
```

Executa:

```text
preflight
source_audit
omega_host_gate
omega_termux_build
```

### Compilar e executar no aparelho

```sh
sh safe-extended run
```

Além da compilação, executa o ELF ARM real e exige código de saída zero.

### CI local principal

```sh
sh safe-extended ci
```

Executa:

```text
preflight
source_audit
omega_host_gate
omega_termux_build
omega_termux_run
compiler_strict_build
compiler_smoke
```

### CI local ampliada

```sh
sh safe-extended ci-full
```

Acrescenta:

```text
runtime_truth
```

Essa etapa usa o validador local existente e requer `bash`, `make` e `python3`.

## Dependências Termux

```sh
pkg install clang make python
```

O executor não instala pacotes automaticamente porque instalação implica rede e alteração do ambiente.

## Segurança por construção

O executor possui as seguintes invariantes:

- nenhuma chamada a GitHub;
- nenhuma rede;
- nenhum `eval`;
- nenhum `sudo` ou `su`;
- execução como root bloqueada por padrão;
- nenhuma interpretação de YAML arbitrário;
- lista fixa de estágios;
- lock contra duas CIs simultâneas;
- falha fechada no primeiro gate inválido;
- `umask 077` para evidências locais;
- logs separados por etapa;
- retenção limitada das execuções mais recentes.

O contrato declarativo está em:

```text
configs/safe-executor-extended.v1.json
```

Ele documenta os perfis, mas não é executado pelo shell.

## Evidências

Cada execução cria:

```text
build/safe-extended/<run_id>/
├── artifacts/
├── logs/
├── stages.jsonl
└── report.json
```

O último resultado fica disponível em:

```sh
sh safe-extended status
```

Estado final possível:

```text
PASS = todas as etapas selecionadas foram realmente executadas
FAIL = uma etapa executou e retornou falha
TOKEN_VAZIO = etapa não executada ou prova ausente
```

Um build local não é promovido a execução de aparelho. `TERMUX_DEVICE_PASS` só nasce quando `omega_termux_run` termina com código zero no Android ARM.

## Limpeza

```sh
sh safe-extended clean
```

Remove somente os artefatos do Safe Executor Extended e seu lock.

## Relação com GitHub Actions

O Safe Executor Extended substitui o workflow hospedado deste corpo. O Git pode continuar sendo usado para versionamento, mas a CI descrita aqui pertence ao aparelho:

```text
GitHub = armazenamento e revisão
Termux = compilação e execução
RafPolimata = validação e prova
```

# Vertical Slice v1 (read-only)

Fluxo implementado:

1. Recebe `intent_ir.json`.
2. Valida os campos obrigatórios de `rafaelia.intent.v1`.
3. Aplica governance gate com allowlist por capability e default deny.
4. Compila `execution_plan.json` apenas com comandos read-only explícitos.
5. Executa localmente `git status` e `git diff --stat`.
6. Captura `stdout`, `stderr`, `exit_code`, timestamps e SHA-256.
7. Gera `execution_result.json` auditável com referências de origem.

## Restrições aplicadas

- Texto livre não é enviado ao shell.
- Capability fora da allowlist resulta em `blocked`.
- Plano compilado contém somente:
  - `git status`
  - `git diff --stat`
- `termux.command.safe` está reservado para execução sandboxed futura (v2) com comandos previamente compilados e sem texto livre.

## Lacunas atuais

- Commit pin dos demais repositórios externos: `TOKEN_VAZIO`.
- Hashes esperados inter-repositório: `TOKEN_VAZIO`.

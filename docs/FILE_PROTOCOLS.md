# Protocolos de arquivo do repositório

> **Entrada canônica:** docs/AGENTES.md §3 (ciclo de sessão — o que commitar, o que deixar como PENDING, artefatos verificáveis) e §7 (CI gates — quais artefatos cada gate valida). Este documento cobre padrões de nomenclatura de arquivo e checklist de evidência mínima por artefato.

| Padrão | Uso |
|---|---|
| `*.md` | documentação humana/auditável |
| `*.sh` | scripts executáveis com `set -eu` |
| `*.py` | validadores auxiliares |
| `*.ops` | manifesto operacional determinístico |
| `*.txt` | saída bruta de ferramenta |
| `*.json` | saída estruturada de validação |
| `*.apk` | artifact gerado, não versionar por padrão |
| `*.so` | artifact gerado, não versionar por padrão |
| `*.keystore` | chave local, nunca versionar |

## Checklist obrigatório

- [ ] Todo script precisa explicar uso com `--help` ou comentário inicial.
- [ ] Toda prova precisa registrar comando, data, commit, ferramenta e saída.
- [ ] Toda lacuna precisa usar `TOKEN_VAZIO`.
- [ ] Todo sucesso precisa ter artefato verificável.
- [ ] Toda alegação técnica precisa apontar para arquivo/prova.

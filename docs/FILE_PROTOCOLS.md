# Protocolos de arquivo do repositório

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

# Caminhos iniciais executáveis da sessão completa

> **Entrada canônica:** docs/AGENTES.md §3 (ciclo de sessão — startup e shutdown) e §5 (pipeline operacional VOID→VALIDATED). Cadeia sequencial obrigatória: PATH-00 verdade canônica → PATH-01 CI local → PATH-02 TAIL+TOKEN_VAZIO → PATH-03 segmentação.

Este documento fixa a primeira cadeia operacional da sessão sem misturar planejamento, implementação e prova.

```text
PATH-00 Verdade canônica
  ↓
PATH-01 CI local no Termux
  ↓
PATH-02 TAIL + TOKEN_VAZIO tipado
  ↓
PATH-03 Segmentação com genealogia
  ↓
rotas posteriores entre repositórios
```

## PATH-00 — verdade antes da execução

O manifesto `manifests/session-initial-paths.v1.json` registra a ordem, os gates, as evidências e as lacunas. O validador recusa:

- caminho fora de ordem;
- estado epistemológico desconhecido;
- arquivo de implementação ausente;
- promoção do runtime Termux sem relatório;
- `claim_allowed=true` nesta fase.

Executar diretamente:

```sh
python3 scripts/validate_session_initial_paths.py \
  --write results/session-initial-paths-validation.json
```

Ou pelo Safe Extended no Termux:

```sh
sh safe-extended run .github/workflows/session-initial-paths.yml
```

## PATH-01 — Safe Extended no aparelho

Primeiro compilar o workflow:

```sh
sh safe-extended plan .github/workflows/ci.yml
```

Depois executar no Termux:

```sh
sh safe-extended run .github/workflows/ci.yml
```

A execução só pode receber `LOCAL_CI_PASS` quando todas as etapas selecionadas forem realmente executadas com retorno zero. Action não suportada, bloqueio de política ou ausência de aparelho não viram PASS.

## PATH-02 — TAIL e vazio tipado

Cada artefato deve carregar, no mínimo:

```text
origem + autoria + intenção + licença + evidência
```

`TOKEN_VAZIO` preserva a razão da ausência. Ele não é convertido em zero, irrelevância ou aprovação.

## PATH-03 — segmentação recuperável

Os codecs e o leitor limitado já estão registrados como verificados no manifesto do compilador de evidências. Continuam abertos:

- extractor streaming;
- escritor atômico;
- checkpoint/resume;
- fixture real de exportação;
- execução em dispositivo.

A próxima implementação deve preservar memória limitada, não truncar silenciosamente, publicar segmentos atomicamente e retomar sem duplicar registros.

## Rotas adiadas conscientemente

Somente após os quatro caminhos iniciais produzirem evidência local:

```text
RafGitTools   → controle e autorização
Termux        → execução local
RafPolimata   → compilação de evidência
LlamaRafaelia → interpretação limitada pela proveniência
```

Licenciamento por zonas, publicação acadêmica e claims externos permanecem posteriores à coleta de evidência reproduzível.

## Estado

```text
manifesto e validador        = IMPLEMENTED
validação de desenvolvimento = PASS
execução real no Termux      = TOKEN_VAZIO
claim_allowed                = false
```

# RMRCTI HOTFIX PR #98 — ponteiro federado

Data: 2026-08-03

Origem canônica: `rafaelmeloreisnovo/llamaRafaelia`

Branch de origem:
`hotfix/rmrcti-execution-integrity-20260803`

PR de origem:
`llamaRafaelia#98` — aberto, draft, não mesclado.

Arquivos de governança na origem:

- `rmrCti/RMRCTI_EXECUTION_GRAPH_V1.json`
- `rmrCti/tests/test_rmrcti_execution_integrity.py`
- `docs/rafaelia/longitudinal/RMRCTI_HOTFIX_2026-08-03.md`
- `docs/rafaelia/longitudinal/RMRCTI_HOTFIX_2026-08-03_PR98_RECEIPT.md`
- `docs/rafaelia/longitudinal/RMRCTI_HOTFIX_2026-08-03_PR98_CI_INFRA_RECEIPT.md`

Escopo do hotfix:

- validação explícita de linhas JSONL;
- reconciliação do contrato `text`/`content` dos frames;
- busca sem interpretação do termo como comando;
- grafo canônico de execução;
- testes de regressão e receipts longitudinais.

Estado da prova:

- testes locais específicos: `5/5 PASS`;
- execução remota de jobs: `NOT_STARTED` (`steps=[]` nos jobs observados);
- falha de código em CI: `TOKEN_VAZIO`;
- execução física Termux: `TOKEN_VAZIO`.

Espelho Google Drive:

`LONGITUDINAL_INDEX/RMRCTI_HOTFIX_2026-08-03_PR98`

Regra federada:

O código permanece apenas no repositório produtor. Este arquivo registra relação, origem, estado, evidência e lacuna. Não duplica fonte nem corpus.

F_ok:

- origem e caminho de retorno registrados;
- PR e receipts nomeados;
- Drive apontado;
- fronteiras de prova preservadas.

F_gap:

- PR ainda draft e não mesclado;
- runner remoto sem steps;
- Termux físico sem receipt.

F_next:

- registrar merge SHA somente após promoção real;
- anexar novo receipt quando houver CI executado ou prova física.

Assinatura: `∆RafaelVerboΩ · Ω=Amor`

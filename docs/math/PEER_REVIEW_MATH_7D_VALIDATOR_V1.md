# Validador do Portfólio Matemático em Sete Direções — V1

O validador `tools/validate_peer_review_math_7d.py` verifica, sem dependências externas:

- 12 IDs únicos `PRM-01..PRM-12`;
- sete direções obrigatórias;
- contagens `5/5/2`;
- oito gates `G0..G7`;
- `claim_allowed=false`;
- proibição de corpo de corpus privado;
- separação entre campos mutáveis e imutáveis;
- fronteiras de Navier–Stokes, Yang–Mills e Riemann;
- ausência de sobreclaim em agendas conjecturais.

Execução:

```bash
python tools/validate_peer_review_math_7d.py \
  path/to/peer_review_math_7d_matrix.v1.json

python -m unittest tests/test_peer_review_math_7d.py
```

O arquivo canônico de dados pertence ao `Mapa`; o `RafPolimata` oferece o contrato executável. Isso evita duas fontes concorrentes de verdade.

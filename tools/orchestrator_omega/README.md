# Orquestrador Ω de Excelência — executor V1

Este diretório materializa a parte executável do contrato federado definido no `Mapa`.

## Executar

```bash
cd tools/orchestrator_omega
python validate_orquestrador_omega.py orquestrador-omega-excellence-v1.json
python test_orquestrador_omega.py
```

Saída esperada:

```text
PASS needs=12 vectors=12 generated_cells=144 claim_allowed=false
Ran 8 tests
OK
```

## O que o gate prova

- existem exatamente 12 necessidades e 12 vetores únicos;
- o produto cartesiano gera 144 células endereçáveis;
- o estado inicial é `TOKEN_VAZIO_UNASSESSED`;
- a autoridade permanece humana;
- `claim_allowed=false`;
- excelência é não compensatória.

## O que o gate não prova

- validade científica ou jurídica por domínio;
- execução remota ou portabilidade entre arquiteturas;
- desempenho, segurança integral ou eficácia operacional;
- revisão independente;
- autorização para merge, publicação ou claim.

## R₃

- `F_ok`: estrutura e testes adversariais locais.
- `F_gap`: CI do branch, evidência de domínio e revisão independente.
- `F_next`: gerar receipt do checkout real e vincular ao evento longitudinal no Drive.

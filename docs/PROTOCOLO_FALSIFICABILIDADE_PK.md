# Protocolo mínimo de falsificabilidade em P(k)

Este documento operacionaliza o primeiro teste "prova ou queda" com um único observable: espectro de potência P(k).

## Objetivo

Comparar **dado observado vs modelo calibrado e congelado** sem ajuste fino durante a execução do gate.

## Regra de congelamento

Antes de executar, congelar:
1. fórmula do modelo,
2. parâmetros do modelo,
3. faixa de k,
4. critérios de aprovação/reprovação.

## Parâmetros calibrados atuais

Os defaults versionados do script foram calibrados offline sobre `data/pk_observado.csv` e ficam congelados para detectar regressões: `a=81724.04434517458`, `n=1.295421709172559`, `kd=0.06846530735536861`. Qualquer novo dataset deve registrar novos parâmetros, critérios e relatório antes de promover o gate.

## Script

- Script: `scripts/first_test_pk.py`
- Entrada padrão: `data/pk_observado.csv`
- Saída padrão: `results/first_test_report.json`

## Formato do CSV

Colunas obrigatórias:
- `k`
- `Pk`
- `sigma`

## Métricas

1. rRMSE normalizado por sigma;
2. cobertura 1σ (fração com |resíduo| ≤ 1);
3. inclinação dos resíduos normalizados vs k.

## Critério padrão

- PASS se:
  - `rrmse <= 1.5`
  - `coverage_1sigma >= 0.60`
  - `|residual_slope| <= 5.0`
- Caso contrário: FAIL.

## Execução

```bash
python3 scripts/first_test_pk.py
```

Para usar dados reais (DESI/Planck/DES), substitua `data/pk_observado.csv` por dataset observacional validado e mantenha os critérios congelados.

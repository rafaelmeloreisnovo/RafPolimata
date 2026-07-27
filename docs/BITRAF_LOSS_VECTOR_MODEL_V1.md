# BITRAF — Contrato de Execução do Índice de Perda V1

Status: `IMPLEMENTED_ON_FIXTURES`  
Causalidade física: `TOKEN_VAZIO`  
Claim: `claim_allowed=false`

## 1. Papel do RafPolimata

O Mapa mantém a ontologia e o ledger. O RafPolimata executa a camada externa, determinística e auditável:

```text
JSONL de observações
→ validação semântica
→ classificação bit/erasure/vazio
→ vetor explícito de 32 features
→ índice SHA3-256
→ consulta por similaridade
→ auditoria de taxas
```

O índice não lê pesos ocultos, não acessa estado físico de transistores e não substitui ECC/FEC.

## 2. Estados preservados

```yaml
MATCH
FLIP_0_TO_1
FLIP_1_TO_0
ERASURE
TOKEN_VAZIO_EXPECTED
TOKEN_VAZIO_OBSERVED
```

Regras:

- `0` é dado conhecido;
- `null + known_erasure=true` é erasure localizado;
- `null + known_erasure=false` é observação ausente;
- `expected=null` impede medir flip;
- causa térmica não é inferida pela classe.

## 3. Vetor externo

As features incluem:

1. coordenadas `x,y,z,t`;
2. índice linear;
3. índice Fibonacci mais próximo;
4. projeção hexagonal de 60° com raio \((\sqrt3/2)^f\);
5. projeção octogonal de 45°;
6. valor e presença esperados;
7. valor e presença observados;
8. erasure conhecido;
9. temperatura e presença;
10. tensão e presença;
11. latência e presença;
12. shard e stripe;
13. densidade e presença da síndrome;
14. número de memberships de paridade;
15. one-hot da classe.

Total: 32 dimensões.

## 4. Contrato de consulta

A consulta retorna:

```yaml
recovery_status: HEURISTIC_ONLY_NOT_ECC
heuristic_candidate_expected_bit: 0 | 1 | null
heuristic_confidence: 0..1
claim_allowed: false
```

O candidato é somente voto ponderado dos vizinhos que possuem referência esperada conhecida. Recuperação exata exige síndrome, FEC, hash ou referência íntegra externa.

## 5. Determinismo e custódia

- registros ordenados por `observation_id`;
- representação JSON canônica;
- `record_sha3_256` para cada observação normalizada;
- `index_sha3_256` para o conjunto ordenado;
- ausência de dependências externas;
- execução adequada a Termux e CI.

## 6. Uso

```bash
python3 scripts/bitraf_loss_vector_index.py build \
  tests/fixtures/bitraf_loss_observations.v1.jsonl \
  --out build/bitraf-loss-index.json

python3 scripts/bitraf_loss_vector_index.py audit \
  build/bitraf-loss-index.json \
  --out build/bitraf-loss-audit.json

python3 scripts/bitraf_loss_vector_index.py query \
  build/bitraf-loss-index.json \
  tests/fixtures/bitraf_loss_query.v1.json \
  --top-k 3 \
  --out build/bitraf-loss-query.json

python3 -m unittest -v tests/test_bitraf_loss_vector_index.py
```

## 7. Testes do primeiro corte

```text
test_classification_distinguishes_zero_erasure_and_void
test_index_is_deterministic
test_audit_rates
test_query_is_explicitly_heuristic
```

## 8. Limites

```yaml
F_ok:
  - taxonomia executável
  - índice determinístico
  - geometria explícita
  - busca heurística rotulada
  - testes sintéticos
F_gap:
  - captura de hardware real
  - endereço físico
  - ECC syndrome Android
  - temperatura e tensão por domínio
  - calibração das features
  - baseline e validação fora da amostra
F_next:
  - ingerir captura real read-only
  - comparar vetor completo contra baseline sem Fibonacci/hexágono
  - medir falsa recuperação
```

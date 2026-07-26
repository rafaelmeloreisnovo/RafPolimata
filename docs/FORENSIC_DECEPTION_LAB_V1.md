# Forensic Deception Lab V1

Estado: `IMPLEMENTED` estrutural + `VERIFIED_LIMITED_LOCAL` em sandbox isolado.  
Claim boundary: `claim_allowed=false`.

## Objetivo

Validar o encadeamento mínimo:

```text
dados canônicos sintéticos
→ projeção por estação
→ relações coordenadas
→ fingerprint keyed e redundante
→ decoys sintéticos
→ manifesto autenticado
→ perda/reordenação
→ recuperação e ranking
```

O laboratório não usa dados reais, InterBase, rede, Android, credenciais, callbacks ou resposta ofensiva.

## Arquivos

```text
tools/forensic_deception_lab.py
tests/test_forensic_deception_lab.py
```

## Execução

```bash
python3 -m py_compile \
  tools/forensic_deception_lab.py \
  tests/test_forensic_deception_lab.py

python3 -m unittest -v tests/test_forensic_deception_lab.py
python3 tools/forensic_deception_lab.py --output build/forensic-deception/report.json
```

## Gates unitários

1. total empresarial dos registros canônicos é preservado;
2. IDs projetados são únicos;
3. relações pai/filho projetadas resolvem;
4. alteração no manifesto invalida o HMAC;
5. contexto correto supera candidato incorreto após perda limitada;
6. tokens de decoy são observáveis.

## Recibo local do conteúdo final

O conteúdo final, incluindo fingerprint derivado por HMAC e faixas determinísticas separadas para IDs de decoy, foi executado novamente em sandbox isolado:

```text
Python: 3.13.5
Plataforma: Linux 6.12.13 x86_64, glibc 2.41
py_compile exit code: 0
unittest exit code: 0
Testes: 5
Resultado: 5 PASS / 0 FAIL
Tempo reportado pelo unittest: 0.003 s
demo exit code: 0
```

SHA-256 do conteúdo executado:

```text
tools/forensic_deception_lab.py
7dc03d68f052f29eecdc655c8fd3e6dfdb7d462c53b1ae00c14a4531703a6d88

tests/test_forensic_deception_lab.py
13108b5357ed9824a2bffeba0b16d77dd032a092700d6658d1b87d82c66b6bd1
```

O ambiente Python emitiu no `stderr` um aviso externo de warmup do `artifact_tool` sem relação com o laboratório; o processo de testes retornou código 0 e todos os cinco casos passaram. O gate remoto deve recalcular os hashes a partir do checkout e preservar stdout/stderr integralmente. Commit não substitui run.

## Resultado observado no demo final

```text
manifest_valid = true
business_invariant.preserved = true
canonical_total_cents = 924960
projected_total_cents = 924960
fingerprint_recovery_confidence = 1.0
candidate_top1.station_id = ESTACAO-07
candidate_top1.score = 1.0
decoy_tokens_observed = 4
```

Esse resultado é uma amostra determinística do demo, não distribuição estatística.

## Limites

- HMAC de laboratório não é assinatura digital;
- projeção do conjunto observado não é FF1/FPE;
- código de repetição não é ECC de produção;
- confiança de recuperação não é probabilidade jurídica;
- não há baseline de falso positivo;
- não há ataques de correlação ou conluio;
- não há benchmark p50/p95/p99;
- não há teste de concorrência;
- não há integração com banco real;
- não há proteção de memória.

## Próximos experimentos

```text
E1 matriz de 1.000 estações sintéticas
E2 subset 5/10/25/50/75%
E3 strip de portadores por classe
E4 correlação entre atributos e portadores
E5 conluio de duas e três cópias
E6 rotação e rollback de época
E7 medição p50/p95/p99
E8 backend InterBase isolado
```

## Falsificadores

O laboratório deve falhar se:

- houver colisão de projeção;
- uma relação ficar órfã;
- um decoy alterar o total canônico;
- um manifesto adulterado for aceito;
- candidato incorreto superar consistentemente o correto;
- o mesmo codeword identificar contextos distintos acima do limite;
- a recuperação depender de ordem física não preservada.

---

`F_ok`: pipeline mínimo executável e testável.  
`F_gap`: robustez adversarial e integração real ausentes.  
`F_next`: executar a matriz multiestação em CI/local e gerar evidence run JSON.

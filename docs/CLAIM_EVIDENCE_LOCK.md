# Claim Evidence Lock

Estado: `REFERENCE` / `AUDIT`

Regra máxima: claim forte exige evidência forte. Hipótese, metáfora, analogia, prova parcial e runtime real são estados diferentes. `TOKEN_VAZIO` é melhor do que sucesso inventado.

| Nível | Significado | Evidência mínima | Gate | Lacuna honesta |
|---|---|---|---|---|
| `CLAIM_DOC` | Apenas documentação ou hipótese | Arquivo e escopo | revisão textual | `TOKEN_VAZIO` para execução |
| `CLAIM_AUDIT` | Há relatório/registro | log, manifesto, hash ou matriz | script de auditoria | `PASS_LIMITED` se parcial |
| `CLAIM_COMPILE` | Compila | comando + flags + saída | compilador retorna 0 | runtime `TOKEN_VAZIO` |
| `CLAIM_RUNTIME` | Executa | comando + entrada + saída + exit code | teste reprodutível | device externo `TOKEN_VAZIO` |
| `CLAIM_DEVICE` | Executa em hardware/device real | device-info + install/run/log | gate manual/device | `DEVICE_REQUIRED` |
| `CLAIM_EXTERNAL` | Reproduzido fora do ambiente local | ambiente externo documentado | relatório assinado/hash | `AUDIT` |
| `CLAIM_REVIEWED` | Revisão qualificada | parecer/review rastreável | aprovação explícita | `PENDING` |

## Políticas por domínio

- Android runtime: exige build, APK, assinatura/verificação quando aplicável, install, launch e logcat. Sem isso: `TOKEN_VAZIO` ou `DEVICE_REQUIRED`.
- Benchmark: exige comando, hardware, flags, baseline, mediana, p95, p99 e raw log. Sem baseline: `PASS_LIMITED` ou `TOKEN_VAZIO`.
- Segurança criptográfica: exige modelo de ameaça e revisão formal. Sem revisão: não declarar segurança forte.
- Ciência/dados: exige dataset real, hash, método, baseline e possibilidade de `FAIL`.
- Compilador: exige entrada, transformação/IR/lowering quando aplicável, output e teste.
- Multi-linguagem: exige build/run/status por linguagem.
- Metáfora/parábola: deve ser traduzida para variável, protocolo, arquivo, teste, limite, evidência ou `TOKEN_VAZIO`.

## Rollback

Se um claim exceder a evidência, rebaixar o nível do claim e preservar o log original. Nunca apagar lacuna para parecer completo.

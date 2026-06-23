# Claim Evidence Lock

Política: claim forte exige evidência forte. A ausência honesta de ferramenta, device, dataset, hardware, raw log, revisão ou execução deve ser registrada como `TOKEN_VAZIO`, `SKIPPED`, `PENDING`, `PASS_LIMITED` ou `DEVICE_REQUIRED`, nunca como PASS inventado.

## Regras de trava

- Claim de Android runtime exige: build, APK, assinatura, install, launch e logcat sem crash.
- Claim de benchmark exige: comando, hardware, flags, baseline, mediana, p95, p99 e raw log.
- Claim de segurança exige: ameaça formal, escopo, revisão e possibilidade de FAIL; não declarar segurança criptográfica sem revisão formal.
- Claim científico exige: dataset real, hash, método, baseline e possibilidade explícita de FAIL.
- Claim de compilador exige: entrada, IR/lowering quando aplicável, output e teste.
- Claim multi-linguagem exige: build/run/status por linguagem e por ambiente.
- Hipótese deve ficar marcada como hipótese.
- Analogia/metáfora/parábola deve ficar marcada como analogia didática, não prova literal.
- Ausência deve ficar marcada como `TOKEN_VAZIO`.

## Níveis de claim

| Nível | Significado | Evidência mínima | Limite |
|---|---|---|---|
| `CLAIM_DOC` | Apenas documentação | Arquivo e seção rastreáveis | Não prova execução |
| `CLAIM_AUDIT` | Há relatório/registro | Relatório, data, comando ou checklist | Pode ser parcial |
| `CLAIM_COMPILE` | Compila | Comando, flags, saída e ambiente | Não prova runtime |
| `CLAIM_RUNTIME` | Executa | Comando de execução, retorno, stdout/stderr/log | Não prova hardware específico |
| `CLAIM_DEVICE` | Executa em hardware/device real | Device info, install/launch/logs, artifacts | Escopo limitado ao device |
| `CLAIM_EXTERNAL` | Reproduzido externamente | Registro independente e versão | Depende da qualidade externa |
| `CLAIM_REVIEWED` | Revisado por terceiro qualificado | Identidade/qualificação, escopo e parecer | Não substitui nova execução |

## Gate de rebaixamento

Se qualquer evidência mínima faltar, o claim deve ser rebaixado para o maior nível comprovável. Exemplo: APK gerado e assinado sem logcat é `CLAIM_AUDIT`/`PASS_LIMITED`, não `CLAIM_RUNTIME`.

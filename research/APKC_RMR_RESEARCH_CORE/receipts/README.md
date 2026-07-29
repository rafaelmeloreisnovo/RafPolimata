# Coupled Build Receipts

**Estado:** `EVIDENCE`  
**Proprietário lógico:** `evidence-custodian`  
**Âncora no mapa:** [`docs/MAPA_ESTRUTURAL_REPOSITORIO.md §4 · research`](../../../docs/MAPA_ESTRUTURAL_REPOSITORIO.md)  
**Documento pai:** [`research/APKC_RMR_RESEARCH_CORE/README.md`](../README.md)

## Propósito

Este diretório armazena os recibos gerados após verificação e compilação acoplada (`make verify && make coupled-build`). Um recibo comprometido no repositório deve registrar obrigatoriamente:

| Campo | Descrição |
|---|---|
| `coupling_id` | ID do nucleus (ex: `APKC-RMR-RESEARCH-CORE-V1-20260726`) |
| `artifact_root` | SHA-256 raiz do artefato selado |
| `compiler` | versão do compilador e flags exatos |
| `flags` | conjunto completo de flags de compilação |
| `object_identities` | SHA-256 por objeto compilado |
| `undefined_symbol_audit` | lista de símbolos indefinidos (deve ser vazia) |
| `execution_environment` | OS, arch, data UTC, commit, branch |

## Regra de geração

```text
verify → generate header → compile → generate receipt
```

Nenhuma saída gerada é automaticamente confiável ou comprometida sem verificação. Recibos existem somente quando o pipeline completo passou.

## Estado atual

`TOKEN_VAZIO` — nenhum recibo comprometido nesta geração. O pipeline `make verify && make coupled-build` precisa ser executado em ambiente com toolchain compatível e o recibo resultante comprometido manualmente após inspeção.

## Como avançar

1. Executar `make verify` — valida markers, SHA-256 e blob identity de cada arquivo
2. Executar `make test` — roda testes do nucleus
3. Executar `make coupled-build` — gera header + compila
4. Capturar saída em arquivo JSON seguindo o schema acima
5. Inspecionar manualmente e commitar somente recibos verificados

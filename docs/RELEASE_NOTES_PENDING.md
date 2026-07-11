# Release Notes (Pending Only)

> **Entrada canônica:** `docs/AGENTES.md` §3 (estados canônicos — PENDING, TOKEN_VAZIO) e §5 (pipeline operacional VOID → VALIDATED).

> Escopo: este arquivo registra apenas itens pendentes, aguardando validação e/ou implementação.

## [RESOLVED] Estrutura inicial de governança IA↔Código↔Documentação

### Data
- Proposta: 2026-05-02
- Resolução: 2026-06-20

### Resumo
- Formalização do fluxo onde IA e humanos validam pedido antes de executar: **IMPLEMENTADO**.
- Princípio de entrega acoplada (código e documentação no mesmo ciclo): **ATIVO**.

### Evidências de execução completa

- **Rastreabilidade automática**: `scripts/validate_claims.sh` (B7) — varre `docs/*.md`
  buscando claims fortes sem `(ref: arquivo:linha)`; 0 FAIL verificado em PR #96.
- **Cadeia de custódia**: `Apkc/proofs/CHAIN_OF_CUSTODY_2026-06-20.md` gerado
  automaticamente por `tools/rafbbs/rafbbs.sh proof_chain` (B6).
- **Auditoria reproduzível**: `docs/CONVERGENCIA_UNICA_CHECKLIST.yml` — 7 blocos
  B1-B7 com status, evidência, arquivo:linha e promotion_gate verificado.
- **Aprovação humana**: PRs #88, #95, #96, #97 mergeados pelo proprietário
  `rafaelmeloreisnovo` após revisão.

### Itens resolvidos

- [x] Template de validação prévia → `docs/MULTI_AI_METHODOLOGY.md` + `CLAUDE.md`.
- [x] Taxonomia de ambiguidade → `CANONICAL_STATES` em `CLAUDE.md`
  (VOID / PENDING / AUDIT / RUNTIME / REFERENCE / TOKEN_VAZIO).
- [x] Rastreabilidade automática → `scripts/validate_claims.sh` (B7).
- [x] Política de bloqueio por baixa confiança → TOKEN_VAZIO protocol
  (claims sem origem permanecem TOKEN_VAZIO, nunca promovidos a PASS sem evidência).

### Riscos residuais monitorados

- Divergência de interpretação em contexto multilíngue → mitigado por B7 claim traceability.
- Evolução de código sem sincronização documental → mitigado por proof_chain pipeline (B6).
- Sobrecarga de governança → mitigado por scripts automáticos (validate_claims.sh, rafbbs.sh).

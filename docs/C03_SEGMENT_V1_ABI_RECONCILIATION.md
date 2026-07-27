# C03 — Segment V1 ABI Reconciliation

**Stack base:** `c02/runtime-truth-receipt-2026-07-26@55b057e22633c4e43814b40d6684315d1911ff50`  
**State:** `CONTRACT_RECONCILED / EXECUTION_PENDING`  
**Claim:** `claim_allowed=false`

## Problema encontrado

O documento `TASK_02_CONVERSATION_SEGMENTS_V1.md` e o codec já implementado usam o mesmo nome `segment.v1`, mas não descrevem o mesmo formato binário.

### Codec congelado e testado

- header: 64 bytes;
- conversation record: 96 bytes;
- message record: 128 bytes;
- identidade: `id_hi + id_lo`, 128 bits opacos;
- roles: `UNKNOWN=0, USER=1, ASSISTANT=2, SYSTEM=3, TOOL=4`;
- conversation `message_count`: 32 bits;
- spans: `source_offset + source_length`;
- message record contém índices, spans de author/content e CRC32C.

### Proposta TASK_02

- identidade e parent identity: 32 bytes cada;
- roles: `SYSTEM=1, USER=2, ASSISTANT=3, TOOL=4, DEVELOPER=5, OTHER=6`;
- conversation `message_count`: 64 bits;
- spans: `source_start + source_end`;
- message record contém `parent_id_hash` e `content_type`.

Como os tamanhos externos também são 96 e 128 bytes, um writer baseado apenas no tamanho poderia gerar um arquivo aceito superficialmente, mas com campos interpretados incorretamente.

## Decisão canônica

1. O codec existente permanece o verdadeiro `segment.v1`.
2. O wire layout v1 é imutável.
3. Roles v1 não podem ser renumeradas.
4. A proposta de IDs completos de 32 bytes passa a ser candidata a `segment.v2`.
5. Um writer v1 deve declarar o modo de identidade e o SHA-256 do contrato ABI.
6. Os campos v1 não podem ser rotulados como BLAKE3 completo de 256 bits.

## Identidade v1

Estado atual:

```yaml
serialized_width_bits: 128
semantics: OPAQUE_CALLER_SUPPLIED_128
blake3_integration: TOKEN_VAZIO_CRYPTO_GATE
```

Depois do gate criptográfico, um modo possível é:

```yaml
identity_mode: BLAKE3_128_TRUNCATED_WITH_FULL_256_IN_MANIFEST_OR_EXTENSION
```

Esse modo precisa ser nomeado exatamente, testado com vetores oficiais e acompanhado do hash completo no manifest ou em extensão versionada. Truncar sem declarar é proibido.

## Contrato legível por máquina

`runtime/conversation_indexer/SEGMENT_V1_ABI_CONTRACT.json` registra:

- magic e versão;
- tamanhos;
- offsets e larguras de todos os campos;
- enumeração de roles;
- semântica de identidade;
- decisões de compatibilidade;
- conflitos conhecidos com a proposta.

## Validador

```sh
python3 scripts/validate_segment_v1_abi_contract.py \
  --repo . \
  --output artifacts/segment-v1-abi/report.json
```

O validador cruza contrato, header, codec e proposta. Ele falha quando:

- macro de tamanho ou versão diverge;
- role é renumerada;
- offset/largura do encoder diverge;
- magic muda;
- a decisão deixa de preservar o v1;
- a proposta de 32 bytes não está explicitamente reclassificada.

## Testes

`tests/test_segment_v1_abi_contract.py` cobre:

- ABI atual reconciliada;
- renumeração de USER detectada;
- deslocamento de `title_offset` detectado.

## Gate do extractor

Após o report passar, o C03 ainda não está fechado. Permanecem:

```yaml
streaming_extractor: TOKEN_VAZIO_NOT_IMPLEMENTED
atomic_writer: TOKEN_VAZIO_NOT_IMPLEMENTED
checkpoint_resume: TOKEN_VAZIO_NOT_IMPLEMENTED
blake3_crypto_gate: TOKEN_VAZIO_NOT_IMPLEMENTED
real_conversations_export: TOKEN_VAZIO_NOT_OBSERVED
```

O próximo subciclo deve implementar o event stream e o extractor sem alterar o wire layout v1. A escrita persistente de identidade permanece bloqueada até existir um modo de identidade testado.

## Fronteira epistemológica

`PASS_WITH_RECONCILED_SPEC_CONFLICTS` prova que a contradição foi identificada, versionada e protegida por teste. Não prova extractor, writer, checkpoint, BLAKE3 ou ingestão real.
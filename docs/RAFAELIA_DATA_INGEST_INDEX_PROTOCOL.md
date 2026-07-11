# RAFAELIA Data Ingest and Index Protocol

## Contract

Freestanding-oriented ingestion for ZIP/ZIP64, TGZ, JSON/JSONL, ChatGPT exports, Git objects and media headers. No full-file loading, no hidden mutation and no content fusion without provenance.

## Identity

```text
content_id   = BLAKE3(raw bytes)
source_id    = BLAKE3(provider || account_scope || root_ref || acquisition_time)
object_id    = BLAKE3(source_id || logical_path || content_id)
relation_id  = BLAKE3(subject || predicate || object || evidence)
```

## Core records

- Object: hash, size, MIME, path, source, observed time, state.
- Conversation: source object, claimed times, first/last message, structural hash.
- Message: parent, role, content hash, ordinal and path hash.
- TemporalEvidence: timestamp, clock domain, origin, confidence and contradiction group.
- Relation: subject, predicate, object, evidence and Q16 weight.

## Deduplication

1. exact raw hash;
2. normalized conversation hash;
3. message-tree path hash;
4. semantic similarity only as a review candidate.

Overlapping exports remain separate source appearances linked to the same content identity.

## Time discrepancy

```text
T = {claimed, container, drive_created, drive_modified,
     git, acquired, ingested, structural}
```

A temporal conflict is recorded, not overwritten.

## Safety limits

- maximum nesting depth;
- maximum expanded bytes;
- compression-ratio budget;
- per-record and per-string limits;
- bounded recursion/state machines;
- checkpoint and resume journal;
- quarantine on malformed/truncated data.

## Output segments

`source.manifest.json`, `objects.segment`, `conversations.segment`, `messages.segment`, `relations.segment`, `timeline.segment`, `audit.jsonl`.

## Initial gate

Process one 150–300 MB ChatGPT export before multi-gigabyte BackReal archives. First emit central-directory inventory and manifest; only then stream `conversations.json`.

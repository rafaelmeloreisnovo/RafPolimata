# RAFAELIA Ω — Contrato de caminhos tipados e redes estruturais V1

**Tipo:** SPEC / AUDIT · **Autoridade local:** RafPolimata · **Implantação:** proposta via PR de revisão · **claim_allowed:** false.

## Contexto e motivação

O objeto de conhecimento NÃO está limitado ao conteúdo de um arquivo. Diretórios, subdiretórios, nomes originais e normalizados, número de bytes, presença de um objeto vazio, árvores de ZIP/TAR, relações de manifesto, fragmentos de hashes, links temporais e referências a outros sistemas podem transportar informação de localização e custódia. Uma consulta pode ser resolvida por relações entre **múltiplos descendentes em vários níveis**.

Esta especificação complementa `docs/RAFAELIA_DATA_INGEST_INDEX_PROTOCOL.md` (ingestão) e não modifica os formatos persistidos de `Apkc/`, `freestanding/`, `runtime/`, nem a autoridade do Mapa. Contrato machine-readable: [`../contracts/rollmap-typed-path-v1.json`](../contracts/rollmap-typed-path-v1.json).

## Identidade com rótulo de algoritmo

```text
APPEARANCE_ID = {provider_scope, provider_object_id, provider_revision}
BYTE_ID       = {algorithm, computed_digest, exact_bytes_scope}
DIR_SNAPSHOT  = {algorithm, canonical_listing_schema, scoped_sorted_entries, digest}
PATH_ID       = {segment_codec_version, ordered_raw_segments, occurrence_id}
HINT          = {source_node, match_kind, matched_span, target_candidate, evidence_ref}
```

- `BYTE_ID` pode ser BLAKE3 como no protocolo anterior, ou SHA-256 onde a implementação realmente calcular SHA-256. Nunca declarar equivalência entre algoritmos.
- `DIR_SNAPSHOT` é o checksum **da listagem capturada e versionada**; não é um hash autenticado do diretório Google Drive, nem uma prova que o provedor listou tudo.
- Nomes repetidos, caminhos com mesma normalização Unicode, e mesmo conteúdo em fontes diferentes preservam aparições independentes. Cada aresta deve declarar origem e evidência.
- `size=0`, `size=TOKEN_VAZIO`, `file_missing`, `directory`, `empty_string`, `false`, `null` são condições semanticamente distintas.

## Consulta por composição e relações

```text
MATCH HAS_DESC(name="manifest") AND HAS_DESC(name="receipt")
MATCH name contains "ψ" OR path contains "Ω"
MATCH child(namefrag="Δ") AND descendant(size=0)
MATCH linked(type=HASH_FRAGMENT_CANDIDATE) WITH NO CLAIM PROMOTION
```

Operadores são avaliados sobre conjuntos de nós, não apenas dentro de um filename. O nó ancestral pode combinar filhos distintos. Proximidade, coocorrência, permutação de tokens em subárvores, símbolos Unicode e fragmentos hexadecimais geram `CANDIDATE` até verificação explícita.

**Fragmentação de hash:** uma implementação local escopada aceita segmentos de diretório `H00=abcd/H04=1234` com offset explícito em dígitos hexadecimais, extensão de 4 a 16 dígitos por parte, duas ou mais partes e cobertura total mínima de oito posições. A ordem física pode variar; o offset governa a posição. Esse cruzamento emite apenas `HASH_PIECES_PATH_CANDIDATE`, nunca igualdade provada. Reconstrução criptográfica completa, algoritmos diferentes, permutações sem offsets e integração distribuída ainda exigem `algorithm`, `scope`, `source_ref`, bytes validados, falsificadores e recibos; permanecem `TOKEN_VAZIO`.

## Unicode, símbolos e limites

Preservar nomes originais por byte com codificação declarada/escape reversível. A chave NFC/casefold é só um índice de pesquisa: duas strings que colidem após normalização NÃO podem ser mescladas. Separadores, NUL, comprimento máximo, pontos reservados, caminhos absolutos e `../` devem respeitar a superfície (Git, POSIX, Android, Drive). Diretórios dentro de arquivos compactados são **nós virtuais**, nunca instruções para extrair arquivos sem gate.

Nomes, hashes e metadados podem revelar informações sensíveis; a indexação pública precisa de classificação/ACL e minimização prévia. Nenhum corpus privado do Google Drive deve ser copiado para este repositório público sem autorização e revisão.

## Fronteiras de produtor/consumidor

| Componente | Responsabilidade |
|---|---|
| **RafPolimata** | Contrato de campos, tipos, codificações, invariantes e falsificadores |
| **RafGitTools** | Motor Python hospedado de índice local e consulta booleana; validações/readback |
| **Mapa** | Autoridade de rotas e edges federadas, quando o handoff for aprovado |
| **Drive** | Fonte de custódia com ACL, revisão, IDs e cursores da API; não POSIX nativo |
| **Supabase** | Eventos e leases quando schema/ACL/autoridade permitirem; não pressuposto |

Implementação candidata RafGitTools: `tools/rollmap/rafaelia_rollmap_v2.py`; contrato complementar: `docs/contracts/ROLLMAP_COMPLEX_CUSTODY_V1.md`. Não duplicar todo o índice nem os bytes de corpus em ambos os repos.

## Gate de STOPWALK e continuidade

```text
SOURCE@revision -> authority/ACL -> contract schema -> baseline predecessor
-> bounded discovery -> typed candidates -> falsifier -> scoped evidence
-> proposal -> approval + valid lease -> mutation -> readback -> receipt
-> successor checkpoint (append-only)
```

Se `authorization`, `privacy_class`, `content_digest`, `lease`, `rollback_ref` ou `readback` for essencial e estiver ausente: `STOPWALK_BLOCKED / TOKEN_VAZIO(type)`. Indexação de metadados não concede permissão de escrita. Backward navigation não implica que ações externas possam ser desfeitas sem compensação.

## Aceitação

1. Parsear este contrato JSON e rejeitar categorias/eixos desconhecidos sem inventar significado.
2. Em fixture sintética multinível, satisfazer `HAS_DESC(A) AND HAS_DESC(B)` com A/B em filhos diferentes.
3. Distinguir zero bytes de ausente e `UNKNOWN`.
4. Preservar UTF-8, bytes originais e ocorrências duplicadas.
5. Tratar `HASH_FRAGMENT_CANDIDATE` como pista, nunca prova, mesmo quando transitiva.
6. Rejeitar paths inseguros em arquivos compactados.
7. Comprovar o handoff somente com commit/artefato/receipt dos dois produtores no mesmo contrato.

**R3:** F_ok=especificação tipada para integração revisável; F_gap=codec de fragmentos multi-nível, Drive pagination, CI cross-repo e hardware não executados; F_next=verificar a HEAD exata do executor e este contrato antes da promoção.

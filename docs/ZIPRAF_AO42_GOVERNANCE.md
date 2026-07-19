# Governança federada do ZIPRAF-AO42

> **Entrada canônica:** docs/AGENTES.md §6 (escalação e conflito — federação de repositórios, gate de promoção e privacidade) e §5 (pipeline operacional — append-only gate, manifest chain, rollback pointer). Este documento define a governança federada do ZIPRAF-AO42: autoridade por camada, invariante de privacidade e gates de cache e append-only.

A governança não replica corpos privados. Ela compila claims, autoridades, ponteiros, gates e rollback.

## Autoridade por camada

```text
Matem-tica-       → definições, teoremas e verificador finito
papers            → síntese acadêmica e claims
ZIPRAF_OMEGA_FULL → writer/leitor append-only
ZIPRAF_CORE       → ABI binária e funções puras
ChipQuantum       → cache matricial/hot path
GAIA_phi          → recibos e custódia por digest
RafPolimata       → gate de promoção e privacidade
```

## Invariante de privacidade

\[
\text{corpo privado fora da autoridade}=\text{PROIBIDO}.
\]

Somente estes elementos podem atravessar repositórios:

```text
repository, commit, path, artifact_id, generation, size_bytes, sha256
```

## Gate do cache

Um cache não é promovido por semelhança de nome ou tamanho. A identidade necessária é:

\[
K=(op,H(input),H(params),H(result),H(code),ABI,version).
\]

`CRC32C` pode servir ao hot path e detecção acidental, mas não substitui SHA-256 na custódia.

## Gate append-only

```text
prefix_preserved
AND latest_zip_directory_readable
AND manifest_chain_verified
AND rollback_pointer_present
```

Falha em qualquer termo mantém o estado `BLOCKED` ou `TOKEN_VAZIO`.

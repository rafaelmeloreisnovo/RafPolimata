# Source SBOM V1 — inventário determinístico e fronteira de licença

**Estado:** `IMPLEMENTED_LOCAL / RELEASE_BINDING_TOKEN_VAZIO`  
**Governança:** `CLOSURE_L11`  
**Gerador:** `scripts/generate_source_sbom_v1.py`  
**Formato:** CycloneDX JSON 1.7

## Escopo

Este mecanismo responde a uma parte do gap `GAP-SEC-SBOM-PROVENANCE`:

```text
tracked source inventory
+ SHA-256 por arquivo
+ Git blob identity
+ commit identity
+ license evidence signals
+ dependency-manifest discovery
+ deterministic receipt
```

Ele **não** fecha sozinho o SBOM de release.

Não demonstra:

- que cada arquivo inventariado foi incorporado a um APK/binário;
- que dependências transitivas foram resolvidas;
- que um arquivo de licença se aplica juridicamente a todo o repositório;
- compatibilidade entre licenças;
- titularidade;
- correspondência exata com um release asset ainda não produzido.

Logo:

```text
SOURCE_SBOM = IMPLEMENTED_LOCAL
RELEASE_SBOM = TOKEN_VAZIO
LICENSE_COMPATIBILITY = TOKEN_VAZIO_OWNER/LEGAL_REVIEW
```

## Determinismo

O gerador usa somente arquivos rastreados por Git:

```sh
git ls-files -z
```

Para cada arquivo registra:

- caminho;
- SHA-256 do conteúdo;
- Git blob SHA-1;
- tamanho;
- evidência de header `SPDX-License-Identifier`, quando observada.

O conjunto é ordenado por caminho e o receipt inclui:

```text
repository_commit
tracked_file_count
tracked_bytes
source_set_hash
sbom_hash
license_inventory_hash
```

Não é incluído timestamp dinâmico no BOM, evitando quebra de determinismo por relógio.

## CycloneDX

O arquivo usa `bomFormat=CycloneDX` e `specVersion=1.7`, versão corrente observada no corte de 2026-08-16.

A escolha do formato fornece interoperabilidade de inventário, mas este repositório não declara certificação CycloneDX nem validação por ferramenta externa apenas por emitir JSON nesse formato.

## Evidência de licença

O inventário procura dois tipos de sinal:

1. nomes de arquivo como `LICENSE`, `COPYING`, `NOTICE`, `COPYRIGHT`;
2. headers `SPDX-License-Identifier` nos primeiros bytes de arquivos rastreados.

Ambos são registrados como **evidência observada**, nunca como parecer jurídico:

```text
legal_conclusion = false
```

Se não houver licença na raiz:

```text
root_license_state = TOKEN_VAZIO
owner_license_decision_state = TOKEN_VAZIO_OWNER_DECISION
claim_allowed = false
```

Isso se conecta a `docs/LICENSE_DECISION_RECORD.md`.

## Dependency manifests

O gerador identifica manifestos conhecidos (`requirements.txt`, `pyproject.toml`, Cargo, Go, Maven, Gradle, npm etc.), mas marca:

```text
REFERENCE_ONLY_NOT_DEPENDENCY_RESOLVED
```

Resolver dependências reais exigirá adapters por ecossistema e, para release, vínculo ao build efetivo.

## Comando

```sh
python3 scripts/generate_source_sbom_v1.py \
  --sbom ci/reports/source-sbom/bom.cdx.json \
  --licenses ci/reports/source-sbom/license-evidence.json \
  --receipt ci/reports/source-sbom/receipt.json
```

Testes:

```sh
python3 -m unittest -v tests.test_generate_source_sbom_v1
```

## Closure material necessária para o gap de release

Para promover `GAP-SEC-SBOM-PROVENANCE` além de implementação local ainda é necessário:

1. escolher/licenciar o repositório por decisão humana;
2. resolver dependências relevantes por ambiente de build;
3. gerar o release artifact corrente;
4. ligar source SBOM / dependency SBOM / release artifact ao mesmo commit;
5. verificar checksums;
6. revisar termos de terceiros;
7. reproduzir o BOM/receipt no gate de release.

## F_ok / F_gap / F_next

```text
F_ok:
  inventário de fonte determinístico + hashes + sinais de licença + receipt

F_gap:
  dependency resolution + legal compatibility + release-asset binding

F_next:
  usar este inventário como entrada do futuro release gate, sem promovê-lo a release SBOM antes do artefato existir
```

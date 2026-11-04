# RafPolimata — Censo de arquivos Git (SOURCE census)

- **Fonte primária:** árvore `2fdac466c472982183e76eef928fb4e62fbc0ae8` do commit `88a6c840fc64271bb7d4f5854be42529ddffec85` de `rafaelmeloreisnovo/RafPolimata` (`main`, observado em 2026-11-04).
- **Método:** Git Trees API com `recursive=1`, resposta `truncated=false`; contar apenas blobs rastreados, sem consultar README como fonte de contagem.
- **Resultado:** 1952 blobs/arquivos, 315 diretórios, 15180001 bytes rastreados, 113 arquivos diretamente na raiz, 63 códigos `RAF_001..063` na raiz, 117 diretórios com exatamente 1 arquivo direto (POSSÍVEIS PONTOS DE DISPERSÃO; podem ter subpastas).
- **Duplicidade exata intra-repositório:** 10 grupos de Git blob SHA-1 repetido, 33 caminhos nesses grupos. Igualdade de SHA de blob Git corresponde a bytes iguais no objeto Git; NÃO é prova de redundância funcional, autoria, ou permissão para remoção.
- **Catálogo histórico:** `results/document-governance/summary.json` relata 1669 arquivos em `2026-10-02`, commit `1b6d6cfe51f5c827201d45f196d8c2bb7478bb19`; diferença líquida de contagem 283 versus este snapshot (não equivale a 283 adições confirmadas).
- **Fronteiras:** SOURCE (paths + blobs) comprovada; ARTIFACT / BUILD REACHABILITY / EXECUTION / TESTS / individual authorship = TOKEN_VAZIO para itens ainda não cruzados com comandos, graph de imports e CI.

## Saídas deste censo

- [CATALOG.tsv](CATALOG.tsv): todos os 1952 caminhos, Git blob SHA-1, tamanho, classe por extensão, colocação estrutural e grupo de hash idêntico.
- [ROOT_FILES.md](ROOT_FILES.md): os 113 arquivos diretamente na raiz com tamanhos e hashes.

## Classificações por tipo (heurísticas por extensão/localização)

| Classe | Itens |
|---|---:|
| DOCUMENT | 601 |
| NATIVE_CODE | 540 |
| SCRIPT | 417 |
| CONFIG_DATA_EVIDENCE | 266 |
| WORKFLOW | 75 |
| OTHER | 45 |
| ARCHIVE_PART | 7 |
| NESTED_WORKFLOW | 1 |

## Primeiros diretórios-família

| Raiz | Arquivos descendentes | Bytes |
|---|---:|---:|
| `docs` | 326 | 2061108 |
| `Apkc` | 219 | 1494379 |
| `tests` | 211 | 1144937 |
| `scripts` | 163 | 1148097 |
| `tools` | 131 | 563195 |
| `.github` | 91 | 276200 |
| `freestanding` | 81 | 269367 |
| `research` | 51 | 169727 |
| `knowledge_base` | 49 | 1102081 |
| `data` | 46 | 170830 |
| `Benchmark` | 40 | 130707 |
| `configs` | 35 | 144256 |
| `experiments` | 33 | 121290 |
| `canonical` | 32 | 94243 |
| `COMPILA` | 27 | 52100 |
| `rafci` | 25 | 183219 |
| `native` | 22 | 59489 |
| `federation` | 20 | 64581 |
| `fractal_core` | 18 | 27995 |
| `auditoria` | 15 | 76916 |
| `rafaelia` | 15 | 87290 |
| `receipts` | 15 | 36135 |
| `RafPictures` | 14 | 34922 |
| `ci` | 13 | 79063 |
| `results` | 13 | 4152984 |

## Duplicações exatas (não apagar)

### DUP01 — 7 caminhos — `e69de29bb2d1d6434b8b29ae775ad8c2e48c5391`
- `Apkc/proofs/out/.gitkeep`
- `Apkc/proofs/runs/20260617T064636Z/g1_syntax.txt`
- `Apkc/proofs/runs/20260816T035020Z/g1_syntax.txt`
- `ci/reports/asm_opt/RAF_009_adc_com_oversampling_x86_64.s`
- `ci/reports/asm_opt/RAF_013_uart_interrupt_driven_com_ring_buffer_x86_64.s`
- `ci/reports/asm_opt/RAF_032_dma_control_block_chain_x86_64.s`
- `tools/rafbbs/logs/.gitkeep`

### DUP02 — 4 caminhos — `11f0d9a1e8fa266ddfbed1e8aa2ea91ca925e52c`
- `Apkc/proofs/runs/20260617T064636Z/g4_arm32_encoders.txt`
- `Apkc/proofs/runs/20260816T034940Z/g4_arm32_encoders.txt`
- `Apkc/proofs/runs/20260816T034943Z/g4_arm32_encoders.txt`
- `Apkc/proofs/runs/20260816T035020Z/g4_arm32_encoders.txt`

### DUP03 — 4 caminhos — `625f6e208ec23ffd16e0fe2725ed6b1f35973453`
- `Apkc/proofs/runs/20260617T064636Z/g5_arm64_encoders.txt`
- `Apkc/proofs/runs/20260816T034940Z/g5_arm64_encoders.txt`
- `Apkc/proofs/runs/20260816T034943Z/g5_arm64_encoders.txt`
- `Apkc/proofs/runs/20260816T035020Z/g5_arm64_encoders.txt`

### DUP04 — 4 caminhos — `e7311b0573967c205c148977c1c0ed0bec9166e0`
- `Apkc/proofs/runs/20260816T034940Z/apkc_hardened.c`
- `Apkc/proofs/runs/20260816T034943Z/apkc_hardened.c`
- `Apkc/proofs/runs/20260816T035020Z/apkc_hardened.c`
- `build/generated/Apkc/apkc.source-cap-hardened.c`

### DUP05 — 3 caminhos — `f0f7262bba6f9ad75bffcf46664f5ceb2902c7cf`
- `Apkc/proofs/runs/20260816T034940Z/g3_verbovivo.txt`
- `Apkc/proofs/runs/20260816T034943Z/g3_verbovivo.txt`
- `Apkc/proofs/runs/20260816T035020Z/g3_verbovivo.txt`

### DUP06 — 3 caminhos — `5c279f7c6a400e85fb7cb40f336edd37996d0f02`
- `Apkc/proofs/runs/20260816T034940Z/g6_apk_generation.txt`
- `Apkc/proofs/runs/20260816T034943Z/g6_apk_generation.txt`
- `Apkc/proofs/runs/20260816T035020Z/g6_apk_generation.txt`

### DUP07 — 2 caminhos — `6c1e957ac8a3ff4236a6f9376294f8036623e278`
- `Apkc/proofs/runs/20260816T034940Z/g0_source_cap.txt`
- `Apkc/proofs/runs/20260816T034943Z/g0_source_cap.txt`

### DUP08 — 2 caminhos — `848dd30a72ea930e41ddb599968b19834ae1ab4b`
- `Apkc/proofs/runs/20260816T034940Z/g1_syntax.txt`
- `Apkc/proofs/runs/20260816T034943Z/g1_syntax.txt`

### DUP09 — 2 caminhos — `06f648b2522a93148b8db9668456a8412953da81`
- `Apkc/proofs/runs/20260816T034940Z/gates.txt`
- `Apkc/proofs/runs/20260816T034943Z/gates.txt`

### DUP10 — 2 caminhos — `132f948d199f6679fcae4f33912e0a3ad69691a3`
- `native/raf_bl0/raf_bl0.c`
- `ports/hafnium/raf_bl0.c`

## Exemplos concretos de diferenças funcionais

- `RAF_001_acesso_direto_a_ddrx_portx_pinx.c` declara internamente ser um **skeleton C** para firmware AVR; arquivo existente ≠ firmware executado.
- `RAF_063_language_completion_freestanding.c` contém função de self-test e includes locais; compilação precisa ser demonstrada separadamente.
- `raf_main.c` inclui `stdio.h` e `string.h`, logo seu perfil é **hosted** no código observado e não deve ser automaticamente agrupado com freestanding sem libc.
- `Apkc/apkc.c` define core de compilador com includes próprios; código presente ≠ APK físico validado.
- O `Makefile` é um **entrypoint seletivo**, não uma lista de compilação de todos os arquivos `RAF_001..063`; os métodos exigem reconciliação por referência direta/indireta e targets CI antes de definir órfãos operacionais.

## Gaps e plano reversível

1. Cruzar cada `path + blob` com `Makefile`, scripts, workflows, includes, imports e testes para obter os estados **REFERENCED / UNREFERENCED_OBSERVED / TOKEN_VAZIO**; ausência de referência textual não basta para provar inutilidade.
2. Verificar autonomia/terceiros/licenças por conteúdo e histórico de autoria individual; GitHub owner ≠ autor de cada arquivo.
3. Classificar `ONE_DIRECT_FILE` como dispersão estrutural, nunca automaticamente órfão funcional.
4. Nenhuma movimentação, deleção, merge, publicação nem execução de CI nesta etapa; rollback = fechar PR / descartar branch sem afetar `main`.

R3=⟨F_ok: árvore e catálogo completos, F_gap: reachability e autoria/CI por arquivo não verificadas, F_next: reconciliação estática de referências na branch⟩. `claim_allowed=false`.

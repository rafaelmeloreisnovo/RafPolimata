# RAFAELIA L0 — source admission from Vectras / Llama / GAIA / Termux / RLL

State: DOCUMENTED_CANDIDATE / SOURCE_POINTER_ONLY / DO_NOT_PROMOTE / no code copy.
Producer authority: RafPolimata freestanding L0. Federated audit owner: Mapa
PR #751, `data/assurance/federated-authorship-intake-20261011.v1.json`.
Rights blocker: repo-wide redistribution LICENSE root TOKEN_VAZIO;
no technical or legal approval inferred.

## Priority intake, with actual original-author boundary

| Upstream/candidate | Observed origin | Current L0 disposition |
|---|---|---|
| xoureldeen Vectras / Android / QEMU | official upstream GPL-2.0-only; its AboutActivity is inherited modified by local project | REFERENCE ONLY; keep upstream credit. Do not import |
| Vectras RAFAELIA RMR Vector Field | project-local traceable contribution (AI assisted), Vectras `engine/rmr/src/rmr_vector_field.c` git blob `8dfb230410b20e36236dbc3863d301e16a4c595d`, SPDX GPL-2.0-only, PR #982 and #1142 ancestry registered | NO COPY. Extract only independently specified algebra after rights clarification, boundary, independent oracle, explicit test/license compatibility |
| llamaRafaelia original llama.cpp | upstream ggml authors MIT | NO COPY, project-local changes only if individually identified |
| llamaRafaelia BitStack | local copyright/license MIT, file `rafaelia-baremetal/bitstack/raf_bitstack.c` blob `523deecf002188dd923a6a0bf15e474d08b82c20` | C_HOSTED_NOT_L0: malloc, calloc, stdio, string, math. Do not relabel as zero dependency |
| GAIA_phi BIT-LAYER `native/raf_bl0/raf_bl0.c` | declares authorial no-includes, blob `132f948d199f6679fcae4f33912e0a3ad69691a3`; repository MIT root contradicted by LICENSE.md | SOURCE CANDIDATE, NO COPY: first prove ancestry/rights, pointer bounds and differential behavior |
| GAIA_phi network `net/gaia_phi.c` | contains network/IO host and syscall bridge | OS ADAPTER OUTSIDE L0, never copy into pure core |
| GAIA-PDS-PHI / GaiaPhiRafcode | conceptual manifesto/legacy catalog; source licenses and authorship partially unknown | REFERENCE, no implementation imported |
| Rafaelia_Private | custom file-scoped proposed rights, private origin | PRIVATE POINTER ONLY; no raw bytes/PII/credentials |
| termux-app-rafacodephi / packages | Termux upstream GPLv3 + exceptions; per-package build licensing | Hosted Android/packaging adapters; no source import into L0 |
| RLL theory | original scholarly content CC BY-SA 4.0, software file-specific | Mathematics stays hypothesis; no runtime/scientific promotion |

## LAMA (barro) — correção de identidade

LAMA é **somente um plugin**, no sentido definido pelo proponente.
Não é LLaMA, não é `ggml-org/llama.cpp` e não deve ser confundido
com o fork `llamaRafaelia`. O caminho do código específico de LAMA-barro
permanece `TOKEN_VAZIO_NOT_IDENTIFIED`; nenhum símbolo, adaptador
ou arquivo foi importado por inferência do nome.

O que o código visível prova é separado: `llamaRafaelia/CMakeLists.txt`
(blob `4ea45cb2dc953a472df184bd63e0657257d42fb2`) define módulos
RAFAELIA opcionais, default OFF, e `common/raf_inference.h`
(blob `50a76fdcda88020f7fd751ee149506ca8ffb084e`)
possui fallback inline sem lógica de fiscalização ao desativar os contratos.
Nem essa extensão nem `BitStack` herdado no checkout podem ser
promovidos como L0 freestanding puro sem depuração e provas específicas.
Somente um contrato plugin ↔ núcleo, versionado e explicitamente rastreado,
pode ser integrado depois de localizar sua fonte autoral.

## Valid L0 transformation sequence (without external runtime)
`SOURCE_POINTER -> RIGHTS/PER_FILE_ANCESTRY -> CLEAN_SPEC ->
ZERO-HEADER PURE-C -> CALLER_OWNED_STATE -> HOSTED_FALSIFIER ->
SIX-ISA_OBJECT_CROSS_BUILD -> EXTERNAL_UNDEFINED_SYMBOLS=0 ->
INDEPENDENT_MODEL_ORACLE -> EXACT_HEAD_CI -> REVIEW/RIGHTS_GATE -> DECISION`.

The existing L0 `freestanding/arch/raf_fs_vector.h`, event JMV PR #431,
ELF/DEX/NUMA PR #432 and virtual bit fabric PR #433 are distinct sources.
This audit neither rebases nor modifies these branches. A new L0 source must be
new expression, not disguised transposition of GPL-2.0-only upstream code.

No libc, malloc, stdint.h, math.h, printf, JNI, Android SDK, QEMU, PROOT,
Termux hosted command, Python, network stack, OS ABI or allocator inside pure L0.
Hosted build tools and model falsifiers live outside target; comments describe
preconditions but are NOT machine executable cleanup instructions.

Do not mark specific ARM/vector/host compiler PASS until actual source exact-head
tests run. Size of model is not hardware observability; `TOKEN_VAZIO != 0`.

## Intake readiness
As of this audit NO external candidate has cleared per-file rights, ancestry,
zero-dep, falsifier and independent reviewer gates together. Consequently
`claim_allowed=false`, and no third-party bytes were imported into this PR.
Rollback = close draft PR; no force push, deletion or upstream attribution change.
F_next = independently examine GAIA BL0 input bounds and Vectras RMR arithmetic,
build semantic *specifications*, then request scoped rights and implement
an original clean-room L0 module with separate negative tests where warranted.

## Tipagem do bloqueio autoral — vínculos de closure, sem licença concedida

A referência `CLOSURE_L11` (`docs/closures/CLOSURE_L11_OPERATIONAL_GAP_TOPOLOGY.md`) governa o registro de `TOKEN_VAZIO`, sua origem, papéis e próximos falsificadores. A referência `CLOSURE_L12` (`docs/closures/CLOSURE_L12_LEGAL_SERVICE_ASSURANCE.md`) é **estrutura de análise jurídica**, não prova de titularidade, permissão de redistribuição, licença-raiz compatível ou autorização para importar arquivos de terceiros.

Os três vazios descritos nesta intake permanecem **materiais**: `TOKEN_VAZIO_LICENSE_ROOT`, `TOKEN_VAZIO_LAMA_SOURCE_ID`, `TOKEN_VAZIO_PHYSICAL_EXECUTION`. Nenhum foi transformado em zero ou PASS. A ligação de closure pode fechar exclusivamente a ausência de roteamento requerida pelo validador, enquanto a PR continua `NÃO PROMOVER` até recibos, rights e revisão independente. Não foi acrescentado conteúdo de outra autoria.

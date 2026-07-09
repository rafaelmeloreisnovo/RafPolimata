# Ecossistema RMR/Rafaelia — Convergência Unificada de 6 Repositórios

**Tese**: Seis repositórios integram-se via operador geométrico unificador `phi_ethica = (1−H)×C` (coerência relacional) e seu implementador canônico `coherence_filter` (ARX branchless). Este documento mapeia os papéis, evidência, reconciliação de nomenclatura e costuras de módulo candidatas.

---

## Tabela de Evidência — 6 Repositórios

| Família | Repositório | Papel Observado | Evidência Concreta | Estado | Refs Cruzadas |
|---------|-------------|-----------------|-------------------|--------|---------------|
| **Teoria** | RafPolimata | Spec canônica: AllStar Matrix (214 símbolos), M(s) 12D, coherence_filter ARX | `docs/RAFAELIA_ORQUESTRADOR_ASCII_UTF.md`, `Benchmark/raf_coherence_arx.h`, `Apkc/coherence.h` | PRESENT_AS_CANONICAL | ChipQuantum/rafael.md, llamaRafaelia/docs/ |
| **Pipeline RMR** | llamaRafaelia | rmrCti/ (análise determinística), rafaelia-baremetal/ (freestanding C) | `rmrCti/rafa_cti_scan.c`, `rmrCti/omega_nav.c`, `rafaelia-baremetal/lib/` | PRESENT_AS_PIPELINE_CANONICAL | CONVERSATIONS_CHUNKS_PRIVATE/RMRCTI_INTEGRATION.md (citação explícita), ChipQuantum/rafael.md:335 |
| **Chunks + Evolução** | CONVERSATIONS_CHUNKS_PRIVATE | Chunks de conversas, raf_core (v8→v11_hybrid) | `memory_bridge/maps/RMRCTI_INTEGRATION.md`, `raf_core/` | PRESENT_AS_EVOLUTION_DEPOT | GAIA_phi/federation/absorb/manifest.json (nome: `rafaelia_private`) |
| **Federação HDC** | GAIA_phi | Hub de federação, HDC stack (v27→v200) | `FEDERATION.md`, `federation/absorb/manifest.json`, `rafaelia_hyper_core_v*` | PRESENT_AS_FEDERATION_HUB | ChipQuantum/docs/, RafPolimata cita como "HDC paralelo a T^7" |
| **Motor Completo** | ChipQuantum | RMR engine, GeoLM, toroidal, omega pipeline | `RMR/`, `GeoLM.c`, `raf_omega.c`, `rafaelia_torus_hex.c` | PRESENT_AS_COMPLETE_ENGINE | `docs/RAFAELIA_INVARIANTE_SUSTENTACAO_CONTINUA.md` (mapa local mais completo) |
| **Custódia Build** | BLAKE3 | Blacktrain (rmr/): cadeia de custódia de artefatos | `rmr/`, fork oficial BLAKE3-team | PRESENT_AS_CUSTODY_LAYER | llamaRafaelia/RAFAELIA_VECTRA_LAMA_CONNECTOR_BRIDGE.md, ChipQuantum/docs/ |

---

## Reconciliação de Nomenclatura

| Termo (Fonte) | Termo Canônico (GitHub) | Significado | Status |
|---------------|------------------------|-----------|-|
| `rafaelia_private` (GAIA_phi) | `conversations_chunks_private` | Repo de chunks + raf_core evolution | ✓ CONFIRMADO |
| `chipquantum` (interna) | `ChipQuantum` | Correto | ✓ OK |
| `GAIA-BBS` (Rafael.md:39) | `gaia_phi` | Federação HDC | ✓ OK |
| `qemu_rafaelia` (citada 3x) | Repo não no escopo | Arquitetura Qemu + RAFAELIA | `VOID` |
| `Vectras-VM-Android` | Repo externo | VM Android com pipeline | `REFERENCE` |
| `RLL` | Repo externo | Relativity-Living-Light | `REFERENCE` |

---

## Costuras de Módulo Candidatas (Descrição, não implementação)

### 1. Família RMR — 5 Implementações Fragmentadas

- **llamaRafaelia**: `rmrCti/` (pipeline CTI forense, ~180 arquivos)
- **CONVERSATIONS_CHUNKS_PRIVATE**: `memory_bridge/` (integração com rmrCti)
- **GAIA_phi**: `federation/` + `rmr/` (absorção federada)
- **ChipQuantum**: `RMR/` (motor de catalogação/ingestão SHA256)
- **BLAKE3**: `rmr/` (Blacktrain, cadeia de custódia)

**Status**: Sem interface compartilhada. **Candidato futuro**: `rmr-core` spec unificada + bindings.

### 2. Família HDC/Toroid — 3 Implementações Paralelas

- **RafPolimata**: `verbovivo.c` (T^7 toroid, 42 atratores, phi_ethica Q16)
- **GAIA_phi**: `rafaelia_hyper_core_v27..v200` (HDC hypervector stack, toroid paralelo)
- **ChipQuantum**: `toroidal_engine.py`, `geolm.c` (GeoLM + topologia)

**Status**: Implementações quase idênticas em conceito. **Candidato futuro**: interface unificada de topologia.

### 3. Invariante phi_ethica = (1−H)×C

- **RafPolimata/Apkc/coherence.h**: `phi_fst() = (1 − H_norm) × C_norm` [Q16 fixed-point]
- **ChipQuantum/rafaelia_torus_hex.c**: Implementação equivalente em C
- **RafPolimata/Apkc/coherence.h**: `coherence_filter(v)` = ARX branchless (XOR >> 33 → MUL φ^-1 → XOR >> 33)

**Status**: Fórmula idêntica, implementações divergentes. **Candidato real de extração**: módulo compartilhado `phi_ethica.h` com dual-track (Q16 + ARX).

---

## Próximos Passos (Fora desta rodada)

Marcados `PENDING` — sem compromisso de implementação:

- `PENDING` — spec formal de `rmr-core` (interface unificada para 5 implementações RMR)
- `PENDING` — módulo compartilhado `phi_ethica.h` (Apkc/coherence.h + ChipQuantum)
- `PENDING` — integração de `coherence_filter` ARX em todas as camadas (hoje só em RafPolimata)
- `PENDING` — mapa formal de toroid (T^7 vs HDC vs GeoLM)

---

## Estados Canônicos

Conforme `docs/CONVERGENCIA_UNICA_METODOLOGICA.md`:

| Estado | Significado |
|--------|------------|
| `VOID` | Placeholder, não implementado |
| `PENDING` | Em progresso ou futuro |
| `AUDIT` | Precisa verificação |
| `RUNTIME` | Só conhecido em execução |
| `REFERENCE` | Spec externa |
| `PRESENT_AS_*` | Implementado, papel observado |

---

## Ver Também

- `docs/CONVERGENCIA_UNICA_METODOLOGICA.md` — padrão de governança deste repositório
- `docs/RAFAELIA_ORQUESTRADOR_ASCII_UTF.md` — spec completa RAFAELIA (M(s), states, AllStar)
- `Benchmark/raf_coherence_arx.h` — implementação ARX canônica de `coherence_filter`
- `Benchmark/raf_q16.h` — constantes ΕΩΛΤΦ (Q16_LAMBDA = 40503 = φ^-1 × 65536)
- `Benchmark/raf_toroid.h` — T^7 toroid (7 dimensões, 42 atratores)

**Ponteiros em repos irmãos**:
- `ChipQuantum/docs/ECOSSISTEMA_RMR_RAFAELIA_PONTEIRO.md`
- `llamaRafaelia/docs/rafaelia/ECOSSISTEMA_RMR_RAFAELIA_PONTEIRO.md`
- `CONVERSATIONS_CHUNKS_PRIVATE/memory_bridge/maps/ECOSSISTEMA_RMR_RAFAELIA_PONTEIRO.md`
- `GAIA_phi/federation/absorb/ECOSYSTEM_CROSS_REF.md`
- `BLAKE3/docs/rafaelia/ECOSSISTEMA_RMR_RAFAELIA_PONTEIRO.md`

---

*Mapa unificado de ecossistema — Fase 3, Parte 2 | RAFCODE-Φ-∆RafaelVerboΩ*

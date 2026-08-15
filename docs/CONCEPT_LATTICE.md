# Concept Lattice: Canonical Navigation Structure

**Purpose:** Single source of truth for how all 150+ documentation files relate to each other and to the 50-phase compiler architecture.

**Pattern:** Hierarchical lattice with 7 roots → projects → phases → components → concepts → implementations.

---

## Lattice Structure (7-Level Hierarchy)

```
Compiler (Core)
├── Phases 1-50 (50 nodes)
│   ├── Phases 1-20: APKc (bare-metal APK compiler)
│   ├── Phases 21-45: Semantic Analysis (type, symbol, CFG, dataflow, opt)
│   └── Phases 46-50: Production (coordinator, diagnostics, deployment)
│
Cognitive Engine (Research)
├── Fiber-H (256-bit Hamming hash, relational matrix)
├── Trinity Core (HDC hypervectors, synaptic attention, engram buffer)
└── T^7 Toroid (7-dim manifold, 42 attractors, φ_ethica coherence)
│
Languages (Polyglot)
├── 12 Language Profiles (C, Py, Go, Rust, Kt, JS, PHP, Sh, Pl, Java, Rb, JSX)
├── Language Dispatch (table-driven, use_script/use_fork/use_asm)
└── FFI & Interop (symbol tables, calling conventions, type marshaling)
│
Architecture (Systems)
├── ARM64 ISA (NEON, FMA, widening, SDOT, scatter/gather, scalar FP)
├── ARM32 ISA (encoders, mnemonic mapping)
├── ELF Format (sections, symbols, relocations, ARM.attributes)
├── APK Format (AXML manifest, DEX bytecode, signatures v1/v2/v3/v3.1)
└── ZIP Archive (compression, alignment, central directory)
│
Operations (Production)
├── CI/CD Pipeline (15+ gates, GitHub Actions, artifact management)
├── Deployment (pre-checks, resource limits, SLA monitoring)
├── Security (signing, audit trails, hardening)
└── Troubleshooting (runbooks, error recovery, rollback)
│
Documentation (Meta)
├── Specifications (SPEC_* formal definitions)
├── Decisions (ADR_* rationale & consequences)
├── Proofs (WITNESS_* evidence & attestation)
├── Runbooks (RUNBOOK_* procedures)
└── Closures (CLOSURE_* gap resolution)
│
Methodology (Process)
├── AI Collaboration (multi-agent protocols, handoff procedures)
├── Verification (testing strategy, golden tests, benchmark baselines)
├── Quality Gates (freestanding, determinism, performance, security)
└── Evolution (roadmap, future phases, scaling)
```

---

## Node Details: All 150+ Files Mapped to Lattice Positions

### ROOT 1: Compiler (Core ~8K lines Apkc)

**Phase 1-5: Lexer**
- Position: Compiler/Phases 1-5/Lexer
- Files:
  - `Apkc/lex_tokenizer.h` — token type definitions
  - `Apkc/lex_buffer.h` — buffered input handling
  - Tests: `tests/test_phase1_lexer.c`
- Related Docs: `docs/SPEC_LEXER.md`, `docs/adr/ADR_0010_TOKENIZATION_STRATEGY.md`

**Phase 6-10: Parser**
- Position: Compiler/Phases 6-10/Parser
- Files:
  - `Apkc/parse_ast_builder.h` — AST node constructors
  - `Apkc/parse_grammar.h` — grammar rules & precedence
  - Tests: `tests/test_phase6_parser.c`
- Related Docs: `docs/SPEC_PARSER.md`, `docs/adr/ADR_0011_PRECEDENCE_CLIMBING.md`

**Phase 11-20: Code Generation (APKc)**
- Position: Compiler/Phases 11-20/APKc
- Core File: `Apkc/apkc.c` (~1300 lines, single translation unit)
- Supporting: `Apkc/arch_arm64.h`, `Apkc/arch_arm32.h`, `Apkc/lang_profile.h`
- Format: `Apkc/fmt_elf.h`, `Apkc/fmt_axml.h`, `Apkc/fmt_zip.h`
- Tests: 100+ in `tests/test_phases_1_to_20.c`
- Related Docs: 
  - `docs/APKC_PROTOCOL.md` (core reference)
  - `docs/adr/ADR_0001_FREESTANDING_NO_MALLOC.md`
  - `docs/adr/ADR_0002_TABLE_DRIVEN_DISPATCH.md`
  - `docs/APKC_ANDROID_PROOF_CHAIN_HARDENING_20260812.md`

**Phase 21-22: Semantic Analysis (Type + Symbol)**
- Position: Compiler/Phases 21-45/SemanticAnalysis/Type-Symbol
- Files:
  - `Apkc/sem_type_system.h` (365 lines)
  - `Apkc/sem_symbol_table.h` (755 lines)
  - Tests: `tests/test_phases_23_to_35.c` (81 tests)
- Related Docs:
  - `docs/SPEC_TYPE_SYSTEM.md` (PENDING — needs formal spec)
  - `docs/SPEC_SYMBOL_RESOLUTION.md` (PENDING)
  - `docs/adr/ADR_0003_HINDLEY_MILNER_INFERENCE.md`

**Phase 23-26: Analysis Pipeline (CFG + Dataflow + Opt + Verify)**
- Position: Compiler/Phases 21-45/SemanticAnalysis/Analysis
- Files:
  - `Apkc/sem_cfg_builder.h` (323 lines)
  - `Apkc/sem_dataflow.h` (355 lines)
  - `Apkc/opt_semantic_fold.h` (329 lines)
  - `Apkc/sem_verifier.h` (292 lines)
- Related Docs:
  - `docs/SPEC_CFG_BUILDER.md` (PENDING)
  - `docs/SPEC_DATAFLOW_ANALYSIS.md` (PENDING)
  - `docs/adr/ADR_0004_FIXED_POINT_ITERATION.md`

**Phase 27-35: Advanced Features (Error Recovery, IDE, Features)**
- Position: Compiler/Phases 21-45/SemanticAnalysis/Advanced
- Files: `Apkc/lang_async_inference.h`, `Apkc/lang_generics.h`, etc.
- Related Docs:
  - `docs/SPEC_ERROR_RECOVERY.md` (PENDING)
  - `docs/SPEC_IDE_INTEGRATION.md` (PENDING)

**Phase 36-45: Optimization Passes (45 passes across 10 phases)**
- Position: Compiler/Phases 21-45/Optimization
- Files: `Apkc/opt_*.h` (multiple optimization modules)
- Related Docs:
  - `docs/SPEC_OPTIMIZATION_FRAMEWORK.md` (PENDING)
  - `docs/OPTIMIZATION_PASS_MATRIX.md` (which pass, when applied, impact estimate)

**Phase 46-48: Production Integration**
- Position: Compiler/Phases 46-50/Integration
- Design: Semantic Coordinator, Diagnostics, Optimization Coordinator
- Status: PENDING (designed, not yet implemented)
- Related Docs:
  - `docs/adr/ADR_0005_PHASE_ORCHESTRATION.md`
  - `docs/RUNBOOK_SEMANTIC_PIPELINE.md` (PENDING)

**Phase 49-50: Validation & Deployment**
- Position: Compiler/Phases 46-50/Validation-Deployment
- Files:
  - `Apkc/val_comprehensive_testing.h` (69 unit tests designed)
  - `Apkc/deploy_production_framework.h` (54 deployment tests designed)
- Status: PENDING (headers created, tests partially implemented)
- Related Docs:
  - `docs/APKC_PROTOCOL.md` (phase descriptions)
  - `docs/TEST_STRATEGY.md` (testing approach)
  - `docs/adr/ADR_0006_DEPLOYMENT_GATES.md`

---

### ROOT 2: Cognitive Engine (Research ~3.5K lines Rafaelia)

**Layer 1: Fiber-H (256-bit Hamming Hash)**
- Position: CognitiveEngine/FiberH
- Files:
  - `rafaelia/fiber_hash.h`
  - `rafaelia/fiber_relmat.c` (relational matrix computation)
- Related Docs:
  - `docs/SPEC_FIBER_H.md` (PENDING — hash function formal spec)
  - `docs/rafaelia/FIBER_H_ALGORITHM.md`

**Layer 1: Trinity Core (HDC Hypervectors)**
- Position: CognitiveEngine/TrinityCPre
- Files:
  - `rafaelia/hdc_hypervec.h`
  - `rafaelia/hdc_synaptic.h` (synaptic attention)
  - `rafaelia/engram_ring.h` (engram buffer)
- Related Docs:
  - `docs/SPEC_HDC_HYPERVECTORS.md` (PENDING)
  - `docs/rafaelia/HYPERDIMENSIONAL_COMPUTING.md`

**Layer 2: T^7 Toroid (7-dimensional manifold, 42 attractors)**
- Position: CognitiveEngine/T7Toroid
- Files:
  - `rafaelia/toroid_7d.h`
  - `Apkc/coherence.h` (phi_fst, phi_attractor implementation)
- Related Docs:
  - `docs/SPEC_T7_TOROID.md` (PENDING — formal manifold definition)
  - `docs/SPEC_PHI_ETHICA_CONVERGENCE.md` (PENDING — convergence proof)
  - `docs/rafaelia/T7_TOROID_MATHEMATICS.md`
  - `docs/TOKEN_VAZIO_PARABOLAS_MESTRES.md` (philosophical context)

**Integration: VerbViVO (Convergence Engine)**
- Position: CognitiveEngine/Integration/VerbViVO
- Files:
  - `rafaelia/verbovivo.c` (~900 lines, entry point: verbovivo_main)
  - `rafaelia/verbovivo.h` (API: vv_init, vv_scan, vv_audit, vv_svg, vv_recall)
- Related Docs:
  - `docs/rafaelia/VERBOVIVO_PIPELINE.md`
  - `docs/SPEC_VERBOVIVO_CONVERGENCE.md` (PENDING)
  - `docs/rafaelia/ENGRAM_SVG_VISUALIZATION.md`

---

### ROOT 3: Languages (Polyglot ~1.2K lines dispatch)

**Language Table (Central Registry)**
- Position: Languages/Dispatch
- File: `Apkc/lang_profile.h` (12 rows: ASM, C, Py, Go, Rust, Kt, JS, PHP, Sh, Pl, Java, Rb, JSX)
- Related Docs:
  - `docs/SPEC_LANGUAGE_DISPATCH.md` (PENDING — table-driven architecture)
  - `docs/adr/ADR_0002_TABLE_DRIVEN_DISPATCH.md`
  - `docs/MULTI_LANGUAGE_MATRIX.md` (coverage, status per language)

**Script Languages (ASM, Py, Sh, Pl, JS, PHP)**
- Position: Languages/Dispatch/ScriptLanguages
- Files:
  - `Apkc/lang_script.h` (18-instruction ARM64 bootstrap for interpreters)
- Related Docs:
  - `docs/adr/ADR_0007_SCRIPT_BOOTSTRAP.md`
  - `docs/APKC_TARGET_ENVIRONMENTS.md` (Termux/proot/dev-lab targets)
  - Tests: `scripts/apkc_lang_coverage.sh` (6 languages tested in CI)

**Compiled Languages (C, C++, Rust, Go, Kotlin, Java)**
- Position: Languages/Dispatch/CompiledLanguages
- Approach: fork_exec_wait (spawn external compiler, wait for output)
- Status: PENDING (use_fork gated by __aarch64__, need CI testing)
- Related Docs:
  - `docs/LANGUAGE_COMPLETION_FREESTANDING_METHODOLOGY.md`
  - `docs/adr/ADR_0008_FORK_EXEC_DISPATCH.md`

**Specialized: JSX (Babel → Node)**
- Position: Languages/Dispatch/JSX
- Approach: Two-stage bootstrap (Babel transpiler + Node interpreter)
- Related Docs: (embedded in `APKC_TARGET_ENVIRONMENTS.md`)

**FFI & Language Interop**
- Position: Languages/FFI
- Status: VOID (no cross-language validation harness yet)
- Closure Path: C↔Rust↔Go test harness (gap VOID-H2)
- Related Docs:
  - `docs/SPEC_FFI_MARSHALING.md` (PENDING)
  - `docs/adr/ADR_0009_CALLING_CONVENTION_MAPPING.md`

---

### ROOT 4: Architecture (ISA + Formats)

**ARM64 Instruction Set**
- Position: Architecture/ARM64ISA
- Files: `Apkc/arch_arm64.h` (~65 instructions: NEON, FMA, widening, SDOT, scatter, scalar FP)
- Related Docs:
  - `docs/SPEC_ARM64_ENCODERS.md` (PENDING — formal instruction encoding)
  - `docs/adr/ADR_0003_ARM64_INSTRUCTION_SELECTION.md`
  - Tests: `tests/test_arm64_encoders.py` (50+ golden tests)

**ARM32 Instruction Set**
- Position: Architecture/ARM32ISA
- Files: `Apkc/arch_arm32.h` (11+ recent mnemonics: mvn, neg, rsb, bic, tst, etc.)
- Status: PENDING (39 mnemonics still unknown in L5, being closed incrementally)
- Related Docs:
  - `docs/adr/ADR_0004_ARM32_MNEMONIC_MAPPING.md`
  - Tests: `tests/test_arm32_encoders.py` (16 golden tests)

**ELF Format (Executable & Linkable)**
- Position: Architecture/Formats/ELF
- Files: `Apkc/fmt_elf.h` (ELF64/ELF32 builder, sections, symbols, relocations)
- Related Docs:
  - `docs/SPEC_ELF_FORMAT.md` (PENDING — section layout, relocation types)
  - `docs/adr/ADR_0005_ELF_SYMBOL_TABLE.md`
  - Tests: `tests/test_elf_validation.sh` (readelf parsing, structure validation)

**AXML Format (Android Binary Manifest)**
- Position: Architecture/Formats/AXML
- Files: `Apkc/fmt_axml.h` (binary XML builder for AndroidManifest.xml)
- Related Docs:
  - `docs/SPEC_AXML_FORMAT.md` (PENDING — binary layout, string pool)
  - `docs/adr/ADR_0006_AXML_BUILDER.md`

**DEX Format (Android Bytecode)**
- Position: Architecture/Formats/DEX
- Status: PENDING (minimal 140-byte DEX created; full Java pipeline incomplete)
- Related Docs:
  - `docs/SPEC_DEX_FORMAT.md` (PENDING — class definitions, method encoding)
  - `docs/adr/ADR_0007_DEX_BYTECODE_GENERATION.md`

**ZIP Archive Format**
- Position: Architecture/Formats/ZIP
- Files: `Apkc/fmt_zip.h` (ZIP writer with CRC32, alignment, central directory)
- Related Docs:
  - `docs/SPEC_ZIP_FORMAT.md` (PENDING — local file header, central directory format)
  - `docs/adr/ADR_0008_ZIP_ARCHIVE_GENERATION.md`
  - Tests: `tests/test_zip_negative.py` (corrupted ZIP rejection)

---

### ROOT 5: Operations (Production)

**CI/CD Pipeline**
- Position: Operations/CI-CD
- Files: `.github/workflows/ci.yml` (15+ gates)
- Related Docs:
  - `docs/CI_COMPILER_EXCELLENCE/README.md` (overview)
  - `docs/CI_COMPILER_EXCELLENCE/FLAGS_MATRIX.md` (compiler flag combinations)
  - `docs/CI_COMPILER_EXCELLENCE/ROLLBACK_FAILSAFE.md` (recovery strategy)

**Deployment Procedures**
- Position: Operations/Deployment
- Files: `Apkc/deploy_production_framework.h` (pre-checks, gates, SLA, runbooks)
- Related Docs:
  - `docs/RUNBOOK_DEPLOY_TO_ANDROID.md` (PENDING)
  - `docs/RUNBOOK_DEPLOY_TO_TERMUX.md` (PENDING)
  - `docs/adr/ADR_0009_DEPLOYMENT_GATES.md`

**Security & Signing**
- Position: Operations/Security
- Related Docs:
  - `docs/APKC_SIGNING_POLICY.md` (debug vs release keystore, SourceStamp)
  - `docs/RUNBOOK_APK_SIGNING.md` (PENDING)
  - `docs/adr/ADR_0010_SIGNING_STRATEGY.md`

**Monitoring & SLAs**
- Position: Operations/Monitoring
- Related Docs:
  - `docs/PERFORMANCE_SLA_BASELINE.md` (PENDING — throughput, latency, availability targets)
  - `docs/RUNBOOK_PERFORMANCE_REGRESSION.md` (PENDING)
  - `docs/adr/ADR_0011_SLA_COMPLIANCE.md`

**Troubleshooting & Runbooks**
- Position: Operations/Runbooks
- Status: VOID (5 runbooks not yet written)
- Closure Path: Phase C1 from plan
- Related Docs: (TBD)

---

### ROOT 6: Documentation (Meta)

**Technical Specifications (SPEC_* files)**
- Position: Documentation/Specifications
- Count: ~15 pending (see list above)
- Related Docs: `docs/SPEC_*.md` (formal definitions, invariants, examples, proofs)

**Architecture Decisions (ADR_* files)**
- Position: Documentation/Decisions
- Count: ~15 pending (freestanding, T^7 toroid, 45 passes, table dispatch, etc.)
- Related Docs: `docs/adr/ADR_*.md` (problem, constraints, solution, consequences)

**Evidence & Proofs (WITNESS_* files)**
- Position: Documentation/Proofs
- Files in: `docs/proofs/WITNESS_*.md` (scripts, output logs, hash attestation)
- Examples:
  - WITNESS_APKC_BUILD (compiler provenance)
  - WITNESS_DETERMINISM (3-build byte-identical)
  - WITNESS_ARM64_E2E (end-to-end ARM64 proof)
  - WITNESS_APK_SIGN (signature verification)

**Operational Procedures (RUNBOOK_* files)**
- Position: Documentation/Runbooks
- Files in: `docs/runbooks/RUNBOOK_*.md` (prerequisites, steps, verification, errors, rollback)
- Examples:
  - RUNBOOK_DEPLOY_TO_ANDROID
  - RUNBOOK_TROUBLESHOOT_RUNTIME_CRASH
  - RUNBOOK_RECOVER_BUILD_FAILURE

**Gap Resolution (CLOSURE_* files)**
- Position: Documentation/Closures
- Files in: `docs/closures/CLOSURE_*.md` (gap ID, root cause, solution, implementation, verification, sign-off)
- Examples:
  - CLOSURE_L1_COMPILER_PROVENANCE
  - CLOSURE_VOID_C1_WITNESS_GENERATION
  - CLOSURE_AUDIT_C1_FREESTANDING_AUDIT

---

### ROOT 7: Methodology (Process)

**AI Collaboration Protocols**
- Position: Methodology/AI-Collaboration
- Related Docs:
  - `docs/AGENTES.md` (startup checklist, non-collision rules, CI gates)
  - `docs/AGENTES_CHECKLIST.md` (per-session checklist)
  - `docs/AGENTES_DECISAO_LOG.md` (conflict resolution log)
  - `docs/MULTI_AI_METHODOLOGY.md` (multi-agent handoff, priorities)
  - `docs/IA_AGENTE_HUMANOS_TECNICO_FORMALIDADE.md` (AI-human protocol)

**Testing & Verification**
- Position: Methodology/Testing
- Related Docs:
  - `docs/TEST_STRATEGY.md` (unit, integration, golden, regression, performance)
  - `docs/BENCHMARK_VISUAL.md` (performance metrics visualization)
  - `tests/test_*.c` (comprehensive test suites per phase)

**Quality Gates**
- Position: Methodology/QualityGates
- Related Docs:
  - `docs/CI_COMPILER_EXCELLENCE/README.md` (15+ gates overview)
  - `docs/adr/ADR_0001_FREESTANDING_NO_MALLOC.md` (freestanding gate)
  - `docs/adr/ADR_0012_DETERMINISM_VERIFICATION.md` (determinism gate)

**Evolution & Roadmap**
- Position: Methodology/Evolution
- Related Docs:
  - `docs/ROADMAP_CODIGO_DOCUMENTACAO_CONSCIENTE.md` (future phases 51-100)
  - `docs/SCIENCE_LEARNING_ENGINE.md` (ML-driven optimization pipeline)
  - Future phases: 51-60 (distributed compilation), 61-70 (JIT), 71+ (self-optimization)

---

## Lattice Navigation Rules

### Rule 1: Every Doc Has a Position
- Format: `Root/Level2/Level3/.../Component`
- Example: `Compiler/Phases 21-45/SemanticAnalysis/Type-Symbol`
- Every `.md` file must declare: `<!-- LATTICE_POSITION: ... -->`

### Rule 2: Every Component Has a Status Badge
- `✅ PASS` (implemented + tested + documented)
- `◐ PENDING` (implemented, not fully tested or documented)
- `⊘ VOID` (not implemented)
- `⚠️ AUDIT` (needs verification)

### Rule 3: Cross-Links Are Bidirectional
- Doc A links to related docs (same lattice level, parent, child)
- Backward links: "See also", "Related to", "Depends on"

### Rule 4: Lattice Position Determines CI Gate
- Position in Architecture/ → architecture tests run first
- Position in Compiler/ → compiler tests run in phase order
- Position in Operations/ → deployment tests run last

---

## Example Lattice Traversal

**Query:** "Where do I find everything about type inference?"

**Lattice Answer:**
```
Compiler/Phases 21-45/SemanticAnalysis/Type-Symbol
├── Code: Apkc/sem_type_system.h, Apkc/sem_type_inference.h
├── Spec: docs/SPEC_TYPE_SYSTEM.md (PENDING)
├── Tests: tests/test_phases_23_to_35.c (81 tests, see type inference cases)
├── ADR: docs/adr/ADR_0003_HINDLEY_MILNER_INFERENCE.md
├── Related Specs:
│   ├── docs/SPEC_SYMBOL_RESOLUTION.md (type → symbol binding)
│   └── docs/SPEC_UNIFICATION_ALGORITHM.md (constraint solving)
├── Performance Baseline: docs/PERFORMANCE_BASELINE_TYPE_INFERENCE.md
└── Known Issues: docs/GAP_INDEX_COMPLETE_AUDIT.md → AUDIT-H1 (formal spec missing)
```

---

## Maintenance: Keep Lattice in Sync

**CI Gate:** `tools/validate_lattice_consistency.sh`
- Scans all .md files for `LATTICE_POSITION` tag
- Verifies no orphaned files
- Checks cross-links are bidirectional
- Fails if lattice inconsistent

**Weekly:** Update lattice positions when new docs added
**Monthly:** Review lattice structure for simplification opportunities

---

## Lattice as Single Source of Truth

From now on:
1. Every new doc starts with lattice position
2. Every code change links to lattice position
3. Every gap closure maps to lattice node
4. Every phase completion updates lattice status

**Benefit:** External stakeholders can navigate with `Root → Phase → Component → Code`.

No more: "Where's the type system documentation?"

**Now:** Go to `Compiler/Phases 21-45/SemanticAnalysis` → find all type docs + code + tests + specs.

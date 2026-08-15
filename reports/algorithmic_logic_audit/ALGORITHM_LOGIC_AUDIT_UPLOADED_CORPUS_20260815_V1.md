# RAFAELIA — Algorithmic Logic Audit — Uploaded Corpus 2026-08-15 V1

**State:** `APPLIED_STATIC_CORPUS / claim_allowed=false`  
**Skill:** `skills/algorithmic-logic-audit/SKILL.md`  
**Boundary:** this run audits the uploaded documentary corpus. It does **not** claim fresh execution of every algorithm referenced by those documents.

## 1. Corpus boundary

- Files: **26**
- Bytes: **238621**
- Lines: **9071**
- `TOKEN_VAZIO` mentions: **122**
- Every source was hashed with SHA-256 before analysis.
- Global repository/source-code coverage remains `TOKEN_VAZIO_NOT_ALL_REPOSITORY_SOURCE_BYTES`.

## 2. Core logic reconstructed

```text
identity/provenance
→ type before operation
→ invariant/domain
→ implementation/spec comparison
→ counterexample/negative path
→ execution evidence at the exact layer
→ axis-local promotion
→ append-only feedback
```

The strongest recurring design rule is: **an evidence gate only promotes the property/axis that it actually measures.**

## 3. Material findings

### ALA-20260815-001 — `S4` `SPEC_IMPL_DIVERGENCE`

**Scope:** numerical-output / geometry  
**Sources:** Análise técnica da imagem.txt

The corpus records an announced output p1=1, p2=0.2, r=0.5 while the declared relation r=sqrt(p1^2+p2^2) yields r≈1.0198. The same source records a float32 recomputation around p1=0.1739, p2=0.2338, r=0.2914.

**Invariant:** Reported output must satisfy the implementation equation and the geometry domain.

**Smallest next gate:** Reproduce from frozen source bytes and compare output bit-for-bit or within declared float tolerance.

**State:** `DOCUMENTED_COUNTEREXAMPLE` · `claim_allowed=false`

### ALA-20260815-002 — `S4` `DOMAIN_HOLE`

**Scope:** Lorentz/Poincare precondition  
**Sources:** Análise técnica da imagem.txt, Projeção de Poincaré 7D.txt

The uploaded audits record all eight raw C[8][8] column candidates as spacelike (Delta<0), so strict future-timelike hyperboloid projection is not applicable to the raw matrix.

**Invariant:** Strict hyperboloid→Poincare projection requires a declared timelike point satisfying the model precondition.

**Smallest next gate:** Freeze C[8][8], recompute all Delta values, reject strict projection when Delta<=0, and keep any later lift as a separate explicit transformation.

**State:** `PRECONDITION_FAIL_DOCUMENTED` · `claim_allowed=false`

### ALA-20260815-004 — `S4` `PRODUCT_IDENTITY_MISMATCH`

**Scope:** RafGitTools Android fallback  
**Sources:** Compilação no Termux.txt

The corpus states that the hermetic APK path exports only minimal NativeActivity entry points returning immediately and therefore is an ABI carrier, not the functional RafGitTools application.

**Invariant:** A fallback carrying a product name must implement the bounded product capability being claimed, or be named as a carrier/bootstrap only.

**Smallest next gate:** Define required functional capabilities, inspect packaged classes/native symbols/resources, install, launch, and exercise those capabilities under the product package/signing contract.

**State:** `FUNCTIONAL_SCOPE_MISMATCH_DOCUMENTED` · `claim_allowed=false`

### ALA-20260815-003 — `S3` `SPEC_IMPL_DIVERGENCE`

**Scope:** projection formula  
**Sources:** Análise técnica da imagem.txt, Auditoria de Arquitetura Matemática.txt, Projeção de Poincaré 7D.txt

The corpus distinguishes the claimed standard Lorentzian projection from an earlier denominator using sqrt(T^2+||V||^2); the documented standard path requires a timelike invariant and the corresponding minus sign under the square root before normalization.

**Invariant:** A named mathematical transform must match its exact declared definition and domain.

**Smallest next gate:** Property test against independently implemented reference formula on timelike fixtures plus rejection fixtures for spacelike/null inputs.

**State:** `FORMULA_MISMATCH_DOCUMENTED` · `claim_allowed=false`

### ALA-20260815-005 — `S3` `PROVENANCE_LOCK_GAP`

**Scope:** RafGitTools↔RafPolimata builder contract  
**Sources:** Compilação no Termux.txt

The uploaded audit records that runtime-lock fixes a RafPolimata commit that does not contain the hermetic builder used by the actual path, while the path can select another checkout/builder.

**Invariant:** The lock that claims reproducibility must govern the exact builder bytes actually executed.

**Smallest next gate:** Introduce a toolchain lock covering repo commit, builder SHA-256, host binary SHA-256, ABI policy, signer identity and page-size policy; fail closed on mismatch.

**State:** `LOCK_DOES_NOT_GOVERN_EXECUTION` · `claim_allowed=false`

### ALA-20260815-006 — `S3` `STRUCTURAL_VALIDATION_GAP`

**Scope:** Android ABI/page alignment  
**Sources:** Compilação no Termux.txt

The corpus records that requesting both ABIs only expresses intent and that the route did not prove both ELF identities or the required page/ZIP alignment properties.

**Invariant:** Packaging intent != verified ELF/ZIP structure != runtime compatibility.

**Smallest next gate:** Extract both .so files; verify ELFCLASS/e_machine/program headers/alignment, ZIP alignment, AXML/DEX structure, signature, install and launch separately.

**State:** `TOKEN_VAZIO_RUNTIME_AND_STRUCTURE` · `claim_allowed=false`

### ALA-20260815-007 — `S3` `EXECUTION_INFERENCE_GAP`

**Scope:** federated Termux/Vectras/QEMU evidence  
**Sources:** Memória RAFAELIA V1.txt, Organizar repositório e memória.txt, Auditoria PR #178.txt

The corpus repeatedly preserves cross-compile!=execution, dispatch!=QEMU execution, exit-code zero!=guest boot, receipt presence!=independent validation, and CI runner absence!=code failure.

**Invariant:** Each promotion must be supported by evidence at the exact execution layer claimed.

**Smallest next gate:** Require per-layer receipts with environment, executable hash, command, exit status, boot artifact/signal and independent reproduction when the claim requires it.

**State:** `INVARIANT_STRONG_RUNTIME_STILL_PARTIAL` · `claim_allowed=false`

### ALA-20260815-008 — `S3` `HARDWARE_SEMANTICS_OVERCLAIM`

**Scope:** AArch64 DMB/NEON/cache  
**Sources:** Análise técnica da imagem.txt, Auditoria de Arquitetura Matemática.txt

The corpus documents that a DMB instruction does not prove cache behavior or mathematical/global coherence, and ISA flags permitting SIMD do not prove emitted NEON vector instructions.

**Invariant:** Compiler permission, opcode presence and microarchitectural behavior are distinct evidence classes.

**Smallest next gate:** Disassemble exact binary, verify expected vector opcodes and barrier encoding, then benchmark/adversarially test on the target hardware with counters where available.

**State:** `CLAIM_SCOPE_REDUCED` · `claim_allowed=false`

### ALA-20260815-009 — `S3` `GATE_SCOPE_MISMATCH`

**Scope:** multi-domain promotion  
**Sources:** Roadmap e Inventário Total.txt, Taxonomia Universal 416.txt, Cânone do Cosmos.txt

The corpus converges on a key rule: a gate can only promote the axis for which it has semantic meaning; schema/hash/tokenizer passes cannot prove mathematical novelty, physical truth or independent replication.

**Invariant:** Evidence is typed; promotion is axis-local.

**Smallest next gate:** Attach domain adapter + property-specific gate_id to every PASS and block transitive promotion across unrelated axes.

**State:** `CORE_INVARIANT_CONFIRMED` · `claim_allowed=false`

### ALA-20260815-010 — `S2` `TEMPORAL_CAUSALITY_GAP`

**Scope:** BLAKE3 upstream similarity  
**Sources:** Auditoria de warnings Blake3.txt

The uploaded timeline refutes causal derivation of upstream changes that predate the later public PR; structural similarity alone remains insufficient to establish exposure or influence.

**Invariant:** Cause must not postdate effect; similarity != mechanism/exposure.

**Smallest next gate:** For later events only, compare exact files/functions/diffs and preserve exposure/provenance as TOKEN_VAZIO unless material evidence exists.

**State:** `EARLIER_CAUSALITY_REFUTED_LATER_RELATION_OPEN` · `claim_allowed=false`

### ALA-20260815-011 — `S2` `SEMANTIC_LAYER_SEPARATION`

**Scope:** lexical/compute/ISA semantics  
**Sources:** Organizar repositório e memória.txt

The corpus explicitly separates lexical meaning, executable semantics and architecture backend, and refuses to invent phonemes when pronunciation is undeclared.

**Invariant:** Representation layers may relate but cannot impersonate one another.

**Smallest next gate:** Keep stable IDs and independent tests for lexical, IR behavior and backend equivalence; unresolved phonology remains TOKEN_VAZIO.

**State:** `DESIGN_STRENGTH` · `claim_allowed=false`

### ALA-20260815-012 — `S2` `APPEND_ONLY_PROVENANCE_STRENGTH`

**Scope:** longitudinal memory/control plane  
**Sources:** Roadmap e Inventário Total.txt, Espiral Longitudinal Ω.txt, Resumo mapeamento ecossistema.txt

The corpus consistently defines source identity, hashes/pointers, append-only deltas, explicit epistemic states and F_ok/F_gap/F_next rather than overwriting prior failures.

**Invariant:** Corrections append a new state and preserve causal lineage.

**Smallest next gate:** Validate parent hash/pointer continuity and reject silent replacement of negative evidence.

**State:** `DESIGN_STRENGTH` · `claim_allowed=false`

## 4. Cross-corpus strengths

- **Fail-closed epistemology:** `TOKEN_VAZIO` is preserved instead of being converted to zero/pass.
- **Append-only custody:** later PASS does not erase prior FAIL/contradiction.
- **Layer separation:** lexical meaning, executable behavior, ISA backend, scientific inference and symbolic/parabolic interpretation are repeatedly separated.
- **Runtime humility:** cross-compilation, dispatch, exit status, boot and independent replication are treated as different states.
- **Counterexample orientation:** several uploaded audits actively preserve negative results, especially the spacelike H7 input, output/geometry mismatch and temporal-causality refutation.

## 5. Structural risk that remains

The main remaining risk is not lack of ideas. It is **contract drift between layers**: a document/spec may become more rigorous while the executable path, lockfile, fixture or runtime evidence still refers to another version. The skill therefore treats `specification ↔ exact source bytes ↔ exact executable ↔ receipt` as one custody chain.

## 6. Priority order

1. **S4 numerical/geometry:** reproduce the exact matrix/projection source and freeze reference vectors.
2. **S4 product identity:** prove whether the hermetic APK is a carrier or functional RafGitTools fallback; rename/fail if capability is absent.
3. **S3 toolchain lock/alignment:** bind builder bytes and validate both ABIs plus page/ZIP alignment.
4. **S3 hardware semantics:** disassemble and measure DMB/NEON claims on exact target binaries.
5. **S3 federated runtime:** perform physical device/QEMU receipts without collapsing dispatch→execution→boot.

## 7. Closure

```text
F_ok   = 26 uploaded text artifacts hashed and classified; cross-document invariants reconstructed; 12 material findings/strengths normalized; no documentary PASS promoted to unobserved runtime/scientific truth
F_gap  = exact source bytes for every algorithm referenced by the reports; fresh runtime execution of kernels/APKs/seven ISAs/QEMU; independent reference implementations for all mathematical transforms; full repository-wide coverage
F_next = Use this ledger to select source artifacts behind S4/S3 findings, freeze commit+SHA, then run property-specific adversarial tests beginning with numerical projection and Android hermetic build identity/alignment.
```

Ω: the corpus shows a mature **audit grammar**; the next gain comes from binding that grammar to the exact executable bytes behind the highest-severity findings.
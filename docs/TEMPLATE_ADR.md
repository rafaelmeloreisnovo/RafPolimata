<!-- TEMPLATE: Architecture Decision Record (ADR_*.md) -->
<!-- LATTICE_POSITION: Documentation/Decisions -->
<!-- USE THIS TEMPLATE FOR: Design rationale, trade-off explanations, decision history -->

# ADR_[Number]: [Title]

**Date:** YYYY-MM-DD  
**Author(s):** Name(s)  
**Status:** 🟢 Accepted | 🟡 Proposed | 🔴 Superseded  
**Lattice Position:** [Root]/[Path]/[Component]  
**Relates to Phases:** [phase numbers, e.g., 1-20, 21-45]  
**Relates to Gap:** (optional, e.g., AUDIT-C1 from GAP_INDEX_COMPLETE_AUDIT.md)  

---

## 1. Context

Describe the issue or decision point that prompted this decision.

### 1.1 Background

What situation led to this decision? What problem are we trying to solve?

**Example:**
```
The compiler needs to support 12 different programming languages (C, Python, Go, Rust, 
Kotlin, JavaScript, PHP, Shell, Perl, Java, Ruby, JSX). Each language has different 
compilation workflows:
- Interpreted: Python, Shell, Perl, PHP need a runtime (e.g., /usr/bin/python3)
- Compiled: C, C++, Go, Rust, Kotlin need an external compiler
- Hybrid: Java needs javac + d8 (DEX bytecode)
- Transpiled: JSX needs Babel + Node

We must choose ONE dispatch architecture that handles all 12 efficiently.
```

### 1.2 Constraints

What limitations or requirements narrow our choices?

**Example Constraints:**
- **Freestanding:** No malloc/libc in Apkc/ (memory-constrained, security-hardened)
- **Single TU:** Apkc/apkc.c is a single translation unit (~1300 lines) for reproducibility
- **Performance:** Dispatch overhead must be <1ms per invocation
- **Simplicity:** Must be understandable by future maintainers (not exotic)
- **Determinism:** Same source → identical binary every build (no hash randomization)
- **Portability:** Must work on x86_64 (build host) and ARM64 (target device)

### 1.3 Stakeholders

Who is affected by this decision?

- **Compiler maintainers** (must implement + debug dispatch logic)
- **Language add-on users** (must be able to add new languages easily)
- **End users** (experience latency/correctness of dispatch)
- **Operations team** (must monitor + troubleshoot language-specific issues)

---

## 2. Decision

State the decision clearly and concisely.

### 2.1 The Choice

**We will use:** [One sentence description of the chosen approach]

**Example:**
```
We will implement a table-driven dispatch mechanism where each language profile 
is a single struct entry in a static array (_lang_table[]), with fields describing 
the language's compiler flags, output format, and execution model.
```

### 2.2 How It Works (Brief)

Provide a short explanation (3-5 sentences):

**Example:**
```
1. User invokes: apkc source.py -o out.apk
2. Detect language from extension: lang_profile_from_ext(".py")
3. Look up in _lang_table[], get profile (use_script=1, interpreter="/usr/bin/python3")
4. Generate ARM64 bootstrap code embedding the interpreter + source
5. Build ELF .so + APK with bootstrap code
```

### 2.3 The Implementation

Link to actual code:

**File:** `Apkc/lang_profile.h`  
**Lines:** 1-50 (table definition)  
**Example entry:**
```c
{
  .ext = ".py",
  .name = "Python",
  .use_script = 1,
  .interpreter = "/usr/bin/python3",
  .flags = "-c",
  .use_fork = 0,
  .use_d8 = 0,
},
```

**Related functions:**
- `lang_profile_from_ext()` → lookup in table
- `gen_script_code64()` → generate bootstrap
- `build_apk()` → integrate into APK

---

## 3. Rationale

Why this decision? What alternatives were considered?

### 3.1 Why This Decision?

**Key advantages:**
1. **Simplicity** — One struct per language, no complex dispatch logic
2. **Extensibility** — Adding a language = 1 row in table + update 1 case in codegen
3. **Maintainability** — Single source of truth (lang_profile.h) for all language configs
4. **Performance** — O(N) table lookup, N=12, negligible overhead
5. **Determinism** — No hash-based ordering (table is ordered, reproducible)

**Why not other approaches?**
- Function pointers per language? Would require conditional compilation, breaks freestanding constraint
- Plugin system? External .so files break reproducibility + increase complexity
- Hardcoded if-else chain? Violates DRY principle, hard to extend

### 3.2 Trade-Offs

What are we giving up?

**Trade-off 1: Extensibility vs. Coupling**
- **Pro:** Adding a language requires only a 1-line table entry
- **Con:** Language-specific code is scattered across multiple functions (codegen, symbol resolution, etc.)
- **Mitigation:** Document where language-specific code lives (SPEC_LANGUAGE_DISPATCH.md)

**Trade-off 2: Flexibility vs. Simplicity**
- **Pro:** Table-driven approach is simple to understand
- **Con:** Cannot easily support languages with exotic features (e.g., self-modifying code)
- **Mitigation:** Define "unsupported features" clearly; users know language limits upfront

**Trade-off 3: Performance vs. Generality**
- **Pro:** Simple dispatch is fast
- **Con:** Cannot defer all language-specific work until runtime (some upfront)
- **Mitigation:** Profile to find bottlenecks; optimize if needed

### 3.3 Alternative Approaches Considered

| Approach | Pros | Cons | Why Rejected |
|----------|------|------|-------------|
| **Table-driven (chosen)** | Simple, extensible, deterministic | Scattered language-specific code | ✅ Selected |
| **Function pointers** | Flexible, modular | Requires conditional compilation, breaks freestanding | ❌ Conflicts with constraints |
| **Plugin system** | Infinite extensibility | Breaks reproducibility, complex, security risk | ❌ Violates invariants |
| **Hardcoded if-else** | Fast, simple | Not maintainable for 12 languages | ❌ Violates DRY |
| **Interpreter pattern** | Elegant, data-driven | Over-engineered for 12 languages | ❌ Premature complexity |

---

## 4. Consequences

What happens if we implement this decision?

### 4.1 Positive Consequences

1. **Easy to add languages** — New developer: copy table entry, edit 1-2 codegen cases
2. **Auditability** — All language profiles in one file (lang_profile.h), easy to audit
3. **Determinism** — No randomness; same input→same binary guaranteed
4. **Performance** — Fast dispatch, minimal overhead

### 4.2 Negative Consequences

1. **Scattered code** — Language-specific logic in `gen_script_code64()`, `asm_insn64()`, symbol table, etc.
   - **Mitigation:** Document dispatch flow clearly (docs/SPEC_LANGUAGE_DISPATCH.md)

2. **No runtime adaptability** — Language profile fixed at compile-time, cannot change behavior at runtime
   - **Mitigation:** Not a requirement; users expect static language profiles

3. **Limited to table size** — Current implementation supports max ~20 languages (fixed array)
   - **Mitigation:** If needed, switch to linked list (minor refactor, ~2 hours)

### 4.3 Long-Term Implications

- **Scaling:** If RafPolimata grows to 100+ languages, table approach may hit limits
  - **Action:** Consider plugin system or dynamic registration in Phase 60+
  
- **Maintenance:** Adding language-specific features (e.g., lazy compilation) may require table schema expansion
  - **Action:** Generalize table entry struct with `void *lang_specific_data` field

- **Testing:** Each language needs regression tests; test matrix grows with languages
  - **Action:** Automate test generation from table (template-based test suite)

---

## 5. Related Decisions

What other architectural decisions depend on or relate to this one?

| ADR | Title | Relationship |
|---|---|---|
| ADR_0001 | Freestanding (no malloc) | Constraint that limits dispatch approach |
| ADR_0003 | Hindley-Milner Type Inference | Type system must work for all 12 languages |
| ADR_0007 | Script Bootstrap Code | Implements use_script dispatch |
| ADR_0008 | Fork/Exec Dispatch | Implements use_fork dispatch |

---

## 6. Verification & Validation

How do we know this decision is working?

### 6.1 Success Criteria

- [ ] All 12 language profiles compile without error
- [ ] Adding new language requires ≤5 lines of code change
- [ ] No performance regression vs. previous dispatch
- [ ] Determinism test passes (3 builds = identical binaries)
- [ ] No malloc/libc detected in Apkc/ by static analysis

### 6.2 Testing Strategy

**Unit tests** (`tests/test_lang_dispatch.c`):
- For each language: test dispatch lookup
- Verify profile fields are correct
- Test edge cases (unknown extension, NULL profile)

**Integration tests** (`scripts/apkc_lang_coverage.sh`):
- For each of 12 languages: compile → APK → validate structure
- 6 languages pass (script-based), 6 PENDING (need ARM hardware)

**Regression tests** (`Benchmark/raf_dispatch_perf.c`):
- Measure lookup time for each language
- Must be <1ms per lookup
- Compare to previous versions (no regression)

### 6.3 Monitoring

In production:
- **Metric:** Lookup time per language (p50, p99)
- **Alert:** If p99 > 10ms (5x normal)
- **Dashboard:** Display language distribution (which languages most used)

---

## 7. Implementation Notes

Details for implementers.

### 7.1 Critical Code Sections

**File:** `Apkc/lang_profile.h`
```c
typedef struct {
  const char *ext;           // ".py", ".go", etc.
  const char *name;          // "Python", "Go"
  u8 use_script;             // 1 if interpreter-based
  u8 use_fork;               // 1 if external compiler
  u8 use_asm;                // 1 if raw ARM64 assembly
  u8 use_d8;                 // 1 if DEX conversion (Java/Kotlin)
  const char *interpreter;   // "/usr/bin/python3" for scripts
  const char *compiler;      // "rustc" for Rust
  const char *flags;         // "-c" for interpreter flags
  // ... more fields
} LangProfile;

#define LANG_TABLE_SIZE 12
extern const LangProfile _lang_table[LANG_TABLE_SIZE];

LangProfile* lang_profile_from_ext(const char *ext);
```

### 7.2 Freestanding Compliance

- ✅ No malloc (static array, no dynamic allocation)
- ✅ No libc includes
- ✅ No external function pointers
- ✅ All data stack-allocated or ROM-allocated

### 7.3 Testing Checklist

- [ ] Each table entry compiles cleanly
- [ ] `lang_profile_from_ext()` returns correct profile for each extension
- [ ] Unknown extension returns NULL (error case)
- [ ] Performance test: lookup time <1ms per language
- [ ] Determinism test: same table → identical binary (3 builds)

---

## 8. Future Improvements

What could be improved or changed in the future?

### 8.1 Phase 51+: Enhanced Language System

If RafPolimata scales beyond 12 languages:

**Option A: Dynamic Registration** (Phase 60)
```c
// Instead of static table, use registration API:
int lang_register(const char *ext, const LangProfile *profile);

// Profiles loaded from /etc/rafpolimata/languages/ or similar
// Trade-off: loses determinism (file order matters), adds complexity
```

**Option B: Trait-Based Dispatch** (Phase 70)
```c
// Instead of single LangProfile struct, dispatch on language capabilities:
// - "supports_generics" → dispatch to generic-aware type checker
// - "supports_async" → dispatch to async-aware CFG builder
// More modular, but more complex
```

### 8.2 Performance Optimization

If lookup becomes bottleneck:
- Current: O(N) linear search, N=12 (negligible)
- Optimization 1: Hash table (O(1) lookup, loses ordering determinism)
- Optimization 2: Binary search (O(log N), requires sorted table)
- Recommendation: Only optimize if profiling shows lookup >1% of compile time

### 8.3 Backward Compatibility

If we ever change table schema:
```c
// Version 1 (current)
typedef struct {
  const char *ext;
  const char *name;
  u8 use_script;
  // ... 
} LangProfile_V1;

// Version 2 (future)
typedef struct {
  const char *ext;
  const char *name;
  u8 use_script;
  // ... more fields
} LangProfile_V2;

// Support both versions for N releases, then deprecate V1
```

---

## 9. References

Links to related docs, specs, and code.

**Related ADRs:**
- ADR_0001: Freestanding architecture
- ADR_0007: Script bootstrap code generation
- ADR_0008: Fork/exec external compiler dispatch

**Related Specs:**
- docs/SPEC_LANGUAGE_DISPATCH.md (formal specification, PENDING)
- docs/APKC_TARGET_ENVIRONMENTS.md (describes Termux/proot/device targets)

**Test Files:**
- tests/test_lang_dispatch.c
- scripts/apkc_lang_coverage.sh
- Benchmark/raf_dispatch_perf.c

**Implementation Files:**
- Apkc/lang_profile.h (language table)
- Apkc/lang_script.h (interpreter bootstrap)
- Apkc/apkc.c (language dispatch logic)

---

## 10. Decision Log

Record decisions, approvals, and changes.

| Date | Event | Notes |
|---|---|---|
| 2026-06-15 | Proposed | Initial table-driven design |
| 2026-06-20 | Approved | Tech lead reviewed, accepted |
| 2026-08-01 | Implemented | All 12 languages in table |
| 2026-08-15 | Validated | Dispatch tests passing (6/12 languages on x86_64, all 12 on design) |
| TBD | Deployed | Production usage begins |

---

**End of ADR Template**

**To use this template:**
1. Copy to `docs/adr/ADR_[Number]_[Title].md`
2. Number sequentially (ADR_0001, ADR_0002, etc.)
3. Fill in all sections (especially 1-3)
4. Get approval from tech lead or architect
5. Mark status as "Accepted" (or "Proposed" if not yet approved)
6. Commit as immutable record (never edit past ADRs, create new ADR if decision changes)

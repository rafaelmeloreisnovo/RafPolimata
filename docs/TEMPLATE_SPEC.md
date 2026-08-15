<!-- TEMPLATE: Technical Specification (SPEC_*.md) -->
<!-- LATTICE_POSITION: Documentation/Specifications -->
<!-- USE THIS TEMPLATE FOR: Formal definitions, algorithms, invariants, proofs -->

# SPEC_[Component Name]: [Short Description]

**Date:** YYYY-MM-DD  
**Author(s):** Name(s)  
**Lattice Position:** Compiler/Phases XX/ComponentName  
**Related Files:** `Apkc/file.h`, `tests/test_file.c`  
**Status:** ✅ PASS | ◐ PENDING | ⊘ VOID | ⚠️ AUDIT  

---

## 1. Formal Definition

### 1.1 Data Types & Structures

Define formally using mathematical notation or pseudocode:

```c
/* Example: Type system */
Type ::= Bool | Int | String | Fun(Type, Type) | App(Type, Type)

/* Example: Control flow graph node */
typedef struct {
  u32 id;
  Instruction *instructions;
  u32 instr_count;
  struct GraphNode *successors[2];  /* for branch/fallthrough */
} GraphNode;
```

### 1.2 Invariants

List invariants that must hold at all times:

- **Invariant I1:** All nodes in graph are reachable from entry node
- **Invariant I2:** No cycles in type unification constraints
- **Invariant I3:** Symbol table is topologically sorted by scope depth

### 1.3 Formal Grammar (if applicable)

```
Expr ::= Lit(value) 
       | Var(name) 
       | App(Expr, Expr) 
       | Lam(Var, Expr)
       | Let(Var, Expr, Expr)

Type ::= TVar(α) | TInt | TBool | TFun(Type, Type)
```

---

## 2. Algorithm or Mechanism

### 2.1 High-Level Overview

Describe in 2-3 sentences what the component does and why it exists.

### 2.2 Step-by-Step Algorithm

If applicable, present pseudocode or step-by-step procedure:

```
Algorithm: TypeInference(expr, context)
  Input: expr, context (symbol table)
  Output: (type, constraints)
  
  1. switch(expr)
  2. case Lit(v):
  3.   return type(v), {}
  4. case Var(x):
  5.   t ← context.lookup(x)
  6.   if t == NULL: error("undefined")
  7.   return t, {}
  8. case App(e1, e2):
  9.   (t1, c1) ← TypeInference(e1, ctx)
  10.  (t2, c2) ← TypeInference(e2, ctx)
  11.  α ← fresh type variable
  12.  c ← {t1 ~ TFun(t2, α)} ∪ c1 ∪ c2
  13.  return α, c
```

### 2.3 Complexity Analysis

- **Time:** O(N log N) for N nodes
- **Space:** O(N) bounded stack allocation
- **Determinism:** Reproducible (no randomness, no hash-based order)

---

## 3. Type Theory or Mathematical Foundation

### 3.1 Formal Notation

If this component has mathematical foundations, state them formally:

**Example: Hindley-Milner Type Inference**

```
Γ ⊢ e : τ   (Γ proves e has type τ)

Γ ⊢ Lit(n) : Int

Γ ⊢ Var(x) : τ   if Γ(x) = ∀α. τ

Γ ⊢ Lam(x:τ1, e) : τ1 → τ2
  if Γ, x:τ1 ⊢ e : τ2
```

### 3.2 Soundness & Completeness

State whether this algorithm/component is:
- **Sound:** If it produces a result, that result is correct
- **Complete:** If a correct result exists, it finds one

**Example:** Type inference is sound (no false positives) and complete (finds most general type).

---

## 4. Examples & Test Cases

### 4.1 Golden Test Cases

Provide 3-5 concrete examples with expected behavior:

**Test 1: Simple Application**
```
Input:  (λx. x + 1) 42
Type:   Int → Int applied to Int
Output: Type ← Int, Constraints ← {}
Status: ✅ PASS
```

**Test 2: Polymorphic Identity**
```
Input:  λx. x
Type:   Inferred
Output: Type ← ∀α. α → α, Constraints ← {}
Status: ✅ PASS
```

**Test 3: Type Mismatch (Error Case)**
```
Input:  (λx. x + 1) true
Type:   Int → Int applied to Bool
Output: Error("Type mismatch: Bool vs Int")
Status: ✅ PASS (error correctly detected)
```

### 4.2 Edge Cases

- Empty input
- Recursive structures
- Deeply nested expressions
- Maximum bounds (e.g., 1000-node graph)

---

## 5. Implementation Notes

### 5.1 Key Functions/Modules

Map formal definitions to actual code:

| Formal | Implementation | File | Lines |
|--------|---|---|---|
| `TypeInference` | `infer_type()` | `sem_type_inference.h` | 150-200 |
| `Unify` | `unify_types()` | `sem_unification.h` | 100-150 |
| `Subst` | `apply_substitution()` | `sem_subst.h` | 50-100 |

### 5.2 Freestanding Constraints

- No malloc (use bounded arrays, stack-only)
- No libc includes
- All external syscalls via `sys.h`
- Fixed-size buffers (specify max capacity)

**Example:**
```c
#define MAX_TYPE_VARS 256
#define MAX_CONSTRAINTS 1024

typedef struct {
  TypeVar vars[MAX_TYPE_VARS];
  Constraint constraints[MAX_CONSTRAINTS];
  u32 var_count;
  u32 constraint_count;
} TypeContext;
```

---

## 6. Verification & Testing

### 6.1 Test Coverage

- **Unit Tests:** [X/Y] test cases passing
  - Inference correctness: 20 tests → ✅ PASS
  - Error detection: 10 tests → ✅ PASS
  - Edge cases: 5 tests → ✅ PASS
  
- **Integration Tests:** [X/Y] end-to-end scenarios
  - Full pipeline: 15 tests → ✅ PASS

- **Regression Tests:** Compare against reference implementation
  - GHC types: 30 test cases → ✅ PASS

- **Property Tests:** (if applicable)
  - Inferred type always unifies with expression
  - Same expression always infers same type (determinism)

### 6.2 Known Limitations

- **Limitation 1:** Rank-2 types not supported (requires two-pass algorithm)
- **Limitation 2:** Type class constraints simplified (monomorphic instantiation only)
- **Limitation 3:** Recursive types require explicit `μ` binder

---

## 7. Proofs (if mathematical component)

### 7.1 Soundness Proof (Sketch)

By induction on expression structure:

- **Base:** Literals and variables are sound (trivial)
- **Step:** If e1:τ1→τ2 and e2:τ1 (by IH), then App(e1, e2):τ2 (by rule)

### 7.2 Determinism Proof

No randomness in algorithm → identical input always produces identical output (bitwise).

Verified by: running same input 3 times, comparing results (SHA256 hash equality).

---

## 8. Performance Characteristics

### 8.1 Benchmark Results

| Input Size | Time (μs) | Memory (KB) | Status |
|---|---|---|---|
| 10 nodes | 50 | 2 | ✅ |
| 100 nodes | 800 | 15 | ✅ |
| 1000 nodes | 15000 | 150 | ✅ |
| 10000 nodes | N/A | >Limit | ⊘ (exceeds 1MB) |

### 8.2 Optimization Opportunities

- Future: Use union-find data structure for faster unification (currently O(n²), could be O(n α(n)))
- Future: Memoize type inference results for commonly-inferred expressions

---

## 9. References & Related Docs

### 9.1 External References

- **ARM ISA Manual:** DDI 0487H.a (for instruction encoding specs)
- **APK Signature Format:** https://source.android.com/docs/security/verifiedboot/verified-boot
- **Research Paper:** "Polymorphic Type Checking" (Hindley, Milner, etc.)

### 9.2 Related Specifications

- See: `docs/SPEC_SYMBOL_RESOLUTION.md` (how types interact with symbols)
- See: `docs/SPEC_UNIFICATION_ALGORITHM.md` (constraint solving in detail)
- See: `docs/adr/ADR_0003_HINDLEY_MILNER_INFERENCE.md` (design rationale)

### 9.3 Implementation References

- Code: `Apkc/sem_type_system.h`, `Apkc/sem_type_inference.h`
- Tests: `tests/test_phases_23_to_35.c` (see type inference test cases)
- Proof: `docs/proofs/WITNESS_TYPE_INFERENCE_<date>.md` (verification evidence)

---

## 10. Sign-Off

| Role | Name | Date | Status |
|---|---|---|---|
| Author | Alice | 2026-08-15 | ✅ DRAFT |
| Reviewer | Bob | TBD | ⊘ PENDING |
| Tech Lead | Carol | TBD | ⊘ PENDING |

**Checklist:**
- [ ] Formal definition is complete and unambiguous
- [ ] Algorithm is described clearly (pseudocode or prose)
- [ ] Examples cover happy path + error cases
- [ ] Invariants are stated and verified
- [ ] Proof of correctness (if applicable) is sound
- [ ] Implementation matches formal spec
- [ ] All tests passing (unit + integration + regression)
- [ ] Performance within SLA
- [ ] Cross-links to related docs added to lattice

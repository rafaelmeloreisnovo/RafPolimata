# Glossary — RafPolimata documentation

**Governance binding: CLOSURE_L11** — unknown-state terminology follows the operational gap topology.

| Term | Meaning in this documentation |
|---|---|
| SOURCE | versioned input: code, data, schema, config or document |
| ARTIFACT | produced output such as object, ELF, DEX, APK, JSON, report or receipt |
| EXECUTION | an observed run in a specified environment |
| EVIDENCE | record that binds source/artifact/execution/result |
| CLAIM | assertion allowed only within demonstrated scope |
| PASS | named gate executed and passed in the declared scope |
| FAIL | named gate executed and failed |
| NOT_RUN | applicable gate not executed |
| TOKEN_VAZIO | evidence absent/insufficient/inapplicable in the cut; not numeric zero |
| PENDING | work or decision remains open |
| AUDIT | provenance/decision trail |
| REFERENCE | specification/explanation, not execution proof |
| IMPLEMENTED | code exists; runtime may still be open |
| GENERATED | derived from generator/policy/commit; do not hand-edit to fake freshness |
| RECEIPT | revision-bound record of an execution/decision/evidence event |
| CLOSURE | governance/evidence object defining how a gap is bound or closed |
| R3 | F_ok + F_gap + F_next feedback close |
| μWRITE | small append-only delta record rather than corpus duplication |
| L0 | in freestanding context, OS-neutral execution primitives; in document governance, physical structure layer — context must be stated |
| RAF Semantic IR | canonical semantic representation used to separate source syntax from target ISA |
| ApkC | repository subsystem for source dispatch and Android-oriented artifact construction |
| Freestanding | profile that avoids hosted runtime dependencies according to the route's explicit contract |
| Device proof | execution evidence from an identified authorized physical/device environment |
| Provider independence | reproduction dimension requiring a separately defined independent provider/executor |
| Historical snapshot | dated/revision-bound truth that is not automatically current |
| Router | short navigation document that points to authoritative sources rather than duplicating them |

## Ambiguity rule

When a term has more than one repository meaning, document the namespace/context rather than assuming a single global definition.

R3 = ⟨F_ok: recurring evidence/governance terms normalized; F_gap: subsystem-specific symbols remain defined by their own specs; F_next: link future docs to the local spec when a term carries domain-specific semantics⟩.

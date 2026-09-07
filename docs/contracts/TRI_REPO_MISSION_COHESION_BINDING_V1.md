# RafPolimata — Tri-Repo Mission Cohesion Binding V1

**Binding ID:** `RAFPOLIMATA-TRI-REPO-MISSION-COHESION-V1`  
**Canonical cohesion source:** `termux-app-rafacodephi@f83921f6a04e99199bdbef6a9d56607c150f4f2c`  
**RafPolimata baseline:** `3956bc6a7d7527ce3535661d170507c761b27fcb`  
**Vectras baseline:** `89e47837e551a565ceb7158cba1ced6954e72d93`  
**claim_allowed:** `false`  
**weight_training_authorized:** `false`  
**Governing closures:** `CLOSURE_L12` for runtime/device/legal-service evidence and `CLOSURE_L13` for provider/ruleset/server/manual/secret audit-readiness authority gaps.

RafPolimata is bound as the analysis/compiler/freestanding/research-validation lane of the program mission. This binding does not grant runtime execution authority, provider/legal/repository authority, manual promotion authority, scientific claim promotion, or model-weight update authority.

## Role boundary

RafPolimata may:

- analyze and classify source within declared scope;
- implement compiler/freestanding/indexing mechanisms already inside approved scope;
- execute bounded source/build/test validators where the environment actually exists;
- emit append-only evidence/receipts and successor records;
- expose reusable contracts to Termux/Vectras without acquiring their authority.

RafPolimata may not infer:

```text
IMPLEMENTED -> EXECUTED
EXECUTED -> DEVICE_PROVEN
BUILD_PROVEN -> RUNTIME_PROVEN
MODEL_OUTPUT -> EVIDENCE
RETRIEVAL_CONTEXT -> WEIGHT_UPDATE
SCIENTIFIC_HYPOTHESIS -> PROMOTED_CLAIM
```

## Cross-repo authority

- `termux-app-rafacodephi`: orchestration + bounded governance + authorized execution-plan composition.
- `Vectras-VM-Android`: optional governed runtime backend.
- `RafPolimata`: analysis/compiler/freestanding/research-validation evidence lane.

## Required invariants

```text
RETRIEVAL_CONTEXT            != WEIGHT_UPDATE
LEARN_APPEND_ONLY            != ONLINE_SELF_TRAINING
CONTINUE_APPROVED_SCOPE      != AUTONOMOUS_GOAL_CREATION
TOKEN_VAZIO                  != 0
SOURCE                       != EXECUTION
EXECUTION                    != EVIDENCE
EVIDENCE                     != CLAIM
DATASET_INFORMS              != MISSION_AUTHORITY
IMPLEMENTED                  != AUTHORIZED
```

`TOKEN_VAZIO` statements in this contract are closure-bound observations, not successful gates. `CLOSURE_L12` and `CLOSURE_L13` preserve the owner/evidence route and do not promote the underlying gap.

## External/runtime gates preserved

```text
Android/Termux físico          = TOKEN_VAZIO_DEVICE              # CLOSURE_L12
multi-repo runtime real        = TOKEN_VAZIO_EXECUTION           # CLOSURE_L12
identidade remota              = TOKEN_VAZIO_RUNTIME             # CLOSURE_L12
autorização provider/legal     = TOKEN_VAZIO_EXTERNAL_AUTHORITY  # CLOSURE_L12 + CLOSURE_L13
ruleset live                   = TOKEN_VAZIO_EXTERNAL_AUTHORITY  # CLOSURE_L13
server-side enforcement        = TOKEN_VAZIO_EXTERNAL_AUTHORITY  # CLOSURE_L13
promoção manual                = TOKEN_VAZIO_MANUAL_AUTHORITY    # CLOSURE_L13
CodeScan credencial/análise    = TOKEN_VAZIO_SECRET              # CLOSURE_L13
treino/fine-tuning de pesos    = NÃO AUTORIZADO
scientific claim promotion     = false
claim_allowed                  = false
```

## Append-only continuation

`LEARN` means observation/action/result/F_ok/F_gap/F_next/successor receipt. It does not mean online self-training and does not modify model weights.

A blocked external gate must remain typed `TOKEN_VAZIO` under its governing closure; work may continue only on independent safe lanes already within approved scope.

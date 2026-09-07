# RafPolimata — Tri-Repo Mission Cohesion Binding V1

**Binding ID:** `RAFPOLIMATA-TRI-REPO-MISSION-COHESION-V1`  
**Canonical cohesion source:** `termux-app-rafacodephi@f83921f6a04e99199bdbef6a9d56607c150f4f2c`  
**RafPolimata baseline:** `3956bc6a7d7527ce3535661d170507c761b27fcb`  
**Vectras baseline:** `89e47837e551a565ceb7158cba1ced6954e72d93`  
**claim_allowed:** `false`  
**weight_training_authorized:** `false`

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

## External/runtime gates preserved

```text
Android/Termux físico          = TOKEN_VAZIO_DEVICE
multi-repo runtime real        = TOKEN_VAZIO_EXECUTION
identidade remota              = TOKEN_VAZIO_RUNTIME
autorização provider/legal     = TOKEN_VAZIO_EXTERNAL_AUTHORITY
ruleset live                   = TOKEN_VAZIO_EXTERNAL_AUTHORITY
server-side enforcement        = TOKEN_VAZIO_EXTERNAL_AUTHORITY
promoção manual                = TOKEN_VAZIO_MANUAL_AUTHORITY
CodeScan credencial/análise    = TOKEN_VAZIO_SECRET
treino/fine-tuning de pesos    = NÃO AUTORIZADO
scientific claim promotion     = false
claim_allowed                  = false
```

## Append-only continuation

`LEARN` means observation/action/result/F_ok/F_gap/F_next/successor receipt. It does not mean online self-training and does not modify model weights.

A blocked external gate must remain typed `TOKEN_VAZIO`; work may continue only on independent safe lanes already within approved scope.

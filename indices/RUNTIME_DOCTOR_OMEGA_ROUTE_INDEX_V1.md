# Runtime Doctor Ω — Route Index V1

Estado: `APPEND_ONLY_DELTA / claim_allowed=false`

Objetivo: permitir reconstrução mínima do contexto operacional sem carregar todo o ecossistema.

## Invariantes

- `memory != proof`
- `observation != diagnosis`
- `diagnosis != patch`
- `ephemeral_patch != permanent_fix`
- `source_fix != build`
- `build != deployed_runtime`
- `tool_presence != capability`
- `candidate != benchmark`
- `GAIA_context != runtime_authority`
- `TOKEN_VAZIO` permanece explícito até fechamento por evidência.

## Índices

- `IDX-RD-00` — `configs/runtime-doctor-skills.v1.json`: registry L0–L7 + L2.5.
- `IDX-RD-01` — `scripts/runtime_doctor_agent.py`: roteador/receipt read-only.
- `IDX-RD-02` — `scripts/frida_runtime_probe.py`: readiness Frida sem attach/injeção.
- `IDX-RD-03` — `configs/runtime-doctor-frida.v1.json`: contrato OBSERVE→DIAGNOSE→PATCH_EPHEMERAL→PROMOTE.
- `IDX-RD-04` — `contracts/runtime-doctor-agent-report.schema.json`: contrato do report.
- `IDX-RD-05` — `tests/test_runtime_doctor_agent.py`: regressão estrutural e fail-closed.
- `IDX-RD-06` — `docs/ECOSYSTEM_BUILD_DOCTOR.md`: scanner estático preexistente.
- `IDX-RD-07` — `docs/C02_RUNTIME_TRUTH_RECEIPT.md`: cadeia de receipt/runtime truth.

## Relações

```text
Termux host diagnostic
   ├─> GPU orchestrator ─> llama backend candidate
   ├─> Frida runtime observer ─> GAIA context router
   │                           └─> RafPolimata evidence doctor
   └─> Vectras runtime ─> QEMU guest path

termux-packages ───────────────> RafPolimata evidence doctor
llama backend ─────────────────> RafPolimata evidence doctor
QEMU runtime ──────────────────> RafPolimata evidence doctor
GAIA context ──────────────────> RafPolimata evidence doctor
                                  └─> L7 prescription + receipt
```

Frida subrota:

```text
READINESS
  -> OBSERVE
  -> DIAGNOSE
  -> PATCH_EPHEMERAL [explicit/manual only]
  -> receipt + rollback
  -> source fix candidate
  -> build/test/runtime gates
  -> PROMOTE
```

## Rotas de recuperação

### ROTA-RD-A — sintoma Android/Termux
`termux_host_diagnostic -> Frida/GPU/VM branch -> evidence doctor -> F_ok/F_gap/F_next`

### ROTA-RD-B — sintoma dinâmico de processo
`frida_runtime_observer -> target identity -> OBSERVE -> DIAGNOSE -> receipt`

### ROTA-RD-C — correção runtime experimental
`diagnosis -> explicit PATCH_EPHEMERAL -> rollback condition -> post-patch receipt -> source fix candidate`

### ROTA-RD-D — GPU/LLaMA
`host diagnostic -> Vulkan/OpenCL/NEON candidate -> llama backend -> benchmark receipt -> evidence doctor`

### ROTA-RD-E — VM/QEMU
`host diagnostic -> Vectras -> QEMU -> guest boot receipt -> evidence doctor`

### ROTA-RD-F — reconstrução longitudinal
`Índice Ω Drive -> este índice -> registry -> exact source/blob/commit -> receipt -> latest F_next`

## TOKEN_VAZIO atuais

- `TOKEN_VAZIO_FRIDA_TOOLS_ON_TARGET_DEVICE`
- `TOKEN_VAZIO_GADGET_EXACT_ARTIFACT_SHA256`
- `TOKEN_VAZIO_DEVELOPER_APP_GADGET_LOAD_RECEIPT`
- `TOKEN_VAZIO_ATTACH_RECEIPT`
- `TOKEN_VAZIO_JAVA_ART_JNI_HOOK_RECEIPT`
- `TOKEN_VAZIO_BACKGROUND_PERSISTENCE_RECEIPT`
- `TOKEN_VAZIO_EPHEMERAL_PATCH_ROLLBACK_RECEIPT`
- `TOKEN_VAZIO_PHYSICAL_DEVICE_REGRESSION_TEST`
- `TOKEN_VAZIO_LLAMA_CPU_VULKAN_COMPARATIVE_BENCHMARK`
- `TOKEN_VAZIO_QEMU_GUEST_BOOT_RECEIPT`
- `TOKEN_VAZIO_REPOSITORY_NOT_FOUND: rafaeliaprivate`

## Platô atual

`IMPLEMENTED_CONFIGURATION / NOT_DEVICE_EXECUTED`

### F_ok
Runtime Doctor base já havia sido mesclado; a extensão Frida foi ligada ao mesmo registry e à mesma autoridade de evidence/receipt, sem criar um segundo Doctor.

### F_gap
Execução física Frida, Gadget, attach/hook, persistência background, patch efêmero e rollback continuam não executados neste índice.

### F_next
Executar o probe read-only no Termux físico, congelar receipt, resolver artefato Gadget por SHA-256 e somente então promover a menor rota OBSERVE autorizada.

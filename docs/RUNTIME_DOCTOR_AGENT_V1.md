# Runtime Doctor Agent V1 — médico técnico do runtime

## Propósito

O Runtime Doctor Agent transforma sintomas difusos do ecossistema em uma sequência auditável:

```text
sintoma/intenção
→ skill
→ probe read-only
→ observação
→ rota
→ prescrição mínima
→ receipt
→ outcome append-only
→ peso de rota futuro
```

Ele **não é um agente médico humano**, não altera o sistema por conta própria e não confunde presença de biblioteca com backend funcional.

## Origem existente reutilizada

O agente não substitui o `Ecosystem Build Doctor` nem o `Runtime Truth Receipt`.

- `scripts/ecosystem_build_doctor.py`: scanner estático read-only para CMake/flags/linker/zumbis/proveniência.
- `scripts/run_runtime_truth_receipt.sh`: gate local com logs, Build Doctor, toolchain e hashes.
- `termux-app-rafacodephi/Arme/Add/diagnose.sh`: diagnóstico ARM32/ARM64, CPU, cache, RAM, page size, OOM, NEON, OpenCL/Vulkan e toolchain.
- `Vectras-VM-Android/Rafaelia/diagnose.sh`: superfície equivalente dentro do fluxo Vectras.
- `termux-app-rafacodephi/Arme/Add/repo_gpu_orch.c`: probe OpenCL/Vulkan e roteamento CPU/GPU candidato.
- `llamaRafaelia`: possui backend Vulkan/ggml; desempenho só pode ser promovido após benchmark real.

## Mapa multinível

```text
L0 INVENTÁRIO
  ↓
L1 TERMUX / ANDROID / ABI / MEMÓRIA / OOM
  ↓
L2 GPU ── OpenCL / Vulkan / NEON / thermal
  ↓                         ↘
L3 VECTRAS / QEMU            L4 LLAMA / GGML / GGUF
  ↓                         ↙
L5 BUILD DOCTOR + RUNTIME TRUTH + RECEIPTS
  ↓
L6 GAIA CONTEXT ROUTER + rotas externas autorizadas
  ↓
L7 PRESCRIÇÃO + RECEIPT + F_ok/F_gap/F_next
```

### Autoridade por corpo

| Corpo | Autoridade no agente |
|---|---|
| `termux-app-rafacodephi` | host Android, ABI, CPU, memória, GPU loader, toolchain |
| `termux-packages` | superfície de receitas/build; não é autoridade do runtime físico |
| `Vectras-VM-Android` | ciclo VM Android e integração com host |
| `qemu_rafaelia` | QEMU/guest; boot real exige receipt próprio |
| `llamaRafaelia` | inferência/backends GGML; performance exige `llama-bench` |
| `RafPolimata` | evidência, Build Doctor, receipts e claim boundary |
| `GAIA_phi` | contexto/API/session routing; não substitui evidência física |
| `rafaeliaprivate` | `TOKEN_VAZIO_REPOSITORY_NOT_FOUND` até resolver o nome exato |

## Verbose

```sh
python3 scripts/runtime_doctor_agent.py \
  --workspace .. \
  --symptom gpu \
  --symptom llama \
  --verbose
```

Sem `--execute-probes`, o agente apenas constrói o mapa e mantém:

```text
runtime_execution = TOKEN_VAZIO_NOT_EXECUTED
```

Para executar somente probes allowlisted e read-only:

```sh
python3 scripts/runtime_doctor_agent.py \
  --workspace .. \
  --symptom termux \
  --symptom gpu \
  --symptom llama \
  --execute-probes \
  --verbose \
  --json-out artifacts/runtime-doctor/report.json \
  --markdown-out artifacts/runtime-doctor/report.md
```

## Repositórios fora da árvore padrão

Podem ser injetados explicitamente:

```sh
python3 scripts/runtime_doctor_agent.py \
  --repo termux-app-rafacodephi=/caminho/termux-app-rafacodephi \
  --repo Vectras-VM-Android=/caminho/Vectras-VM-Android \
  --repo llamaRafaelia=/caminho/llamaRafaelia \
  --execute-probes
```

## Aprendizado evolutivo

O agente não reescreve seus próprios algoritmos. O aprendizado V1 é um **peso evidencial append-only**.

Outcome:

```json
{"timestamp":"...","skill_id":"llama_backend_doctor","result":"PASS"}
```

Registro:

```sh
python3 scripts/runtime_doctor_agent.py \
  --append-outcome artifacts/runtime-doctor/outcomes.jsonl \
  --outcome llama_backend_doctor=PASS
```

Consulta futura:

```sh
python3 scripts/runtime_doctor_agent.py \
  --history artifacts/runtime-doctor/outcomes.jsonl \
  --symptom llama
```

O peso usa suavização de Laplace somente quando existem outcomes `PASS/FAIL`. Sem histórico:

```text
TOKEN_VAZIO_NO_OUTCOME_HISTORY
```

Assim, repetição sem evidência não vira confiança.

## Rotas GPU/LLM

### Vulkan observado

```text
Vulkan loader observado
→ LLAMA_VULKAN_CANDIDATE
→ build GGML_VULKAN=ON
→ llama-bench CPU × Vulkan
→ receipt
→ somente então decisão de backend
```

### OpenCL observado

```text
OpenCL presente
→ OPENCL_GENERAL_GPU_CANDIDATE
→ kernel mínimo
→ receipt
```

Presença de OpenCL **não** promove automaticamente um backend LLaMA.

### Vulkan ausente

```text
Vulkan ausente
→ LLAMA_CPU_FALLBACK
→ NEON quando observado
→ benchmark
```

## VM

```text
Termux host
→ Vectras runtime
→ QEMU route
→ guest boot
→ logs/exit code/hash
→ dedicated guest-boot receipt
```

Compilar QEMU não prova guest boot. Abrir Vectras não prova estabilidade da VM.

## Segurança

Por padrão:

```yaml
mode: READ_ONLY_DIAGNOSTIC
automatic_repair: false
automatic_install: false
automatic_delete: false
claim_allowed: false
```

Os probes aceitos são comandos explícitos e não podem conter `rm`, `sudo`, `su`, instalação de pacotes, `git push`, `git reset` ou `git clean`.

## Falsificadores do agente

- probe retorna exit code não zero;
- probe diz PASS mas não entrega JSON esperado;
- biblioteca Vulkan/OpenCL existe, mas backend mínimo falha;
- benchmark GPU é pior ou instável contra CPU;
- ABI detectada diverge do artefato carregado;
- VM não chega ao guest boot observável;
- histórico de skill diverge dos receipts;
- rota aponta para repositório inexistente;
- contexto de GAIA tenta promover claim de hardware sem receipt.

## Estado V1

```text
F_ok   = registry + agent + schema + testes + verbose + learning ledger
F_gap  = execução nos clones físicos completos e benchmark/guest-boot reais
F_next = executar no Termux real, congelar receipt e só depois promover rotas vencedoras
```

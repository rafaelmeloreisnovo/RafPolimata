# CONVERGÊNCIA ECOSSISTEMA RMR/RAFAELIA — segunda rodada (llamaRafaelia / GAIA_phi / CONVERSATIONS_CHUNKS_PRIVATE / ChipQuantum / BLAKE3)

Data: 2026-07-03
Branch alvo: `claude/module-structure-docs-4hwasf`
Status: `AUDITABLE_SYNTHESIS_SEED`
Regra: nenhuma afirmação técnica é promovida a prova sem arquivo, linha, comando, hash, teste ou artefato.

> Este documento é a continuação de `docs/CONVERGENCIA_UNICA_METODOLOGICA.md`
> (que cobriu `RafPolimata` × `ChipQuantum` × `Vectras-VM-Android`). Ele não
> substitui aquele documento — estende a mesma metodologia auditável para
> quatro repositórios adicionais do mesmo autor: `llamaRafaelia`, `GAIA_phi`,
> `CONVERSATIONS_CHUNKS_PRIVATE` e `BLAKE3`, e reconcilia divergências de
> nomenclatura encontradas entre eles. **Esta rodada é só mapeamento/
> documentação — nenhum módulo de código compartilhado é criado aqui.**

---

## 1. Tese curta

O autor já vinha construindo, em paralelo e em sessões de IA distintas, uma
"federação" mental entre repositórios — mas de forma fragmentada: cada repo
nomeia um subconjunto diferente de irmãos, às vezes com nomes diferentes do
nome real no GitHub, e nenhum repositório até esta rodada tinha uma tabela
única cruzando todos. Este documento consolida o que já existe como
evidência verificável (arquivo + trecho), não o que é aspiracional.

```text
metáfora pedagógica ≠ mecanismo comprovado
citação cruzada em doc ≠ integração de código
nome informal (ex.: "rafaelia_private") ≠ identidade de repositório confirmada
TOKEN_VAZIO > inferência sem origem
```

---

## 2. Tabela de evidência — repositórios investigados nesta rodada

| Família | Repositório | Papel observado | Evidência concreta | Estado | Refs cruzadas já existentes no próprio repo |
|---|---|---|---|---|---|
| llamaRafaelia | `rafaelmeloreisnovo/llamaRafaelia` | Fork do llama.cpp + camada forense `rmrCti/` (scanner determinístico) + camada freestanding `rafaelia-baremetal/` (núcleo cognitivo RAFAELIA) | `rmrCti/rafa_cti_scan.c`, `rmrCti/triad_cti_couple.py`, `rmrCti/omega_nav.c`, `rafaelia-baremetal/rafaelia/raf_rafaelia_core.c`, `docs/RAFAELIA_VECTRA_LAMA_CONNECTOR_BRIDGE.md` | `PRESENT_WITH_INTERNAL_BRIDGE_DOC` | Cita `Vectras-VM-Android`, `BLAKE3` ("Blacktrain"), RLL. **Não cita** `RafPolimata`, `GAIA_phi`, `CONVERSATIONS_CHUNKS_PRIVATE`. |
| GAIA_phi | `rafaelmeloreisnovo/GAIA_phi` | Autodeclarado "hub central de federação" do ecossistema RMR/Rafaelia | `FEDERATION.md`, `federation/absorb/manifest.json`, `ARCHITECTURE.md` | `PRESENT_AS_SELF_DECLARED_FEDERATION_HUB` | Federa `chipquantum`, `rafaelia_private`, `llamaRafaelia` — nomes próprios, não os nomes exatos do GitHub. |
| CONVERSATIONS_CHUNKS_PRIVATE | `rafaelmeloreisnovo/CONVERSATIONS_CHUNKS_PRIVATE` | Chunks de conversas + pilha Python "RAFAELIA" evoluindo (raf_core v8→v11_hybrid, raf_omni, raf_evolve) | `memory_bridge/maps/RMRCTI_INTEGRATION.md`, `memory_bridge/maps/RMRCTI_BRANCH_STATUS.md`, `README.md` | `PRESENT_WITH_EXPLICIT_UPSTREAM_POINTER` | Identifica `llamaRafaelia/rmrCti/` (branch `codex/reconcile-rmrcti-docs-exec-v2`) como fonte canônica de scanning. Com alta confiança, é o repo que GAIA_phi chama de `rafaelia_private` (ver seção 3). |
| ChipQuantum | `rafaelmeloreisnovo/ChipQuantum` | Motor criptográfico freestanding (AES/ChaCha20/X25519) + `GeoLM` (modelo geométrico vetorial) + pipeline `raf_omega` (42 estágios) | `docs/RAFAELIA_INVARIANTE_SUSTENTACAO_CONTINUA.md`, `rafael.md:335`, `raf_aes128.h`, `raf_omega.c`, `RMR/docs/ARCHITECTURE.md` | `PRESENT_AS_MOST_COMPLETE_EXISTING_MAP` | Já lista `ChipQuantum`, `Vectras-VM-Android`, `RafPolimata`, `GAIA_phi`, `qemu_rafaelia`, RLL, e cruza explicitamente para `llamaRafaelia/rmrCti/` (`rafael.md:335`). Ver seção 4. |
| BLAKE3 | `rafaelmeloreisnovo/BLAKE3` | Fork oficial do BLAKE3-team + camada isolada `rmr/` ("Blacktrain": cadeia de custódia/hash de artefatos de build, não nova criptografia) | `FORK_NOTES.md`, `rmr/PROVENIENCE.md`, `docs/RAFAELIA_BLACKTRAIN_READY_STATUS.md` | `PRESENT_AS_ISOLATED_CUSTODY_LAYER_NO_SIBLING_LINKS` | Não referencia nenhum irmão, apesar de ser citado pelo connector bridge do llamaRafaelia e pela lista de licenças do próprio RafPolimata (`docs/LICENCAS_COMPARADAS.md`). |
| RafPolimata | `rafaelmeloreisnovo/RafPolimata` | APKc freestanding + motor cognitivo `verbovivo` (Fiber-H + T^7 toroid) | `docs/CONVERGENCIA_UNICA_METODOLOGICA.md`, `rafaelia/verbovivo.c`, `Apkc/coherence.h` | `PRESENT_WITH_GAPS` (já classificado na rodada anterior) | Cobre `ChipQuantum`/`Vectras-VM-Android`. **Não é citado por nome em nenhum dos outros 5 repos** (gap simétrico — ver seção 3). |

---

## 3. Divergências de nomenclatura a reconciliar

| Nome informal encontrado | Onde aparece | Identidade real (GitHub) | Confiança |
|---|---|---|---|
| `rafaelia_private` | `GAIA_phi/federation/absorb/manifest.json` | `CONVERSATIONS_CHUNKS_PRIVATE` | Alta — ambos descrevem o mesmo papel (chunks/memória de conversas + pilha RAFAELIA), mas nenhum arquivo cita o hash/URL exato do outro repo para confirmar 1:1. Tratar como `AUDIT`, não `PASS`, até um dos dois repos citar o nome GitHub exato do outro. |
| `chipquantum` (minúsculo) | `GAIA_phi/federation/absorb/manifest.json` | `ChipQuantum` | Alta — mesmo padrão de nome, só capitalização. |
| `GAIA-BBS` | `RafPolimata/docs/RAFAELIA_MORPHCORE_OPERATION_PROTOCOL.md:39` | Provavelmente `GAIA_phi`, mas pode também aludir ao estilo "BBS" do `omega_nav.c` do llamaRafaelia (`rmrCti/` é descrito como "navegar conversas como um BBS antigo") | Baixa — `TOKEN_VAZIO` até algum repo confirmar a que "GAIA-BBS" se refere. |
| `qemu_rafaelia` | Citado em `ChipQuantum/docs/RAFAELIA_INVARIANTE_SUSTENTACAO_CONTINUA.md` e em `RafPolimata/docs/RAFAELIA_MORPHCORE_OPERATION_PROTOCOL.md:39` | Nenhum repositório com esse nome está no escopo desta sessão | `VOID` — repo citado por dois irmãos mas não investigável agora. |
| `RafPolimata` ausente | Nenhum dos 5 repos investigados nesta rodada | `RafPolimata` (este repositório) | Gap confirmado — nenhuma menção pelo nome em `llamaRafaelia`, `GAIA_phi`, `CONVERSATIONS_CHUNKS_PRIVATE` ou `BLAKE3`; `ChipQuantum` é o único que já o lista (`docs/RAFAELIA_INVARIANTE_SUSTENTACAO_CONTINUA.md`). |

---

## 4. ChipQuantum como o mapa mais completo já existente

`ChipQuantum/docs/RAFAELIA_INVARIANTE_SUSTENTACAO_CONTINUA.md` já mantém uma
tabela multi-repo (`ChipQuantum`, `Vectras-VM-Android`, `RafPolimata`,
`GAIA_phi`, `qemu_rafaelia`, RLL) e `ChipQuantum/rafael.md:335` cruza
explicitamente para `llamaRafaelia/rmrCti/ (CTI runtime)`. Isso faz do
ChipQuantum o único repositório, dos seis investigados, que já tinha
consciência de mais de dois irmãos ao mesmo tempo. Recomendação: manter este
documento e `ChipQuantum/docs/RAFAELIA_INVARIANTE_SUSTENTACAO_CONTINUA.md`
sincronizados manualmente em rodadas futuras — nenhuma automação de sync foi
criada nesta rodada (`PENDING`).

---

## 5. Costuras de módulo candidatas (descrição, não implementação)

Nenhuma destas costuras é implementada nesta rodada. Elas são registradas
como candidatas `PENDING` para uma futura etapa de criação de módulo real,
conforme pedido do autor ("primeiro a documentação... depois a criação de
módulos").

### 5.1. Família RMR duplicada (5 implementações independentes)

| Repositório | Implementação local do conceito "RMR" | Papel |
|---|---|---|
| llamaRafaelia | `rmrCti/` (scanner C + navegador OMEGA) | Forense/CTI sobre exports brutos |
| CONVERSATIONS_CHUNKS_PRIVATE | `memory_bridge/` | Ponte de memória comprimida entre sessões |
| GAIA_phi | `federation/` + `rmr/` | Federação/absorção de outros repos |
| ChipQuantum | `RMR/` (`rmr_app`) | Catalogação/ingestão de material conceitual (`Outro/*.txt`) via SHA-256 |
| BLAKE3 | `rmr/` ("Blacktrain") | Cadeia de custódia de build/benchmark do binário BLAKE3 |

Todas usam vocabulário parecido (hash, custódia, manifesto, auditoria) mas
**não compartilham nenhuma interface, header ou biblioteca comum** hoje.
Candidato a uma futura especificação `rmr-core` (formato de manifesto único,
esquema de hash comum) — não construído nesta rodada.

### 5.2. Família HDC / hipervetor / toroidal duplicada

| Repositório | Implementação | Linguagem |
|---|---|---|
| RafPolimata | `rafaelia/verbovivo.c` (Fiber-H 256-bit + T^7 toroid, 1024-dim HDC) | C |
| GAIA_phi | `rafaelia_hyper_core_v27.py` → `rafaelia_omni_core_v107.py` → `rafaelia_trinity_core_v200.py` (`class HyperVectorMath`) | Python |
| ChipQuantum | `src/toroidal_engine.py`, `src/toroidal_engine_baremetal.py`, `geolm.c` | Python + C |
| CONVERSATIONS_CHUNKS_PRIVATE | `Matem-tica-/docs/SEMENTES_MATEMATICAS_IMPORTADAS.md` ("espaço toroidal T^7"), `corr/gaia_vec_build.c` | Markdown/C |

Quatro implementações independentes do mesmo conceito matemático, em
linguagens diferentes, sem spec compartilhada. `PENDING`.

### 5.3. `phi_ethica` — duplicação verificável e mais concreta

A fórmula `phi_ethica = (1 − H) × C` aparece de forma quase idêntica em:

- `Apkc/coherence.h` (RafPolimata) — `phi_fst()`, Q16 fixed-point.
- `ChipQuantum/rafaelia_torus_hex.c` e `ChipQuantum/RAFAELIA_OMEGA.sh`.

Esta é a costura mais concreta e menos especulativa das três — mesma
fórmula, dois repositórios, já auditável linha a linha. Candidata natural a
um header `phi_ethica.h` compartilhado numa etapa futura de módulo, mas
**não extraído nesta rodada**.

---

## 6. Próximos passos (explicitamente fora desta rodada)

- Registrar em `llamaRafaelia/docs/RAFAELIA_VECTRA_LAMA_CONNECTOR_BRIDGE.md`
  uma entrada para `RafPolimata`, `GAIA_phi` e `CONVERSATIONS_CHUNKS_PRIVATE`
  (hoje ausentes) — documentação, feita nesta mesma rodada via ponteiro
  (ver `docs/rafaelia/ECOSSISTEMA_RMR_RAFAELIA_PONTEIRO.md` naquele repo).
- Decidir se `rafaelia_private` (GAIA_phi) e `CONVERSATIONS_CHUNKS_PRIVATE`
  são de fato o mesmo repositório ou dois momentos distintos do mesmo
  projeto — hoje é `AUDIT`, não `PASS`.
- Avaliar criação de uma spec `rmr-core` comum (seção 5.1) — não iniciar sem
  medir o custo de migrar as 5 implementações existentes.
- Avaliar extração de `phi_ethica.h` compartilhado entre RafPolimata e
  ChipQuantum (seção 5.3) — candidata mais madura para uma primeira PR de
  módulo real, por ter menor superfície e maior sobreposição comprovada.
- Investigar `qemu_rafaelia`, `Vectras-VM-Android`, RLL, `ZIPRAF_OMEGA_FULL`
  e `MemRafcode` numa rodada futura — hoje `VOID`/fora de escopo.

FIAT LUX — ΣΩΔΦBITRAF

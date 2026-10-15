# BITRAF — Ponte entre carrier lógico, leitura física e memória multidimensional (V1)

**Estado:** `DRAFT / SOURCE_SPEC_ONLY / NO_PROMOTE / claim_allowed=false`  
**Data:** 2026-10-15  
**Autoridade:** RafPolimata — carrier de estados e composição low-level; não é instrumento de NAND/DRAM nem autoridade de claims científicos.

## Fronteira existente

A fonte `rafaelia/bitraf_matrix.h` implementa `S=(x,y,z,v,p)`, com `10^3×4×2=8000` estados e um núcleo `8^3×4×2=4096`. Esse endereçamento geométrico é um **carrier lógico determinístico**. Não é tradução bijetiva de bloco lógico de arquivo, FTL, setor NAND, célula, posição física ou estado quântico; mapa de conversão ausente permanece `TOKEN_VAZIO_PHYSICAL_ADDRESS`.

`INDEX8000 != FLASH_PBA != FILE_OFFSET != BIT_READING != ENTROPY`.

## Porta candidata de observação, não alteração do carrier

Uma leitura externa poderá produzir `R=(source,device_class,logical_block,physical_binding,t_monotonic,observer,expected,observed,error_kind,noise_kind,latency,temperature,voltage,integrity,ecc_syndrome,context,provenance)`, com campos ausentes tipados. O adaptador opcional deve passar uma cópia imutável com ID e epoch à projeção BITRAF e retornar `relation_id`; sem reinferir origem física por geometria.

Distinções de domínio: `ZERO`, `TOKEN_VAZIO`, `BIT_FLIP`, `ERASURE_KNOWN_POSITION`, `OMISSION`, `DETECTED_NOT_CORRECTED`, `CORRECTED_VERIFIED`, `DEGRADED`, `ANCHOR_LOGICAL`, `FINGERPRINT_CANDIDATE` e `ENTROPY_CANDIDATE`. Uma mesma amostra pode ter vários papéis, sem que rótulo contraditório vire resultado validado.

## L/O/T e operadores

- L: repetição em janela/epoch, drift e taxas por endereço sem apagar estados antigos.
- O: leituras realmente independentes, diagnósticos ECC, controle de cache; espelho do mesmo log não conta.
- T: geometria ↔ ECC ↔ armazenamento ↔ material ↔ energia ↔ áudio somente com ponte/unidades.
- Derivada discreta: delta entre séries comparáveis; antiderivada: acumulação com estado inicial, sem perda de histórico; inversa: conjunto de predecessores quando projeção não injetiva.
- Latência: registrar acesso, timeout, jitter, possível benefício do atraso versus custo e deadline; sem custo medido = `TOKEN_VAZIO_UTILITY`.

## Gates negativos antes de qualquer integração

1. Enumerar reversibilidade e relações do carrier original sem alteração; testes do HEAD vigente.
2. Mutar um único campo: detectar identidade alterada, sem colapsar `UNKNOWN` para zero.
3. Proibir `physical_address_evidence=true` quando só há endereço lógico.
4. Testar estabilidade de âncora e variabilidade temporal como conceitos separados; variação não é TRNG certificado.
5. Simular cache/remapeamento, bit flip, omissão e posição de erasure; não "recuperar" sem ECC/digest.
6. Contar custos de memória/latência do adaptador e realizar A/B com baseline em escopo conhecido.
7. Manter **falsificador source-side**, `CI_HEAD`, `DEVICE`, `INDEPENDENT_REVIEW`, `LICENSE` independentes.

**F_ok:** contrato e pontes definidas; **F_gap:** adaptador/execução/telemetria/entropia não implementados aqui; **F_next:** fixture read-only source-side separando carrier de observação física. Nenhum código/bitraf existente foi substituído.

## Custódia das lacunas: L11 e L2

`CLOSURE_L11` em `docs/closures/CLOSURE_L11_OPERATIONAL_GAP_TOPOLOGY.md` rege o **registro e o roteamento** de `TOKEN_VAZIO_PHYSICAL_ADDRESS`, `TOKEN_VAZIO_UTILITY` e da tipagem de bit flip/erasure/omissão. Esse contrato não converte os vazios em `0`, `PASS` ou previsão física.

`CLOSURE_L2` em `docs/closures/CLOSURE_L2_RUNTIME_EVIDENCE.md` governa o **método** de obtenção de recibos Android; o experimento BITRAF desta PR não executou essa captura. No HEAD da PR, runtime físico, revisão independente, autorização de licença e aplicação de proteção GitHub permanecem não atestados. A referência a closure significa responsabilidade rastreável, jamais fechamento material dos gaps.

# RafPolimata — L0 intake de charts, grafos e fórmulas via NOVOexport

STATUS: **DOCUMENTED_INTERFACE_ONLY / NÃO PROMOVER / claim_allowed=false**.
Autoridade de origem: Drive privado e produtor de cada dado/teoria. Mapa
PR (NOVOexport chart-route audit) gere apenas projeção pública sanitizada.
Este contrato é novo, sem fonte privada, parser ou executável transplantados.

## Fronteiras do build

- L0 autoral = C freestanding, estado e buffers do chamador, sem
  libc/stdint.h/math.h/malloc/exceções/OS/JNI/Java/syscalls/extern nativo.
- `void` não é estratégia de otimização: contratos de retorno tipados e
  `static inline` devem preservar erro/estado observável. Comentários não
  são pass de compilador. Usar flags/objdump/nm e gate de symbols.
- OOXML zip, Drive API, parser XML e plotter de UI são adapters hospedados
  de **RafGitTools/Termux**, não devem contaminar `freestanding/`.
- Entrada futura L0 de gráfico seria uma `ChartSeriesView` de bytes
  já validados, com comprimento, tipo escalar, origem opaca, unidade, escala,
  missingness explícita, política de arredondamento e limite overflow.
  A implementação **não existe neste PR**: `TOKEN_VAZIO_INTERFACE_IMPLEMENTED`.
- Separar `SOURCE_RECEIPT`, `MODEL_COMPUTATION`, `PLOT_SPEC`, `RENDER`,
  `RUNTIME_MEASUREMENT` e `SCIENCE_CLAIM`. Ao gerar plot não gerar prova.
- Toda saída porta `claim_allowed=false` até gates de revisão e runtime.

## Fonte de referência consultada

O Sheets `Formula & Calculation Memory Index Ω` possui abas
`FORMULAS`, `GRAPH_NAMESPACE_CROSSWALK_V2_20260915`,
`JUNCTION_MATRIX_V1_20260915`, entre outras. A entrada
`κ=√3/2` tipa constante real, `S(d)=κ^d` tipa domínio contínuo e
`r_(n+1)=κ*r_n` tipa recorrência discreta. São espécies distintas.
Exigir `domain`, `assumptions`, `derivative_operator`,
`inverse_conditions`, `singularity` e `source_identity` antes de
aceitar transformação matemática. Expressões não numéricas/sem
condições vão para `TOKEN_VAZIO_NUMERIC_ENCODING`, não para zero.

A matriz distingue:
- C42(±6,±7): V42 E84 grau4;
- Icosphere f=2: V42 E120 F80 Euler2, graus 5/6;
- graph zone physical: 137 janelas, zona parcial final;
- graph zone metrics histórico: 136 registros; isso não mede todas
  as 137 janelas por identidade;
- BitGraph: ObjectID→Digest→PayloadSpan→BlockOffset→ModuleEdge→Epoch;
- grafo semântico: REL/CORR/INT/CTX/RAPPORT, evidência weight de
  RAPPORT permanece zero.

**Proibido**: colapsar pelo mesmo literal `42`, tratar `∆` runtime
como discriminante, `φ` razão áurea como `Φ_ethica` ou
Poincaré.disk como Poincaré.return.

## Alvos falsificadores antes do primeiro código

1. Dois grafos com V=42 e E diferentes não passam `ISOMORPHIC`.
2. Evento `USER_INPUT` não vira `ASSISTANT_OUTPUT`.
3. `UNKNOWN/TOKEN_VAZIO` preserva tipo e não vira 0.
4. Eixo de tempo, unidade, escala e domínio incompatíveis falham.
5. Roundtrip fonte→série→reconstrução preserva valores/ordem e hash
   dos bytes autorizados, sem publicar material privado.
6. Tipos `OBSERVED`, `MODELLED`, `SMOKE`, `PHYSICAL`, `SCIENTIFIC`
   não se promovem uns aos outros por proximidade.
7. Se input truncado/série sem workbook, barrar cálculo como dado real.
8. Compilar L0 com flags freestanding para x86/ARM/RISC-V; verificar
   símbolo indefinido vazio e sem caminho de syscall.
9. RLL robust/fisica permanece BLOQUEADA sem parâmetros/priors,
   covariâncias, comparadores LCDM/CPL, backend externo e reprodução.
10. Reprodução independente não pode herdar oracle do código candidato.

## Protocolos futuros — nenhum implícito

`RafGitTools/private input adapter → typed borrowed view →
RafPolimata/source-blind deterministic kernel → typed result →
RafGitTools/private sealed receipt → Mapa/sanitized projection`

Dependências: revisão de direitos por arquivo, source origin, sample de
um chart original com workbook, sem dados privados na PR, aceite explícito
do usuário, freestanding/cross compiler gate exact-head e dispositivo
independente quando alegado.

R3: F_ok = revisão de contrato e matriz concreta identificada no Drive.
F_gap = interface implementada, dataset original e prova source-side
`TOKEN_VAZIO`. F_next = primeiro test harness sintético e mínimo
semantical oracle, a partir de licença e fonte liberadas.
Rollback: fechar PR draft. Histórico intacto.

## S2 — integração da única interação lógica Ω (sem colapso de fonte)

O produtor RafPolimata é a autoridade de **matemática tipada**, núcleo
freestanding e falsificadores locais, enquanto o roteador público de
sessão é `Mapa:data/assurance/rafaelia-unified-session-audit.v1.json`
(PR #752 DRAFT). A matriz possui 26 famílias F01..F26, 25 lacunas
G001..G025, 55 nós, 99 arestas **somente de roteamento**.

Famílias produtoras relevantes: F03 geometria topológica, F04 zonas,
F05 BitGraph, F10 gráficos RLL, F11 conjuntos/vazios, F12 fórmulas,
F18 L0/ELF/DEX/NUMA/JMV, F21 replay/watchdogs, F26 validação acadêmica.

G007 equivalência, G015 dispositivo e G021 replay permanecem
`TOKEN_VAZIO` mesmo que o roteador passe seu CI. O núcleo **não**
deve transformar `NUMERIC_ZERO`, `EMPTY_SET`, `NULL_VALUE`,
`UNDEFINED`, `NO_REAL_SOLUTION`, `MISSING_OBSERVATION`,
`NOT_APPLICABLE`, `APPROX_ZERO`, `RESIDUE_ZERO` e
`TOKEN_VAZIO` num mesmo número: o discriminante e o motivo
seguem preservados.

Máquina de estados recomendada para um futuro contrato L0:
`OPEN → STRUCTURALLY_BOUND_CANDIDATE → EXTERNAL_PROVIDER_REVIEW`.
O segundo estado **não é PASS**. Cada novo estado deve ser
impressão separada (predecessor + nova evidência), não reescrita da
fonte. Um `uint64` preenchido, mesmo não nulo, não autentica
uma pessoa, um dispositivo ou uma licença.

Pendente: tentativa de escrita de fonte C desta rodada bloqueada pela
ferramenta; `TOKEN_VAZIO_SOURCE_PATCH`. Este documento não substitui
código, compilação nem teste no PR. Sem mudança de kernel ou adaptação
hosted nesta etapa. Reabrir implementação só com autoridade e CI
source-side reproduzível, em PR isolado e reversível.

## S3 — contraprova de proveniência observada no Drive

A busca global do Drive retornou charts que, após inspeção de
`parent_ids` até seus ancestrais, **não pertencem ao ramo NOVOexport
verificado**. A associação anterior era inferência indevida e fica
explicitamente corrigida, com predecessor preservado.

| Família | Ancestral observado | Regra de admissão |
| --- | --- | --- |
| XML Office chart1..6 | `metasploit-framework/data/exploits/CVE-2013-3906/word/charts` | Amostra de fixture de segurança de terceiros, jamais fonte científica/autorais sem novo vínculo |
| Dart `charts/` | `dev/integration_tests/new_gallery/lib/studies/rally` | Exemplo upstream Flutter UI, não motor matemático RAFAELIA |
| Graphviz DOT/PNG | `RAFAELIA_UPLOAD/DIAGRAMAS` | Diagrama roteável, autoria/execução não demonstradas |

**Escopo da observação:** metadados e caminho de pastas do provedor,
não análise jurídica/licença, abertura segura do pacote OOXML original,
prova criptográfica completa ou inspeção de conteúdo executável.

**Segurança:** não abrir/executar payload CVE, não processar macro,
externalData, script ou extração automática fora de sandbox de origem.
É proibido copiar a fixture de terceiros para implementação autoral.
Sem consentimento e direitos, G011/G012 permanecem abertos.
As seis amostras são **out-of-scope de NOVOexport** salvo novo
`source_ref` independente e verificável. `GLOBAL_SEARCH !=
NOVOEXPORT_MEMBERSHIP`.

## S4 — retomada da implementação efetiva de vazio tipado

A tentativa anterior de escrever código L0 foi bloqueada por ferramenta;
o histórico permanece. Nesta retomada, uma implementação C freestanding
nova foi criada em `freestanding/include/raf_fs_typed_gap.h` com:
10 discriminantes independentes; inicialização de lacuna somente para
`TOKEN_VAZIO`; vínculo de evidência **CANDIDATE**, nunca estado
`VERIFIED` ou `PASS`; predecessor intocado; rejeição de
referências nulas/iguais, alias in-place e estados inválidos.

`freestanding/tests/raf_fs_typed_gap_test.c` cobre 33 verificações,
`verify_typed_gap.sh` executa host -O0/-O2/-O3, aplica contrato L0,
compila x86_64, i686, ARMv7, AArch64, RV32 e RV64 e inspeciona
`nm -u`. `.github/workflows/freestanding-typed-gap.yml` roda em PR.
Primeiro readback de CI: run 38190911220 SUCCESS; zero undefined nos seis
objetos, sem prova de dispositivo, assinatura ou observação externa.

`claim_allowed=false`: direitos raiz, revisão independente e execução
real continuam P0. Hash/IDs recebidos do chamador são opacos e **não**
autenticados pela primitiva. G015/ G021 permanecem vazios para evidência
externa e replay independente.


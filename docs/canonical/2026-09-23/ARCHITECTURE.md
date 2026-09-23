# Arquitetura — visão navegável

**Base:** main@f22efc099ac530d946ff2ec34954455f75632e92  
**Escopo:** relações observáveis entre os principais planos do RafPolimata.

## 1. Fluxo conceitual

~~~mermaid
flowchart LR
    S[Source] --> F[Frontend / parser / dispatch]
    F --> IR[RAF Semantic IR / canonical operation]
    IR --> K[Strict kernel / freestanding C or ISA]
    K --> A[Architecture backend]
    A --> ART[Artifact: object ELF DEX APK report]
    ART --> EX[Execution]
    EX --> EV[Evidence / receipt]
    EV --> CL[Claim gate]
    CL -->|sufficient| P[Promotable claim]
    CL -->|insufficient| TV[TOKEN_VAZIO / gap]
~~~

A seta indica rota de trabalho, não que todas as etapas estejam implementadas para todo frontend.

## 2. Planos separados

~~~mermaid
flowchart TB
    DOC[Documentation and governance]
    CI[GitHub Actions / gates]
    TOOL[Python and shell orchestration]
    APKC[ApkC language dispatch and Android path]
    SEM[Semantic IR / registries]
    FS[Freestanding L0]
    SYS[Optional syscall layer]
    ARCH[ISA / SIMD / scalable / matrix profiles]
    EVID[Proofs evidence receipts]

    DOC --> CI
    DOC --> EVID
    CI --> TOOL
    CI --> APKC
    CI --> SEM
    CI --> FS
    APKC --> SEM
    APKC --> FS
    SEM --> FS
    FS --> ARCH
    SYS -. separate .-> FS
    ARCH --> EVID
    APKC --> EVID
    SEM --> EVID
~~~

## 3. Fronteiras críticas

### Freestanding L0

O contrato em freestanding/ exige, entre outras invariantes:
- sem syscall no L0;
- sem heap/GC;
- sem CRT/libc hospedado sob responsabilidade do núcleo;
- estado pertencente ao caller;
- residual lanes explícitas;
- instruções de arquitetura isoladas.

O diretório syscall/ é uma camada opcional e não deve ser usado para reclassificar L0 como dependente de OS.

### ApkC

Apkc/ mantém dispatch de linguagens, rotas Android/DEX/ELF/ZIP, validação e provas. Reconhecer uma extensão ou chamar um compilador externo não prova que a linguagem está concluída no perfil estrito.

### Semantic IR

A documentação existente separa semântica do programa da ISA. O objetivo arquitetural é permitir que frontends semanticamente equivalentes convirjam para uma representação canônica antes da seleção de backend.

### Evidência

Provas são revision-bound. Um output histórico pode demonstrar um evento passado sem demonstrar o estado de main.

## 4. Relação com o ecossistema

Conforme o router longitudinal usado neste corte:

~~~text
Drive       -> memória documental, índice, corpus e receipts longitudinais
GitHub      -> código, schema, spec, testes, CI e docs próximas ao código
RafPolimata -> normalização, validação, proof-runs, relatórios e evidência
RafGitTools -> plano de controle, jobs, gates e eventos
RLL         -> claims científicos/cosmológicos e seus falsificadores
Papers      -> síntese publicável
Termux      -> runtime local Android quando autorizado e observado
~~~

Essas são fronteiras de autoridade, não dependências obrigatórias de build.

## 5. Escala de maturidade por objeto

~~~text
DECLARED
  -> SOURCE_PRESENT
  -> STATIC_VALIDATED
  -> BUILT
  -> EXECUTED
  -> RECEIPT_BOUND
  -> REPRODUCED
  -> CLAIM_ELIGIBLE
~~~

Cada transição exige sua própria evidência. Pular uma etapa por inferência é proibido.

## 6. Anti-regressão

- source presence não vira runtime;
- codegen não vira device proof;
- CI verde não vira certificação;
- cross-implementation não vira provider independence;
- hash de bytes não prova verdade semântica;
- documento generated não é atualizado manualmente.

R3 = ⟨F_ok: planos e fronteiras explicitados; F_gap: cobertura de backend/runtime continua por componente; F_next: usar a matriz de linguagens e os gates como ponte para evidência por revisão⟩.

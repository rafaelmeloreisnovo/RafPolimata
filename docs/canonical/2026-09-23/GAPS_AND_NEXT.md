# Gaps observados e próximos gates

**Governance binding: CLOSURE_L11** — explicit unknown-state markers are governed by the operational gap topology closure; the binding does not promote the underlying gap.

**Base:** main@f22efc099ac530d946ff2ec34954455f75632e92  
**Regra:** gap documental não é automaticamente gap de implementação; gap de implementação não é automaticamente falha.

## P0 — verdade corrente e gates

### GAP-DOC-BASE-001 — routers datados estavam atrás de main
Estado antes deste corte: README e docs/INDEX roteavam 9ed0b8aa... de 2026-09-06 enquanto main observado é f22efc09....

Fechamento deste corte: nova camada canônica ligada a f22efc09..., preservando snapshots históricos.

### GAP-CI-TV-002 — CI principal vermelho
Evidence: run 35574321825, main@f22efc09....

- validator unit tests: 19/19 PASS;
- strict changed-lines scan: 5 errors;
- overall job: FAIL;
- gates posteriores: skipped.

F_next: abrir o artifact/report exato, fechar cada finding com closure válida, rerodar no commit sucessor.

### GAP-DOC-GEN-003 — generated governance freshness
Os outputs de governança committed eram históricos no router anterior.

F_next: executar o gerador oficial + check + tests em revisão exata; nunca corrigir generated manualmente.

## P1 — linguagens e build

### GAP-LANG-KT-001
Perfis Kotlin/DEX existem, mas zero arquivos .kt foram observados no tree corrente.

F_next: somente se Kotlin fizer parte do escopo executável, versionar fixture mínima + gate + receipt.

### GAP-LANG-JAVA-001
Perfil Java/DEX e provas históricas existem, mas zero arquivos .java foram observados no tree corrente.

F_next: versão corrente deve ligar fonte real ao pipeline antes de claim de cobertura.

### GAP-LANG-RUST-001
Existe native/raf_hash_fabric_v1/rust/lib.rs e rust-check via rustc; não foi observado Cargo.toml.

Isso não é necessariamente defeito: o módulo pode deliberadamente usar rustc direto. A decisão deve ser documentada antes de adicionar Cargo.

### GAP-LANG-CPP-001
Existem apenas dois .cpp observados. C++ é real, mas não deve ser descrito como corpus amplo.

## P1 — GitHub/community

### GAP-GH-LICENSE-001
Nenhuma licença reconhecida no nível raiz foi observada. Existe licença específica em native/raf_hash_fabric_v1.

F_next: decisão autoral/jurídica explícita antes de qualquer LICENSE global.

### GAP-GH-METADATA-002
Description, homepage e topics não estavam preenchidos.

F_next: definir metadata curta e factual; isso exige mudança de settings, não só commit.

### GAP-GH-RULES-003
Ruleset atual bloqueia deletion e non-fast-forward, mas review/status-check requirements não foram observados.

F_next: decidir, em mudança administrativa separada, quais checks realmente são required sem bloquear rotas que ainda falham legitimamente.

### GAP-GH-PAGES-004
Pages está desabilitado.

Estado: OPTIONAL. Markdown no Git continua suficiente. Só habilitar Pages se houver objetivo de site/documentação publicada.

### GAP-GH-DISCUSSIONS-005
Discussions está habilitado; categorias/forms não foram auditados.

F_next: configurar apenas se Discussions for usado como canal operacional.

### GAP-GH-RELEASE-006
V1.0.0 é prerelease, sem assets e sem corpo de release.

F_next: antes de nova release, ligar tag, build, checksums, provenance, install/runtime scope e notes.

## P1 — workflows

### GAP-WF-BIB-001
Daily Bibliography Evolution apresentou falhas recorrentes na amostra recente.

F_next: abrir o run recente, identificar primeiro step real de falha e corrigir sem converter erro externo em PASS.

### GAP-WF-CATALOG-002
50 workflows aumentam superfície operacional.

F_next: o catálogo desta camada reduz navegação; eventual consolidação deve ser precedida por grafo de triggers, duplicidade e dependências, não por exclusão por nome.

## P2 — documentação

### GAP-DOC-LINK-001
Este corte não adiciona um link-checker novo.

F_next: após review do padrão documental, considerar lychee/markdown-link-check ou validador próprio, preservando links históricos/permalinks.

### GAP-DOC-ADR-002
Não existe, neste corte, um catálogo ADR consolidado.

F_next: criar ADRs apenas para decisões novas ou decisões históricas com proveniência fechada.

### GAP-DOC-API-003
Não há API reference unificada para todos os módulos.

F_next: gerar por domínio a partir de headers/schemas somente onde a interface seja estável.

## Condição de parada

Não há mais conteúdo factual a promover neste corte sem uma destas ações externas:
- executar gates;
- ler um novo artifact/log específico;
- decidir política jurídica;
- alterar repository settings;
- adicionar implementação;
- reproduzir em device/provider.

Essas ações são o próximo ciclo e não devem ser simuladas por documentação.

R3 = ⟨F_ok: gaps principais tipados por domínio; F_gap: execução/settings/decisões externas permanecem reais; F_next: primeiro fechar GAP-CI-TV-002 e GAP-DOC-GEN-003, depois tratar gaps de linguagem e governança GitHub conforme autoridade⟩.

# Mapa inicial — componentes, provas e arquivos sem rota confirmada

> **Estado:** `BOOTSTRAP_REFERENCE`. Este arquivo é regenerado integralmente por
> `scripts/apkc_first_part_gate.py`; a execução acrescenta SHA-256, tamanho,
> referência em índice e rota para todos os arquivos versionados examinados.

## Núcleos já localizados

| Corpo | Caminhos | Estado | Rota |
|---|---|---|---|
| Compilador APK | `Apkc/apkc.c`, `Apkc/lang_profile.h` | IMPLEMENTED / runtime parcial | manter em `Apkc/`; provar por perfil |
| Assembler ARM | `Apkc/arch_arm32.h`, `Apkc/arch_arm64.h` | IMPLEMENTED / cobertura variável | corpus regressivo por instrução |
| ELF | `Apkc/fmt_elf.h` | IMPLEMENTED estrutural | validar `.so` do mesmo APK/run |
| DEX | `Apkc/fmt_dex.h` | IMPLEMENTED mínimo | separar DEX estrutural de DEX funcional |
| Toolchain Termux | `Apkc/sys.h` | IMPLEMENTED | executar resolução real de clang/java/d8 |
| Java/Groovy | `scripts/apkc_java_to_jar.sh`, `scripts/apkc_groovy_to_jar.sh` | IMPLEMENTED | executar JAR→D8→DEX |
| Prova de build | `tools/raf_source_to_binary_proof.sh` | gate v2 IMPLEMENTED | executar A64+A32 e promover somente completo |
| Provas canônicas | `Apkc/proofs/out/` | CONTRADICTION/TOKEN_VAZIO no corte | substituir por um único run coerente |
| Navegador local | `raf_shell/raf_shell.c` | TUI file browser IMPLEMENTED | não classificar como navegador web |
| Navegador ASM/TLS | origem não localizada | TOKEN_VAZIO | localizar em outro diretório/repositório ou implementar separadamente |
| Gate geral | `scripts/apkc_first_part_gate.py` | IMPLEMENTED | gerar JSON e este mapa |

## Zonas que o gerador examina

```text
raiz do repositório
├── arquivos fora do conjunto canônico
├── docs/ sem referência em índice
├── scripts/ sem referência em workflow/documentação
├── tools/ sem entrada canônica
└── ApkC/ artefatos, fontes e provas sem relação explícita
```

## Rotas produzidas

| Rota | Significado |
|---|---|
| `INDEXED` | arquivo já citado por entrada canônica |
| `ADD_TO_CANONICAL_INDEX` | preservar no diretório e adicionar relação documental |
| `MOVE_OR_INDEX` | decidir destino; não apagar nem mover automaticamente |

## Comando de materialização completa

```sh
python3 scripts/apkc_first_part_gate.py \
  --write results/apkc-first-part-gate.json \
  --write-map docs/generated/REPOSITORY_LOOSE_FILES_MAP.md
```

A versão materializada substitui esta tabela inicial por inventário por arquivo,
com hash e tamanho. Até essa execução, nenhuma contagem total é promovida.

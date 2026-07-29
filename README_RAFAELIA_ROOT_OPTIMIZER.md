# RAFAELIA ROOT C→ASM Optimizer

**Estado:** `ACTIVE`  
**Proprietário lógico:** `compiler-maintainer`  
**Âncora no mapa:** [`docs/MAPA_ESTRUTURAL_REPOSITORIO.md §4 · root-compiler`](docs/MAPA_ESTRUTURAL_REPOSITORIO.md)

Versão com raiz: parser explícito, AST mínimo, IR auditável, scheduler por bloco básico, branchless real para `if/else` simples e backends separados para ARM64 e x86-64.

## Escopo honesto

Este projeto não pretende ser um compilador C completo. Ele opera sobre um subconjunto C-like seguro:

- atribuições simples;
- expressões binárias com `+`, `-`, `*`, `&`, `|`, `^`, `<<`, `>>`;
- `if/else` com comparação simples;
- geração ARM64/AArch64 ou x86-64;
- relatório JSON de auditoria com hashes.

## Uso

```bash
python3 raf_c_to_asm_root_optimizer.py --target arm64 raiz_example.c --audit raiz_audit_arm64.json > raiz_output_arm64.s
python3 raf_c_to_asm_root_optimizer.py --target x86_64 raiz_example.c --audit raiz_audit_x86_64.json > raiz_output_x86_64.asm
```

## Diferença para o protótipo original

- corrige o grafo de dependência;
- não remove `cmp` antes de seleção condicional;
- cria AST antes do IR;
- gera `mov dst, src` antes de `add/sub/mul` quando necessário;
- separa backend ARM64 de x86-64;
- rastreia origem por linha e hash de instrução;
- salva auditoria JSON.

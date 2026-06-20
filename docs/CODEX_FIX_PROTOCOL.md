# Protocolo Codex/IA para Correção de Compilação (S38)

## Objetivo

Definir como um agente de IA (Codex, Claude Code, ou similar) deve diagnosticar
e corrigir erros de compilação neste projeto sem quebrar invariantes de arquitetura.

## Invariantes que nunca podem ser quebradas

Antes de qualquer correção, o agente deve verificar que sua alteração não viola:

1. **Sem malloc/calloc/free em `Apkc/` e hot paths** — freestanding constraint
2. **Sem `#include <stdio.h>` / `<stdlib.h>` em `Apkc/`** — usa `sys.h` com `svc`/`swi`
3. **`lang_profile_find()` deve guardar NULL para nomes desconhecidos** — callers devem checar
4. **Registradores AVR acessados apenas via `RAFA_MMIO8(addr)`** — sem cast direto de ponteiro
5. **TOKEN_VAZIO = return 0 quando hardware ausente** — nunca inventar sucesso

## Fluxo de diagnóstico

### Passo 1: Isolar o erro

```bash
# Compilar arquivo com saída estruturada
gcc -std=c11 -O2 -Wall -Wextra -I. -c RAF_XXX_*.c 2>&1 | \
  grep -E "^.+:[0-9]+:[0-9]+: (error|warning):" | \
  head -20
```

### Passo 2: Categorizar o erro

| Categoria | Padrão de erro | Ação recomendada |
|-----------|---------------|-----------------|
| Tipo incompatível | `error: incompatible type` | Verificar se precisa de cast explícito ou typedef |
| Símbolo não definido | `error: implicit declaration` | Adicionar `#include` correto ou declaração forward |
| Atributo desconhecido | `warning: unknown attribute` | Usar `__attribute__((X))` em vez de C23 `[[X]]` |
| Unused result | `warning: ignoring return value` | `ssize_t n = f(); (void)n;` |
| Signed/unsigned mismatch | `warning: comparison` | Cast explícito ou mudar tipo da variável |
| Unused variable | `warning: unused variable` | `(void)var;` após uso legítimo |
| Missing return | `warning: control reaches end` | Adicionar `return 0;` ou `return TOKEN_VAZIO;` |

### Passo 3: Script de diagnóstico automatizado

```bash
bash scripts/raf_codex_diagnose.sh [arquivo_ou_padrão]
```

Produz saída estruturada adequada para input de IA:
- Lista de erros com categoria
- Contexto de 3 linhas ao redor de cada erro
- Invariantes violadas (se detectadas)

### Passo 4: Verificar que a correção não introduz regressão

```bash
# Freestanding audit (Apkc/ não pode ter libc)
bash scripts/ci_freestanding_audit.sh

# Size regression (cada arquivo < 4096B .text)
bash scripts/raf_binary_size_test.sh

# Baseline comparison
bash scripts/raf_baseline_measure.sh <arquivo_corrigido>
```

## Script de diagnóstico

```bash
#!/usr/bin/env bash
# Uso: bash scripts/raf_codex_diagnose.sh [arquivo.c]
# Se nenhum arquivo for passado, diagnostica todos os RAF_0xx_*.c

FILES=${1:-$(ls RAF_0[0-9][0-9]_*.c 2>/dev/null)}
for f in ${FILES}; do
    echo "=== ${f} ==="
    gcc -std=c11 -O2 -Wall -Wextra -I. -c "${f}" -o /dev/null 2>&1 | \
      grep -E "(error|warning):" | sed 's/^/  /'
done
```

## Padrões de correção aprovados

### AVR register access (M001-M020)
```c
/* CORRETO: via macro RAFA_MMIO8 */
RAFA_MMIO8(AVR_DDRB_ADDR)  |= (uint8_t)(1u << 5);

/* ERRADO: cast de ponteiro direto */
*((volatile uint8_t *)0x24) |= (1 << 5);  /* não faça isso */
```

### Unused result de syscall (M036-M038)
```c
/* CORRETO */
ssize_t n = read(fd, buf, sizeof(buf));
(void)n;

/* ERRADO — dispara -Wunused-result */
(void)read(fd, buf, sizeof(buf));
```

### Return TOKEN_VAZIO (todos)
```c
/* CORRETO — hardware ausente = return 0, nunca -1 para "ausente" */
if (fd < 0) return 0;   /* TOKEN_VAZIO */

/* ERRADO — -1 significa "erro de implementação", não "hardware ausente" */
if (fd < 0) return -1;
```

### GNU SOURCE redefine (M036)
```c
/* CORRETO — guard contra redef */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <sched.h>
```

## Regra de ouro

> Se a correção adiciona mais de 5 linhas ou muda a interface de uma função,
> pare e consulte o humano. Correções de compilação devem ser cirúrgicas.

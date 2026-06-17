# Política de Código Experimental

## Separação entre experimental e validado

Este diretório (`experimental/`) contém código em estado de rascunho, protótipos,
investigações e implementações que ainda não passaram pela validação de cadeia de
custódia do projeto.

## Estados canônicos

O projeto usa os seguintes estados (definidos em `CLAUDE.md`):

| Estado | Significado |
|--------|-------------|
| `VOID` | Placeholder, não implementado |
| `PENDING` | Em progresso |
| `AUDIT` | Precisa verificação contra spec |
| `RUNTIME` | Conhecido apenas em runtime |
| `REFERENCE` | Spec externa (RFC/ISO/ARM ISA/Android ABI) |
| `TOKEN_VAZIO` | Hardware ausente — retorna 0 sem erro, nunca fabrica sucesso |
| `COMPILE_OK` | Compilado limpo; não executável no host por falta de hardware |
| `EXECUTA_PASS` | Executado no host com rc=0 e assert verificado |

## Regras para este diretório

1. **Qualquer arquivo aqui tem estado implícito `PENDING` ou `AUDIT`.**
2. Nenhum arquivo em `experimental/` é importado por código de produção
   (`RAF_0xx_*.c`, `Apkc/`, `rafaelia/`, `Benchmark/`).
3. Para promover código de experimental para validado:
   a. Escrever selftest com assert verificável (EXECUTA_PASS) ou gate de hardware (COMPILE_OK).
   b. Adicionar entrada no `RAF_CHECKLIST_96_ITEMS.md` com ref ao arquivo.
   c. Mover o arquivo para a raiz do projeto ou subdiretório apropriado.
   d. Registrar no `Apkc/proofs/` se o método for parte da cadeia de custódia.
4. CI não compila arquivos em `experimental/` — eles podem ter warnings ou erros.

## Código atualmente em estado AUDIT no projeto principal

Os seguintes arquivos no projeto principal têm seções marcadas como `AUDIT`
(precisam verificação adicional contra spec ou hardware real):

- `RAF_031_pwm_por_clock_manager.c` — sequência CM PASSWD depende de revisão BCM2835 ARM Peripherals §6.3
- `RAF_034_fifo_pwm_para_audio.c` — frequência de saída depende de clock tree BCM2835 real
- `docs/CICLOS_ESTIMADOS_VS_MEDIDOS.md` — coluna "medido" marcada TOKEN_VAZIO onde hardware ausente

## Como adicionar código experimental

Crie um arquivo com o prefixo `exp_` e um comentário de cabeçalho indicando o estado:

```c
/*
 * exp_<nome>.c — EXPERIMENTAL / PENDING
 * Propósito: <descrição do que está sendo investigado>
 * Próximo passo: <o que precisa ser feito para promover a validado>
 * Não incluir em build de produção.
 */
```

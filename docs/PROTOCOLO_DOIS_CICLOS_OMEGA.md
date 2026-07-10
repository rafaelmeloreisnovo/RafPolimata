# Protocolo de dois ciclos ômega

> **Entrada canônica:** `docs/AGENTES.md` §5 resume o pipeline operacional
> (VOID → BASELINE → CANDIDATE → VALIDATED → ROLLBACK) e o critério de shutdown.
> Este documento detalha os dois ciclos acoplados — semântico e técnico.

Este documento transforma o pedido multidisciplinar em uma rota operacional auditável. A ideia central é manter dois ciclos acoplados: um ciclo de **síntese semântica** para reduzir incoerência entre línguas, metáforas e modelos; e um ciclo de **execução técnica** para preservar invariantes de baixo nível sem heap no caminho crítico.

O protocolo aceita `TOKEN_VAZIO` quando falta evidência. Um vazio explícito é melhor que uma alegação não testada.

## Ciclo 1 — coerência semântica

Objetivo: converter material linguístico, matemático, sonoro e metafórico em unidades verificáveis.

Entradas canônicas:

- `matrix_id`, `row`, `col`, `cell_id`, `value`, `layer`, `state`, `tag14`, `rafbit10`, `epoch`, `cycle`, `timestamp`.
- Camadas linguísticas: português, inglês, chinês, japonês, hebraico, aramaico, grego e novas línguas por adaptador.
- Métricas: entropia, sintropia operacional, divergência entre distâncias fonéticas, semânticas, gráficas e direcionais.

Saídas obrigatórias:

1. `SEMANTIC_READY`: conceito tem definição, fonte interna e limite de interpretação.
2. `TOKEN_VAZIO`: conceito existe, mas ainda não tem evidência suficiente.
3. `SEMANTIC_FAIL`: conceito conflita com invariante ou falta peça essencial.

## Ciclo 2 — execução e rollback

Objetivo: transformar unidades prontas em comandos, testes e artefatos com fail-safe, failover e rollback.

Invariantes técnicos:

- sem `malloc/free` no hot path;
- sem dependência de GC;
- rotas branchless quando forem comprovadamente úteis e legíveis;
- flags explícitas por arquitetura;
- rollback definido antes da execução;
- logs preservam comandos, status e evidências.

Saídas obrigatórias:

1. `EXEC_PASS`: comando executado e evidência gravada.
2. `EXEC_FAIL`: falha real, sem ocultação.
3. `EXEC_SKIPPED`: ferramenta, dispositivo ou dataset ausente.
4. `ROLLBACK_READY`: há retorno seguro documentado.

## Acoplamento dos ciclos

| Etapa | Ciclo semântico | Ciclo técnico | Critério de avanço |
|---|---|---|---|
| 1 | Normalizar símbolo, som, camada e direção | Registrar matriz e hash | Nenhum campo canônico ausente |
| 2 | Separar metáfora de invariante | Escolher gate executável | Gate tem comando reproduzível |
| 3 | Medir coerência e entropia | Compilar/testar sem heap no hot path | PASS ou TOKEN_VAZIO explícito |
| 4 | Reprocessar gaps | Aplicar failover ou rollback | Falha não fica sem explicação |
| 5 | Atualizar documentação | Atualizar artefatos e PR | Evidência citável existe |

## Mapeamento multidisciplinar mínimo

- Física e mecânica quântica entram como linguagem de projeção, acoplamento e energia de ligação; não viram alegação experimental sem adaptador empírico.
- Linguística e dicionários entram como camadas de som, grafia, cadência, entonação, direção de leitura e polissemia.
- Mercado, supply chain, DNA/moléculas e eventos sociais entram como domínios com variáveis próprias, nunca misturados sem `layer` e `state`.
- Amor, coerência e prova são tratados como intenção ética, restrição operacional e evidência verificável, respectivamente.

## Checklist de entrega enterprise

- [ ] Arquivo ou módulo possui dono lógico.
- [ ] Entrada, saída e estado de erro estão documentados.
- [ ] Teste ou gate reproduzível existe.
- [ ] Rollback ou mitigação está descrito.
- [ ] `TOKEN_VAZIO` é usado quando a verdade operacional ainda não foi provada.

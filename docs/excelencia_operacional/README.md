# Excelência Operacional — RafPolimata

Este diretório define o contrato canônico para transformar fontes brutas em artefatos estruturados, limitados, reproduzíveis e auditáveis.

## Missão

O RafPolimata é o plano de dados determinístico da arquitetura federada. Ele deve:

1. ler por streaming e com memória limitada;
2. preservar o original em modo somente leitura;
3. produzir identidade por conteúdo e proveniência separada;
4. normalizar sem apagar divergências;
5. emitir índices, manifests, checkpoints e evidências;
6. recusar promoção de hipótese sem prova;
7. conservar `TOKEN_VAZIO` como estado válido.

## Invariante operacional

```text
identidade + proveniência + contexto + privacidade
+ estado epistêmico + transformação + evidência
+ reversibilidade + próximo passo
```

Nenhuma otimização pode remover um desses elementos.

## Pipeline mínimo

```text
SOURCE_READ_ONLY
  -> PREFLIGHT
  -> STREAM_PARSE
  -> NORMALIZE
  -> HASH_AND_PROVENANCE
  -> INDEX
  -> VALIDATE
  -> EMIT_EVIDENCE
  -> CHECKPOINT
```

## Estados oficiais

- `VERIFIED`: prova vinculada e reproduzível.
- `PASS_LIMITED`: passou dentro de limites declarados.
- `FAILED`: critério explícito não atendido.
- `BLOCKED`: dependência ausente.
- `CONTRADICTED`: evidência refuta o claim.
- `TOKEN_VAZIO`: evidência insuficiente; não promover.

## Documentos

- `INVARIANTE_GEOMETRICA_SISTEMATICA.md`: estrutura comum entre dados, transformações e evidência.
- `CONTRATO_DE_EXECUCAO.md`: precondições, limites, saídas e rollback.
- `CHECKLIST_DE_FECHAMENTO.md`: critérios objetivos de conclusão.

## Regra de ouro

> Processar tudo não é excelência. Excelência é processar o necessário, dentro de limites, com prova suficiente para repetir, auditar e corrigir.

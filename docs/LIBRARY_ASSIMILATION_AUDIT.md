# Auditoria de assimilação de bibliotecas — M063

O comando abaixo inspeciona uma biblioteca sem executar o código candidato:

```bash
python3 scripts/raf_library_assimilation_audit.py \
  caminho/da/biblioteca \
  --language c \
  --output ci/reports/minha-biblioteca.assimilation.json
```

A ferramenta usa somente a biblioteca padrão do Python no **plano de construção**. Nenhum componente Python entra no kernel ou no binário final.

## Inventário

O recibo registra:

- SHA-256 de cada fonte e da árvore ordenada;
- arquivos de licença encontrados;
- sinais de heap e runtime gerenciado;
- exceções, unwind e reflexão;
- threads e scheduler;
- carregamento dinâmico;
- criação de processos externos;
- I/O hospedado;
- containers que exigem reescrita;
- decisão preliminar e falsificadores.

## Estados

```text
CANDIDATE_FOR_KERNEL_EXTRACTION
CANDIDATE_REQUIRES_REWRITE
LOWERING_REQUIRED
REJECTED_RUNTIME_UNTIL_REWRITE
```

Nenhum desses estados permite claim final. Mesmo uma biblioteca sem padrões proibidos precisa de:

```text
licença compatível
+ kernel canônico
+ buffer estático/do caller
+ vetores bit-exatos
+ zero símbolo indefinido
+ gate do artefato final
+ ciclos medidos no alvo
```

## Selftest

```bash
python3 scripts/raf_library_assimilation_audit.py --selftest
```

O teste cobre:

- kernel C puro como candidato;
- Python hospedado como lowering obrigatório;
- uso de `malloc` como bloqueio até reescrita.

A varredura é conservadora e baseada em sinais textuais. Falso positivo é possível e deve ser revisado; falso negativo continua possível. Por isso `claim_allowed` permanece `false` em todo recibo preliminar.

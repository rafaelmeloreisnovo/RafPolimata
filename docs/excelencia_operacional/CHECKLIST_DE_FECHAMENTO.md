# Checklist de Fechamento

## Fonte e custódia

- [ ] original permanece somente leitura;
- [ ] `content_id` calculado por algoritmo e versão declarados;
- [ ] localização separada da identidade;
- [ ] origem, aquisição e transformação registradas;
- [ ] divergências temporais preservadas.

## Segurança e limites

- [ ] classe de privacidade definida;
- [ ] memória, tempo, entrada e saída limitados;
- [ ] proteção contra ZIP bomb, path traversal e registros gigantes;
- [ ] nenhum segredo em log ou artefato;
- [ ] falha converge para estado seguro.

## Implementação

- [ ] parser incremental ou justificativa explícita;
- [ ] alocações verificadas;
- [ ] overflow aritmético tratado;
- [ ] formatos e endianness declarados;
- [ ] modo determinístico disponível;
- [ ] checkpoint retomável.

## Prova

- [ ] testes positivos;
- [ ] entradas truncadas e malformadas;
- [ ] testes de limite;
- [ ] sanitizers quando aplicáveis;
- [ ] builds das arquiteturas declaradas;
- [ ] known-answer tests para hashes e CRCs;
- [ ] relatório e checksums emitidos.

## Estado epistêmico

- [ ] observado separado de inferido;
- [ ] hipótese possui falsificador;
- [ ] resultado negativo preservado;
- [ ] `TOKEN_VAZIO` não foi promovido sem evidência;
- [ ] claim máximo compatível com a prova.

## Fechamento

Somente marcar `VERIFIED` quando todos os itens obrigatórios estiverem satisfeitos e vinculados a um `run_id`. Caso contrário, usar `PASS_LIMITED`, `BLOCKED`, `FAILED`, `CONTRADICTED` ou `TOKEN_VAZIO`.

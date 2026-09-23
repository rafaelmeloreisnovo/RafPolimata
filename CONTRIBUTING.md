# Contribuindo com o RafPolimata

Obrigado por contribuir. O projeto aceita melhorias de código, testes, documentação, schemas, evidência e reprodução, mas exige que o nível de prova fique explícito.

## Antes de alterar

Leia:
1. [README.md](README.md)
2. [docs/canonical/README.md](docs/canonical/README.md)
3. [docs/AGENTES.md](docs/AGENTES.md)
4. [docs/DOCUMENT_GOVERNANCE.md](docs/DOCUMENT_GOVERNANCE.md)
5. documentação específica do subsistema.

## Regra central

~~~text
SOURCE != ARTIFACT != EXECUTION != EVIDENCE != CLAIM
TOKEN_VAZIO != FAIL != PASS
~~~

Não transforme arquivo presente em execução, execução local em device proof, nem CI verde em certificação.

## Fluxo recomendado

1. Crie issue quando o problema/escopo ainda precisar ser definido.
2. Trabalhe em branch pequena e temática.
3. Faça a menor mudança reversível que feche o gap.
4. Adicione ou atualize testes/falsificadores.
5. Execute os gates aplicáveis.
6. Registre comandos, commit, artifacts e hashes quando materiais.
7. Atualize documentação/índice somente quando o estado mudar.
8. Abra PR usando o template.
9. Mantenha gaps restantes como TOKEN_VAZIO/PENDING explícito.

## Código

- Preserve fronteiras entre freestanding/, syscall/, ApkC/, compiler/ e tooling.
- Não introduza heap/runtime oculto em código cujo contrato o proíba.
- Não enfraqueça warnings/gates apenas para obter verde.
- Falha fechada é preferível a fallback silencioso quando o contrato exige evidência.
- Dependências externas precisam de proveniência, licença e justificativa.

## Documentação

Siga [docs/canonical/2026-09-23/DOCUMENTATION_STYLE.md](docs/canonical/2026-09-23/DOCUMENTATION_STYLE.md).

Arquivos generated devem ser regenerados pelo executor declarado, nunca corrigidos manualmente.

## Testes

Escolha os gates mínimos suficientes ao escopo. Rotas úteis incluem:

~~~sh
make help
make compiler-contract
make compiler-selftest
make language-contract
make encoders
python3 -m unittest
~~~

Não marque como executado o que não foi executado no ambiente do PR.

## Evidência

Quando a mudança sustenta claim de runtime, performance, device, segurança ou ciência, ligue:
- commit;
- ambiente;
- comando;
- input;
- output;
- resultado;
- artifact/hash;
- falsificador;
- gap residual.

## Segurança

Vulnerabilidades não devem ser publicadas com detalhes exploráveis. Siga [.github/SECURITY.md](.github/SECURITY.md).

## Licença e terceiros

Não presuma que a licença de native/raf_hash_fabric_v1 cobre o repositório inteiro. Conteúdo de terceiros deve preservar sua própria proveniência/licença. Questões de escopo permanecem REVIEW_REQUIRED quando não houver autoridade suficiente.

## PRs

Um PR deve informar:
- objetivo;
- autoridade/source;
- arquivos;
- testes/gates;
- evidência;
- estado do claim;
- TOKEN_VAZIO/gaps;
- risco e rollback;
- R3.

Contribuição aceita não implica validação científica, certificação ou autorização comercial além dos termos aplicáveis.

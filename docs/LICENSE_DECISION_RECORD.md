# License Decision Record — RafPolimata

**Estado:** `TOKEN_VAZIO_OWNER_DECISION`  
**Área:** jurídico / comercial / supply-chain  
**Responsável lógico:** `human-authorizer` com revisão `security-license`  
**Gap:** `GAP-LEGAL-LICENSE-DECISION`

## 1. Fato observado

No corte de 2026-08-16:

- o metadata do repositório GitHub informa `license: null`;
- não foi encontrado arquivo `LICENSE` na raiz;
- existe documentação comparativa em `docs/LICENCAS_COMPARADAS.md`, mas ela não substitui a escolha jurídica do proprietário.

Portanto:

```text
public_repository != open_source_license
```

O GitHub documenta que, na ausência de licença, aplicam-se por padrão os direitos autorais correspondentes; uma decisão de licença deve ser explícita se a intenção for conceder permissões de uso, modificação ou distribuição.

Fonte de referência: GitHub Docs, “Licensing a repository”.

## 2. Por que este gap é P0

A decisão afeta diretamente:

- redistribuição de binários e código;
- contribuição externa;
- uso comercial por terceiros;
- integração de componentes com licenças diferentes;
- publicação acadêmica de artefatos;
- release assets;
- SBOM e THIRD_PARTY_NOTICES;
- contratos de suporte e eventual dual licensing.

Escolher uma licença errada ou presumir direitos inexistentes é mais grave que deixar o campo como `TOKEN_VAZIO` temporariamente.

## 3. Decisões que o agente não pode tomar sozinho

Somente o proprietário humano pode determinar a intenção jurídica principal. Exemplos de famílias que podem ser avaliadas, sem recomendação automática:

| Modelo | Efeito geral | Revisão necessária |
|---|---|---|
| permissivo | facilita reuso/distribuição com obrigações de aviso | autoria, patentes, terceiros |
| copyleft | permite reuso sob obrigações recíprocas específicas | compatibilidade de licenças e distribuição |
| proprietário | mantém concessões sob termos próprios | termos comerciais, contribuições, distribuição |
| dual/multi-license | separa cenários comunitário/comercial | titularidade integral e administração de direitos |

A tabela é apenas uma taxonomia operacional. Termos concretos dependem do texto efetivamente escolhido e do contexto jurídico.

## 4. Pré-condições para a decisão

Antes de adicionar um `LICENSE`, levantar:

1. **autoria e titularidade** — quais arquivos são originais, derivados ou importados;
2. **terceiros** — dependências, snippets, assets, fontes, datasets e código incorporado;
3. **termos existentes** — headers, licenças por subdiretório, forks e upstreams;
4. **patentes/marcas**, se aplicável ao modelo pretendido;
5. **objetivo comercial** — adoção aberta, serviço, licenciamento proprietário, pesquisa ou combinação;
6. **contribuições futuras** — necessidade ou não de DCO/CLA/política de contribuição;
7. **release** — o que será distribuído e sob quais permissões.

## 5. Artefatos de fechamento

O gap só pode mudar de `TOKEN_VAZIO` quando existirem:

```text
owner_decision
+ inventory_of_third_party_terms
+ compatibility_review
+ chosen_license_or_explicit_proprietary_terms
+ repository_documentation_update
```

Se a decisão for manter todos os direitos reservados sem conceder licença ampla, isso também precisa ser documentado explicitamente como decisão do proprietário; não deve ser inferido por agente.

## 6. Relação com comercialização

A topologia define:

```text
GAP-REL-CURRENT-RELEASE-ASSETS
  requires GAP-LEGAL-LICENSE-DECISION

GAP-COM-SUPPORT-LIFECYCLE
  requires GAP-LEGAL-LICENSE-DECISION
```

Isso evita publicar release comercialmente utilizável antes de esclarecer permissões e obrigações.

## 7. Limite jurídico

Este registro organiza o processo técnico e a proveniência da decisão. Não substitui aconselhamento jurídico profissional, análise de titularidade ou parecer de compatibilidade de licenças.

# License Decision Record — RafPolimata

**Estado raiz:** `TOKEN_VAZIO_OWNER_DECISION`  
**Decisão escopada 2026-08-28:** `RAF_HASH_FABRIC_V1 -> PolyForm-Noncommercial-1.0.0 + commercial agreement`  
**Área:** jurídico / comercial / supply-chain  
**Responsável lógico:** `human-authorizer` com revisão `security-license`  
**Gap raiz:** `GAP-LEGAL-LICENSE-DECISION`

## 1. Fato observado

O repositório contém materiais com regimes distintos, inclusive conteúdo com declaração SPDX própria. Portanto:

```text
public_repository != open_source_license
module_license != automatic_root_relicense
```

Uma política nova só cobre material cuja titularidade e escopo estejam explicitamente identificados. Terceiros e arquivos com licença própria permanecem sob seus termos.

## 2. Decisão escopada — RAF Hash Fabric V1

Para `native/raf_hash_fabric_v1/`, os arquivos originais enumerados em `LICENSE_SCOPE_V1.json` serão oferecidos sob **PolyForm Noncommercial License 1.0.0**, com:

- `LICENSE.md` apontando para os termos oficiais e Required Notice;
- escopo explícito e versionado;
- direitos comerciais/produção fora da concessão não comercial tratados por contrato escrito separado;
- exclusão explícita de terceiros, algoritmos/especificações, marcas, patentes e arquivos com outra licença.

A opção por uma licença padronizada reduz ambiguidade em comparação com criar um texto de licença autoral próprio. A autoria/proveniência continua registrada por notice, hashes, manifestos e contratos.

## 3. Por que o gap raiz continua aberto

A decisão do módulo **não** autoriza afirmar que todo RafPolimata está sob PolyForm. O fechamento raiz ainda exige:

1. inventário de autoria/titularidade;
2. inventário de dependências/snippets/assets/datasets e material importado;
3. identificação de headers/SPDX/licenças por subárvore;
4. compatibilidade e obrigações de redistribuição;
5. patentes/marcas quando materialmente relevantes;
6. política de contribuições/DCO/CLA para futura relicença comercial;
7. definição do que será efetivamente distribuído.

## 4. Terceiros e criptografia

O registro `legal/THIRD_PARTY_CRYPTO_LICENSE_REGISTER_V1.md` separa algoritmo, especificação, implementação upstream e código autoral. BLAKE3, por exemplo, mantém sua identidade e termos upstream quando código upstream for copiado/vendorized; um adaptador original não cria propriedade sobre o algoritmo ou projeto upstream.

## 5. Modelo comercial

```text
public research/noncommercial permission where scoped
+ separate commercial license/order
+ MSA/SOW
+ DPA/transfer schedule when personal data applies
+ security assurance
+ supplier/license register
+ release evidence
```

Nenhuma permissão comercial é inferida apenas porque o código está publicamente visível.

## 6. Gate de release comercial

`COMMERCIAL_CLEAR=true` somente para artefato/versionamento que feche:

```text
owner/authority
+ provenance
+ third-party inventory
+ license compatibility
+ commercial grant/order
+ security evidence
+ privacy/transfer gates when applicable
+ release receipt
```

Caso contrário: `COMMERCIAL_CLEAR=false` e a lacuna permanece `TOKEN_VAZIO`.

## 7. Limite jurídico

Este registro organiza a decisão técnica e a cadeia de custódia. Não substitui parecer jurídico profissional nem torna uma licença incompatível compatível por declaração. A revisão por advogado qualificado continua recomendada antes de exploração comercial material, especialmente em contratos internacionais, patentes, consumidor, exportação/sanções e setores regulados.

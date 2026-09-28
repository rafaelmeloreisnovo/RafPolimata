# RafPolimata — suplemento de maturidade 2026-09-28

**Observed implementation base:** main@f840fde2de65edacf8b2abbee8ca9d1f9c912b6d  
**Predecessor review head:** PR#358@f35fc8402ea790a8dead96817c0847bbaf9c1a98  
**State:** CANONICAL_CANDIDATE / REVIEW_REQUIRED  
**claim_allowed:** false

Este suplemento não reescreve o corte amplo de 2026-09-23. Ele materializa os gaps de maturidade pedidos depois do hardening de fronteiras e ABI ARM32.

## Rotas

1. [Maturity and Productization V1](MATURITY_AND_PRODUCTIZATION_V1.md)
2. [Threat Model V1](../../security/THREAT_MODEL_RAFPOLIMATA_V1.md)
3. contracts/rafpolimata_api_abi_v1.json
4. sdk/rafpolimata_v1/
5. tests/maturity/
6. .github/workflows/maturity-evidence.yml

## Regra

reference implementation
!= specialized implementation
!= property sample
!= benchmark
!= physical/device proof
!= independent-provider reproduction

O workflow cross-environment usa checkout limpo em duas imagens de runner. Isso testa independência de estado local e variação de ambiente, mas continua sendo o mesmo provider de CI.

R3 = ⟨F_ok: maturity layer materialized as code+contracts+gates; F_gap: physical energy/device and independent-provider reproduction remain evidence-gated; F_next: exact-head CI then external/device receipts⟩.

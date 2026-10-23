# RAFAELIA Ω — RafPolimata ApkC como laboratório de compilação federado (C04)

**Estágio:** SOURCE_IMPLEMENTED / EXACT_HEAD_CI_PENDING / DRAFT / NÃO PROMOVER.

## Objetivo mensurável
Criar um **candidato** a APK Android dual-ABI por GitHub Actions, reutilizando os próprios falsificadores e o `scripts/apkc_verified_build.py` que já existem no RafPolimata, sem recriar ou substituir o compilador ApkC. Este pacote só adiciona workflow/contrato de interoperação; não muda o código das rotas de compilação, ABI, assinaturas, licenças ou artefatos originais.

O RafGitTools faz a composição e o controle de estado; o Mapa possui autoridade federada de roteamento; o RafPolimata continua sendo autor e proprietário de seus artefatos e seus testes. Nenhuma alteração em um repo presume sucesso de outro.

## Fluxo de prova por etapa
1. `SOURCE_COMMIT` = SHA exato do HEAD do PR, checkout sem credencial persistente.
2. Host (GitHub Actions) verifica toolchain; **Clang/QEMU/Ubuntu/Python são dependências do ambiente de compilação**, não do core freestanding. Se indisponíveis, a execução falha — nunca `SKIPPED=PASS`.
3. Executar testes existentes `tests.test_apkc_verified_build`, `tests.test_validate_apkc_formats` e `tests.test_apkc_structural_closure_receipt`.
4. Reproduzir `tools/raf_source_to_binary_proof.sh`, cuja auditoria constrói ARM32 e ARM64 e rejeita saída parcial.
5. Executar `scripts/apkc_verified_build.py` com `--require-both`, preservando o validador preexistente de ZIP/APK/DEX/ELF. O aplicativo C04 demonstra empacotamento estrutural: **não** é uma interface de navegação, um orquestrador de IA, uma estação operacional no aparelho nem um navegador funcional.
6. Confirmar SHA do APK igual ao relatório; gravar `candidate-receipt.json`, com `claim_allowed=false`, físico `NOT_RUN`, assinatura `TOKEN_VAZIO`.
7. Enviar apenas candidato/recibos como artifact temporário; **sem GitHub Release, sem deploy, sem instalação, sem credencial/PAT, sem executar código privado**.

## Falsificação e rollback
Falhar imediatamente para: SHA de origem divergente, binário ausente, compilação parcial ARM32/ARM64, QEMU ausente, teste anterior falho, `validation.state != PASS`, relatório sem identidade da saída, APK vazio ou divergência SHA. Uma falha desse escopo não é licença para relaxar o falsificador.

Rollback: reverter os arquivos novos deste PR Draft; `main` continua intacta.

## Próximas fronteiras
- **P0**: executar workflow em PR HEAD exato, provar artefato com hash, revisar pela equipe humana e preservar erro real em vez de inventar sucesso.
- **P0**: build independente/reprodutível e revisão de proveniência/licenças.
- **P0 físico**: APK assinado para teste controlado, instalação, launch/dlopen no ARM32 e ARM64, registros com hash de pacote idêntico e privacidade/proteção infantil.
- **P1**: futura UI do laboratório em campo, se autorizada e implementada em uma ponte Android separada do core L0; não usar o APK C04 como prova desse produto.

**Governança:** referências orientativas de DMAIC/Six Sigma, ISO 8000/9001, família ISO/IEC 27000, NIST, legislação aplicável de proteção de dados e direitos de crianças/adolescentes. Nenhuma certificação, conformidade jurídica abrangente ou autorização comercial decorre deste PR.

`SOURCE != ARTIFACT != EXECUTION != EVIDENCE != CLAIM`
`TOKEN_VAZIO != 0`
`HOST_PASS != DEVICE_PASS`
`claim_allowed=false, no_merge=true, no_release=true`

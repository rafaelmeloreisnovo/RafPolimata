# ApkC — valor e gaps

> Estas faixas são estimativas heurísticas para priorização técnica/acadêmica. Não são avaliação financeira formal, recomendação de investimento ou garantia de mercado.

## Valor conservador

| Estado | Faixa heurística |
|---|---:|
| Conservador atual | US$ 25k–75k |
| Com APK validado | US$ 100k–300k+ |
| Com CI verde + artifacts | US$ 300k–750k |
| Com adoção externa | US$ 1M+ |

## Valor potencial

- Micro-toolchain Android freestanding em C, com escrita direta de ZIP/APK, AXML, DEX e ELF.
- Superfície pequena para auditoria técnica e acadêmica.
- Caminho didático para estudar formatos Android sem depender de toolchains completos.

## Gaps técnicos

| Gap | Evidência necessária | Status |
|---|---|---|
| Assinatura APK | `apksigner verify --verbose` | PASS (v1/v2/v3 true; debug key; ver `Apkc/proofs/out/apksigner-verify.txt`) |
| Instalação real | `adb install -r` em device autorizado | PASS_LIMITED (package:com.rafael.teste visível; stdout completo de `adb install -r` ausente) |
| Runtime NativeActivity | `logcat` filtrado sem crash | TOKEN_VAZIO |
| Compatibilidade Android ampla | matriz API/ABI | NOT_RUN |
| Testes regressivos de formatos | corpus de APKs e validadores | NOT_RUN |

## Gaps acadêmicos

- Especificar invariantes formais por formato: ZIP, AXML, DEX, ELF.
- Comparar contra toolchains Android oficiais em reprodutibilidade e tamanho.
- Documentar limites, ameaças à validade e casos negativos.

## Gates que aumentam valuation

- CI com artifacts de `Apkc/proofs/out/`.
- APK assinado e verificado em ambiente reproduzível.
- Instalação e execução comprovadas em device físico/emulador.
- Relatório acadêmico com métodos, resultados e falsificabilidade.

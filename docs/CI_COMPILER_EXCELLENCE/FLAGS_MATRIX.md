# Matriz de flags — ApkC freestanding

| Perfil | Flags | Motivo | Risco | Status |
|---|---|---|---|---|
| host-syntax | `-std=c11 -Wall -Wextra -fsyntax-only` | provar sintaxe mínima sem link | não gera binário | PASS quando compilador host entende o código |
| arm64-object | `--target=aarch64-linux-android21 -std=c11 -Wall -Wextra -c` | verificar objeto ARM64 sem libc/heap | linker Android pode faltar | PASS/TOKEN_VAZIO |
| arm64-link | `-std=c11 -Wall -Wextra -nostdlib -static` | gerar binário freestanding executável | depende de linker/ABI | TOKEN_VAZIO até prova |
| apk-generate | `Apkc/proofs/out/apkc hello.s.txt -o ...` | gerar APK real | só válido com binário executável | TOKEN_VAZIO até prova |

## Diretriz

Não promover `TOKEN_VAZIO` para `PASS`. A flag só vira gate forte quando houver artefato arquivado e comando reproduzível.

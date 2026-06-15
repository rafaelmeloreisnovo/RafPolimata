# ApkC evidence summary 2026-06-14

- hello.apk SHA-256: a331d0248d01d8e7030291e93905c2e2f046cf7cb5ba4ecaf02609cec273c024
- hello-signed.apk SHA-256: 063c1b61c35e45f3cf253d42c99bfcd58910162c46ba6c5160846b56651dcc28
- APK package: com.rafael.teste
- App label: RafaelTeste
- Native library name: hello
- Unsigned APK entries: lib/armeabi-v7a/libhello.so, AndroidManifest.xml, classes.dex
- Signed APK entries: unsigned entries plus META-INF/APKC-DEB.SF, META-INF/APKC-DEB.RSA, META-INF/MANIFEST.MF
- apksigner result: v1 true, v2 true, v3 true, v3.1 false, v4 false, SourceStamp false
- signer: CN=ApkC Debug, O=Rafael, C=BR
- package visibility proof: package:com.rafael.teste
- DEX SHA-1: PASS, header and computed value 9ea7c00884bffbdbbab055ddc3ad6565050fc4e4
- ELF ARM32: PASS, ELF32 ARM EABI5 shared object
- ELF ARM64: SKIP, not present in uploaded APK
- readelf on APK container: FAIL expected, target is not an ELF object
- runtime logcat: TOKEN_VAZIO
- source-to-binary build transcript for apkc: TOKEN_VAZIO

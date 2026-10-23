#!/usr/bin/env bash
# Build a signed DEBUG-only, minimal Android launcher around the authorial L0.
# Do not call L0 hosted: it is in the APK as pure freestanding ELF.
# SDK/D8/Java/keytool are HOST build-time tools, not core runtime dependencies.
set -euo pipefail
sdk="${ANDROID_SDK_ROOT:-${ANDROID_HOME:-}}"
test -n "$sdk" || { echo "::error::ANDROID_SDK_ROOT missing"; exit 2; }
platform="$sdk/platforms/android-35/android.jar"
tools="$sdk/build-tools/35.0.0"
for path in "$platform" "$tools/aapt2" "$tools/d8" "$tools/zipalign" "$tools/apksigner"; do
  test -e "$path" || { echo "::error::missing provisioned build tool: $path"; exit 3; }
done
for exe in javac java keytool clang readelf python3; do command -v "$exe" >/dev/null || exit 4; done

root="field_lab/android/app"
out="artifacts/field-lab"
mkdir -p "$out"
scratch="$(mktemp -d)"
trap 'rm -rf "$scratch"' EXIT
chmod 700 "$scratch"
mkdir -p "$scratch/classes" "$scratch/dex" "$scratch/abi/armeabi-v7a" "$scratch/abi/arm64-v8a"

# Compile both primitive ABI JNI entry and L0 together, with no imports to libc.
src=field_lab/l0/raf_field_lab_core.c
edge=field_lab/android/raf_field_lab_jni.c
common=(-std=c11 -Os -Wall -Wextra -Werror -ffreestanding -fno-builtin
        -nostdinc -nostdlib -fPIC -shared -fno-stack-protector
        -fno-unwind-tables -fno-asynchronous-unwind-tables -fuse-ld=lld
        -Wl,--no-undefined -Wl,--build-id=none
        -Wl,-soname,libraffieldlab.so)
clang --target=armv7a-linux-androideabi24 "${common[@]}" "$src" "$edge" \
    -o "$scratch/abi/armeabi-v7a/libraffieldlab.so"
clang --target=aarch64-linux-android24 "${common[@]}" "$src" "$edge" \
    -o "$scratch/abi/arm64-v8a/libraffieldlab.so"
for abi in armeabi-v7a arm64-v8a; do
  lib="$scratch/abi/$abi/libraffieldlab.so"
  if readelf -d "$lib" | grep -F '(NEEDED)'; then
    echo "::error::L0+JNI has DT_NEEDED dynamic dependency ($abi)"; exit 5
  fi
  if command -v llvm-nm >/dev/null && test -n "$(llvm-nm -u "$lib")"; then
    echo "::error::L0+JNI imports undefined symbols ($abi)"; exit 6
  fi
  readelf -Ws "$lib" | grep -F 'Java_io_rafaelia_fieldlab_FieldLabActivity_nativeSelfTest'
done

# Java is only the Android system-backed activity/loader; no third-party .jar.
javac -source 8 -target 8 -encoding UTF-8 -classpath "$platform" \
    -d "$scratch/classes" "$root/src/io/rafaelia/fieldlab/FieldLabActivity.java"
mapfile -t javac_classes < <(find "$scratch/classes" -type f -name '*.class' | sort)
test "${#javac_classes[@]}" -ge 1
"$tools/d8" --lib "$platform" --min-api 24 --output "$scratch/dex" "${javac_classes[@]}"
test -s "$scratch/dex/classes.dex"

# Official Android manifest compilation, unlike the prior 140-byte empty DEX.
"$tools/aapt2" link --manifest "$root/AndroidManifest.xml" \
    -I "$platform" --min-sdk-version 24 --target-sdk-version 28 \
    -o "$scratch/manifest-only.apk"

python3 - "$scratch/manifest-only.apk" "$scratch/dex/classes.dex" \
  "$scratch/abi" "$scratch/unsigned-packed.apk" <<'PY'
import sys,zipfile
from pathlib import Path
manifest, dex, base, out = map(Path,sys.argv[1:])
with zipfile.ZipFile(manifest) as source,zipfile.ZipFile(out,"w",allowZip64=False) as z:
    names=source.namelist()
    if "AndroidManifest.xml" not in names:
        raise SystemExit("aapt2 did not generate compiled binary AndroidManifest.xml")
    z.writestr("AndroidManifest.xml",source.read("AndroidManifest.xml"),
               compress_type=zipfile.ZIP_STORED)
    z.write(dex,"classes.dex",compress_type=zipfile.ZIP_STORED)
    for abi in ("armeabi-v7a","arm64-v8a"):
        z.write(base/abi/"libraffieldlab.so",
                "lib/"+abi+"/libraffieldlab.so",
                compress_type=zipfile.ZIP_STORED)
PY
"$tools/zipalign" -p -f 4 "$scratch/unsigned-packed.apk" "$scratch/zipaligned.apk"
"$tools/zipalign" -c -p 4 "$scratch/zipaligned.apk"
# Ephemeral test key generated for this run, not an authorial/prod certificate.
keytool -genkeypair -noprompt -keystore "$scratch/debug-only.jks" \
  -storepass "android" -keypass "android" -alias "raffieldlabdebug" \
  -keyalg RSA -keysize 2048 -validity 30 \
  -dname "CN=RAFAELIA Field Lab Debug, OU=CI Test Only, O=RAFAELIA" >/dev/null 2>&1
"$tools/apksigner" sign --ks "$scratch/debug-only.jks" \
    --ks-key-alias "raffieldlabdebug" --ks-pass pass:android \
    --key-pass pass:android --min-sdk-version 24 \
    --v1-signing-enabled true --v2-signing-enabled true \
    --out "$out/field-lab-install-debug.apk" "$scratch/zipaligned.apk"
"$tools/apksigner" verify --verbose --min-sdk-version 24 "$out/field-lab-install-debug.apk"
"$tools/aapt2" dump badging "$out/field-lab-install-debug.apk" > "$out/install-debug-badging.txt"
grep -Fq "package: name='io.rafaelia.fieldlab'" "$out/install-debug-badging.txt"
grep -Fq "launchable-activity: name='io.rafaelia.fieldlab.FieldLabActivity'" "$out/install-debug-badging.txt"
if grep -Eq '^uses-permission' "$out/install-debug-badging.txt"; then
  echo "::error::Unreviewed Android permission in debug app"; exit 7
fi
python3 - "$out/field-lab-install-debug.apk" "$out/install-debug-receipt.json" <<'PY'
import hashlib,json,sys,zipfile
from pathlib import Path
apk,receipt=map(Path,sys.argv[1:])
expected={"AndroidManifest.xml","classes.dex",
          "lib/armeabi-v7a/libraffieldlab.so",
          "lib/arm64-v8a/libraffieldlab.so"}
with zipfile.ZipFile(apk) as z:
    names=z.namelist()
    if len(names)!=len(set(names)) or not expected.issubset(names):
        raise SystemExit("debug APK inventory fail-closed")
    if len(z.read("classes.dex"))<512:
        raise SystemExit("DEX still empty/skeleton")
    if any(z.getinfo(name).file_size>131072 for name in expected):
        raise SystemExit("unbounded component")
    dex=hashlib.sha256(z.read("classes.dex")).hexdigest()
    libs={abi:hashlib.sha256(z.read(f"lib/{abi}/libraffieldlab.so")).hexdigest()
          for abi in ("armeabi-v7a","arm64-v8a")}
record={
 "schema":"rafpolimata.field-lab-android-installability.v1",
 "state":"PASS_SCOPED_SIGNED_DEBUG_AND_MANIFEST",
 "package":"io.rafaelia.fieldlab",
 "target_sdk":28, "min_sdk":24,
 "apk_bytes":apk.stat().st_size,
 "apk_sha256":hashlib.sha256(apk.read_bytes()).hexdigest(),
 "dex_sha256":dex,"native_abi_sha256":libs,
 "debug_signature":"VERIFIED_USING_APKSIGNER_EPHEMERAL_NONAUTHORIAL",
 "launcher":"io.rafaelia.fieldlab.FieldLabActivity",
 "android_permissions":[],
 "native_jni_bridge":"SOURCE_COMPILED_AND_SYMBOL_PRESENT",
 "android_install":"TOKEN_VAZIO_NOT_RUN",
 "android_launch":"TOKEN_VAZIO_NOT_RUN",
 "physical_device":"TOKEN_VAZIO_NOT_RUN",
 "runtime_ui":"TOKEN_VAZIO_NOT_RUN",
 "claim_allowed":False,"release_allowed":False,
 "note":"Signed debug APK is candidate for manual installation, not device-proven."
}
receipt.write_text(json.dumps(record,indent=2,sort_keys=True)+"\n")
print("FIELD_LAB_ANDROID_DEBUG="+record["state"]+
      " bytes="+str(record["apk_bytes"])+
      " sha256="+record["apk_sha256"])
PY

#!/usr/bin/env sh
set -eu

cc="${CC:-gcc}"
build_dir="build_host_check"
mkdir -p "$build_dir"
log_file="$build_dir/rafaelia_cc.log"
: > "$log_file"

ok=0
fail=0

compile_obj() {
  src="$1"
  obj="$build_dir/$(basename "$src" .c).o"
  if "$cc" -std=c11 -Wall -Wextra -Werror -I. -IBenchmark -c "$src" -o "$obj" >>"$log_file" 2>&1; then
    echo "OK   $src"
    ok=$((ok+1))
  else
    echo "FAIL $src"
    cat "$log_file"
    fail=$((fail+1))
  fi
}

# Núcleo do compilador host: deve ser estrito e bloqueante.
for f in raf_main.c raf_frontend.c raf_cpu.c raf_asm_emit.c raf_precomp.c; do
  compile_obj "$f"
done

# Arquivos de métodos são opcionais nesta árvore. Se existirem, também entram
# na checagem; se não existirem, o script não deve produzir falso negativo.
method_count=0
for f in methods/*.c; do
  [ -e "$f" ] || continue
  method_count=$((method_count+1))
  compile_obj "$f"
done
if [ "$method_count" -eq 0 ]; then
  echo "SKIP methods/*.c (diretório ausente ou sem fontes C)"
fi

echo "ok=$ok fail=$fail"
[ "$fail" -eq 0 ]

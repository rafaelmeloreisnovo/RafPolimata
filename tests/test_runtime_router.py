"""tests/test_runtime_router.py — CI wrapper for raf_runtime_router_test.c.

Compiles Benchmark/raf_runtime_router_test.c with gcc and executes the binary.
Exit code mirrors the C binary: 0 = all cases PASS, non-zero = first failure.
"""
import subprocess
import sys
import os

REPO_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

def main():
    binary = os.path.join(REPO_ROOT, "build_runtime_router_test")
    src    = os.path.join(REPO_ROOT, "Benchmark", "raf_runtime_router_test.c")
    inc    = os.path.join(REPO_ROOT, "Benchmark")

    compile_cmd = [
        "gcc", "-std=c11", "-Wall", "-Wextra", "-Werror",
        f"-I{inc}", src, "-o", binary,
    ]
    result = subprocess.run(compile_cmd, capture_output=True, text=True)
    if result.returncode != 0:
        print("COMPILE FAIL:", result.stderr, file=sys.stderr)
        sys.exit(1)

    run_result = subprocess.run([binary], capture_output=True, text=True)
    if run_result.stdout:
        print(run_result.stdout, end="")
    if run_result.returncode != 0:
        print(f"RUNTIME ROUTER TEST FAIL (exit {run_result.returncode})", file=sys.stderr)
        if run_result.stderr:
            print(run_result.stderr, file=sys.stderr)
        sys.exit(run_result.returncode)

    print("RUNTIME_ROUTER_TEST PASS")

if __name__ == "__main__":
    main()

import importlib.util
from pathlib import Path

P=Path(__file__).resolve().parents[1]/'scripts'/'patch_apkc_source_cap.py'
spec=importlib.util.spec_from_file_location('p',P)
m=importlib.util.module_from_spec(spec); spec.loader.exec_module(m)

base='prefix\n'+m.OLD+'suffix\n'
out=m.transform(base)
assert m.OLD not in out
assert out.count('source exceeds SRC_CAP') == 1
assert 'if (n<0) { pr_err("source read failed\\n")' in out
assert 'i32 n = os_read(fd, &extra, 1u);' in out
assert 'if (n>0) { pr_err("source exceeds SRC_CAP\\n")' in out
assert '_src_local[src_len] = 0;' in out

for bad in ('no anchor', m.OLD+m.OLD):
    try: m.transform(bad)
    except ValueError: pass
    else: raise AssertionError('anchor mismatch must fail closed')
print('PASS source-cap transformer: exact anchor + overflow probe + read-error guard')

# Build and run

## Deterministic C simulation
```sh
clang -O2 -Wall -Wextra -Werror corrected/voynich_toroidal_v2.c -o voynich_toroidal_v2
./voynich_toroidal_v2 144000 20
```

## Conservative image-order experiment
```sh
python corrected/voynich_analysis_v2.py /path/to/images --pattern '*.jpg' --steps 64 --output report.json
```

## Manifest-first image downloader
```sh
python corrected/voynich_downloader_v2.py --max-files 0
# Inspect SOURCE_MANIFEST.json before allowing downloads.
```

No program in this package claims decipherment, historical key, Arabic semantics, or 7D attractors.

"""Command-line entry point: python -m rafpictures."""
import argparse
import json
import sys

from .contract import RafError
from .pack import pack
from .verify import inspect, extract, export_preview

def main(argv=None):
    parser = argparse.ArgumentParser(
        description="RafPictures: lossless, multi-axis image archives")
    sub = parser.add_subparsers(dest="command", required=True)
    p = sub.add_parser("pack")
    p.add_argument("output")
    p.add_argument("images", nargs="*")
    p.add_argument("--depth", action="append", default=[])
    p.add_argument("--mask", action="append", default=[])
    p.add_argument("--preview")
    p.add_argument("--coords", help="JSON indexed by 0-based source position")
    for cmd in ("inspect", "verify"):
        p = sub.add_parser(cmd)
        p.add_argument("archive")
    p = sub.add_parser("extract")
    p.add_argument("archive")
    p.add_argument("destination")
    p = sub.add_parser("preview")
    p.add_argument("archive")
    p.add_argument("destination")
    args = parser.parse_args(argv)
    try:
        if args.command == "pack":
            result = pack(args.output, args.images, args.depth,
                          args.mask, args.preview, args.coords)
        elif args.command == "inspect":
            result = inspect(args.archive, full=False)
        elif args.command == "verify":
            result = inspect(args.archive, full=True)
        elif args.command == "extract":
            result = extract(args.archive, args.destination)
        else:
            result = export_preview(args.archive, args.destination)
        print(json.dumps({"state": "PASS", "result": result},
                         indent=2, ensure_ascii=False, allow_nan=False))
        return 0
    except (RafError, OSError, ValueError) as exc:
        print("FAIL: " + str(exc), file=sys.stderr)
        return 2

if __name__ == "__main__":
    raise SystemExit(main())

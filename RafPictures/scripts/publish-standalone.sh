#!/bin/sh
# Manual-only publisher. Requires an authenticated GitHub CLI owner session.
set -eu
OWNER="${OWNER:-rafaelmeloreisnovo}"
NAME="${NAME:-RafPictures}"
VISIBILITY="${VISIBILITY:-private}"
if [ "$VISIBILITY" != private ] && [ "$VISIBILITY" != public ]; then
    echo "ERROR: VISIBILITY must be private or public" >&2
    exit 2
fi
command -v gh >/dev/null 2>&1 || { echo "ERROR: GitHub CLI (gh) missing" >&2; exit 2; }
command -v git >/dev/null 2>&1 || { echo "ERROR: git missing" >&2; exit 2; }
SCRIPT_DIR=$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd)
SOURCE=$(CDPATH='' cd -- "$SCRIPT_DIR/.." && pwd)
HOST=$(CDPATH='' cd -- "$SOURCE/.." && pwd)
LOGIN=$(gh api user --jq '.login')
if [ "$LOGIN" != "$OWNER" ]; then
    echo "ERROR: authenticated account differs from target owner" >&2
    exit 3
fi
if gh repo view "$OWNER/$NAME" >/dev/null 2>&1; then
    echo "ERROR: destination already exists; no overwrite" >&2
    exit 4
fi
WORK=$(mktemp -d)
trap 'if [ -n "$WORK" ] && [ -d "$WORK" ]; then rm -rf -- "$WORK"; fi' EXIT HUP INT TERM
cp -R "$SOURCE/." "$WORK/"
SRC_SHA=$(git -C "$HOST" rev-parse HEAD)
printf 'origin=rafaelmeloreisnovo/RafPolimata\nsource_commit=%s\nsource_path=RafPictures/\nsource_pr=https://github.com/rafaelmeloreisnovo/RafPolimata/pull/450\n' "$SRC_SHA" > "$WORK/SOURCE_REF.txt"
git -C "$WORK" init -q -b main
git -C "$WORK" add -A
git -C "$WORK" -c user.name="$LOGIN" -c user.email="$LOGIN@users.noreply.github.com" commit -qm "init: RafPictures V1 from RafPolimata PR450"
if [ "$VISIBILITY" = private ]; then
    gh repo create "$OWNER/$NAME" --private --description "RafPictures: lossless multidimensional image format"
else
    gh repo create "$OWNER/$NAME" --public --description "RafPictures: lossless multidimensional image format"
fi
gh auth setup-git
git -C "$WORK" remote add origin "https://github.com/$OWNER/$NAME.git"
git -C "$WORK" push -u origin main
echo "PUBLISHED: https://github.com/$OWNER/$NAME"

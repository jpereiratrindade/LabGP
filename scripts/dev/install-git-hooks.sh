#!/usr/bin/env bash
set -euo pipefail

repo_root=$(git rev-parse --show-toplevel)
cd "$repo_root"

mkdir -p .githooks
chmod +x .githooks/pre-commit

git config core.hooksPath .githooks

echo "Git hooks instalados."
echo "Hooks path: $(git config --get core.hooksPath)"

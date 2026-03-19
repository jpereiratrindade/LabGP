#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR="${BUILD_DIR:-build-quality}"
MAX_CPP_LINES="${MAX_CPP_LINES:-900}"
COGNITIVE_THRESHOLD="${COGNITIVE_THRESHOLD:-40}"
FUNCTION_LINES_THRESHOLD="${FUNCTION_LINES_THRESHOLD:-220}"
FUNCTION_STATEMENTS_THRESHOLD="${FUNCTION_STATEMENTS_THRESHOLD:-140}"

BASE_REF="${BASE_REF:-}"
FALLBACK_BASE_REF="${FALLBACK_BASE_REF:-}"
HEAD_REF="${HEAD_REF:-HEAD}"

if [[ -z "$BASE_REF" || "$BASE_REF" == "0000000000000000000000000000000000000000" ]]; then
    BASE_REF="$FALLBACK_BASE_REF"
fi

if [[ -z "$BASE_REF" || "$BASE_REF" == "0000000000000000000000000000000000000000" ]]; then
    echo "[architecture-gate] Base ref indefinida; pulando gate para evitar falso positivo."
    exit 0
fi

mapfile -t added_cpp < <(git diff --name-status --diff-filter=A "$BASE_REF" "$HEAD_REF" -- src include tests | awk '$1 == "A" {print $2}' | grep -E '\.cpp$' || true)

if [[ ${#added_cpp[@]} -eq 0 ]]; then
    echo "[architecture-gate] Nenhum .cpp novo detectado."
    exit 0
fi

echo "[architecture-gate] .cpp novos detectados:"
printf ' - %s\n' "${added_cpp[@]}"

line_fail=0
for f in "${added_cpp[@]}"; do
    lines=$(wc -l < "$f")
    if (( lines > MAX_CPP_LINES )); then
        echo "[architecture-gate] FAIL: $f tem $lines linhas (limite: $MAX_CPP_LINES)."
        line_fail=1
    else
        echo "[architecture-gate] OK: $f com $lines linhas."
    fi
done

if (( line_fail != 0 )); then
    exit 1
fi

if ! command -v clang-tidy >/dev/null 2>&1; then
    echo "[architecture-gate] clang-tidy indisponivel; pulando gate de complexidade."
    exit 0
fi

if [[ ! -f "$BUILD_DIR/compile_commands.json" ]]; then
    echo "[architecture-gate] compile_commands.json ausente em '$BUILD_DIR'; pulando gate de complexidade."
    exit 0
fi

echo "[architecture-gate] Rodando gate de complexidade (clang-tidy) para .cpp novos..."

for f in "${added_cpp[@]}"; do
    clang-tidy "$f" \
        -p "$BUILD_DIR" \
        --quiet \
        --warnings-as-errors='*' \
        --config="{Checks: '-*,readability-function-cognitive-complexity,readability-function-size', CheckOptions: [{key: readability-function-cognitive-complexity.Threshold, value: '$COGNITIVE_THRESHOLD'}, {key: readability-function-size.LineThreshold, value: '$FUNCTION_LINES_THRESHOLD'}, {key: readability-function-size.StatementThreshold, value: '$FUNCTION_STATEMENTS_THRESHOLD'}]}"
done

echo "[architecture-gate] OK: limites arquiteturais atendidos para novos .cpp."

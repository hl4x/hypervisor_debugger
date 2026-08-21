#!/usr/bin/env bash
SCRIPT_FILE="$(realpath "${BASH_SOURCE[0]}")"
SCRIPT_DIR="$(dirname "$SCRIPT_FILE")"
gdb -x "$SCRIPT_DIR/gdbscript" "$SCRIPT_DIR/../.output/kernel"
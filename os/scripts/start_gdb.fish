#!/usr/bin/env fish
set SCRIPT_FILE (realpath (status --current-filename))
set SCRIPT_DIR (dirname "$SCRIPT_FILE")
gdb -x "$SCRIPT_DIR/gdbscript" "$SCRIPT_DIR/../.output/kernel"
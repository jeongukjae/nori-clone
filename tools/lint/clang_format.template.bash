#!/usr/bin/env bash

if ! cd "$BUILD_WORKSPACE_DIRECTORY"; then
  echo "Unable to change to workspace (BUILD_WORKSPACE_DIRECTORY: ${BUILD_WORKSPACE_DIRECTORY})"
  exit 1
fi

CLANG_FORMAT_CMD=@@CMD@@
ARGS=@@ARGS@@

find . \
  -type "${FIND_FILE_TYPE:-f}" \
  @@EXCLUDE_PATTERNS@@ \
  \( -name '*.h' \
    -o -name '*.hh' \
    -o -name '*.cc' \
    -o -name '*.c' \
  \) -print | xargs "$CLANG_FORMAT_CMD" "${ARGS[@]}"

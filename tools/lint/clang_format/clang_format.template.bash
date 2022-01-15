#!/usr/bin/env bash

if [[ ! -z "${TEST_WORKSPACE+x}" && -z "${BUILD_WORKSPACE_DIRECTORY+x}" ]]; then
  FIND_FILE_TYPE="l"
else
  # Change into the workspace directory if this is _not_ a test
  if ! cd "$BUILD_WORKSPACE_DIRECTORY"; then
    echo "Unable to change to workspace (BUILD_WORKSPACE_DIRECTORY: ${BUILD_WORKSPACE_DIRECTORY})"
    exit 1
  fi
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

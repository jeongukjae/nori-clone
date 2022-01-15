#!/usr/bin/env bash

set -ex

JAVA_FORMAT_JAR=@@JAR@@
java_format_jar=$(readlink "$JAVA_FORMAT_JAR")
ARGS=@@ARGS@@

if ! cd "$BUILD_WORKSPACE_DIRECTORY"; then
  echo "Unable to change to workspace (BUILD_WORKSPACE_DIRECTORY: ${BUILD_WORKSPACE_DIRECTORY})"
  exit 1
fi

find . \
  -type "${FIND_FILE_TYPE:-f}" \
  @@EXCLUDE_PATTERNS@@ \
  \( -name '*.java' \
  \) -print | xargs java -jar "$java_format_jar" "${ARGS[@]}"

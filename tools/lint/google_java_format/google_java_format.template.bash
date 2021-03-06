#!/usr/bin/env bash

set -ex

JAVA_FORMAT_JAR=@@JAR@@
java_format_jar=$(readlink "$JAVA_FORMAT_JAR")
ARGS=@@ARGS@@

if [[ ! -z "${TEST_WORKSPACE+x}" && -z "${BUILD_WORKSPACE_DIRECTORY+x}" ]]; then
  FIND_FILE_TYPE="l"
else
  # Change into the workspace directory if this is _not_ a test
  if ! cd "$BUILD_WORKSPACE_DIRECTORY"; then
    echo "Unable to change to workspace (BUILD_WORKSPACE_DIRECTORY: ${BUILD_WORKSPACE_DIRECTORY})"
    exit 1
  fi
fi

FILES=$(find . \
  -type ${FIND_FILE_TYPE:-f} \
  @@EXCLUDE_PATTERNS@@ \
  \( -name '*.java' \
  \) -print)
echo $FILES | xargs java \
  --add-opens java.base/java.util=ALL-UNNAMED \
  --add-opens jdk.compiler/com.sun.tools.javac.api=ALL-UNNAMED \
  --add-opens jdk.compiler/com.sun.tools.javac.comp=ALL-UNNAMED \
  --add-opens jdk.compiler/com.sun.tools.javac.file=ALL-UNNAMED \
  --add-opens jdk.compiler/com.sun.tools.javac.jvm=ALL-UNNAMED \
  --add-opens jdk.compiler/com.sun.tools.javac.main=ALL-UNNAMED \
  --add-opens jdk.compiler/com.sun.tools.javac.parser=ALL-UNNAMED \
  --add-opens jdk.compiler/com.sun.tools.javac.processing=ALL-UNNAMED \
  --add-opens jdk.compiler/com.sun.tools.javac.tree=ALL-UNNAMED \
  --add-opens jdk.compiler/com.sun.tools.javac.util=ALL-UNNAMED \
  -jar "$java_format_jar" "${ARGS[@]}"

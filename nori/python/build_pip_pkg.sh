#!/usr/bin/env bash
set -ex

PLATFORM="$(uname -s | tr 'A-Z' 'a-z')"
function is_windows() {
  if [[ "${PLATFORM}" =~ (cygwin|mingw32|mingw64|msys)_nt* ]]; then
    true
  else
    false
  fi
}

if is_windows; then
  PIP_FILE_PREFIX="bazel-bin/nori/python/build_pip_pkg.exe.runfiles/nori-clone/"
else
  PIP_FILE_PREFIX="bazel-bin/nori/python/build_pip_pkg.runfiles/nori-clone/"
fi

function main() {
  DEST=${1}

  # Check dest parameter
  if [[ -z ${DEST} ]]; then
    echo "No destination dir provided"
    exit 1
  fi

  # create if not exist
  mkdir -p ${DEST}
  if [[ ${PLATFORM} == "darwin" ]]; then
    DEST=$(pwd -P)/${DEST}
  else
    DEST=$(readlink -f "${DEST}")
  fi
  echo "=== destination directory: ${DEST}"

  # Create temp directory and copy all required files
  TMPDIR=$(mktemp -d -t tmp.XXXXXXXXXX)
  echo $(date) : "=== Using tmpdir: ${TMPDIR}"

  echo "=== Copy Python binding files"
  cp "${PIP_FILE_PREFIX}nori/python/setup.py" "${TMPDIR}"
  cp "${PIP_FILE_PREFIX}nori/python/LICENSE" "${TMPDIR}"
  cp "${PIP_FILE_PREFIX}nori/python/README.md" "${TMPDIR}"
  cp "${PIP_FILE_PREFIX}nori/python/MANIFEST.in" "${TMPDIR}"
  rsync -avm -L --exclude='*_test.py' "${PIP_FILE_PREFIX}nori/python/nori" "${TMPDIR}"
  rsync -avm -L "${PIP_FILE_PREFIX}dictionary" "${TMPDIR}/nori"

  # Build bdist wheel file
  pushd ${TMPDIR}
  echo $(date) : "=== Building wheel"
  python setup.py bdist_wheel
  cp dist/*.whl "${DEST}"
  popd
  rm -rf ${TMPDIR}
  echo $(date) : "=== Output wheel file is in: ${DEST}"
}

main "$@"

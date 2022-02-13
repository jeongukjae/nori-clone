# Development notes for nori-clone

## Build and test all

```sh
bazel build //...
bazel test //...
```

## Code style check

We use `clang-format` for C/C++, `google-java-format` for Java, and `buildifier` for Starlark. You can run each code style checker as follows.

```sh
bazel test //:lint_all # run all checkers
bazel test //:buildifier_test
bazel test //:clang_format_test
bazel test //:google_java_format_test

# To apply code formatter, you can run as follows
bazel test //:buildifier
bazel test //:clang_format
bazel test //:google_java_format
```

## Testing

You can run all test cases as follows.

```sh
bazel test //...
```

If you want specific test cases (i.e. test cases for golang bindings), you can run

```sh
bazel test //nori/go/...
```

## Packaging

### C Api Packaging

To bind nori-clone for golang or another languages, you can package C Api of nori-clone as follows.

```sh
$ bazel build //nori/c:libnori_c
INFO: Analyzed target //nori/c:libnori_c (0 packages loaded, 0 targets configured).
INFO: Found 1 target...
Target //nori/c:libnori_c up-to-date:
  bazel-bin/nori/c/libnori_c.tar.gz
...
```

Then you can install library and header.

```sh
sudo tar -C /usr/local -xzf bazel-bin/nori/c/libnori_c.tar.gz
sudo ldconfig /usr/local/lib
```

Or, you can install in non-system directory.

```sh
export LIBRARY_PATH=$LIBRARY_PATH:/path/to/lib
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/path/to/lib
# in MacOS
# export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:/path/to/lib
```

### Python packaging

You can build python wheel package as follows.

```sh
$ bazel build //nori/python:build_pip_pkg
INFO: Analyzed target //nori/python:build_pip_pkg (0 packages loaded, 3 targets configured).
INFO: Found 1 target...
Target //nori/python:build_pip_pkg up-to-date:
  bazel-bin/nori/python/build_pip_pkg
...
$ ./bazel-bin/nori/python/build_pip_pkg artifacts
```

You can find your own wheel file in `./artifacts`.

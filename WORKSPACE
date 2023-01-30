workspace(name = "nori-clone")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive", "http_jar")

## Dependencies
http_archive(
    name = "com_google_absl",
    sha256 = "70a2e30f715a7adcf5b7fcd2fcef7b624204b8e32ede8a23fd35ff5bd7d513b0",
    strip_prefix = "abseil-cpp-20230125.0",
    url = "https://github.com/abseil/abseil-cpp/archive/refs/tags/20230125.0.zip",
)

http_archive(
    name = "com_github_gflags_gflags",
    sha256 = "34af2f15cf7367513b352bdcd2493ab14ce43692d2dcd9dfc499492966c64dcf",
    strip_prefix = "gflags-2.2.2",
    url = "https://github.com/gflags/gflags/archive/v2.2.2.tar.gz",
)

http_archive(
    name = "com_google_protobuf",
    sha256 = "22fdaf641b31655d4b2297f9981fa5203b2866f8332d3c6333f6b0107bb320de",
    strip_prefix = "protobuf-21.12",
    url = "https://github.com/protocolbuffers/protobuf/archive/v21.12.tar.gz",
)

load("@com_google_protobuf//:protobuf_deps.bzl", "protobuf_deps")

protobuf_deps()

# TODO: replace glog with absl logging.
http_archive(
    name = "com_github_google_glog",
    sha256 = "21bc744fb7f2fa701ee8db339ded7dce4f975d0d55837a97be7d46e8382dea5a",
    strip_prefix = "glog-0.5.0",
    url = "https://github.com/google/glog/archive/v0.5.0.zip",
)

http_archive(
    name = "com_github_google_re2",
    sha256 = "3a20f05c57f907f78b817a53f2fb6e48077d2b1d0b17b39caf875c20f262230b",
    strip_prefix = "re2-2021-11-01",
    url = "https://github.com/google/re2/archive/2021-11-01.zip",
)

http_archive(
    name = "icu",
    build_file = "//third_party/icu:icu.bzl",
    patch_args = ["-p1"],
    patches = ["//third_party/icu:udata.patch"],
    sha256 = "65271a83fa81783d1272553f4564965ac2e32535a58b0b8141e9f4003afb0e3a",
    strip_prefix = "icu-release-64-2",
    url = "https://github.com/unicode-org/icu/archive/release-64-2.tar.gz",
)

http_archive(
    name = "com_github_google_snappy",
    build_file = "//third_party/snappy:snappy.bzl",
    sha256 = "e170ce0def2c71d0403f5cda61d6e2743373f9480124bcfcd0fa9b3299d428d9",
    strip_prefix = "snappy-1.1.9",
    url = "https://github.com/google/snappy/archive/1.1.9.zip",
)

http_archive(
    name = "darts_clone",
    build_file = "//third_party/darts_clone:darts.bzl",
    sha256 = "c97f55d05c98da6fcaf7f9ecc6a6dc6bc5b18b8564465f77abff8879d446491c",
    strip_prefix = "darts-clone-e40ce4627526985a7767444b6ed6893ab6ff8983",
    # darts_clone 0.32h
    url = "https://github.com/s-yata/darts-clone/archive/e40ce4627526985a7767444b6ed6893ab6ff8983.zip",
)

# Test Deps
http_archive(
    name = "com_google_googletest",
    sha256 = "ffa17fbc5953900994e2deec164bb8949879ea09b411e07f215bfbb1f87f4632",
    strip_prefix = "googletest-1.13.0",
    url = "https://github.com/google/googletest/archive/refs/tags/v1.13.0.zip",
)

# Build deps
http_archive(
    name = "io_bazel_rules_go",
    sha256 = "19ef30b21eae581177e0028f6f4b1f54c66467017be33d211ab6fc81da01ea4d",
    urls = [
        "https://mirror.bazel.build/github.com/bazelbuild/rules_go/releases/download/v0.38.0/rules_go-v0.38.0.zip",
        "https://github.com/bazelbuild/rules_go/releases/download/v0.38.0/rules_go-v0.38.0.zip",
    ],
)

load("@io_bazel_rules_go//go:deps.bzl", "go_register_toolchains", "go_rules_dependencies")

go_rules_dependencies()

go_register_toolchains(version = "1.19")

http_archive(
    name = "com_github_bazelbuild_buildtools",
    sha256 = "05eff86c1d444dde18d55ac890f766bce5e4db56c180ee86b5aacd6704a5feb9",
    strip_prefix = "buildtools-6.0.0",
    url = "https://github.com/bazelbuild/buildtools/archive/refs/tags/6.0.0.tar.gz",
)

# Python Rules
http_archive(
    name = "pybind11_bazel",
    sha256 = "c2ceb548c3a377ab2d686dde7bd4ba47a53cc68a518f21fbbcd77a694bb0b6e4",
    strip_prefix = "pybind11_bazel-5f458fa53870223a0de7eeb60480dd278b442698",
    urls = ["https://github.com/pybind/pybind11_bazel/archive/5f458fa53870223a0de7eeb60480dd278b442698.zip"],
)

http_archive(
    name = "pybind11",
    build_file = "@pybind11_bazel//:pybind11.BUILD",
    sha256 = "5d8c4c5dda428d3a944ba3d2a5212cb988c2fae4670d58075a5a49075a6ca315",
    strip_prefix = "pybind11-2.10.3",
    urls = ["https://github.com/pybind/pybind11/archive/v2.10.3.tar.gz"],
)

load("@pybind11_bazel//:python_configure.bzl", "python_configure")

python_configure(name = "local_config_python")

# Java Rules
http_archive(
    name = "rules_jvm_external",
    sha256 = "cd1a77b7b02e8e008439ca76fd34f5b07aecb8c752961f9640dea15e9e5ba1ca",
    strip_prefix = "rules_jvm_external-4.2",
    url = "https://github.com/bazelbuild/rules_jvm_external/archive/4.2.zip",
)

load("@rules_jvm_external//:defs.bzl", "maven_install")

maven_install(
    artifacts = [
        "org.apache.lucene:lucene-core:8.11.1",
        "org.apache.lucene:lucene-analyzers-nori:8.11.1",
    ],
    repositories = [
        "https://maven.google.com",
        "https://repo1.maven.org/maven2",
    ],
)

http_jar(
    name = "com_github_google_google_java_format",
    sha256 = "a036ac9392ff6f2e668791324c26bd73963b09682ed4a0d4cbc117fd6ea3fe55",
    url = "https://github.com/google/google-java-format/releases/download/v1.13.0/google-java-format-1.13.0-all-deps.jar",
)

# Packaging
http_archive(
    name = "rules_pkg",
    sha256 = "eea0f59c28a9241156a47d7a8e32db9122f3d50b505fae0f33de6ce4d9b61834",
    urls = [
        "https://mirror.bazel.build/github.com/bazelbuild/rules_pkg/releases/download/0.8.0/rules_pkg-0.8.0.tar.gz",
        "https://github.com/bazelbuild/rules_pkg/releases/download/0.8.0/rules_pkg-0.8.0.tar.gz",
    ],
)

load("@rules_pkg//:deps.bzl", "rules_pkg_dependencies")

rules_pkg_dependencies()

workspace(name = "nori-clone")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive", "http_jar")

# Deps
load("//nori:workspace.bzl", "nori_workspace")

nori_workspace()

# build Tools
http_archive(
    name = "io_bazel_rules_go",
    sha256 = "2b1641428dff9018f9e85c0384f03ec6c10660d935b750e3fa1492a281a53b0f",
    urls = [
        "https://mirror.bazel.build/github.com/bazelbuild/rules_go/releases/download/v0.29.0/rules_go-v0.29.0.zip",
        "https://github.com/bazelbuild/rules_go/releases/download/v0.29.0/rules_go-v0.29.0.zip",
    ],
)

load("@io_bazel_rules_go//go:deps.bzl", "go_register_toolchains", "go_rules_dependencies")

go_rules_dependencies()

go_register_toolchains(version = "1.17")

http_archive(
    name = "com_github_bazelbuild_buildtools",
    sha256 = "ae34c344514e08c23e90da0e2d6cb700fcd28e80c02e23e4d5715dddcb42f7b3",
    strip_prefix = "buildtools-4.2.2",
    url = "https://github.com/bazelbuild/buildtools/archive/refs/tags/4.2.2.tar.gz",
)

# protobuf
http_archive(
    name = "com_google_protobuf",
    strip_prefix = "protobuf-3.19.4",
    sha256 = "3bd7828aa5af4b13b99c191e8b1e884ebfa9ad371b0ce264605d347f135d2568",
    url = "https://github.com/protocolbuffers/protobuf/archive/v3.19.4.tar.gz",
)

load("@com_google_protobuf//:protobuf_deps.bzl", "protobuf_deps")

protobuf_deps()

# Test Deps
http_archive(
    name = "com_google_googletest",
    sha256 = "0eab4e490851b09de09e815954554459606edb1d775c644f4a31ff6b331c524b",
    strip_prefix = "googletest-e2f3978937c0244508135f126e2617a7734a68be",
    url = "https://github.com/google/googletest/archive/e2f3978937c0244508135f126e2617a7734a68be.zip",
)

# Python Rules
http_archive(
    name = "pybind11_bazel",
    patch_args = ["-p1"],
    patches = ["//third_party:pybind11_bazel.patch"],
    sha256 = "fec6281e4109115c5157ca720b8fe20c8f655f773172290b03f57353c11869c2",
    strip_prefix = "pybind11_bazel-72cbbf1fbc830e487e3012862b7b720001b70672",
    url = "https://github.com/pybind/pybind11_bazel/archive/72cbbf1fbc830e487e3012862b7b720001b70672.zip",
)

http_archive(
    name = "pybind11",
    build_file = "@pybind11_bazel//:pybind11.BUILD",
    sha256 = "057fb68dafd972bc13afb855f3b0d8cf0fa1a78ef053e815d9af79be7ff567cb",
    strip_prefix = "pybind11-2.9.0",
    url = "https://github.com/pybind/pybind11/archive/v2.9.0.tar.gz",
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

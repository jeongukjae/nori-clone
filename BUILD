# Linter, Formatter Rules
# buildifier for starlark, clang_format for c/c++, google_java_format for java
load("@com_github_bazelbuild_buildtools//buildifier:def.bzl", "buildifier", "buildifier_test")
load("//tools/lint/clang_format:def.bzl", "clang_format", "clang_format_test")
load("//tools/lint/google_java_format:def.bzl", "google_java_format", "google_java_format_test")

buildifier(name = "buildifier")

buildifier_test(
    name = "buildifier_test",
    srcs = glob([
        "**/*.bzl",
        "**/*.bazel",
        "**/BUILD",
    ]) + ["WORKSPACE"],
)

clang_format(
    name = "clang_format",
    exclude_patterns = ["./third_party/*"],
)

clang_format_test(
    name = "clang_format_test",
    exclude_patterns = ["./third_party/*"],
    config = ".clang-format",
    srcs = [
        "//nori/c:srcs",
        "//nori/c:headers",
    ],
)

google_java_format(name = "google_java_format")

google_java_format_test(
    name = "google_java_format_test",
    srcs = [
        "//tools/comparison:nori_runner_srcs",
    ],
)

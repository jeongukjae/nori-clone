load("@com_github_bazelbuild_buildtools//buildifier:def.bzl", "buildifier", "buildifier_test")
load("//tools/lint:clang_format.bzl", "clang_format", "clang_format_test")

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
    exclude_patterns = [
        "./third_party/*",
    ],
)

clang_format_test(
    name = "clang_format_test",
    exclude_patterns = [
        "./third_party/*",
    ],
)

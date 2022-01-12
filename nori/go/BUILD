load("@bazel_tools//tools/build_defs/pkg:pkg.bzl", "pkg_tar")

package(default_visibility = ["//visibility:private"])

licenses(["notice"])

filegroup(
    name = "headers",
    srcs = [
        "wrapper.h",
    ]
)

cc_library(
    name = "nori_wrapper",
    srcs = ["wrapper.cc"],
    hdrs = [":headers"],
    deps = ["//nori/lib:nori"]
)

pkg_tar(
    name = "cheaders",
    srcs = [":headers"],
    package_dir = "include/",
    strip_prefix = "/",
)

pkg_tar(
    name = "clib",
    srcs = [":nori_wrapper"],
    package_dir = "lib",
)

pkg_tar(
    name = "libnori_go",
    extension = "tar.gz",
    deps = [":cheaders", ":clib"],
)

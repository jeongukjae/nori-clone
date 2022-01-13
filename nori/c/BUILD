load("@bazel_tools//tools/build_defs/pkg:pkg.bzl", "pkg_tar")

package(default_visibility = [
    "//nori:__pkg__",
    "//nori:__subpackages__",
])

licenses(["notice"])

filegroup(
    name = "headers",
    srcs = ["c_api.h"],
)

cc_library(
    name = "c_api",
    srcs = [
        "c_api.cc",
        ":headers",
    ],
    alwayslink = True,
    deps = ["//nori/lib:nori"],
)

cc_binary(
    name = "libnori_c_api.so",
    deps = [":c_api"],
    linkshared = True,
)

cc_test(
    name = "nori_c_api_test",
    srcs = ["c_api_test.cc"],
    data = [
        "//dictionary",
    ],
    deps = [
        ":c_api",
        "@com_google_googletest//:gtest_main",
    ],
)

pkg_tar(
    name = "cheaders",
    srcs = [":headers"],
    package_dir = "include/",
    strip_prefix = "/",
)

pkg_tar(
    name = "clib",
    srcs = [":libnori_c_api.so"],
    package_dir = "lib",
)

pkg_tar(
    name = "libnori_c",
    extension = "tar.gz",
    deps = [
        ":cheaders",
        ":clib",
    ],
)

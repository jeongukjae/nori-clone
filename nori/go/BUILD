load("@bazel_tools//tools/build_defs/pkg:pkg.bzl", "pkg_tar")
load("@io_bazel_rules_go//go:def.bzl", "go_library", "go_test")

package(default_visibility = ["//visibility:private"])

licenses(["notice"])

go_library(
    name = "nori",
    srcs = ["nori.go"],
    cdeps = [":nori_wrapper"],
    cgo = True,
    importpath = "github.com/jeongukjae/nori-clone/nori/go",
)

go_test(
    name = "nori_test",
    srcs = ["nori_test.go"],
    data = ["//dictionary"],
    embed = [":nori"],
)

filegroup(
    name = "go_srcs",
    srcs = glob(["*.go"]) + ["go.mod"],
)

filegroup(
    name = "headers",
    srcs = ["wrapper.h"],
)

cc_library(
    name = "nori_wrapper",
    srcs = ["wrapper.cc"],
    hdrs = [":headers"],
    deps = ["//nori/lib:nori"],
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
    deps = [
        ":cheaders",
        ":clib",
    ],
)

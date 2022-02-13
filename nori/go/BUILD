load("@io_bazel_rules_go//go:def.bzl", "go_library", "go_test")

package(default_visibility = ["//visibility:public"])

licenses(["notice"])

go_library(
    name = "nori",
    srcs = ["nori.go"],
    cdeps = select({
        # because of the rpath setups in bazel, we cannot link shared library with cc_import
        "@platforms//os:osx": ["//nori/c:c_api"],
        "//conditions:default": ["//nori/c:shared_c_api"],
    }),
    cgo = True,
    clinkopts = select({
        "@platforms//os:osx": [],
        "//conditions:default": ["-ldl"],
    }),
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

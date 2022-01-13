load("@io_bazel_rules_go//go:def.bzl", "go_library", "go_test")

package(default_visibility = ["//visibility:private"])

licenses(["notice"])

go_library(
    name = "nori",
    srcs = ["nori.go"],
    cdeps = ["//nori/c:c_api"],
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

load("@rules_proto//proto:defs.bzl", "proto_library")

package(default_visibility = ["//visibility:public"])

cc_proto_library(
    name = "dictionary_cc_proto",
    deps = [":dictionary_proto"],
)

proto_library(
    name = "dictionary_proto",
    srcs = ["dictionary.proto"],
)

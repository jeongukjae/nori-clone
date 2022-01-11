load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

package(
    default_visibility = [
        "//nori:__pkg__",
        "//nori:__subpackages__",
    ],
    licenses = ["notice"],
)

cc_library(
    name = "dictionary",
    srcs = ["dictionary.cc"],
    hdrs = ["dictionary.h"],
    deps = [
        "//nori/lib:utils",
        "//nori/lib/protos:dictionary_cc_proto",
        "@com_github_google_glog//:glog",
        "@com_github_google_re2//:re2",
        "@com_github_google_snappy//:snappy",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
        "@darts_clone",
        "@icu//:common",
    ],
)

cc_test(
    name = "dictionary_test",
    srcs = ["dictionary_test.cc"],
    data = ["//dictionary"],
    linkstatic = True,
    deps = [
        ":dictionary",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "builder",
    srcs = ["builder.cc"],
    hdrs = ["builder.h"],
    deps = [
        ":dictionary",
        "//nori/lib:utils",
        "//nori/lib/protos:dictionary_cc_proto",
        "@com_github_google_glog//:glog",
        "@com_github_google_snappy//:snappy",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
        "@darts_clone",
    ],
)

cc_test(
    name = "builder_test",
    srcs = ["builder_test.cc"],
    data = ["//testdata:dictionaryBuilder"],
    linkstatic = True,
    deps = [
        ":builder",
        "@com_google_googletest//:gtest_main",
    ],
)

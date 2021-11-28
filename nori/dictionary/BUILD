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
)

cc_test(
    name = "dictionary_test",
    srcs = ["dictionary_test.cc"],
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
        "//nori:constant",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/status",
        "@com_github_google_glog//:glog",
    ],
)

cc_test(
    name = "builder_test",
    srcs = ["builder_test.cc"],
    data = ["//testdata:listDirectory"],
    deps = [
        ":builder",
        "@com_google_googletest//:gtest_main",
    ],
)

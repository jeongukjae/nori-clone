package(default_visibility = ["//visibility:public"])

filegroup(
    name = "clang_format_files",
    srcs = glob([
        "*.cc",
        "*.h",
    ]) + ["//nori/lib/dictionary:clang_format_files"],
)

cc_library(
    name = "nori",
    deps = [
        ":graphviz_visualize",
        ":tokenizer",
        ":utils",
        "//nori/lib/dictionary",
        "//nori/lib/protos:dictionary_cc_proto",
    ],
)

cc_library(
    name = "tokenizer",
    srcs = ["tokenizer.cc"],
    hdrs = ["tokenizer.h"],
    deps = [
        ":graphviz_visualize",
        ":utils",
        "//nori/lib/dictionary",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
        "@icu//:common",
    ],
)

cc_test(
    name = "tokenizer_test",
    srcs = ["tokenizer_test.cc"],
    data = [
        "//dictionary",
        "//dictionary:legacy_dictionary",
    ],
    deps = [
        ":tokenizer",
        "@com_google_absl//absl/log:check",
        "@com_google_googletest//:gtest",
    ],
)

cc_library(
    name = "graphviz_visualize",
    srcs = ["graphviz_visualize.cc"],
    hdrs = ["graphviz_visualize.h"],
    deps = [
        "//nori/lib/dictionary",
        "@com_google_absl//absl/strings",
    ],
)

cc_test(
    name = "graphviz_visualize_test",
    srcs = ["graphviz_visualize_test.cc"],
    deps = [
        ":graphviz_visualize",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "utils",
    srcs = ["utils.cc"],
    hdrs = ["utils.h"],
    deps = [
        "//nori/lib/protos:dictionary_cc_proto",
        "//third_party/icu/data:icu_normalization_data",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
        "@icu//:common",
    ],
)

cc_test(
    name = "utils_test",
    srcs = ["utils_test.cc"],
    data = ["//testdata:listDirectory"],
    deps = [
        ":utils",
        "@com_google_googletest//:gtest_main",
    ],
)

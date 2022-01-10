load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_binary(
    name = "build_dictionary",
    srcs = ["build_dictionary.cc"],
    deps = [
        "//nori/dictionary:builder",
        "@com_github_gflags_gflags//:gflags",
        "@com_github_google_glog//:glog",
        "@com_google_absl//absl/strings",
    ],
)

cc_binary(
    name = "check_dictionary",
    srcs = ["check_dictionary.cc"],
    deps = [
        "//nori/dictionary:dictionary",
        "@com_github_gflags_gflags//:gflags",
        "@com_github_google_glog//:glog",
        "@com_google_absl//absl/strings",
    ],
)

cc_binary(
    name = "visualize_graphviz",
    srcs = ["visualize_graphviz.cc"],
    deps = [
        "//nori/dictionary:dictionary",
        "//nori:tokenizer",
        "@com_github_gflags_gflags//:gflags",
        "@com_github_google_glog//:glog",
        "@com_google_absl//absl/strings",
    ],
)

cc_binary(
    name = "check_tokenize",
    srcs = ["check_tokenize.cc"],
    deps = [
        "//nori/dictionary:dictionary",
        "//nori:tokenizer",
        "@com_github_gflags_gflags//:gflags",
        "@com_github_google_glog//:glog",
        "@com_google_absl//absl/strings",
    ],
)

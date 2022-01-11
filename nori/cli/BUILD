load("@rules_cc//cc:defs.bzl", "cc_binary")

package(
    default_visibility = [
        "//nori:__pkg__",
        "//nori:__subpackages__",
    ],
    licenses = ["notice"],
)

cc_binary(
    name = "build_dictionary",
    srcs = ["build_dictionary.cc"],
    deps = [
        "//nori/lib:nori",
        "//nori/lib/dictionary:builder",
        "@com_github_gflags_gflags//:gflags",
        "@com_github_google_glog//:glog",
        "@com_google_absl//absl/strings",
    ],
)

cc_binary(
    name = "check_dictionary",
    srcs = ["check_dictionary.cc"],
    data = ["//dictionary"],
    deps = [
        "//nori/lib:nori",
        "@com_github_gflags_gflags//:gflags",
        "@com_github_google_glog//:glog",
        "@com_google_absl//absl/strings",
    ],
)

cc_binary(
    name = "visualize_graphviz",
    srcs = ["visualize_graphviz.cc"],
    data = ["//dictionary"],
    deps = [
        "//nori/lib:nori",
        "@com_github_gflags_gflags//:gflags",
        "@com_github_google_glog//:glog",
        "@com_google_absl//absl/strings",
    ],
)

cc_binary(
    name = "check_tokenize",
    srcs = ["check_tokenize.cc"],
    data = ["//dictionary"],
    deps = [
        "//nori/lib:nori",
        "@com_github_gflags_gflags//:gflags",
        "@com_github_google_glog//:glog",
        "@com_google_absl//absl/strings",
    ],
)

package(default_visibility = ["//visibility:public"])

filegroup(
    name = "clang_format_files",
    srcs = glob(["*.cc"]),
)

cc_binary(
    name = "build_dictionary",
    srcs = ["build_dictionary.cc"],
    deps = [
        "//nori/lib:nori",
        "//nori/lib/dictionary:builder",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/strings",
    ],
)

cc_binary(
    name = "check_dictionary",
    srcs = ["check_dictionary.cc"],
    data = ["//dictionary"],
    deps = [
        "//nori/lib:nori",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/strings",
    ],
)

cc_binary(
    name = "visualize_graphviz",
    srcs = ["visualize_graphviz.cc"],
    data = ["//dictionary"],
    deps = [
        "//nori/lib:nori",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/strings",
    ],
)

cc_binary(
    name = "check_tokenize",
    srcs = ["check_tokenize.cc"],
    data = ["//dictionary"],
    deps = [
        "//nori/lib:nori",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/strings",
    ],
)

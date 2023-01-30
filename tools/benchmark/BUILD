load("@io_bazel_rules_go//go:def.bzl", "go_binary")

package(default_visibility = ["//visibility:public"])

filegroup(
    name = "nori_runner_srcs",
    srcs = ["NoriRunner.java"],
)

java_binary(
    name = "nori_runner",
    srcs = [":nori_runner_srcs"],
    data = ["data.txt"],
    main_class = "NoriRunner",
    deps = [
        "@maven//:org_apache_lucene_lucene_analyzers_nori",
        "@maven//:org_apache_lucene_lucene_core",
    ],
)

py_binary(
    name = "nori_clone_runner_py",
    srcs = ["nori_clone_runner.py"],
    data = [
        "data.txt",
        "//dictionary",
    ],
    imports = ["../../nori/python"],
    main = "nori_clone_runner.py",
    deps = ["//nori/python/nori:nori_py"],
)

py_binary(
    name = "pynori_runner",
    srcs = ["pynori_runner.py"],
    data = ["data.txt"],
    main = "pynori_runner.py",
)

py_binary(
    name = "kiwipiepy_runner",
    srcs = ["kiwipiepy_runner.py"],
    data = ["data.txt"],
    main = "kiwipiepy_runner.py",
)

cc_binary(
    name = "nori_clone_runner_cc",
    srcs = ["nori_clone_runner.cc"],
    data = [
        "data.txt",
        "//dictionary",
    ],
    deps = [
        "//nori/lib:nori",
        "@com_github_gflags_gflags//:gflags",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/strings",
    ],
)

go_binary(
    name = "nori_clone_runner_go",
    srcs = ["nori_clone_runner.go"],
    data = [
        "data.txt",
        "//dictionary",
    ],
    deps = ["//nori/go:nori"],
)

sh_binary(
    name = "run_all",
    srcs = ["run_all.sh"],
    data = [
        "data.txt",
        ":kiwipiepy_runner",
        ":nori_clone_runner_cc",
        ":nori_clone_runner_go",
        ":nori_clone_runner_py",
        ":nori_runner",
        "//dictionary",
    ],
)

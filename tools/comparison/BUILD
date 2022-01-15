package(default_visibility = ["//visibility:public"])

filegroup(
    name = "nori_runner_srcs",
    srcs = ["NoriRunner.java"],
)

java_binary(
    name = "nori_runner",
    srcs = [":nori_runner_srcs"],
    main_class = "NoriRunner",
    deps = [
        "@maven//:org_apache_lucene_lucene_analyzers_nori",
        "@maven//:org_apache_lucene_lucene_core",
    ],
)

py_binary(
    name = "nori_clone_runner",
    srcs = ["nori_clone_runner.py"],
    data = ["//dictionary/legacy:dictionary"],
    imports = ["../../nori/python"],
    deps = ["//nori/python/nori:nori_py"],
)

sh_binary(
    name = "compare_outputs",
    srcs = ["compare_outputs.sh"],
    data = [
        "data.txt",
        ":nori_clone_runner",
        ":nori_runner",
        "//dictionary/legacy:dictionary",
    ],
)

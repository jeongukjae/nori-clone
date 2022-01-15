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
    ],
)

filegroup(
    name = "nori_visualizer_srcs",
    srcs = ["NoriVisualizer.java"],
)

java_binary(
    name = "nori_visualizer",
    srcs = [":nori_visualizer_srcs"],
    main_class = "NoriVisualizer",
    deps = [
        "@maven//:org_apache_lucene_lucene_analyzers_nori",
        "@maven//:org_apache_lucene_lucene_core",
    ],
)

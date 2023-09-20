package(
    default_visibility = ["//visibility:public"],
    licenses = ["notice"],
)

filegroup(
    name = "dictionary",
    srcs = [
        "latest-dictionary.nori",
        "latest-userdict.txt",
        ":legacy_dictionary",
    ],
)

filegroup(
    name = "legacy_dictionary",
    srcs = [
        "legacy-dictionary.nori",
        "legacy-userdict.txt",
    ],
)

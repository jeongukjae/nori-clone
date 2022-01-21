package(default_visibility = ["//visibility:public"])

licenses(["notice"])

filegroup(
    name = "license",
    srcs = ["COPYING"],
)

cc_library(
    name = "snappy",
    srcs = [
        "snappy.cc",
        "snappy-sinksource.cc",
        "snappy-stubs-internal.cc",
    ],
    hdrs = [
        "snappy.h",
        "snappy-internal.h",
        "snappy-sinksource.h",
        "snappy-stubs-internal.h",
        "snappy-stubs-public.h",
    ],
)

genrule(
    name = "snappy_stubs_public_h",
    srcs = ["snappy-stubs-public.h.in"],
    outs = ["snappy-stubs-public.h"],
    cmd = ("sed " +
           "-e 's/$${\\(.*\\)_01}/\\1/g' " +
           "-e 's/$${SNAPPY_MAJOR}/1/g' " +
           "-e 's/$${SNAPPY_MINOR}/1/g' " +
           "-e 's/$${SNAPPY_PATCHLEVEL}/9/g' " +
           "$< >$@"),
)

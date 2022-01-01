"""
Builds ICU library.

This code is from https://github.com/tensorflow/text/blob/master/third_party/icu/BUILD.bzl
"""

package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # Apache 2.0

exports_files(["icu4c/LICENSE"])

cc_library(
    name = "headers",
    hdrs = glob(["icu4c/source/common/unicode/*.h"]),
    includes = ["icu4c/source/common"],
    deps = [],
)

cc_library(
    name = "common",
    hdrs = glob(["icu4c/source/common/unicode/*.h"]),
    includes = ["icu4c/source/common"],
    deps = [":icuuc"],
)

cc_library(
    name = "icuuc",
    srcs = glob(
        [
            "icu4c/source/common/*.c",
            "icu4c/source/common/*.cpp",
            "icu4c/source/stubdata/*.cpp",
        ],
    ),
    hdrs = glob([
        "icu4c/source/common/*.h",
    ]),
    copts = [
        "-DU_COMMON_IMPLEMENTATION",
    ] + select({
        ":apple": [
            "-Wno-shorten-64-to-32",
            "-Wno-unused-variable",
        ],
        ":windows": [
            "/utf-8",
            "/DLOCALE_ALLOW_NEUTRAL_NAMES=0",
        ],
        "//conditions:default": [],
    }),
    tags = ["requires-rtti"],
    visibility = [
        "//visibility:private",
    ],
    deps = [
        ":headers",
    ],
)

config_setting(
    name = "apple",
    values = {"cpu": "darwin"},
)

config_setting(
    name = "windows",
    values = {"cpu": "x64_windows"},
)

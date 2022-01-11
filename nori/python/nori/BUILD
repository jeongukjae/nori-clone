load("@pybind11_bazel//:build_defs.bzl", "pybind_extension")

package(default_visibility = ["//visibility:public"])

pybind_extension(
    name = "bind",
    srcs = ["bind.cc"],
    deps = [
        "//nori/lib:nori",
        "@com_google_absl//absl/strings",
    ],
)

py_library(
    name = "nori_py",
    srcs = ["__init__.py"],
    data = [
        ":bind.so",
    ],
)

py_test(
    name = "bind_test",
    srcs = ["bind_test.py"],
    deps = [":nori_py"],
    data = ["//dictionary"],
    imports = [".."],
)

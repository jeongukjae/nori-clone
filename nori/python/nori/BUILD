load("@pybind11_bazel//:build_defs.bzl", "pybind_extension")

package(default_visibility = ["//visibility:public"])

filegroup(
    name = "clang_format_files",
    srcs = glob(["*.cc"]),
)

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
        "bind.pyi",
        ":bind.so",
    ],
)

py_test(
    name = "bind_test",
    srcs = ["bind_test.py"],
    data = ["//dictionary"],
    imports = [".."],
    deps = [":nori_py"],
)

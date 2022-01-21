load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("//third_party:repo.bzl", "clean_dep", "clean_deps")

def configure_icu():
    http_archive(
        name = "icu",
        build_file = clean_dep("//third_party/icu:BUILD.bzl"),
        patch_args = ["-p1"],
        patches = clean_deps(["//third_party/icu:udata.patch"]),
        sha256 = "65271a83fa81783d1272553f4564965ac2e32535a58b0b8141e9f4003afb0e3a",
        strip_prefix = "icu-release-64-2",
        url = "https://github.com/unicode-org/icu/archive/release-64-2.tar.gz",
    )

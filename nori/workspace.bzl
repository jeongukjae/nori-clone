load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("//third_party/icu:workspace.bzl", "configure_icu")
load("//third_party/snappy:workspace.bzl", "configure_snappy")
load("//third_party/darts_clone:workspace.bzl", "configure_darts_clone")

def nori_workspace():
    http_archive(
        name = "com_google_absl",
        sha256 = "32a00f5834195d6656097c800a773e2fc766741e434d1eff092ed5578a21dd3a",
        strip_prefix = "abseil-cpp-ec0d76f1d012cc1a4b3b08dfafcfc5237f5ba2c9",
        url = "https://github.com/abseil/abseil-cpp/archive/ec0d76f1d012cc1a4b3b08dfafcfc5237f5ba2c9.zip",
    )

    http_archive(
        name = "com_github_gflags_gflags",
        sha256 = "34af2f15cf7367513b352bdcd2493ab14ce43692d2dcd9dfc499492966c64dcf",
        strip_prefix = "gflags-2.2.2",
        url = "https://github.com/gflags/gflags/archive/v2.2.2.tar.gz",
    )

    http_archive(
        name = "com_github_google_glog",
        sha256 = "21bc744fb7f2fa701ee8db339ded7dce4f975d0d55837a97be7d46e8382dea5a",
        strip_prefix = "glog-0.5.0",
        url = "https://github.com/google/glog/archive/v0.5.0.zip",
    )

    http_archive(
        name = "com_github_google_re2",
        sha256 = "3a20f05c57f907f78b817a53f2fb6e48077d2b1d0b17b39caf875c20f262230b",
        strip_prefix = "re2-2021-11-01",
        url = "https://github.com/google/re2/archive/2021-11-01.zip",
    )

    configure_icu()
    configure_snappy()
    configure_darts_clone()

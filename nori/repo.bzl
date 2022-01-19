load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

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
        name = "icu",
        build_file = "//third_party/icu:BUILD.bzl",
        patch_args = ["-p1"],
        patches = ["//third_party/icu:udata.patch"],
        sha256 = "65271a83fa81783d1272553f4564965ac2e32535a58b0b8141e9f4003afb0e3a",
        strip_prefix = "icu-release-64-2",
        url = "https://github.com/unicode-org/icu/archive/release-64-2.tar.gz",
    )

    http_archive(
        name = "darts_clone",
        build_file = "//third_party:darts_clone.BUILD",
        sha256 = "c97f55d05c98da6fcaf7f9ecc6a6dc6bc5b18b8564465f77abff8879d446491c",
        strip_prefix = "darts-clone-e40ce4627526985a7767444b6ed6893ab6ff8983",
        # darts_clone 0.32h
        url = "https://github.com/s-yata/darts-clone/archive/e40ce4627526985a7767444b6ed6893ab6ff8983.zip",
    )

    http_archive(
        name = "com_github_google_snappy",
        build_file = "//third_party:snappy.BUILD",
        sha256 = "e170ce0def2c71d0403f5cda61d6e2743373f9480124bcfcd0fa9b3299d428d9",
        strip_prefix = "snappy-1.1.9",
        url = "https://github.com/google/snappy/archive/1.1.9.zip",
    )

    http_archive(
        name = "com_github_google_re2",
        sha256 = "3a20f05c57f907f78b817a53f2fb6e48077d2b1d0b17b39caf875c20f262230b",
        strip_prefix = "re2-2021-11-01",
        url = "https://github.com/google/re2/archive/2021-11-01.zip",
    )

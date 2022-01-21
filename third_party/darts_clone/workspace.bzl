load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def configure_darts_clone():
    http_archive(
        name = "darts_clone",
        build_file = "//third_party/darts_clone:BUILD.bzl",
        sha256 = "c97f55d05c98da6fcaf7f9ecc6a6dc6bc5b18b8564465f77abff8879d446491c",
        strip_prefix = "darts-clone-e40ce4627526985a7767444b6ed6893ab6ff8983",
        # darts_clone 0.32h
        url = "https://github.com/s-yata/darts-clone/archive/e40ce4627526985a7767444b6ed6893ab6ff8983.zip",
    )

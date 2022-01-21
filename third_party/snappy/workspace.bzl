load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def configure_snappy():
    http_archive(
        name = "com_github_google_snappy",
        build_file = "//third_party/snappy:BUILD.bzl",
        sha256 = "e170ce0def2c71d0403f5cda61d6e2743373f9480124bcfcd0fa9b3299d428d9",
        strip_prefix = "snappy-1.1.9",
        url = "https://github.com/google/snappy/archive/1.1.9.zip",
    )

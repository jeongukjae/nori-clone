workspace(name = "nori")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

# RULES
http_archive(
    name = "rules_cc",
    sha256 = "68cece0593cca62ba7bcf47b6627f97d55fb9127041572767606f984c2c6ee9e",
    strip_prefix = "rules_cc-081771d4a0e9d7d3aa0eed2ef389fa4700dfb23e",
    urls = ["https://github.com/bazelbuild/rules_cc/archive/081771d4a0e9d7d3aa0eed2ef389fa4700dfb23e.zip"],
)

http_archive(
    name = "rules_proto",
    sha256 = "66bfdf8782796239d3875d37e7de19b1d94301e8972b3cbd2446b332429b4df1",
    strip_prefix = "rules_proto-4.0.0",
    urls = [
        "https://mirror.bazel.build/github.com/bazelbuild/rules_proto/archive/refs/tags/4.0.0.tar.gz",
        "https://github.com/bazelbuild/rules_proto/archive/refs/tags/4.0.0.tar.gz",
    ],
)

load("@rules_proto//proto:repositories.bzl", "rules_proto_dependencies", "rules_proto_toolchains")

rules_proto_dependencies()

rules_proto_toolchains()

# LIBRARIES
http_archive(
    name = "com_google_absl",
    sha256 = "32a00f5834195d6656097c800a773e2fc766741e434d1eff092ed5578a21dd3a",
    strip_prefix = "abseil-cpp-ec0d76f1d012cc1a4b3b08dfafcfc5237f5ba2c9",
    urls = ["https://github.com/abseil/abseil-cpp/archive/ec0d76f1d012cc1a4b3b08dfafcfc5237f5ba2c9.zip"],
)

http_archive(
    name = "com_google_googletest",
    sha256 = "0eab4e490851b09de09e815954554459606edb1d775c644f4a31ff6b331c524b",
    strip_prefix = "googletest-e2f3978937c0244508135f126e2617a7734a68be",
    urls = ["https://github.com/google/googletest/archive/e2f3978937c0244508135f126e2617a7734a68be.zip"],
)

http_archive(
    name = "com_github_google_benchmark",
    sha256 = "bb9f771e942387c4e000c5bb067ef225a6ef3a9b5a2ce9084ede8dcdaf230bfd",
    strip_prefix = "benchmark-fe2e8aa1b4b01a8d2a7675c1edb3fb0ed48ce11c",
    urls = ["https://github.com/google/benchmark/archive/fe2e8aa1b4b01a8d2a7675c1edb3fb0ed48ce11c.zip"],
)

http_archive(
    name = "com_github_google_glog",
    build_file = "@//third_party:glog_no_gflags.BUILD",
    sha256 = "21bc744fb7f2fa701ee8db339ded7dce4f975d0d55837a97be7d46e8382dea5a",
    strip_prefix = "glog-0.5.0",
    urls = ["https://github.com/google/glog/archive/v0.5.0.zip"],
)

http_archive(
    name = "icu",
    build_file = "//third_party/icu:BUILD",
    patch_args = ["-p1"],
    patches = ["//third_party/icu:udata.patch"],
    sha256 = "10cd92f1585c537d937ecbb587f6c3b36a5275c87feabe05d777a828677ec32f",
    strip_prefix = "icu-release-64-2",
    urls = ["https://github.com/unicode-org/icu/archive/release-64-2.zip"],
)

http_archive(
    name = "darts_clone",
    build_file = "//third_party:darts_clone.BUILD",
    sha256 = "c97f55d05c98da6fcaf7f9ecc6a6dc6bc5b18b8564465f77abff8879d446491c",
    strip_prefix = "darts-clone-e40ce4627526985a7767444b6ed6893ab6ff8983",
    urls = [
        # darts_clone 0.32h
        "https://github.com/s-yata/darts-clone/archive/e40ce4627526985a7767444b6ed6893ab6ff8983.zip",
    ],
)

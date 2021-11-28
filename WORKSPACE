workspace(name = "nori")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "com_google_absl",
    strip_prefix = "abseil-cpp-ec0d76f1d012cc1a4b3b08dfafcfc5237f5ba2c9",
    sha256 = "32a00f5834195d6656097c800a773e2fc766741e434d1eff092ed5578a21dd3a",
    urls = ["https://github.com/abseil/abseil-cpp/archive/ec0d76f1d012cc1a4b3b08dfafcfc5237f5ba2c9.zip"],
)

http_archive(
    name = "rules_cc",
    strip_prefix = "rules_cc-081771d4a0e9d7d3aa0eed2ef389fa4700dfb23e",
    urls = ["https://github.com/bazelbuild/rules_cc/archive/081771d4a0e9d7d3aa0eed2ef389fa4700dfb23e.zip"],
    sha256 = "68cece0593cca62ba7bcf47b6627f97d55fb9127041572767606f984c2c6ee9e",
)

http_archive(
    name = "com_google_googletest",
    strip_prefix = "googletest-e2f3978937c0244508135f126e2617a7734a68be",
    sha256 = "0eab4e490851b09de09e815954554459606edb1d775c644f4a31ff6b331c524b",
    urls = ["https://github.com/google/googletest/archive/e2f3978937c0244508135f126e2617a7734a68be.zip"],
)

http_archive(
    name = "com_github_google_benchmark",
    urls = ["https://github.com/google/benchmark/archive/fe2e8aa1b4b01a8d2a7675c1edb3fb0ed48ce11c.zip"],
    sha256 = "bb9f771e942387c4e000c5bb067ef225a6ef3a9b5a2ce9084ede8dcdaf230bfd",
    strip_prefix = "benchmark-fe2e8aa1b4b01a8d2a7675c1edb3fb0ed48ce11c",
)

http_archive(
    name = "com_github_google_glog",
    sha256 = "21bc744fb7f2fa701ee8db339ded7dce4f975d0d55837a97be7d46e8382dea5a",
    strip_prefix = "glog-0.5.0",
    build_file = "@//third_party:glog_no_gflags.BUILD",
    urls = ["https://github.com/google/glog/archive/v0.5.0.zip"],
)

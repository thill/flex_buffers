load("@rules_cc//cc:defs.bzl", "cc_test")
load("//bazel:cc_opts.bzl", "default_copts")

cc_library(
    name = "tests",
    testonly = True,
    srcs = glob(["**/*.cc"]),
    hdrs = glob(["**/*.h"]),
    copts = default_copts(),
    visibility = ["//visibility:private"],
    deps = [
        "//:flexbuf",
        "@com_github_catchorg_catch2//:lib",
    ],
)

cc_test(
    name = "test",
    copts = default_copts(),
    deps = [
        ":tests",
        "@com_github_catchorg_catch2//:main",
    ],
)

load("@rules_cc//cc:defs.bzl", "cc_test")
load("//bazel:cc_opts.bzl", "default_copts")

cc_library(
    name = "tests",
    srcs = glob(["**/*.cc"]),
    hdrs = glob(["**/*.h"]),
    visibility = ["//visibility:private"],
    copts = default_copts(),
    testonly = True,
    deps = [
        "@com_github_catchorg_catch2//:lib",
        "//:flexbuf",
    ]
)

cc_test(
    name = "test",
    copts = default_copts(),
    deps = [
        ":tests",
        "@com_github_catchorg_catch2//:main",
    ],
)

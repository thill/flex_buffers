load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "lib",
    hdrs = glob(["include/fmt/*.h"]),
    defines = ["FMT_HEADER_ONLY"],
    includes = ["include"],
    visibility = ["//visibility:public"],
)

workspace(name = "flex_buffers")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "rules_cc",
    urls = ["https://github.com/bazelbuild/rules_cc/archive/77099ee80d15004ddbac96234761c0a564479131.zip"],
    strip_prefix = "rules_cc-77099ee80d15004ddbac96234761c0a564479131",
    sha256 = "f1a11c4b697d988d8ea3de71d7f79cab9c8f599b32017f203a5e037709a6a3b9"
)

http_archive(
   name = "rules_foreign_cc",
   strip_prefix = "rules_foreign_cc-04c04fe7d2fa09e46c630c37d8f32e56598527ca",
   url = "https://github.com/bazelbuild/rules_foreign_cc/archive/04c04fe7d2fa09e46c630c37d8f32e56598527ca.zip",
   sha256 = "3b69272ed947fc018c40185f3b3b0ee1060c311c84fba42aae866612da989b61",
)
load("@rules_foreign_cc//:workspace_definitions.bzl", "rules_foreign_cc_dependencies")
rules_foreign_cc_dependencies();

COM_GITHUB_FMTLIB_FMT_TAG = "7.1.3"
COM_GITHUB_FMTLIB_FMT_SHA = "5d98c504d0205f912e22449ecdea776b78ce0bb096927334f80781e720084c9f"
http_archive(
    name = "com_github_fmtlib_fmt",
    build_file = "//bazel:com_github_fmtlib_fmt.BUILD",
    sha256 = COM_GITHUB_FMTLIB_FMT_SHA,
    url = "https://github.com/fmtlib/fmt/releases/download/%s/fmt-%s.zip" % (COM_GITHUB_FMTLIB_FMT_TAG, COM_GITHUB_FMTLIB_FMT_TAG),
    strip_prefix = "fmt-%s" % COM_GITHUB_FMTLIB_FMT_TAG,
)

COM_GITHUB_CATCHORG_CATCH2_TAG = "2.13.4"
COM_GITHUB_CATCHORG_CATCH2_SHA = "f9f957db59e29b099f9eb10f9dcec1bbdbc6da07b9dd8b6b3149ed6a57f986da"
http_archive(
    name = "com_github_catchorg_catch2",
    build_file = "//bazel:com_github_catchorg_catch2.BUILD",
    sha256 = COM_GITHUB_CATCHORG_CATCH2_SHA,
    url = "https://github.com/catchorg/Catch2/archive/v%s.zip" % COM_GITHUB_CATCHORG_CATCH2_TAG,
    strip_prefix = "Catch2-%s" % COM_GITHUB_CATCHORG_CATCH2_TAG,
)


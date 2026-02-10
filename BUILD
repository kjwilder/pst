load("@rules_cc//cc:cc_binary.bzl", "cc_binary")
load("@rules_cc//cc:cc_library.bzl", "cc_library")
load("@rules_cc//cc:cc_test.bzl", "cc_test")

cc_binary(
    name = "pst",
    srcs = ["pst.cc"],
    deps = [":pst_lib"],
)

cc_library(
    name = "pst_lib",
    srcs = ["pst_lib.cc"],
    hdrs = ["pst.h"],
)

cc_test(
    name = "pst_test",
    srcs = ["pst_test.cc"],
    data = [
        "testdata/sample1.txt",
        "testdata/sample2.txt",
        "testdata/sample3.txt",
        "testdata/sample1.txt.ps",
        "testdata/sample2.txt.ps",
        "testdata/sample3.txt.ps",
    ] + glob(["fonts/*.nfm"]),
    deps = [
        ":pst_lib",
        "@googletest//:gtest_main",
    ],
    size = "small",
)

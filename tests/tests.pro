TEMPLATE = subdirs
SUBDIRS += \
    auto \
    common \
    benchmarks

auto.depends = common
benchmarks.depends = common

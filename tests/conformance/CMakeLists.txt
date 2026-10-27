cmake_minimum_required(VERSION 3.28)

# The chip's conformance tree (Kvasir_SDK tests/conformance/conformance.cmake): this package's headers compiled for the
# target with a firmware's flags, against the generated register headers. Building is the test - the contract rows
# (conformance.cpp), the dependent register writes (instantiate.cpp) and every public header alone. The chip and core
# SVDs convert with the write-only guard on the way.
#
# cmake -S tests/conformance -B tests/conformance/build -G Ninja [-DCOMPILE_TARGET=arm_gcc] && cmake --build
# tests/conformance/build
#
# Kvasir_SDK and kvasir_devices: the checkouts next to this one, else fetched (FETCHCONTENT_SOURCE_DIR_KVASIR_SDK /
# _KVASIR_DEVICES point a lone checkout at local copies).

get_filename_component(CHIP_ROOT "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)
# Kvasir_SDK: KVASIR_ROOT, else the checkout next to this repository, else fetched; its submodules and the other
# packages: kvasir_locate (Kvasir_SDK cmake/kvasir_locate.cmake)
if(NOT KVASIR_ROOT AND EXISTS "${CMAKE_CURRENT_LIST_DIR}/../../../Kvasir_SDK/cmake/kvasir_locate.cmake")
    get_filename_component(KVASIR_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../../Kvasir_SDK" ABSOLUTE)
elseif(NOT KVASIR_ROOT)
    include(FetchContent)
    FetchContent_Declare(
        kvasir_sdk
        GIT_REPOSITORY https://github.com/kvasir-io/Kvasir_SDK.git
        GIT_TAG master
        GIT_SHALLOW TRUE
        GIT_SUBMODULES "" SOURCE_SUBDIR fetch-only)
    FetchContent_MakeAvailable(kvasir_sdk)
    set(KVASIR_ROOT "${kvasir_sdk_SOURCE_DIR}")
endif()
include("${KVASIR_ROOT}/cmake/kvasir_locate.cmake")
kvasir_locate(SDK_SUBMODULES ALL DEVICES)
if(NOT KVASIR_DEVICES_ROOT)
    FetchContent_Declare(
        kvasir_devices
        GIT_REPOSITORY https://github.com/kvasir-io/kvasir_devices.git
        GIT_TAG master
        GIT_SHALLOW TRUE
        SOURCE_SUBDIR none)
    FetchContent_MakeAvailable(kvasir_devices)
    set(KVASIR_DEVICES_ROOT ${kvasir_devices_SOURCE_DIR})
endif()

include(${KVASIR_ROOT}/tests/conformance/conformance.cmake)
kvasir_conformance_begin()

project(chip_atsame51_conformance CXX C ASM)

include(${KVASIR_ROOT}/cmake/util.cmake)

kvasir_conformance_project(
    NAME
    chip_atsame51_conformance
    SOURCES
    conformance.cpp
    INSTANTIATE
    instantiate.cpp
    CONFIG_HEADER
    kvasir_config.hpp
    HEADERS
    ${CHIP_ROOT}/src/chip/*.hpp
    ${CHIP_ROOT}/src/chip/atsam_common/*.hpp
    NOT_ALONE
    not_alone.txt)

# What the E5x refuses at compile time (tree_must_fail.cpp): TREE_FAIL=0 compiles in `all` (the control), each case must
# fail with its own message (ctest in this tree, both compilers).
kvasir_conformance_must_fail(
    NAME
    tree_must_fail
    PROJECT
    chip_atsame51_conformance
    SOURCE
    tree_must_fail.cpp
    DEFINE
    TREE_FAIL
    CASES
    "1|f_CPU above the part's maximum"
    "2|this SAM part has no such clock source"
    "3|FDPLL reference: wanted 32000[.][.]3200000 Hz, got 4000000"
    "4|the FDPLL setting does not make [.]cpu from [.]source[.]hz"
    "5|the E5x DFLL48M open loop as the reset leaves it"
    "6|the E5x has XOSC0 and XOSC1"
    "7|SAM E5x XOSC: a crystal of 8[.][.]48 MHz"
    "8|or an XIN clock up to 48 MHz"
    "9|no setting makes 119999999 Hz from 8000000 Hz")

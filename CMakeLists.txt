cmake_minimum_required(VERSION 3.22)

# Aculele: a physically modelled ukulele built on Acustra's engine. The
# JUCE-free engine and performer, the demo renderer and two analysis tools;
# there is no plug-in or editor yet.
project(Aculele VERSION 0.4.12 LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ACULELE_BUILD_TOOLS "Build the demo renderer and the analysis tools" ON)
option(ACULELE_ENABLE_SANITIZERS "Instrument the DSP, tests and tools with ASan and UBSan" OFF)
option(ACULELE_BUILD_FUZZER "Build AculeleDspFuzz as a Clang libFuzzer target" OFF)
option(ACULELE_REQUIRE_PYTHON_TESTS
    "Fail to configure unless Python 3 is found for Aculele.DemoMidiSources (CI sets it)" OFF)

if(ACULELE_ENABLE_SANITIZERS)
    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        message(FATAL_ERROR "ACULELE_ENABLE_SANITIZERS needs Clang or GCC")
    endif()
    add_compile_options(-fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
endif()

include(CTest)
find_package(Threads REQUIRED)

set(ACULELE_DSP_SOURCES
    Source/DSP/AculeleEngine.cpp
    Source/DSP/AculelePerformer.cpp)

add_library(AculeleDSP STATIC
    ${ACULELE_DSP_SOURCES}
    Source/DSP/AculeleEngine.h
    Source/DSP/AculelePerformer.h
    Source/DSP/AculelePresets.h)
target_include_directories(AculeleDSP PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Source")
target_compile_features(AculeleDSP PUBLIC cxx_std_20)
set_target_properties(AculeleDSP PROPERTIES POSITION_INDEPENDENT_CODE ON)
if(MSVC)
    target_compile_options(AculeleDSP PRIVATE /W4 /permissive-)
else()
    target_compile_options(AculeleDSP PRIVATE -Wall -Wextra -Wpedantic)
endif()

# A Reason Rack Extension compiles this engine and performer itself, as C++17
# (the Jukebox toolchain forces -std=c++17) with -Wall -Wextra -Werror. Build
# them that way here too, so a C++20-only construct or a new warning fails in
# this repository rather than only in the Rack Extension's.
if(NOT MSVC)
    add_library(AculeleDSPCxx17 OBJECT ${ACULELE_DSP_SOURCES})
    target_include_directories(AculeleDSPCxx17 PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Source")
    set_target_properties(AculeleDSPCxx17 PROPERTIES
        CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)
    target_compile_options(AculeleDSPCxx17 PRIVATE -Wall -Wextra -Werror)
endif()

if(ACULELE_BUILD_TOOLS)
    # One README demo per factory preset from Docs/audio/midi (Tools/
    # PresetDemoRenderer.h is the shared portfolio copy; do not edit it here).
    add_executable(RenderAculelePresetDemos Tools/RenderPresetDemos.cpp)
    target_link_libraries(RenderAculelePresetDemos PRIVATE AculeleDSP Threads::Threads)

    add_executable(AculeleProbe Tools/AculeleProbe.cpp)
    target_link_libraries(AculeleProbe PRIVATE AculeleDSP)

    add_executable(AculeleBodyModes Tools/AculeleBodyModes.cpp)
    target_link_libraries(AculeleBodyModes PRIVATE AculeleDSP)

    add_executable(AculeleBridgeMobility Tools/AculeleBridgeMobility.cpp)
    target_link_libraries(AculeleBridgeMobility PRIVATE AculeleDSP)

    # Writes LevelMatchData.h and reports each preset on Reason's channel meter.
    add_executable(AculeleLevels Tools/AculeleLevels.cpp)
    target_link_libraries(AculeleLevels PRIVATE AculeleDSP Threads::Threads)
endif()

if(BUILD_TESTING)
    if(ACULELE_REQUIRE_PYTHON_TESTS)
        find_package(Python3 REQUIRED COMPONENTS Interpreter)
    else()
        find_package(Python3 COMPONENTS Interpreter)
    endif()

    add_executable(AculeleTuningTests Tests/TuningTests.cpp)
    target_link_libraries(AculeleTuningTests PRIVATE AculeleDSP)
    add_test(NAME Aculele.Tuning COMMAND AculeleTuningTests)

    add_executable(AculeleChordFillTests Tests/ChordFillTests.cpp)
    target_link_libraries(AculeleChordFillTests PRIVATE AculeleDSP)
    add_test(NAME Aculele.ChordFill COMMAND AculeleChordFillTests)

    add_executable(AculeleNoteChangeTests Tests/NoteChangeTests.cpp)
    target_link_libraries(AculeleNoteChangeTests PRIVATE AculeleDSP)
    add_test(NAME Aculele.NoteChange COMMAND AculeleNoteChangeTests)

    add_executable(AculeleStrumDirectionTests Tests/StrumDirectionTests.cpp)
    target_link_libraries(AculeleStrumDirectionTests PRIVATE AculeleDSP)
    add_test(NAME Aculele.StrumDirection COMMAND AculeleStrumDirectionTests)

    add_executable(AculeleChuckTests Tests/ChuckTests.cpp)
    target_link_libraries(AculeleChuckTests PRIVATE AculeleDSP)
    add_test(NAME Aculele.Chuck COMMAND AculeleChuckTests)

    add_executable(AculelePreparedDispersionTests Tests/PreparedDispersionTests.cpp)
    target_link_libraries(AculelePreparedDispersionTests PRIVATE AculeleDSP)
    add_test(NAME Aculele.PreparedDispersion COMMAND AculelePreparedDispersionTests)

    # U4 input fuzzing (Tests/DspFuzz.cpp). The replay build runs on every
    # platform: committed reproducers in Tests/Fixtures/fuzz, then seeded
    # random inputs. The libFuzzer build compiles the DSP with coverage, so it
    # needs Clang; its short run is the fuzz tier.
    add_executable(AculeleDspFuzzReplay Tests/DspFuzz.cpp)
    target_link_libraries(AculeleDspFuzzReplay PRIVATE AculeleDSP)
    target_compile_definitions(AculeleDspFuzzReplay PRIVATE PROTOCODUS_FUZZ_MAIN)
    file(GLOB ACULELE_FUZZ_REPRODUCERS CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/Tests/Fixtures/fuzz/*")
    add_test(NAME Aculele.DspFuzzReplay
             COMMAND AculeleDspFuzzReplay -seed=1 -runs=300 -max_len=1024 ${ACULELE_FUZZ_REPRODUCERS})
    set_tests_properties(Aculele.DspFuzzReplay PROPERTIES TIMEOUT 300)
    if(ACULELE_BUILD_FUZZER)
        if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            message(FATAL_ERROR "ACULELE_BUILD_FUZZER needs Clang")
        endif()
        add_executable(AculeleDspFuzz Tests/DspFuzz.cpp ${ACULELE_DSP_SOURCES})
        target_include_directories(AculeleDspFuzz PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Source")
        target_compile_options(AculeleDspFuzz PRIVATE
            -fsanitize=fuzzer,address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer)
        target_link_options(AculeleDspFuzz PRIVATE -fsanitize=fuzzer,address,undefined)
        file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/fuzz-corpus" "${CMAKE_CURRENT_BINARY_DIR}/fuzz-artifacts")
        add_test(NAME Aculele.DspFuzz
                 COMMAND AculeleDspFuzz -max_total_time=120 -seed=1 -max_len=4096 -rss_limit_mb=4096
                         "-artifact_prefix=${CMAKE_CURRENT_BINARY_DIR}/fuzz-artifacts/"
                         "${CMAKE_CURRENT_BINARY_DIR}/fuzz-corpus")
        set_tests_properties(Aculele.DspFuzz PROPERTIES LABELS fuzz TIMEOUT 600)
    endif()

    if(Python3_Interpreter_FOUND)
        # The committed demo phrases are what the writer writes (G3a).
        add_test(NAME Aculele.DemoMidiSources
                 COMMAND "${Python3_EXECUTABLE}" -B
                         "${CMAKE_CURRENT_SOURCE_DIR}/Tools/write_preset_midi.py" --check)
    endif()

    if(ACULELE_BUILD_TOOLS)
        # Every preset demo renders finite, audible, under the ceiling, 8-12 s
        # long and bit-identical on a second render.
        add_test(NAME Aculele.AudioDemos
                 COMMAND RenderAculelePresetDemos --jobs 4 --check "${CMAKE_CURRENT_SOURCE_DIR}/Docs/audio/midi")
        set_tests_properties(Aculele.AudioDemos PROPERTIES LABELS audio TIMEOUT 600)
        # Each preset on Reason's channel meter: typical playing at -9 VU,
        # every Capture and Picking near it, the loudest chords out of the red
        # (Tools/AculeleLevels.cpp).
        add_test(NAME Aculele.Levels COMMAND AculeleLevels --check --jobs 4)
        set_tests_properties(Aculele.Levels PROPERTIES LABELS audio TIMEOUT 900)
        # The bridge's level against the one measured ukulele (Elie et al. 2011).
        add_test(NAME Aculele.BridgeMobility COMMAND AculeleBridgeMobility --check)
        if(CMAKE_BUILD_TYPE STREQUAL "Release" AND NOT ACULELE_ENABLE_SANITIZERS)
            add_test(NAME Aculele.CpuBudget
                     COMMAND RenderAculelePresetDemos --cpu --json "${CMAKE_BINARY_DIR}/cpu.json"
                             "${CMAKE_CURRENT_SOURCE_DIR}/Docs/audio/midi")
            set_tests_properties(Aculele.CpuBudget PROPERTIES LABELS perf RUN_SERIAL TRUE TIMEOUT 600)
        endif()
    endif()

    # Every test carries exactly one tier label so CI selects tiers with
    # `ctest -L`; tests not labelled above are unit contracts.
    get_property(ACULELE_TESTS DIRECTORY PROPERTY TESTS)
    foreach(test IN LISTS ACULELE_TESTS)
        get_test_property(${test} LABELS test_labels)
        if(NOT test_labels)
            set_tests_properties(${test} PROPERTIES LABELS unit)
        endif()
    endforeach()
endif()

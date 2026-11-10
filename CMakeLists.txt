cmake_minimum_required(VERSION 3.22)

# Aculele: a physically modelled ukulele built on Acustra's engine. The
# JUCE-free engine and performer, the demo renderer and two analysis tools;
# there is no plug-in or editor yet.
project(Aculele VERSION 0.4.64 LANGUAGES CXX)

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
    # The Rack's layout too: four times the delay line, for 192 kHz. No other
    # target here builds it; Aculele.SampleRateLayoutParity plays it.
    target_compile_definitions(AculeleDSPCxx17 PUBLIC ACULELE_EXTENDED_SAMPLE_RATES=1)
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

    add_executable(AculeleRadiation Tools/AculeleRadiation.cpp)
    target_link_libraries(AculeleRadiation PRIVATE AculeleDSP)

    # Writes LevelMatchData.h and reports each preset on Reason's channel meter.
    add_executable(AculeleLevels Tools/AculeleLevels.cpp)
    target_link_libraries(AculeleLevels PRIVATE AculeleDSP Threads::Threads)

    # Each string's level and brightness in every shape, wood and Picking.
    add_executable(AculeleEvenness Tools/AculeleEvenness.cpp)
    target_link_libraries(AculeleEvenness PRIVATE AculeleDSP Threads::Threads)
    add_executable(AculeleNoteSurvey Tools/AculeleNoteSurvey.cpp)
    target_link_libraries(AculeleNoteSurvey PRIVATE AculeleDSP Threads::Threads)
endif()

if(BUILD_TESTING)
    if(ACULELE_REQUIRE_PYTHON_TESTS)
        find_package(Python3 REQUIRED COMPONENTS Interpreter)
    else()
        find_package(Python3 COMPONENTS Interpreter)
    endif()

    add_executable(AculeleLowRateCaptureTests Tests/LowRateCaptureTests.cpp)
    target_link_libraries(AculeleLowRateCaptureTests PRIVATE AculeleDSP)
    add_test(NAME Aculele.LowRateCapture COMMAND AculeleLowRateCaptureTests)
    set_tests_properties(Aculele.LowRateCapture PROPERTIES LABELS unit)

    add_executable(AculeleOutputCeilingTests Tests/OutputCeilingTests.cpp)
    target_link_libraries(AculeleOutputCeilingTests PRIVATE AculeleDSP)
    add_test(NAME Aculele.OutputCeiling COMMAND AculeleOutputCeilingTests)
    set_tests_properties(Aculele.OutputCeiling PROPERTIES LABELS unit)

    add_executable(AculelePiezoCircuitTests Tests/PiezoCircuitTests.cpp)
    target_link_libraries(AculelePiezoCircuitTests PRIVATE AculeleDSP)
    target_compile_definitions(AculelePiezoCircuitTests PRIVATE
        ACULELE_PIEZO_FIXTURES="${CMAKE_CURRENT_SOURCE_DIR}/Tests/Fixtures/piezo-circuit")
    add_test(NAME Aculele.PiezoCircuit COMMAND AculelePiezoCircuitTests)
    set_tests_properties(Aculele.PiezoCircuit PROPERTIES LABELS unit)

    add_executable(AculelePiezoLoadTests Tests/PiezoLoadTests.cpp)
    target_link_libraries(AculelePiezoLoadTests PRIVATE AculeleDSP)
    add_test(NAME Aculele.PiezoLoad COMMAND AculelePiezoLoadTests)
    set_tests_properties(Aculele.PiezoLoad PROPERTIES LABELS unit)

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

    add_executable(AculeleHandGestureTests Tests/HandGestureTests.cpp)
    target_link_libraries(AculeleHandGestureTests PRIVATE AculeleDSP)
    add_test(NAME Aculele.HandGestures COMMAND AculeleHandGestureTests)

    add_executable(AculelePreparedDispersionTests Tests/PreparedDispersionTests.cpp)
    target_link_libraries(AculelePreparedDispersionTests PRIVATE AculeleDSP)
    add_test(NAME Aculele.PreparedDispersion COMMAND AculelePreparedDispersionTests)

    # Includes the engine recurrence directly to inspect inline state handovers.
    add_executable(AculeleDispersionStateTests Tests/DispersionStateTests.cpp)
    target_include_directories(AculeleDispersionStateTests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Source")
    target_compile_features(AculeleDispersionStateTests PRIVATE cxx_std_17)
    add_test(NAME Aculele.DispersionState COMMAND AculeleDispersionStateTests)

    add_executable(AculelePlateBandTests Tests/PlateBandTests.cpp)
    target_link_libraries(AculelePlateBandTests PRIVATE AculeleDSP)
    add_test(NAME Aculele.PlateBand COMMAND AculelePlateBandTests)

    add_executable(AculeleRadiationRatesTests Tests/RadiationRateTests.cpp)
    target_link_libraries(AculeleRadiationRatesTests PRIVATE AculeleDSP)
    add_test(NAME Aculele.RadiationRates COMMAND AculeleRadiationRatesTests)

    # The Rack's layout renders the bytes this library's does.
    if(NOT MSVC)
        add_executable(AculeleNormalLayoutParity Tests/SampleRateLayoutParity.cpp)
        target_link_libraries(AculeleNormalLayoutParity PRIVATE AculeleDSP)
        add_executable(AculeleExtendedLayoutParity Tests/SampleRateLayoutParity.cpp)
        target_link_libraries(AculeleExtendedLayoutParity PRIVATE AculeleDSPCxx17)
        target_include_directories(AculeleExtendedLayoutParity
            PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Source")
        set_target_properties(AculeleExtendedLayoutParity PROPERTIES
            CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)
        add_test(NAME Aculele.SampleRateLayoutParity
            COMMAND "${CMAKE_COMMAND}"
                "-Dnormal=$<TARGET_FILE:AculeleNormalLayoutParity>"
                "-Dextended=$<TARGET_FILE:AculeleExtendedLayoutParity>"
                "-DWORK=${CMAKE_CURRENT_BINARY_DIR}/sample-rate-layout-parity"
                -P "${CMAKE_CURRENT_SOURCE_DIR}/Tests/CheckSampleRateLayoutParity.cmake")
    endif()

    add_executable(AculeleConstructionChangeTests Tests/ConstructionChangeTests.cpp)
    target_link_libraries(AculeleConstructionChangeTests PRIVATE AculeleDSP)
    add_test(NAME Aculele.ConstructionChange COMMAND AculeleConstructionChangeTests)

    add_executable(AculeleThumbDynamicsTests Tests/ThumbDynamicsTests.cpp)
    target_link_libraries(AculeleThumbDynamicsTests PRIVATE AculeleDSP)
    add_test(NAME Aculele.ThumbDynamics COMMAND AculeleThumbDynamicsTests)

    add_executable(AculelePluckPhysicsTests Tests/PluckPhysicsTests.cpp)
    target_link_libraries(AculelePluckPhysicsTests PRIVATE AculeleDSP)
    add_test(NAME Aculele.PluckPhysics COMMAND AculelePluckPhysicsTests)

    add_executable(AculeleResetTests Tests/ResetTests.cpp)
    target_link_libraries(AculeleResetTests PRIVATE AculeleDSP)
    add_test(NAME Aculele.Reset COMMAND AculeleResetTests)
    # No note waits for the dispersion designs (AculeleEngine::ready).
    add_executable(AculeleReadyTests Tests/ReadyTests.cpp)
    target_link_libraries(AculeleReadyTests PRIVATE AculeleDSP)
    add_test(NAME Aculele.Ready COMMAND AculeleReadyTests)

    # The re-pluck arrival rings clear and copy only their pending slots.
    add_executable(AculeleRepluckArrivalsTests Tests/RepluckArrivalsTests.cpp)
    target_link_libraries(AculeleRepluckArrivalsTests PRIVATE AculeleDSP)
    add_test(NAME Aculele.RepluckArrivals COMMAND AculeleRepluckArrivalsTests)

    # A sounding instrument comes to exact rest, held strings included, and
    # computes on no subnormal number there.
    add_executable(AculeleRestTests Tests/RestTests.cpp)
    target_link_libraries(AculeleRestTests PRIVATE AculeleDSP)
    add_test(NAME Aculele.Rest COMMAND AculeleRestTests)

    # A retained wave reads only the part of its lines captureTail gives it.
    add_executable(AculeleTailReachTests Tests/TailReachTests.cpp)
    target_link_libraries(AculeleTailReachTests PRIVATE AculeleDSP)
    target_include_directories(AculeleTailReachTests PRIVATE Tools)
    add_test(NAME Aculele.TailReach
             COMMAND AculeleTailReachTests "${CMAKE_CURRENT_SOURCE_DIR}/Docs/audio/midi")

    add_executable(AculeleVibratoTests Tests/VibratoTests.cpp)
    target_link_libraries(AculeleVibratoTests PRIVATE AculeleDSP)
    add_test(NAME Aculele.Vibrato COMMAND AculeleVibratoTests)

    add_executable(AculeleStringAxialForceTests Tests/StringAxialForceTests.cpp)
    target_link_libraries(AculeleStringAxialForceTests PRIVATE AculeleDSP)
    add_test(NAME Aculele.StringAxialForce COMMAND AculeleStringAxialForceTests)

    add_executable(AculeleBridgeWorkTests Tests/BridgeWorkTests.cpp)
    target_link_libraries(AculeleBridgeWorkTests PRIVATE AculeleDSP)
    add_test(NAME Aculele.BridgeWork COMMAND AculeleBridgeWorkTests)

    add_executable(AculeleFingerLookaheadTests Tests/FingerLookaheadTests.cpp)
    target_link_libraries(AculeleFingerLookaheadTests PRIVATE AculeleDSP)
    add_test(NAME Aculele.FingerLookahead COMMAND AculeleFingerLookaheadTests)

    # U11: every control reaches held and new notes after long play.
    add_executable(AculeleParameterReachTests Tests/ParameterReachTests.cpp)
    target_link_libraries(AculeleParameterReachTests PRIVATE AculeleDSP Threads::Threads)
    add_test(NAME Aculele.ParameterReach COMMAND AculeleParameterReachTests --reach --jobs 4)
    set_tests_properties(Aculele.ParameterReach PROPERTIES TIMEOUT 1800)

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
        # The radiation's fall above 1 kHz against the one measured ukulele
        # (Labrana et al. 2026).
        add_test(NAME Aculele.Radiation COMMAND AculeleRadiation --check)
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

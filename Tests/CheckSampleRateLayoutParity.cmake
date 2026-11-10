# Aculele.SampleRateLayoutParity: the two builds of
# Tests/SampleRateLayoutParity.cpp must write the same bytes.
foreach(layout IN ITEMS normal extended)
    execute_process(COMMAND "${${layout}}" "${WORK}-${layout}.f32"
                    RESULT_VARIABLE result)
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "${layout} layout parity renderer failed: ${result}")
    endif()
endforeach()
execute_process(COMMAND "${CMAKE_COMMAND}" -E compare_files
                "${WORK}-normal.f32" "${WORK}-extended.f32"
                RESULT_VARIABLE difference)
if(NOT difference EQUAL 0)
    message(FATAL_ERROR "The Rack's layout and the plug-in's render different bytes")
endif()
message(STATUS "The two layouts' output bytes match at 44.1, 48, 96 and 192 kHz")

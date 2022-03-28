set(PROJECT_NAME pvc-disasm)

################################################################################
# Source groups
################################################################################
set(_
    "pvc-disasm.cpp"
)
source_group("Исходные файлы" FILES ${_})

set(ALL_FILES
    ${_}
)

################################################################################
# Target
################################################################################
add_executable(${PROJECT_NAME} ${ALL_FILES})

use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
set(ROOT_NAMESPACE PVC16)

if("${CMAKE_GENERATOR_PLATFORM}" STREQUAL "x64")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION_RELEASE "TRUE"
    )
elseif("${CMAKE_GENERATOR_PLATFORM}" STREQUAL "x86")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION_RELEASE "TRUE"
    )
endif()
################################################################################
# Compile definitions
################################################################################
if("${CMAKE_GENERATOR_PLATFORM}" STREQUAL "x64")
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        "$<$<CONFIG:Release>:"
            "NDEBUG"
        ">"
        "_CONSOLE;"
        "UNICODE;"
        "_UNICODE"
    )
elseif("${CMAKE_GENERATOR_PLATFORM}" STREQUAL "x86")
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        "$<$<CONFIG:Debug>:"
            "_DEBUG"
        ">"
        "$<$<CONFIG:Release>:"
            "NDEBUG"
        ">"
        "WIN32;"
        "_CONSOLE;"
        "UNICODE;"
        "_UNICODE"
    )
endif()

################################################################################
# Compile and link options
################################################################################
if(TRUE)
    target_compile_options(${PROJECT_NAME} PRIVATE
        -std=c++20
        -flto
        -O3
        -march=native
        -mfpmath=sse
        ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT}
        ${DEFAULT_CXX_EXCEPTION_HANDLING}
    )
    target_link_options(${PROJECT_NAME} PRIVATE
        -flto
    )
    if("${CMAKE_GENERATOR_PLATFORM}" STREQUAL "x64")
        target_compile_options(${PROJECT_NAME} PRIVATE
        )
    elseif("${CMAKE_GENERATOR_PLATFORM}" STREQUAL "x86")
        target_compile_options(${PROJECT_NAME} PRIVATE
        )
    endif()
    if("${CMAKE_GENERATOR_PLATFORM}" STREQUAL "x64")
        target_link_options(${PROJECT_NAME} PRIVATE
        )
    elseif("${CMAKE_GENERATOR_PLATFORM}" STREQUAL "x86")
        target_link_options(${PROJECT_NAME} PRIVATE
        )
    endif()
else()

endif()

################################################################################
# Dependencies
################################################################################
if("${CMAKE_GENERATOR_PLATFORM}" STREQUAL "x64")
endif()
target_link_libraries(${PROJECT_NAME} PRIVATE "${ADDITIONAL_LIBRARY_DEPENDENCIES}")


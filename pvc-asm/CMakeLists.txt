set(PROJECT_NAME pvc-asm)

################################################################################
# Source groups
################################################################################
set(Header_Files
    "compiler.h"
    "lexer.h"
    "syntaxer.h"
    "tokenizer.h"
    "utility.h"
    "eval.h"
)
source_group("Header Files" FILES ${Header_Files})

set(Source_Files
    "compiler.cpp"
    "lexer.cpp"
    "pvc-asm.cpp"
    "syntaxer.cpp"
    "tokenizer.cpp"
    "utility.cpp"
    "eval.cpp"
)
source_group("Source Files" FILES ${Source_Files})

set(ALL_FILES
    ${Header_Files}
    ${Source_Files}
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


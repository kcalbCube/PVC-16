set(PROJECT_NAME pvc-16)
################################################################################
# Source groups
################################################################################
set(Header_Files
    "vmflags.h"
    "bus.h"
    "decoder.h"
    "device.h"
    "interrupt.h"
    "map.h"
    "memory.h"
    "mutility.h"
    "opcode.h"
    "registers.h"
    "registers_define.h"
    "stack.h"
    "unittesting.h"
    "utility.h"
    "video.h"
)
source_group("Header Files" FILES ${Header_Files})

set(Source_Files
    "bus.cpp"
    "decoder.cpp"
    "device.cpp"
    "interrupt.cpp"
    "memory.cpp"
    "PVC-16.cpp"
    "registers.cpp"
    "stack.cpp"
    "utility.cpp"
    "video.cpp"
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

if("${ARCH}" STREQUAL "x64")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION_RELEASE "TRUE"
    )
elseif("${ARCH}" STREQUAL "x86")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION_RELEASE "TRUE"
    )
endif()

include_directories("${SDL2_PATH}")
# todo: add sdl2 searching
target_link_options(${PROJECT_NAME} PRIVATE "-lSDL2")
target_link_libraries(${PROJECT_NAME} PRIVATE "SDL2")
################################################################################
# Compile and link options
################################################################################
if(TRUE)
    target_compile_options(${PROJECT_NAME} PRIVATE
        -std=c++2b
        -flto
        -O0
        -march=native
        -mfpmath=sse
        ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT}
        ${DEFAULT_CXX_EXCEPTION_HANDLING}
    )
    target_link_options(${PROJECT_NAME} PRIVATE
        -flto
    )
    if("${ARCH}" STREQUAL "x64")
        target_compile_options(${PROJECT_NAME} PRIVATE
        )
    elseif("${ARCH}" STREQUAL "x86")
        target_compile_options(${PROJECT_NAME} PRIVATE
        )
    endif()
    if("${ARCH}" STREQUAL "x64")
        target_link_options(${PROJECT_NAME} PRIVATE
        )
    elseif("${ARCH}" STREQUAL "x86")
        target_link_options(${PROJECT_NAME} PRIVATE
        )
    endif()
else()

endif()

################################################################################
# Dependencies
################################################################################
if("${ARCH}" STREQUAL "x64")
    set(ADDITIONAL_LIBRARY_DEPENDENCIES
        "$<$<CONFIG:Debug>:"
            "SDL2;"
            "SDL2_ttf"
        ">"
    )
endif()
target_link_libraries(${PROJECT_NAME} PRIVATE "${ADDITIONAL_LIBRARY_DEPENDENCIES}")


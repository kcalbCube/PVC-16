cmake_minimum_required(VERSION 3.7 FATAL_ERROR)

set(CMAKE_SYSTEM_VERSION 10.0 CACHE STRING "" FORCE)

project(PVC-16 CXX)

set(CMAKE_CXX_COMPILER "/usr/bin/g++")

################################################################################
# Set target arch type if empty. Visual studio solution generator provides it.
################################################################################
message("${CMAKE_GENERATOR_PLATFORM} architecture in use")

set(ARCH, "x64")

set(CMAKE_THREAD_LIBS_INIT "-lpthread")
set(CMAKE_HAVE_THREADS_LIBRARY 1)
set(CMAKE_USE_WIN32_THREADS_INIT 0)
set(CMAKE_USE_PTHREADS_INIT 1)
set(THREADS_PREFER_PTHREAD_FLAG ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

################################################################################
# Nuget packages function stub.
################################################################################
function(use_package TARGET PACKAGE VERSION)
    message(WARNING "No implementation of use_package. Create yours. "
                    "Package \"${PACKAGE}\" with version \"${VERSION}\" "
                    "for target \"${TARGET}\" is ignored!")
endfunction()

include_directories("${CMAKE_SOURCE_DIR}/include")
include_directories("${CMAKE_SOURCE_DIR}/include/magic_enum-master/include")
include_directories("${CMAKE_SOURCE_DIR}/include/boost_1_78_0")
include_directories("${CMAKE_SOURCE_DIR}/include/args-6.3.0")

################################################################################
# Common utils
################################################################################
include(CMake/Utils.cmake)

set(SDL2_PATH "${CMAKE_SOURCE_DIR}/include/sdl2/include")

add_compile_options("-g")

################################################################################
# Additional Global Settings(add specific info there)
################################################################################
include(CMake/GlobalSettingsInclude.cmake OPTIONAL)



################################################################################
# Use solution folders feature
################################################################################
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

#add_compile_options (-fcolor-diagnostics)

################################################################################
# Sub-projects
################################################################################
add_subdirectory(PVC-16)
add_subdirectory(pvc-asm)
add_subdirectory(pvc-disasm)

cmake_minimum_required(VERSION 3.26.0)
project(ChessBot VERSION 0.1.0 LANGUAGES C CXX)

set(FullOutputDir "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${FullOutputDir}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${FullOutputDir}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${FullOutputDir}")

set(SrcPath "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(HeaderPath "${CMAKE_CURRENT_SOURCE_DIR}/include")
file(GLOB_RECURSE Sources CONFIGURE_DEPENDS "${SrcPath}/*.cpp" "${SrcPath}/*.c")

add_executable(${PROJECT_NAME} ${Sources})
target_include_directories(${PROJECT_NAME} PRIVATE ${HeaderPath})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:/W4>
        $<$<CONFIG:Release>:/O2>
    )
else()
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:-Wextra -Wunused -Wconversion -Wpedantic>
        $<$<CONFIG:Release>:-O3>
    )
endif()

cmake_minimum_required(VERSION 4.2)
project(deepgram_project)

set(CMAKE_CXX_STANDARD 20)

list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew" "/usr/local")

find_package(Boost REQUIRED)

set(SOURCE_FILES main.cpp)
add_executable(deepgram_project_run ${SOURCE_FILES})

include_directories(Source)
add_subdirectory(Source)

target_link_libraries(deepgram_project_run Source)
target_link_libraries(Source ${Boost_LIBRARIES})
target_include_directories(deepgram_project_run PRIVATE ${Boost_INCLUDE_DIRS})
add_subdirectory(tests)

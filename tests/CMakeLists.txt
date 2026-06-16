set (Boost_USE_STATIC_LIBS OFF)
find_package (Boost REQUIRED COMPONENTS unit_test_framework)
include_directories (${Boost_INCLUDE_DIRS})

add_executable (tests_run Data_cache_tests.cpp)
target_link_libraries (tests_run ${Boost_LIBRARIES})
target_link_libraries (tests_run Source)
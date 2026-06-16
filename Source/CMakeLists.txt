project(deepgram_project)

set(HEADER_FILES
        Connection_worker.h
        Data_cache.cpp
        Data_cache.h
)

set(SOURCE_FILES
        Connection_worker.cpp
        Data_cache.cpp
        Data_cache.h
        third_party/dr_wav.h
)

add_library(Source STATIC ${SOURCE_FILES} ${HEADER_FILES})
include_directories (${Boost_INCLUDE_DIRS})

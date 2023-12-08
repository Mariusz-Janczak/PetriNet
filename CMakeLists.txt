cmake_minimum_required(VERSION 3.10)

project(PetriNet CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(src)

set(SOURCES
    # src/automat.cpp
    # src/colored.cpp
    # src/encoding.cpp
    src/graph.cpp
    src/petri.cpp
    src/tools.cpp
)

set(HEADERS
    # src/automat.h
    # src/colored.h
    # src/encoding.h
    src/graph.h
    src/petri.h
    src/tools.h
    src/defs.h
    src/types.h
)

add_executable(petriNet ${SOURCES} ${HEADERS})

if(MSVC)
    target_compile_options(petriNet PRIVATE /W4)
else()
    target_compile_options(petriNet PRIVATE -Wall -Wextra -Wpedantic)
endif()

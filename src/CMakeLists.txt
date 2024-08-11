set(BINARY ${CMAKE_PROJECT_NAME})

# add executables
file(GLOB_RECURSE SOURCES LIST_DIRECTORIES true *.h *.cc)

set(SOURCES ${SOURCES})

add_library(${BINARY}_lib STATIC ${SOURCES})
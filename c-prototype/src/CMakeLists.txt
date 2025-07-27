set(BINARY ${CMAKE_PROJECT_NAME})
file(GLOB_RECURSE SOURCES LIST_DIRECTORIES true *.h *.c *.cpp)

find_package(OpenSSL REQUIRED)
set(SOURCES ${SOURCES})

add_executable(${BINARY}_run ${SOURCES})
add_library(${BINARY}_lib STATIC ${SOURCES})

target_link_libraries(${BINARY}_run OpenSSL::SSL OpenSSL::Crypto)
configure_file(${CMAKE_SOURCE_DIR}/hashes.txt ${CMAKE_BINARY_DIR}/src/hashes.txt COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/points.txt ${CMAKE_BINARY_DIR}/src/points.txt COPYONLY)
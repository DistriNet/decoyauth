set(BENCH_BINARY ${CMAKE_PROJECT_NAME})

file(GLOB_RECURSE TEST_SOURCES LIST_DIRECTORIES false *.h *.c *.cpp)

add_executable(${BENCH_BINARY}_bench ${TEST_SOURCES})

find_package(OpenSSL REQUIRED)
find_package(benchmark REQUIRED)

target_link_libraries(${BENCH_BINARY}_bench PUBLIC ${CMAKE_PROJECT_NAME}_lib benchmark::benchmark OpenSSL::SSL OpenSSL::Crypto)
configure_file(${CMAKE_SOURCE_DIR}/hashes.txt ${CMAKE_BINARY_DIR}/bench/hashes.txt COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/points.txt ${CMAKE_BINARY_DIR}/bench/points.txt COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/encoded_points.txt ${CMAKE_BINARY_DIR}/bench/encoded_points.txt COPYONLY)

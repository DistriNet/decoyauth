set(TST_BINARY ${CMAKE_PROJECT_NAME})

file(GLOB_RECURSE TEST_SOURCES LIST_DIRECTORIES false *.h *.c *.cpp)

add_executable(${TST_BINARY}_tst ${TEST_SOURCES})
add_test(NAME ${TST_BINARY}_tst COMMAND ${TST_BINARY})

find_package(OpenSSL REQUIRED)
target_link_libraries(${TST_BINARY}_tst PUBLIC ${CMAKE_PROJECT_NAME}_lib gtest OpenSSL::SSL OpenSSL::Crypto)
configure_file(${CMAKE_SOURCE_DIR}/hashes.txt ${CMAKE_BINARY_DIR}/tst/hashes.txt COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/points.txt ${CMAKE_BINARY_DIR}/tst/points.txt COPYONLY)
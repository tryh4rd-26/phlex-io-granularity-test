cmake_minimum_required(VERSION 3.10)

add_executable(multicont_read_test multicont_read_test.cpp)
target_link_libraries(multicont_read_test ${ROOT_LIBS} WireDict)
target_include_directories(multicont_read_test PRIVATE ${CMAKE_SOURCE_DIR}/include)
set_target_properties(multicont_read_test PROPERTIES
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_executable(lazy_index lazy_index.cpp)
target_link_libraries(lazy_index ${ROOT_LIBS} WireDict)
target_include_directories(lazy_index PRIVATE ${CMAKE_SOURCE_DIR}/include)
set_target_properties(lazy_index PROPERTIES
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

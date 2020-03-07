set(BINARY ${CMAKE_PROJECT_NAME}_test)

file(GLOB_RECURSE TEST_SOURCES LIST_DIRECTORIES false *.h *.cpp)

set(SOURCES ${TEST_SOURCES})

add_executable(${BINARY} ${TEST_SOURCES})

add_test(NAME ${BINARY} COMMAND ${BINARY})

if(MSVC)
  target_compile_options(${CMAKE_PROJECT_NAME}_test PRIVATE /W3 /WX)
else()
  target_compile_options(${CMAKE_PROJECT_NAME}_test PRIVATE -Wall -Wextra -pedantic -Werror)
endif()

target_link_libraries(${BINARY} PUBLIC gtest)
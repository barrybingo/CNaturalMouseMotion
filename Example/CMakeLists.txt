set(BINARY ${CMAKE_PROJECT_NAME})

file(GLOB_RECURSE EXAMPLE_SOURCES LIST_DIRECTORIES false *.h *.cpp)

set(SOURCES ${EXAMPLE_SOURCES})

add_executable(${BINARY} ${EXAMPLE_SOURCES})

if(UNIX)
    find_package(X11 REQUIRED)
    include_directories(${X11_INCLUDE_DIR})

    target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC ${X11_INCLUDE_DIR})
    target_link_libraries(${CMAKE_PROJECT_NAME} ${X11_LIBRARIES})
endif()

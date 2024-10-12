
set(CMAKE_BUILD_TYPE Release)
set (CMAKE_CXX_FLAGS_RELEASE "-O3")

add_executable(thisproj thisproj.cpp)
target_link_libraries(thisproj asyncio)

add_executable(asiotcp asiotcp.cpp)
add_executable(oldasio oldasio.cpp)

# add_executable(testev testev.cpp)
# target_link_libraries(testev ev)

add_executable(libevpp libevpp.cpp)
target_link_libraries(libevpp ev)

add_executable(luv luv.cpp)
target_link_libraries(luv uv)
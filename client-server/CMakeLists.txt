project(client_server)

set(SOURCE_EXE src/server.cpp)

add_executable(server ${SOURCE_EXE})

target_link_libraries(server liburing.so)

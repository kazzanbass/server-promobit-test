project(client_server)

set(SOURCE_EXE server.cpp)
set(LIBURING /../lib/liburing.so)

add_executable(server ${SOURCE_EXE})

target_link_libraries(server ${LIBURING})

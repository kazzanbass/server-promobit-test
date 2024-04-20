project(client_server)

set(SOURCE_EXE src/server.cpp)
set(LIBURING lib/liburing.so)

add_library(liburing SHARED ${LIBURING})
add_executable(server ${SOURCE_EXE})

target_link_libraries(server liburing)

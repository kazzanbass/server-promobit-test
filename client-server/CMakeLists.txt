project(client_server)

set(SOURCE_EXE server.cpp)
set(ERR_LIB errproc.cpp)

add_library(errproc STATIC ${ERR_LIB})
add_executable(server ${SOURCE_EXE})

target_link_libraries(server errproc /usr/lib/liburing.so)

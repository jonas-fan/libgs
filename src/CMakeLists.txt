add_library(${TARGET_NAME} STATIC
    ./gs.c
    ./socket.c
    ./unix_socket.c
)

target_link_libraries(${TARGET_NAME}
    pthread
)
project(libgs-test C)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -Wall -Wextra -pedantic -std=c99 -g -D_GNU_SOURCE")

# receiver
add_executable(receiver
    ./receiver.c
)

target_link_libraries(receiver
    ${TARGET_NAME}
    pthread
)

# sender
add_executable(sender
    ./sender.c
)

target_link_libraries(sender
    ${TARGET_NAME}
    pthread
)

# thread
#add_executable(thread
#    ./thread.c
#)

#target_link_libraries(thread
#    ${TARGET_NAME}
#)
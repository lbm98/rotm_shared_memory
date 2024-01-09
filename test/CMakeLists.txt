add_executable(test_mutator_shm_server test_mutator_shm_server.cc)
target_link_libraries(test_mutator_shm_server shm_ipc)

add_executable(test_mutator_shm_client test_mutator_shm_client.cc)
target_link_libraries(test_mutator_shm_client shm_ipc)

add_test(
        NAME test_mutator_shm_server
        COMMAND test_mutator_shm_server
)
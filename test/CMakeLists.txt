add_executable(test_mutator_shm_server test_mutator_shm_server.cc)
add_executable(test_mutator_shm_client test_mutator_shm_client.cc)

add_test(
        NAME test_mutator_shm_server
        COMMAND test_mutator_shm_server
)
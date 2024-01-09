#include <thread>
#include <cassert>

#include "shared_memory/mutator_shm.h"

size_t mutate(
        uint8_t *packet_data,
        size_t packet_data_size,
        size_t packet_max_data_size,
        int packet_type
) {
    assert (packet_data_size == 3);
    assert (packet_max_data_size == 5);
    assert (packet_data[0] == 0x01);
    assert (packet_data[1] == 0x02);
    assert (packet_data[2] == 0x03);
    assert (packet_type == 1);

    packet_data[0] = 0x04;
    packet_data[1] = 0x05;
    packet_data[2] = 0x06;
    packet_data[3] = 0x07;
    return 4;
}

int main() {
    MutatorSharedMemoryServer mutator_server;

    std::thread mutator_server_thread([&mutator_server]{
        mutator_server.run(mutate);
    });

    int ret = std::system("./test_mutator_shm_client");
    assert(ret == 0);

    mutator_server.stop();
    mutator_server_thread.join();

    return 0;
}
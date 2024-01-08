#include <cassert>

#include "shared_memory/mutator_shm.h"

int main() {
    const int packet_max_data_size = 5;
    MutatorSharedMemoryClient mutator_client;

    std::vector<uint8_t> packet_data {0x01, 0x02, 0x03};
    size_t size = packet_data.size();
    packet_data.resize(packet_max_data_size);
    size_t new_size = mutator_client.mutate(packet_data.data(), size, 1);
    packet_data.resize(new_size);

    assert (packet_data[0] == 0x04);
    assert (packet_data[1] == 0x05);
    assert (packet_data[2] == 0x06);
    assert (packet_data[3] == 0x07);
    assert (packet_data.size() == 4);

    return 0;
}
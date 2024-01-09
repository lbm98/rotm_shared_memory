#include <cassert>

#include "shared_memory/mutator_shm.h"

int main() {
    MutatorSharedMemoryClient mutator_client;

    std::vector<uint8_t> packet_data {0x01, 0x02, 0x03};
    mutator_client.mutate_vec(packet_data, 5, 1);

    assert (packet_data.size() == 4);
    assert (packet_data[0] == 0x04);
    assert (packet_data[1] == 0x05);
    assert (packet_data[2] == 0x06);
    assert (packet_data[3] == 0x07);

    return 0;
}
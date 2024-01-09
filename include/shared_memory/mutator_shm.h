#pragma once

#include <cstdint>
#include <cstddef>
#include <functional>
#include <atomic>
#include <vector>

#include <sys/mman.h>
#include <semaphore.h>

#define MUTATOR_SHM_PATH "/mutator_shm"
#define MUTATOR_SHM_SIZE sizeof(MutatorSharedMemoryBuffer)

#define MUTATOR_PACKET_DATA_BUFFER_SIZE 16384

#define MUTATOR_RESPONSE_SUCCESS  1
#define MUTATOR_RESPONSE_FAILURE  2

struct MutatorSharedMemoryBuffer {
    sem_t sem1;
    sem_t sem2;

    uint8_t response_code;

    uint8_t packet_type;
    uint16_t packet_data_size;
    uint16_t packet_max_data_size;
    uint8_t packet_data[MUTATOR_PACKET_DATA_BUFFER_SIZE];
};

class MutatorSharedMemoryServer {
public:
    using MutatorRequestHandler = std::function<
            size_t(
                    uint8_t *packet_data,
                    size_t packet_data_size,
                    size_t packet_max_data_size,
                    int packet_type
            )>;

    explicit MutatorSharedMemoryServer();

    void stop();

    void run(const MutatorRequestHandler &mutator_request_handler);

    void respond_failure();

    void respond_success();

private:
    MutatorSharedMemoryBuffer *shm_buf;

    std::atomic<bool> stop_flag;
};

class MutatorSharedMemoryClient {
public:
    explicit MutatorSharedMemoryClient();

    size_t mutate(
            uint8_t *packet_data,
            size_t packet_data_size,
            size_t packet_max_data_size,
            int packet_type
    );

    void mutate_vec(std::vector<uint8_t> &vec, size_t packet_max_data_size, int packet_type);

private:
    MutatorSharedMemoryBuffer *shm_buf;
};
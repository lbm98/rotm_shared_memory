#pragma once

#include <functional>
#include <stdexcept>
#include <atomic>
#include <utility>
#include <cstring>
#include <limits>
#include <vector>

#include <sys/mman.h>
#include <semaphore.h>

#include "common_shm.h"

#define MUTATOR_SHM_PATH "/mutator_shm"
#define MUTATOR_SHM_SIZE sizeof(MutatorSharedMemoryBuffer)

#define MUTATOR_MAX_PACKET_DATA_SIZE 8192

#define RESPONSE_SUCCESS  1
#define RESPONSE_FAILURE  2

struct MutatorSharedMemoryBuffer {
    sem_t sem1;
    sem_t sem2;

    uint8_t response_code;

    uint8_t packet_type;
    uint16_t packet_data_size;
    uint8_t packet_data[MUTATOR_MAX_PACKET_DATA_SIZE];
};

class MutatorSharedMemoryServer {
public:
    using MutatorRequestHandler = std::function<
            size_t(
                    uint8_t *packet_data,
                    size_t packet_data_size,
                    int packet_type
            )>;

    explicit MutatorSharedMemoryServer()
            : stop_flag{false} {
        void *shm_ptr = create_shared_memory(MUTATOR_SHM_PATH, MUTATOR_SHM_SIZE);
        shm_buf = static_cast<MutatorSharedMemoryBuffer *>(shm_ptr);

        if (sem_init(&shm_buf->sem1, 1, 0) == -1)
            throw std::runtime_error("sem_init should work");
        if (sem_init(&shm_buf->sem2, 1, 0) == -1)
            throw std::runtime_error("sem_init should work");
    }

    void stop() {
        if (shm_unlink(MUTATOR_SHM_PATH) == -1)
            throw std::runtime_error("shm_unlink should work");

        stop_flag = true;
    }

    void run(const MutatorRequestHandler &mutator_request_handler) {

        while (not stop_flag) {

            //
            // Wait for request
            //

            timespec ts{};
            if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
                throw std::runtime_error("clock_gettime should work");

            ts.tv_sec += 1;
            int ret = sem_timedwait(&shm_buf->sem1, &ts);
            if (ret == -1) {
                if (errno == ETIMEDOUT)
                    continue;
                else
                    throw std::runtime_error("sem_timedwait should work");
            }

            //
            // Check request
            //

            if (shm_buf->packet_data_size > MUTATOR_MAX_PACKET_DATA_SIZE) {
                shm_buf->response_code = RESPONSE_FAILURE;
                if (sem_post(&shm_buf->sem2) == -1)
                    throw std::runtime_error("sem_post should work");
                continue;
            }

            //
            // Create response
            //

            size_t new_size = mutator_request_handler(
                    shm_buf->packet_data,
                    shm_buf->packet_data_size,
                    shm_buf->packet_type
            );

            shm_buf->packet_data_size = static_cast<uint16_t>(new_size);

            //
            // Send response
            //

            shm_buf->response_code = RESPONSE_SUCCESS;
            if (sem_post(&shm_buf->sem2) == -1)
                throw std::runtime_error("sem_post should work");
        }
    }

private:
    MutatorSharedMemoryBuffer *shm_buf;

    std::atomic<bool> stop_flag;
};

class MutatorSharedMemoryClient {
public:
    MutatorSharedMemoryClient() {
        void *shm_ptr = map_shared_memory(MUTATOR_SHM_PATH, MUTATOR_SHM_SIZE);
        shm_buf = static_cast<MutatorSharedMemoryBuffer *>(shm_ptr);
    }

    size_t mutate(
            uint8_t *packet_data,
            size_t packet_data_size,
            int packet_type
    ) {

        //
        // Check request
        //

        if (packet_data_size > std::numeric_limits<uint16_t>::max())
            throw std::runtime_error("Request should be valid");

        if (packet_type > std::numeric_limits<uint8_t>::max())
            throw std::runtime_error("Request should be valid");

        if (packet_data_size > MUTATOR_MAX_PACKET_DATA_SIZE)
            throw std::runtime_error("Request should be valid");

        //
        // Create request
        //

        std::memcpy(shm_buf->packet_data, packet_data, packet_data_size);
        shm_buf->packet_data_size = static_cast<uint16_t>(packet_data_size);
        shm_buf->packet_type = static_cast<uint8_t>(packet_type);

        //
        // Send request
        //

        if (sem_post(&shm_buf->sem1) == -1)
            throw std::runtime_error("sem_post should work");

        //
        // Wait for response
        //

        if (sem_wait(&shm_buf->sem2) == -1)
            throw std::runtime_error("sem_wait should work");

        //
        // Check response
        //

        if (shm_buf->response_code != RESPONSE_SUCCESS)
            throw std::runtime_error("Request should succeed");

        if (shm_buf->packet_data_size > MUTATOR_MAX_PACKET_DATA_SIZE)
            throw std::runtime_error("Request should be valid");

        //
        // Return response
        //

        std::memcpy(packet_data, shm_buf->packet_data, shm_buf->packet_data_size);
        return shm_buf->packet_data_size;
    }

    void mutate_vec(std::vector<uint8_t> &vec, int packet_type) {
        size_t size = vec.size();
        vec.resize(MUTATOR_MAX_PACKET_DATA_SIZE);
        size_t new_size = mutate(vec.data(), size, packet_type);
        vec.resize(new_size);
    }

private:
    MutatorSharedMemoryBuffer *shm_buf;
};
#include <stdexcept>
#include <limits>
#include <cstring>
#include <cassert>

#include "shared_memory/common_shm.h"
#include "shared_memory/mutator_shm.h"


MutatorSharedMemoryServer::MutatorSharedMemoryServer()
        : stop_flag{false} {
    void *shm_ptr = create_shared_memory(MUTATOR_SHM_PATH, MUTATOR_SHM_SIZE);
    shm_buf = static_cast<MutatorSharedMemoryBuffer *>(shm_ptr);

    if (sem_init(&shm_buf->sem1, 1, 0) == -1)
        throw std::runtime_error("sem_init should work");
    if (sem_init(&shm_buf->sem2, 1, 0) == -1)
        throw std::runtime_error("sem_init should work");
}

void MutatorSharedMemoryServer::stop() {
    if (shm_unlink(MUTATOR_SHM_PATH) == -1)
        throw std::runtime_error("shm_unlink should work");

    stop_flag = true;
}

void MutatorSharedMemoryServer::run(const MutatorRequestHandler &mutator_request_handler) {

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

        if (shm_buf->packet_data_size > shm_buf->packet_max_data_size) {
            respond_failure();
            continue;
        }

        if (shm_buf->packet_max_data_size > MUTATOR_PACKET_DATA_BUFFER_SIZE) {
            respond_failure();
            continue;
        }

        //
        // Create response
        //

        size_t new_size = mutator_request_handler(
                shm_buf->packet_data,
                shm_buf->packet_data_size,
                shm_buf->packet_max_data_size,
                shm_buf->packet_type
        );

        assert (new_size <= shm_buf->packet_max_data_size);

        shm_buf->packet_data_size = static_cast<uint16_t>(new_size);

        //
        // Send response
        //

        respond_success();
    }
}

void MutatorSharedMemoryServer::respond_failure() {
    shm_buf->response_code = MUTATOR_RESPONSE_FAILURE;
    if (sem_post(&shm_buf->sem2) == -1)
        throw std::runtime_error("sem_post should work");
}

void MutatorSharedMemoryServer::respond_success() {
    shm_buf->response_code = MUTATOR_RESPONSE_SUCCESS;
    if (sem_post(&shm_buf->sem2) == -1)
        throw std::runtime_error("sem_post should work");
}


MutatorSharedMemoryClient::MutatorSharedMemoryClient() {
    void *shm_ptr = map_shared_memory(MUTATOR_SHM_PATH, MUTATOR_SHM_SIZE);
    shm_buf = static_cast<MutatorSharedMemoryBuffer *>(shm_ptr);
}

size_t MutatorSharedMemoryClient::mutate(
        uint8_t *packet_data,
        size_t packet_data_size,
        size_t packet_max_data_size,
        int packet_type
) {

    //
    // Check request
    //

    if (packet_data_size > std::numeric_limits<uint16_t>::max())
        throw std::runtime_error("Request should be valid");

    if (packet_max_data_size > std::numeric_limits<uint16_t>::max())
        throw std::runtime_error("Request should be valid");

    if (packet_data_size > packet_max_data_size)
        throw std::runtime_error("Request should be valid");

    if (packet_max_data_size > MUTATOR_PACKET_DATA_BUFFER_SIZE)
        throw std::runtime_error("Request should be valid");

    if (packet_type > std::numeric_limits<uint8_t>::max())
        throw std::runtime_error("Request should be valid");

    //
    // Create request
    //

    std::memcpy(shm_buf->packet_data, packet_data, packet_data_size);
    shm_buf->packet_data_size = static_cast<uint16_t>(packet_data_size);
    shm_buf->packet_max_data_size = static_cast<uint16_t>(packet_max_data_size);
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

    if (shm_buf->response_code != MUTATOR_RESPONSE_SUCCESS)
        throw std::runtime_error("Request should succeed");

    if (shm_buf->packet_max_data_size != packet_max_data_size)
        throw std::runtime_error("Request should be valid");

    if (shm_buf->packet_data_size > shm_buf->packet_max_data_size)
        throw std::runtime_error("Request should be valid");

    if (shm_buf->packet_type != packet_type)
        throw std::runtime_error("Request should be valid");

    //
    // Return response
    //

    std::memcpy(packet_data, shm_buf->packet_data, shm_buf->packet_data_size);
    return shm_buf->packet_data_size;
}

void MutatorSharedMemoryClient::mutate_vec(std::vector<uint8_t> &vec, size_t packet_max_data_size, int packet_type) {
    size_t size = vec.size();
    vec.resize(packet_max_data_size);
    size_t new_size = mutate(vec.data(), size, packet_max_data_size, packet_type);
    vec.resize(new_size);
}

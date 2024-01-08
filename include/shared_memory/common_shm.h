#include <stdexcept>

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

void *create_shared_memory(const char *shm_path, size_t shm_size) {
    shm_unlink(shm_path);

    int shm_fd = shm_open(
            shm_path,
            O_CREAT | O_EXCL | O_RDWR,
            0600
    );
    if (shm_fd == -1)
        throw std::runtime_error("shm_open should work");

    if (ftruncate(shm_fd, static_cast<__off_t>(shm_size)) == -1)
        throw std::runtime_error("ftruncate should work");

    void *shm_ptr = mmap(
            nullptr,
            shm_size,
            PROT_READ | PROT_WRITE,
            MAP_SHARED,
            shm_fd,
            0
    );
    if (shm_ptr == MAP_FAILED)
        throw std::runtime_error("mmap should work");

    return shm_ptr;
}

void *map_shared_memory(const char *shm_path, size_t shm_size) {
    int shm_fd = shm_open(
            shm_path,
            O_RDWR,
            0
    );
    if (shm_fd == -1)
        throw std::runtime_error("shm_open should work");

    void *shm_ptr = mmap(
            nullptr,
            shm_size,
            PROT_READ | PROT_WRITE,
            MAP_SHARED,
            shm_fd,
            0
    );
    if (shm_ptr == MAP_FAILED)
        throw std::runtime_error("mmap should work");

    return shm_ptr;
}
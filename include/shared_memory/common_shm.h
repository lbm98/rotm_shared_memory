#pragma once

#include <cstddef>

void *create_shared_memory(const char *shm_path, size_t shm_size);

void *map_shared_memory(const char *shm_path, size_t shm_size);
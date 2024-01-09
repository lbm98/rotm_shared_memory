add_library(shm_ipc common_shm.cc mutator_shm.cc)
target_include_directories(shm_ipc PUBLIC ${PROJECT_SOURCE_DIR}/include)
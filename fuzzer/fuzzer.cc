#include <iostream>

#include <thread>
#include <stdexcept>
#include <cstdlib>

#include "shared_memory/mutator_shm.h"

#include "mutator_request_handler.h"
#include "corpus.h"

enum class ExitKind {
    Ok,
    Crash
};

ExitKind run_network() {
//    int ret = std::system("python3 ../../network.py --debug_ue");
    int ret = std::system("python3 ../../network.py");

    if (ret == -1)
        throw std::runtime_error("system() should work");
    if (ret == 0)
        return ExitKind::Ok;
    if (ret == 1)
        return ExitKind::Crash;
    throw std::runtime_error("Network should exit with a return-code of 0 or 1");
}

void mutator_timeout_handler() {
    // send SIGALRM signals to ue and gnb
    // wait for done
    // add input to corpus
    // restart
}

int main() {
    Input input;
    Corpus corpus;
    Corpus crashes;

    MutatorSharedMemoryServer mutator_server;

    RadamsaMutatorRequestHandler mutator_request_handler(input, 2);
    std::thread mutator_server_thread([&mutator_server, &mutator_request_handler]{
        mutator_server.run(mutator_request_handler);
    });

    for (int i = 0; i < 10; i++) {
        ExitKind exit_kind = run_network();

        if (exit_kind == ExitKind::Ok) {
            corpus.add(input);
        } else {
            std::cout << "Crash!\n";
            crashes.add(input);
            break;
        }

        input.clear();
    }

    mutator_server.stop();
    mutator_server_thread.join();

    corpus.write_to_disk("corpus");
    crashes.write_to_disk("crashes");

    return 0;
}
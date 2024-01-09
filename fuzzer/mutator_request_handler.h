#include <random>

extern "C" {
#include "radamsa.h"
}

#include "corpus.h"

class RadamsaMutatorRequestHandler {
public:
    explicit RadamsaMutatorRequestHandler(Input& input, unsigned int seed) : input{input}, rng{seed} {
        radamsa_init();
    }

    size_t operator()(
            uint8_t *packet_data,
            size_t packet_data_size,
            size_t max_packet_data_size,
            int packet_type
    ) {
        size_t new_size = radamsa_inplace(
                packet_data,
                packet_data_size,
                max_packet_data_size,
                rng()
        );

        input.add(Packet{packet_data, packet_data_size, packet_type});

        return new_size;
    }

private:
    Input& input;
    std::mt19937 rng;
};
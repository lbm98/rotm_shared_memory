#pragma once

#include <mutex>
#include <vector>
#include <cstdint>
#include <fstream>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

class Packet {
public:
    explicit Packet(std::vector<uint8_t> data, int type) : data{std::move(data)}, type{type} {}

    explicit Packet(uint8_t *data, size_t size, int type) : data{data, data + size}, type{type} {}

    json to_json() const {
        return {
                {"type", type},
                {"data", data}
        };
    }

private:
    std::vector<uint8_t> data;
    int type;
};

class Input {
public:
    explicit Input(std::vector<Packet> packets = {}) : packets{std::move(packets)} {}

    Input(const Input &other) : packets(other.packets) {}

    Input(Input &&other) noexcept: packets(std::move(other.packets)) {}

    void add(const Packet &packet) {
        std::lock_guard<std::mutex> lock(mutex);
        packets.emplace_back(packet);
    }

    void clear() {
        std::lock_guard<std::mutex> lock(mutex);
        packets.clear();
    }

    json to_json() const {
        json obj;
        for (const auto &packet: packets)
            obj["input"].push_back(packet.to_json());
        return obj;
    }

private:
    std::vector<Packet> packets;
    std::mutex mutex;
};

class Corpus {
public:
    explicit Corpus(std::vector<Input> corpus = {}) : corpus{std::move(corpus)} {}

    Corpus(const Corpus &other) : corpus(other.corpus) {}

    Corpus(Corpus &&other) noexcept: corpus(std::move(other.corpus)) {}

    void add(const Input &input) {
        std::lock_guard<std::mutex> lock(mutex);
        corpus.emplace_back(input);
    }

    void clear() {
        std::lock_guard<std::mutex> lock(mutex);
        corpus.clear();
    }

    json to_json() const {
        json obj;
        for (const auto &input: corpus)
            obj["corpus"].push_back(input.to_json());
        return obj;
    }

    void write_to_disk(const std::string &filename) const {
        std::ofstream out(filename);
        if (not out.is_open())
            throw std::runtime_error("file should be open");

        out << std::setw(4) << to_json();
        out.close();
    }

private:
    std::vector<Input> corpus;
    std::mutex mutex;
};
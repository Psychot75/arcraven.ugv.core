#pragma once
#include <cstddef>
#include <cstdint>
#include <filesystem>

#include "models/enums/CommandAuthority.hpp"

namespace arcraven::ugv {

struct PersistentStateV1 {
    uint32_t schema_version = 1;
    uint32_t reserved = 0;
    uint8_t calibrated_ok = 0; // store as byte to avoid ABI bool size issues
    uint8_t last_authority = static_cast<uint8_t>(arcraven::ugv::CommandAuthority::Unknown);
    uint16_t padding = 0;
};

struct StateFileHeader {
    uint32_t magic = 0x41524355u; // 'ARCU'
    uint32_t version = 1;
    uint32_t payload_size = sizeof(PersistentStateV1);
    uint32_t payload_crc32 = 0;
};

uint32_t crc32_ieee(const uint8_t* data, size_t len);

class StateStore {
public:
    explicit StateStore(std::filesystem::path path);

    bool load(PersistentStateV1& out);
    bool save(const PersistentStateV1& s);

private:
    std::filesystem::path path_;
};

} // namespace arcraven::ugvcore

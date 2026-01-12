#include "core/StateStore.hpp"

#include <fstream>
#include <string>
#include <system_error>

#include "utils/Logger.hpp"

namespace arcraven::ugv {

uint32_t crc32_ieee(const uint8_t* data, size_t len) {
    uint32_t crc = 0xFFFFFFFFu;
    for (size_t i = 0; i < len; ++i) {
        crc ^= static_cast<uint32_t>(data[i]);
        for (int b = 0; b < 8; ++b) {
            const uint32_t mask = (crc & 1u) ? 0xFFFFFFFFu : 0u;
            crc = (crc >> 1u) ^ (0xEDB88320u & mask);
        }
    }
    return ~crc;
}

StateStore::StateStore(std::filesystem::path path) : path_(std::move(path)) {}

bool StateStore::load(PersistentStateV1& out) {
    std::ifstream in(path_, std::ios::binary);
    if (!in.good()) return false;

    StateFileHeader hdr{};
    in.read(reinterpret_cast<char*>(&hdr), sizeof(hdr));
    if (!in.good()) return false;

    if (hdr.magic != 0x41524355u || hdr.version != 1) {
        ARC_LOG_WARN("StateStore: invalid header (magic/version mismatch)");
        return false;
    }
    if (hdr.payload_size != sizeof(PersistentStateV1)) {
        ARC_LOG_WARN("StateStore: payload size mismatch");
        return false;
    }

    PersistentStateV1 tmp{};
    in.read(reinterpret_cast<char*>(&tmp), sizeof(tmp));
    if (!in.good()) return false;

    const uint32_t crc = crc32_ieee(reinterpret_cast<const uint8_t*>(&tmp), sizeof(tmp));
    if (crc != hdr.payload_crc32) {
        ARC_LOG_WARN("StateStore: CRC mismatch (corrupt state file?)");
        return false;
    }

    out = tmp;
    return true;
}

bool StateStore::save(const PersistentStateV1& s) {
    std::filesystem::create_directories(path_.parent_path());

    StateFileHeader hdr{};
    hdr.payload_crc32 = crc32_ieee(reinterpret_cast<const uint8_t*>(&s), sizeof(s));

    const auto tmp_path = path_.string() + ".tmp";
    std::ofstream out(tmp_path, std::ios::binary | std::ios::trunc);
    if (!out.good()) return false;

    out.write(reinterpret_cast<const char*>(&hdr), sizeof(hdr));
    out.write(reinterpret_cast<const char*>(&s), sizeof(s));
    out.flush();
    out.close();

    std::error_code ec;
    std::filesystem::rename(tmp_path, path_, ec);
    if (ec) {
        // Windows rename-over-existing can fail; try remove then rename.
        std::filesystem::remove(path_, ec);
        ec.clear();
        std::filesystem::rename(tmp_path, path_, ec);
    }
    return !ec;
}

} // namespace arcraven::ugv

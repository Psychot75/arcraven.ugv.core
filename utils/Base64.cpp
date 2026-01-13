#include "utils/Base64.hpp"

#include <array>
#include <cctype>

namespace arcraven::utils {

static constexpr char kEncodeTable[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

std::string base64_encode(const std::string& input) {
    std::string out;
    out.reserve(((input.size() + 2) / 3) * 4);

    size_t i = 0;
    while (i + 2 < input.size()) {
        const uint32_t chunk = (static_cast<uint32_t>(static_cast<unsigned char>(input[i])) << 16) |
                               (static_cast<uint32_t>(static_cast<unsigned char>(input[i + 1])) << 8) |
                               static_cast<uint32_t>(static_cast<unsigned char>(input[i + 2]));
        out.push_back(kEncodeTable[(chunk >> 18) & 0x3F]);
        out.push_back(kEncodeTable[(chunk >> 12) & 0x3F]);
        out.push_back(kEncodeTable[(chunk >> 6) & 0x3F]);
        out.push_back(kEncodeTable[chunk & 0x3F]);
        i += 3;
    }

    const size_t remaining = input.size() - i;
    if (remaining == 1) {
        const uint32_t chunk = static_cast<uint32_t>(static_cast<unsigned char>(input[i])) << 16;
        out.push_back(kEncodeTable[(chunk >> 18) & 0x3F]);
        out.push_back(kEncodeTable[(chunk >> 12) & 0x3F]);
        out.push_back('=');
        out.push_back('=');
    } else if (remaining == 2) {
        const uint32_t chunk = (static_cast<uint32_t>(static_cast<unsigned char>(input[i])) << 16) |
                               (static_cast<uint32_t>(static_cast<unsigned char>(input[i + 1])) << 8);
        out.push_back(kEncodeTable[(chunk >> 18) & 0x3F]);
        out.push_back(kEncodeTable[(chunk >> 12) & 0x3F]);
        out.push_back(kEncodeTable[(chunk >> 6) & 0x3F]);
        out.push_back('=');
    }

    return out;
}

std::string base64_decode(const std::string& input, bool& ok) {
    static std::array<int, 256> table = [] {
        std::array<int, 256> t{};
        t.fill(-1);
        for (int i = 0; i < 64; ++i) {
            t[static_cast<unsigned char>(kEncodeTable[i])] = i;
        }
        t[static_cast<unsigned char>('=')] = 0;
        return t;
    }();

    ok = true;
    std::string out;
    out.reserve((input.size() / 4) * 3);

    if (input.size() % 4 != 0) {
        ok = false;
        return {};
    }

    for (size_t i = 0; i < input.size(); i += 4) {
        int vals[4];
        for (int j = 0; j < 4; ++j) {
            const unsigned char c = static_cast<unsigned char>(input[i + j]);
            int v = table[c];
            if (v < 0) {
                ok = false;
                return {};
            }
            vals[j] = v;
        }

        const uint32_t chunk = (static_cast<uint32_t>(vals[0]) << 18) |
                               (static_cast<uint32_t>(vals[1]) << 12) |
                               (static_cast<uint32_t>(vals[2]) << 6) |
                               static_cast<uint32_t>(vals[3]);
        out.push_back(static_cast<char>((chunk >> 16) & 0xFF));
        if (input[i + 2] != '=') {
            out.push_back(static_cast<char>((chunk >> 8) & 0xFF));
        }
        if (input[i + 3] != '=') {
            out.push_back(static_cast<char>(chunk & 0xFF));
        }
    }

    return out;
}

} // namespace arcraven::utils

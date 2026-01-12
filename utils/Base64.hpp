#pragma once

#include <string>

namespace arcraven::utils {

std::string base64_encode(const std::string& input);
std::string base64_decode(const std::string& input, bool& ok);

} // namespace arcraven::utils

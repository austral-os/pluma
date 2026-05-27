#pragma once

#include <string>
#include <vector>

namespace pluma_app {
namespace utils {

class FontUtils {
public:
  // Returns a sorted list of unique font family names installed on the system
  static std::vector<std::string> get_installed_fonts();
};

} // namespace utils
} // namespace pluma_app

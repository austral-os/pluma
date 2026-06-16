#pragma once

#include "Widgets/PlumaOptionItem.hpp"
#include <string>

namespace pluma_app {

class OptionButton : public PlumaOptionItem {
public:
  OptionButton(const std::string& text, const std::string& icon_name = "");
  ~OptionButton() override = default;
};

} // namespace pluma_app

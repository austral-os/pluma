#pragma once

#include "Widgets/PlumaOptionItem.hpp"
#include <pluma/Layout/PageSize.hpp>
#include <string>

namespace pluma_app {

class SizeButton : public PlumaOptionItem {
public:
  SizeButton(const std::string& name, const std::string& dims, pluma::PageSize size);
  ~SizeButton() override = default;

  pluma::PageSize get_size() const;

private:
  pluma::PageSize m_size;
};

} // namespace pluma_app

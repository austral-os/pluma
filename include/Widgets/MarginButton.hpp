#pragma once

#include "Widgets/PlumaOptionItem.hpp"
#include <pluma/Layout/PageSize.hpp>
#include <string>

namespace pluma_app {

class MarginButton : public PlumaOptionItem {
public:
  MarginButton(const std::string& name, const std::string& dims, int t, int b, int l, int r);
  ~MarginButton() override = default;

  pluma::PageMargins get_margins() const;

private:
  pluma::PageMargins m_margins;
};

} // namespace pluma_app

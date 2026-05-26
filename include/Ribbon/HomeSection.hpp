#pragma once

#include <horizon/RibbonToolbar.hpp>
#include <horizon/Label.hpp>

namespace pluma_app {

class HomeSection {
public:
  HomeSection(horizon::RibbonToolbar *ribbon, int tab_index);
  ~HomeSection() = default;

private:
  horizon::RibbonSection *m_section_font = nullptr;
  horizon::Label *m_label = nullptr;
};

} // namespace pluma_app

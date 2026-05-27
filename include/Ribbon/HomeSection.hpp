#pragma once

#include <horizon/RibbonToolbar.hpp>
#include <horizon/Combo.hpp>
#include <horizon/GroupButton.hpp>
#include <horizon/Label.hpp>

namespace pluma_app {

class HomeSection {
public:
  HomeSection(horizon::RibbonToolbar *ribbon, int tab_index);
  ~HomeSection() = default;

  horizon::Combo* combo_font_family() const { return m_combo_font_family; }
  horizon::Combo* combo_font_size() const { return m_combo_font_size; }

private:
  horizon::RibbonSection *m_section_font = nullptr;
  horizon::Combo *m_combo_font_family = nullptr;
  horizon::Combo *m_combo_font_size = nullptr;
  horizon::GroupButton *m_group_font_size = nullptr;
  horizon::GroupButton *m_group_styles = nullptr;
  horizon::GroupButton *m_group_colors = nullptr;
};

} // namespace pluma_app

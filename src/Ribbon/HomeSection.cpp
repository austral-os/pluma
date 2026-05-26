#include "Ribbon/HomeSection.hpp"

namespace pluma_app {

HomeSection::HomeSection(horizon::RibbonToolbar *ribbon, int tab_index) {
  m_section_font = ribbon->add_section(tab_index, "Font");

  auto lbl = std::make_unique<horizon::Label>();
  lbl->set_text("Ribbon toolbar");
  
  m_label = lbl.get();
  m_section_font->add_widget(std::move(lbl));
}

} // namespace pluma_app

#pragma once

#include <horizon/RibbonToolbar.hpp>
#include <horizon/EventsManager.hpp>
#include <pluma/Style/StyleProperties.hpp>
#include <map>

namespace pluma_app {

class ImageFormatSection {
public:
  ImageFormatSection(horizon::RibbonToolbar *ribbon, int tab_index);
  ~ImageFormatSection() = default;

  horizon::EventsManager<pluma::TextWrapMode> when_wrap_mode_selected;
  
  void update_active_mode(pluma::TextWrapMode mode);

private:
  horizon::RibbonSection *m_section_format = nullptr;
  std::map<pluma::TextWrapMode, class PlumaToolbarButton*> m_wrap_buttons;
};

} // namespace pluma_app

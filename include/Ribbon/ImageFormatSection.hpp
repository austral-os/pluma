#pragma once

#include <horizon/RibbonToolbar.hpp>
#include <horizon/EventsManager.hpp>
#include <pluma/Style/StyleProperties.hpp>

namespace pluma_app {

class ImageFormatSection {
public:
  ImageFormatSection(horizon::RibbonToolbar *ribbon, int tab_index);
  ~ImageFormatSection() = default;

  horizon::EventsManager<pluma::TextWrapMode> when_wrap_mode_selected;

private:
  horizon::RibbonSection *m_section_format = nullptr;
};

} // namespace pluma_app

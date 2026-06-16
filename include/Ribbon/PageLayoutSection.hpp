#pragma once

#include <horizon/RibbonToolbar.hpp>
#include <horizon/GroupButton.hpp>
#include <horizon/ToolbarButton.hpp>
#include <horizon/EventsManager.hpp>
#include <pluma/Layout/PageSize.hpp>
#include "Widgets/OptionButton.hpp"

namespace pluma_app {

class PageLayoutSection {
public:
  PageLayoutSection(horizon::RibbonToolbar *ribbon, int tab_index);
  ~PageLayoutSection() = default;

  horizon::ToolbarButton* btn_margins() const { return m_btn_margins; }
  OptionButton* btn_orientation() const { return m_btn_orientation; }
  OptionButton* btn_size() const { return m_btn_size; }

  horizon::EventsManager<pluma::PageMargins> when_margin_selected;
  horizon::EventsManager<bool> when_orientation_selected;

private:
  horizon::RibbonSection *m_section_setup = nullptr;
  horizon::ToolbarButton *m_btn_margins = nullptr;
  OptionButton *m_btn_orientation = nullptr;
  OptionButton *m_btn_size = nullptr;
};

} // namespace pluma_app

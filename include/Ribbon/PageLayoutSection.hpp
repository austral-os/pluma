#pragma once

#include <horizon/RibbonToolbar.hpp>
#include <horizon/GroupButton.hpp>
#include <horizon/ToolbarButton.hpp>
#include <horizon/EventsManager.hpp>
#include <pluma/Layout/PageSize.hpp>
#include "Widgets/OptionButton.hpp"
#include "Widgets/SizeButton.hpp"
#include <horizon/RibbonButton.hpp>

namespace pluma_app {

class PageLayoutSection {
public:
  PageLayoutSection(horizon::RibbonToolbar *ribbon, int tab_index);
  ~PageLayoutSection() = default;

  horizon::RibbonButton* btn_margins() const { return m_btn_margins; }
  horizon::RibbonButton* btn_orientation() const { return m_btn_orientation; }
  horizon::RibbonButton* btn_size() const { return m_btn_size; }

  horizon::EventsManager<pluma::PageMargins> when_margin_selected;
  horizon::EventsManager<bool> when_orientation_selected;
  horizon::EventsManager<pluma::PageSize> when_size_selected;

private:
  horizon::RibbonSection *m_section_setup = nullptr;
  horizon::RibbonButton *m_btn_margins = nullptr;
  horizon::RibbonButton *m_btn_orientation = nullptr;
  horizon::RibbonButton *m_btn_size = nullptr;
};

} // namespace pluma_app

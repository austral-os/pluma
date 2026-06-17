#pragma once

#include <horizon/RibbonToolbar.hpp>
#include <horizon/EventsManager.hpp>

namespace pluma_app {

class TableLayoutSection {
public:
  TableLayoutSection(horizon::RibbonToolbar *ribbon, int tab_index);
  ~TableLayoutSection() = default;

  horizon::EventsManager<int> when_insert_above_clicked;
  horizon::EventsManager<int> when_insert_below_clicked;
  horizon::EventsManager<int> when_insert_left_clicked;
  horizon::EventsManager<int> when_insert_right_clicked;

private:
  horizon::RibbonSection *m_section_rows_cols = nullptr;
  
  class PlumaToolbarButton* m_btn_insert_above = nullptr;
  class PlumaToolbarButton* m_btn_insert_below = nullptr;
  class PlumaToolbarButton* m_btn_insert_left = nullptr;
  class PlumaToolbarButton* m_btn_insert_right = nullptr;
};

} // namespace pluma_app

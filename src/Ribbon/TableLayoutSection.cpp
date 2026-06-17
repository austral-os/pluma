#include "Ribbon/TableLayoutSection.hpp"
#include <horizon/ToolbarButton.hpp>
#include <horizon/Widget.hpp>
#include <horizon/I18n.hpp>
#include "Widgets/PlumaToolbarButton.hpp"

namespace pluma_app {

TableLayoutSection::TableLayoutSection(horizon::RibbonToolbar *ribbon, int tab_index) {
  m_section_rows_cols = ribbon->add_section(tab_index, "Rows & Columns");

  auto wrap_container = std::make_unique<horizon::Widget>();
  wrap_container->set_layout_type(horizon::WIDGET_LAYOUT_HORIZONTAL);
  wrap_container->set_spacing(4);
  wrap_container->set_fixed_size(280);

  auto create_button = [this](const std::string& name, const std::string& icon_name, class PlumaToolbarButton*& btn_ptr) {
      auto btn = std::make_unique<PlumaToolbarButton>(name, icon_name);
      btn->set_size(64, 64);
      btn->set_icon_size(32);
      btn->set_fixed_size(64);
      btn_ptr = btn.get();
      return btn;
  };

  wrap_container->add_child(create_button("Insert Above", "pluma-tbl-ins-row-before", m_btn_insert_above));
  wrap_container->add_child(create_button("Insert Below", "pluma-tbl-ins-row-after", m_btn_insert_below));
  wrap_container->add_child(create_button("Insert Left", "pluma-tbl-ins-column-before", m_btn_insert_left));
  wrap_container->add_child(create_button("Insert Right", "pluma-tbl-ins-column-after", m_btn_insert_right));

  m_btn_insert_above->when_click.connect([this](auto&) { int dummy = 0; when_insert_above_clicked.run(dummy); });
  m_btn_insert_below->when_click.connect([this](auto&) { int dummy = 0; when_insert_below_clicked.run(dummy); });
  m_btn_insert_left->when_click.connect([this](auto&) { int dummy = 0; when_insert_left_clicked.run(dummy); });
  m_btn_insert_right->when_click.connect([this](auto&) { int dummy = 0; when_insert_right_clicked.run(dummy); });

  m_section_rows_cols->add_widget(std::move(wrap_container));

  m_section_merge = ribbon->add_section(tab_index, "Merge");

  auto merge_container = std::make_unique<horizon::Widget>();
  merge_container->set_layout_type(horizon::WIDGET_LAYOUT_HORIZONTAL);
  merge_container->set_spacing(4);
  merge_container->set_fixed_size(210); // 3 buttons of 64 + spacing

  merge_container->add_child(create_button("Merge Cells", "pluma-tbl-merge-cells", m_btn_merge_cells));
  merge_container->add_child(create_button("Split Cells", "pluma-tbl-split-cells", m_btn_split_cells));
  merge_container->add_child(create_button("Split Table", "pluma-tbl-split", m_btn_split_table));

  m_btn_merge_cells->when_click.connect([this](auto&) { int dummy = 0; when_merge_cells_clicked.run(dummy); });
  m_btn_split_cells->when_click.connect([this](auto&) { int dummy = 0; when_split_cells_clicked.run(dummy); });
  m_btn_split_table->when_click.connect([this](auto&) { int dummy = 0; when_split_table_clicked.run(dummy); });

  m_section_merge->add_widget(std::move(merge_container));
}

} // namespace pluma_app

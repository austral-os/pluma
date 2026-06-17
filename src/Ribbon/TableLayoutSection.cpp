#include "Ribbon/TableLayoutSection.hpp"
#include <horizon/ToolbarButton.hpp>
#include <horizon/Widget.hpp>
#include <horizon/I18n.hpp>
#include "Widgets/PlumaToolbarButton.hpp"
#include <horizon/Vault.hpp>
#include <horizon/Label.hpp>
#include <horizon/Spacer.hpp>
#include <horizon/Application.hpp>
#include "Widgets/OptionButton.hpp"
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
  m_btn_split_table->when_click.connect([this](auto&) { int dummy = 0; when_split_table_clicked.run(dummy); });

  auto vault = std::make_unique<horizon::Vault>();
  auto vault_content = std::make_unique<horizon::Widget>();
  vault_content->set_layout_type(horizon::WIDGET_LAYOUT_VERTICAL);
  vault_content->set_spacing(4);
  vault_content->set_margin(8);
  vault_content->set_size(240, 110);

  auto title = std::make_unique<horizon::Label>(horizon::i18n().tr("pluma-writer.table.split_cells"));
  title->set_font_size(16);
  title->set_font_weight(horizon::FONT_WEIGHT_BOLD);
  title->set_fixed_size(24);
  vault_content->add_child(std::move(title));

  auto create_split_item = [this](const std::string& name, const std::string& icon_name, bool horizontal) {
      auto btn = std::make_unique<OptionButton>(name, icon_name);
      btn->when_click.connect([this, horizontal](auto&) {
          int dummy = 0;
          if (horizontal) {
              when_split_cells_horizontally_clicked.run(dummy);
          } else {
              when_split_cells_vertically_clicked.run(dummy);
          }
          if (m_btn_split_cells && m_btn_split_cells->application()) {
              m_btn_split_cells->application()->close_vault();
          }
      });
      return btn;
  };

  vault_content->add_child(create_split_item(horizon::i18n().tr("pluma-writer.table.split_horizontally"), "pluma-tbl-split-cells", true));
  vault_content->add_child(create_split_item(horizon::i18n().tr("pluma-writer.table.split_vertically"), "pluma-tbl-split-cells", false));

  vault->set_content(std::move(vault_content));
  m_btn_split_cells->set_vault(std::move(vault));

  m_section_merge->add_widget(std::move(merge_container));
}

} // namespace pluma_app

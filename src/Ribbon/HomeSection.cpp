#include "Ribbon/HomeSection.hpp"
#include "utils/FontUtils.hpp"

#include <Spacer.hpp>
#include <horizon/Widget.hpp>

namespace pluma_app {

HomeSection::HomeSection(horizon::RibbonToolbar *ribbon, int tab_index) {
  m_section_font = ribbon->add_section(tab_index, "Font");

  auto col_container = std::make_unique<horizon::Widget>();
  col_container->set_layout_type(horizon::WIDGET_LAYOUT_VERTICAL);
  col_container->set_spacing(4);
  col_container->set_fixed_size(450);

  auto row1 = std::make_unique<horizon::Widget>();
  row1->set_layout_type(horizon::WIDGET_LAYOUT_HORIZONTAL);
  row1->set_spacing(4);

  auto combo_family = std::make_unique<horizon::Combo>();
  auto fonts = utils::FontUtils::get_installed_fonts();
  int idx = 0;
  int selected_idx = 0;
  for (const auto &f : fonts) {
    combo_family->add_item(f, f);
    if (f == "Arial" || f == "Liberation Sans" ||
        f == "Ubuntu") { // Try to find a reasonable default
      selected_idx = idx;
    }
    idx++;
  }
  combo_family->set_selected_item_index(selected_idx);

  m_combo_font_family = combo_family.get();

  auto combo_size = std::make_unique<horizon::Combo>();
  combo_size->add_item("10", "10");
  combo_size->add_item("12", "12");
  combo_size->add_item("14", "14");
  combo_size->add_item("16", "16");
  combo_size->add_item("18", "18");
  combo_size->add_item("24", "24");
  combo_size->add_item("36", "36");
  combo_size->add_item("48", "48");
  combo_size->add_item("72", "72");
  combo_size->set_selected_item_index(1);
  combo_size->set_fixed_size(80);
  m_combo_font_size = combo_size.get();

  auto group_btn = std::make_unique<horizon::GroupButton>();
  group_btn->add_item("A+");
  group_btn->add_item("A-");
  group_btn->set_fixed_size(80);
  m_group_font_size = group_btn.get();

  row1->add_child(std::move(combo_family));
  row1->add_child(std::move(combo_size));
  row1->add_child(std::move(group_btn));

  auto row2 = std::make_unique<horizon::Widget>();
  row2->set_layout_type(horizon::WIDGET_LAYOUT_HORIZONTAL);
  row2->set_spacing(4);

  auto btn_styles = std::make_unique<MultiToggleGroupButton>();
  btn_styles->set_fixed_size(240);
  btn_styles->add_item("B");
  btn_styles->add_item("I");
  btn_styles->add_item("U");
  btn_styles->add_item("x²");
  btn_styles->add_item("x₂");
  m_group_styles = btn_styles.get();

  auto btn_colors = std::make_unique<horizon::GroupButton>();
  btn_colors->set_fixed_size(80);
  btn_colors->add_item("A_");
  btn_colors->add_item("ab");
  m_group_colors = btn_colors.get();

  row2->add_child(std::move(btn_styles));
  row2->add_child(std::move(btn_colors));
  // row2->add_child(horizon::Spacer());

  col_container->add_child(std::move(row1));
  col_container->add_child(horizon::Spacer(1));
  col_container->add_child(std::move(row2));

  m_section_font->add_widget(std::move(col_container));
}

} // namespace pluma_app

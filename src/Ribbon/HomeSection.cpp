#include "Ribbon/HomeSection.hpp"
#include "utils/FontUtils.hpp"

#include <Spacer.hpp>
#include <horizon/Widget.hpp>
#include <horizon/Notification.hpp>

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

  auto set_tooltip = [](horizon::Widget* w, const std::string& msg) {
      auto t = std::make_unique<horizon::Notification>();
      t->set_message(msg);
      w->set_tooltip(std::move(t));
  };

  auto group_btn = std::make_unique<horizon::GroupButton>();
  group_btn->add_item("A+");
  group_btn->add_item("A-");
  if (group_btn->children().size() >= 2) {
      set_tooltip(group_btn->children()[0].get(), "Aumentar tamaño de fuente");
      set_tooltip(group_btn->children()[1].get(), "Disminuir tamaño de fuente");
  }
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
  if (btn_styles->children().size() >= 5) {
      set_tooltip(btn_styles->children()[0].get(), "Negrita");
      set_tooltip(btn_styles->children()[1].get(), "Cursiva");
      set_tooltip(btn_styles->children()[2].get(), "Subrayado");
      set_tooltip(btn_styles->children()[3].get(), "Superíndice");
      set_tooltip(btn_styles->children()[4].get(), "Subíndice");
  }
  m_group_styles = btn_styles.get();

  auto btn_colors = std::make_unique<horizon::GroupButton>();
  btn_colors->set_fixed_size(80);
  btn_colors->add_item("A_");
  btn_colors->add_item("ab");
  if (btn_colors->children().size() >= 2) {
      set_tooltip(btn_colors->children()[0].get(), "Color de texto");
      set_tooltip(btn_colors->children()[1].get(), "Color de fondo");
  }
  m_group_colors = btn_colors.get();

  row2->add_child(std::move(btn_styles));
  row2->add_child(std::move(btn_colors));
  // row2->add_child(horizon::Spacer());

  col_container->add_child(std::move(row1));
  col_container->add_child(horizon::Spacer(1));
  col_container->add_child(std::move(row2));

  m_section_font->add_widget(std::move(col_container));

  // Paragraph Section
  m_section_paragraph = ribbon->add_section(tab_index, "Paragraph");

  auto para_container = std::make_unique<horizon::Widget>();
  para_container->set_layout_type(horizon::WIDGET_LAYOUT_VERTICAL);
  para_container->set_spacing(4);
  para_container->set_fixed_size(180);

  auto para_row1 = std::make_unique<horizon::Widget>();
  para_row1->set_layout_type(horizon::WIDGET_LAYOUT_HORIZONTAL);
  para_row1->set_spacing(4);

  auto btn_lists = std::make_unique<horizon::GroupButton>();
  btn_lists->set_fixed_size(80);
  btn_lists->add_item("-");
  btn_lists->add_item("1.");
  if (btn_lists->children().size() >= 2) {
      set_tooltip(btn_lists->children()[0].get(), "Viñetas");
      set_tooltip(btn_lists->children()[1].get(), "Numeración");
  }
  m_group_lists = btn_lists.get();

  auto btn_indent = std::make_unique<horizon::GroupButton>();
  btn_indent->set_fixed_size(80);
  btn_indent->add_item("<");
  btn_indent->add_item(">");
  if (btn_indent->children().size() >= 2) {
      set_tooltip(btn_indent->children()[0].get(), "Disminuir sangría");
      set_tooltip(btn_indent->children()[1].get(), "Aumentar sangría");
  }
  m_group_indent = btn_indent.get();

  para_row1->add_child(std::move(btn_lists));
  para_row1->add_child(std::move(btn_indent));

  auto para_row2 = std::make_unique<horizon::Widget>();
  para_row2->set_layout_type(horizon::WIDGET_LAYOUT_HORIZONTAL);
  para_row2->set_spacing(4);

  auto btn_align = std::make_unique<MultiToggleGroupButton>();
  btn_align->set_fixed_size(160);
  btn_align->add_item("L");
  btn_align->add_item("C");
  btn_align->add_item("R");
  btn_align->add_item("J");
  if (btn_align->children().size() >= 4) {
      set_tooltip(btn_align->children()[0].get(), "Alinear a la izquierda");
      set_tooltip(btn_align->children()[1].get(), "Centrar");
      set_tooltip(btn_align->children()[2].get(), "Alinear a la derecha");
      set_tooltip(btn_align->children()[3].get(), "Justificar");
  }
  m_group_alignment = btn_align.get();

  para_row2->add_child(std::move(btn_align));

  para_container->add_child(std::move(para_row1));
  para_container->add_child(horizon::Spacer(1));
  para_container->add_child(std::move(para_row2));

  m_section_paragraph->add_widget(std::move(para_container));
}

} // namespace pluma_app

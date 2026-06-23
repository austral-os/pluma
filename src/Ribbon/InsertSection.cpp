#include "Ribbon/InsertSection.hpp"
#include <horizon/Icon.hpp>
#include <horizon/Label.hpp>
#include <horizon/RibbonButton.hpp>
#include <horizon/Vault.hpp>
#include <horizon/Widget.hpp>

namespace pluma_app {

InsertSection::InsertSection(horizon::RibbonToolbar *ribbon, int tab_index) {
  // Page Section
  m_section_page = ribbon->add_section(tab_index, "Page");

  auto btn_page_break = std::make_unique<horizon::RibbonButton>();
  btn_page_break->set_text("Page Break");
  btn_page_break->set_icon(
      "pluma-page-break"); // Assuming icon name exists or generic
  btn_page_break->set_button_size(horizon::RibbonButtonSize::Large);
  m_btn_page_break = btn_page_break.get();
  m_section_page->add_widget(std::move(btn_page_break));

  auto col_wrapper_page = std::make_unique<horizon::Widget>();
  col_wrapper_page->set_layout_type(horizon::WIDGET_LAYOUT_HORIZONTAL);
  col_wrapper_page->set_fixed_size(130);

  auto col_page = std::make_unique<horizon::Widget>();
  col_page->set_layout_type(horizon::WIDGET_LAYOUT_VERTICAL);
  col_page->set_spacing(2);

  auto btn_blank_page = std::make_unique<horizon::RibbonButton>();
  btn_blank_page->set_text("Blank Page");
  btn_blank_page->set_icon("pluma-blank-page");
  btn_blank_page->set_button_size(horizon::RibbonButtonSize::Small);
  btn_blank_page->set_text_position(
      horizon::RibbonButtonTextPosition::RightOfIcon);
  m_btn_blank_page = btn_blank_page.get();
  col_page->add_child(std::move(btn_blank_page));

  auto btn_line_break = std::make_unique<horizon::RibbonButton>();
  btn_line_break->set_text("Line Break");
  btn_line_break->set_icon("pluma-line-break");
  btn_line_break->set_button_size(horizon::RibbonButtonSize::Small);
  btn_line_break->set_text_position(
      horizon::RibbonButtonTextPosition::RightOfIcon);
  m_btn_line_break = btn_line_break.get();
  col_page->add_child(std::move(btn_line_break));

  col_wrapper_page->add_child(std::move(col_page));
  m_section_page->add_widget(std::move(col_wrapper_page));

  // Table Section
  m_section_table = ribbon->add_section(tab_index, "Table");
  auto btn_table = std::make_unique<horizon::RibbonButton>();
  btn_table->set_text("Table");
  btn_table->set_icon("pluma-newtable");
  btn_table->set_button_size(horizon::RibbonButtonSize::Large);
  m_btn_table = btn_table.get();
  m_section_table->add_widget(std::move(btn_table));

  // Image Section
  m_section_image = ribbon->add_section(tab_index, "Images");
  auto btn_image = std::make_unique<horizon::RibbonButton>();
  btn_image->set_text("Image");
  btn_image->set_icon("pluma-image");
  btn_image->set_button_size(horizon::RibbonButtonSize::Large);
  m_btn_image = btn_image.get();
  m_section_image->add_widget(std::move(btn_image));

  // Links Section
  m_section_links = ribbon->add_section(tab_index, "Links");

  auto btn_hyperlink = std::make_unique<horizon::RibbonButton>();
  btn_hyperlink->set_text("Hyperlink");
  btn_hyperlink->set_icon("pluma-link");
  btn_hyperlink->set_button_size(horizon::RibbonButtonSize::Large);
  m_btn_hyperlink = btn_hyperlink.get();
  m_section_links->add_widget(std::move(btn_hyperlink));

  auto col_wrapper_links = std::make_unique<horizon::Widget>();
  col_wrapper_links->set_layout_type(horizon::WIDGET_LAYOUT_HORIZONTAL);
  col_wrapper_links->set_fixed_size(150);

  auto col_links = std::make_unique<horizon::Widget>();
  col_links->set_layout_type(horizon::WIDGET_LAYOUT_VERTICAL);
  col_links->set_spacing(2);

  auto btn_bookmark = std::make_unique<horizon::RibbonButton>();
  btn_bookmark->set_text("Bookmark");
  btn_bookmark->set_icon("pluma-bookmark");
  btn_bookmark->set_button_size(horizon::RibbonButtonSize::Small);
  btn_bookmark->set_text_position(
      horizon::RibbonButtonTextPosition::RightOfIcon);
  m_btn_bookmark = btn_bookmark.get();
  col_links->add_child(std::move(btn_bookmark));

  auto btn_cross_ref = std::make_unique<horizon::RibbonButton>();
  btn_cross_ref->set_text("Cross-reference");
  btn_cross_ref->set_icon("pluma-cross-ref");
  btn_cross_ref->set_button_size(horizon::RibbonButtonSize::Small);
  btn_cross_ref->set_text_position(
      horizon::RibbonButtonTextPosition::RightOfIcon);
  m_btn_cross_ref = btn_cross_ref.get();
  col_links->add_child(std::move(btn_cross_ref));

  col_wrapper_links->add_child(std::move(col_links));
  m_section_links->add_widget(std::move(col_wrapper_links));

  // Fields Section
  m_section_fields = ribbon->add_section(tab_index, "Fields");
  auto btn_field = std::make_unique<horizon::RibbonButton>();
  btn_field->set_text("Field");
  btn_field->set_icon("pluma-field");
  btn_field->set_button_size(horizon::RibbonButtonSize::Large);

  // Create vault for Field
  auto vault = std::make_unique<horizon::Vault>();
  auto vault_content = std::make_unique<horizon::Widget>();
  vault_content->set_layout_type(horizon::WIDGET_LAYOUT_VERTICAL);
  vault_content->set_spacing(4);
  vault_content->set_margin(8);
  vault_content->set_size(180, 160);

  auto create_field_item = [this](const std::string &name) {
    auto btn = std::make_unique<horizon::RibbonButton>();
    btn->set_text(name);
    btn->set_button_size(horizon::RibbonButtonSize::Small);
    btn->set_text_position(horizon::RibbonButtonTextPosition::RightOfIcon);
    return btn;
  };

  vault_content->add_child(create_field_item("Page number"));
  vault_content->add_child(create_field_item("Page count"));
  vault_content->add_child(create_field_item("Date"));
  vault_content->add_child(create_field_item("Time"));

  vault->set_content(std::move(vault_content));
  btn_field->set_vault(std::move(vault));

  m_btn_field = btn_field.get();
  m_section_fields->add_widget(std::move(btn_field));
}

} // namespace pluma_app

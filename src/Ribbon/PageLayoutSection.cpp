#include "Ribbon/PageLayoutSection.hpp"
#include <horizon/Widget.hpp>
#include <horizon/Icon.hpp>
#include <horizon/Label.hpp>
#include <horizon/Notification.hpp>
#include <horizon/Vault.hpp>
#include <horizon/Application.hpp>
#include <horizon/Spacer.hpp>
#include "Widgets/MarginButton.hpp"
#include "Widgets/OptionButton.hpp"

namespace pluma_app {

PageLayoutSection::PageLayoutSection(horizon::RibbonToolbar *ribbon, int tab_index) {
  m_section_setup = ribbon->add_section(tab_index, "Page Setup");

  auto btn_margins =
      std::make_unique<horizon::ToolbarButton>("Margins", "pluma-margins");
  btn_margins->set_size(64, 64);
  btn_margins->set_icon_size(32);
  btn_margins->set_fixed_size(64);
  

  auto create_margin_item = [this](const std::string& name, const std::string& dims, int t, int b, int l, int r) {
      auto btn = std::make_unique<MarginButton>(name, dims, t, b, l, r);

      btn->when_click.connect([this, m = btn->get_margins()](auto&) {
          pluma::PageMargins margins = m;
          when_margin_selected.run(margins);
          if (m_btn_margins && m_btn_margins->application()) {
              m_btn_margins->application()->close_vault();
          }
      });

      return btn;
  };

  auto vault = std::make_unique<horizon::Vault>();
  auto vault_content = std::make_unique<horizon::Widget>();
  vault_content->set_layout_type(horizon::WIDGET_LAYOUT_VERTICAL);
  vault_content->set_spacing(4);
  vault_content->set_margin(8);

  auto title = std::make_unique<horizon::Label>("Márgenes");
  title->set_font_size(20);
  title->set_font_weight(horizon::FONT_WEIGHT_BOLD);
  title->set_fixed_size(24);
  vault_content->add_child(std::move(title));

  vault_content->add_child(create_margin_item("Normal", "Sup: 2.54 cm   Inf: 2.54 cm\nIzq: 2.54 cm   Der: 2.54 cm", 1440, 1440, 1440, 1440));
  vault_content->add_child(create_margin_item("Estrecho", "Sup: 1.27 cm   Inf: 1.27 cm\nIzq: 1.27 cm   Der: 1.27 cm", 720, 720, 720, 720));
  vault_content->add_child(create_margin_item("Moderado", "Sup: 2.54 cm   Inf: 2.54 cm\nIzq: 1.91 cm   Der: 1.91 cm", 1440, 1440, 1080, 1080));
  vault_content->add_child(create_margin_item("Ancho", "Sup: 2.54 cm   Inf: 2.54 cm\nIzq: 5.08 cm   Der: 5.08 cm", 1440, 1440, 2880, 2880));

  vault_content->add_child(horizon::Spacer());

  vault->set_content(std::move(vault_content));
  btn_margins->set_vault(std::move(vault));

  m_btn_margins = btn_margins.get();
  m_section_setup->add_widget(std::move(btn_margins));

  auto set_tooltip = [](horizon::Widget *w, const std::string &msg) {
    auto t = std::make_unique<horizon::Notification>();
    t->set_message(msg);
    w->set_tooltip(std::move(t));
  };

  auto col_wrapper = std::make_unique<horizon::Widget>();
  col_wrapper->set_layout_type(horizon::WIDGET_LAYOUT_HORIZONTAL);
  col_wrapper->set_fixed_size(180); // Sets width to 180

  // Contenedor vertical para "Orientation" y "Size"
  auto col_container = std::make_unique<horizon::Widget>();
  col_container->set_layout_type(horizon::WIDGET_LAYOUT_VERTICAL);
  col_container->set_position_type(horizon::WidgetPositionTypes::FILL);
  col_container->set_spacing(4);

  // Botón "Orientation"
  auto btn_orient = std::make_unique<OptionButton>("Orientación", "pluma-orientation");
  set_tooltip(btn_orient.get(), "Orientación de página");
  m_btn_orientation = btn_orient.get();

  auto create_orient_item = [this](const std::string& name, const std::string& icon_name, bool landscape) {
      auto btn = std::make_unique<OptionButton>(name, icon_name);
      btn->when_click.connect([this, landscape](auto&) {
          bool landscape_val = landscape;
          when_orientation_selected.run(landscape_val);
          if (m_btn_orientation && m_btn_orientation->application()) {
              m_btn_orientation->application()->close_vault();
          }
      });
      return btn;
  };

  auto vault_orient = std::make_unique<horizon::Vault>();
  auto vault_orient_content = std::make_unique<horizon::Widget>();
  vault_orient_content->set_layout_type(horizon::WIDGET_LAYOUT_VERTICAL);
  vault_orient_content->set_spacing(4);
  vault_orient_content->set_margin(8);
  vault_orient_content->set_size(160, 100);

  auto title_orient = std::make_unique<horizon::Label>("Orientación");
  title_orient->set_font_size(20);
  title_orient->set_font_weight(horizon::FONT_WEIGHT_BOLD);
  title_orient->set_fixed_size(24);
  vault_orient_content->add_child(std::move(title_orient));

  vault_orient_content->add_child(create_orient_item("Vertical", "pluma-orientation", false));
  vault_orient_content->add_child(create_orient_item("Horizontal", "pluma-orientation", true));

  vault_orient->set_content(std::move(vault_orient_content));
  btn_orient->set_vault(std::move(vault_orient));

  col_container->add_child(std::move(btn_orient));

  // Botón "Size"
  auto btn_size = std::make_unique<OptionButton>("Tamaño", "pluma-size");
  set_tooltip(btn_size.get(), "Tamaño de página");
  m_btn_size = btn_size.get();
  col_container->add_child(std::move(btn_size));

  col_wrapper->add_child(std::move(col_container));
  m_section_setup->add_widget(std::move(col_wrapper));
}

} // namespace pluma_app

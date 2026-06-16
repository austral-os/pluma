#include "Ribbon/PageLayoutSection.hpp"
#include <horizon/Widget.hpp>
#include <horizon/Icon.hpp>
#include <horizon/Label.hpp>
#include <horizon/Notification.hpp>
#include <horizon/Vault.hpp>
#include <horizon/Application.hpp>
#include <horizon/Spacer.hpp>
#include "Widgets/MarginButton.hpp"

namespace pluma_app {

PageLayoutSection::PageLayoutSection(horizon::RibbonToolbar *ribbon, int tab_index) {
  m_section_setup = ribbon->add_section(tab_index, "Page Setup");

  auto btn_margins =
      std::make_unique<horizon::ToolbarButton>("Margins", "pluma-margins");
  btn_margins->set_size(64, 64);
  btn_margins->set_icon_size(32);
  btn_margins->set_fixed_size(64);
  
  auto app = ribbon->application();

  auto create_margin_item = [this, app](const std::string& name, const std::string& dims, int t, int b, int l, int r) {
      auto btn = std::make_unique<MarginButton>(name, dims, t, b, l, r);

      btn->when_click.connect([this, m = btn->get_margins(), app](auto&) {
          pluma::PageMargins margins = m;
          when_margin_selected.run(margins);
          app->close_vault();
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

  // Contenedor vertical para "Orientation" y "Size"
  auto col_container = std::make_unique<horizon::Widget>();
  col_container->set_layout_type(horizon::WIDGET_LAYOUT_VERTICAL);
  col_container->set_spacing(4);
  col_container->set_fixed_size(50);

  // Botón "Orientation" (sin texto, como el de tabla en Insert)
  auto btn_orient = std::make_unique<horizon::ToolbarButton>("", "pluma-orientation");
  btn_orient->set_icon_size(24);
  btn_orient->set_fixed_size(36);
  set_tooltip(btn_orient.get(), "Orientación de página");
  m_btn_orientation = btn_orient.get();
  col_container->add_child(std::move(btn_orient));

  // Botón "Size" (sin texto, como el de forma en Insert)
  auto btn_size = std::make_unique<horizon::ToolbarButton>("", "pluma-size");
  btn_size->set_icon_size(24);
  btn_size->set_fixed_size(36);
  set_tooltip(btn_size.get(), "Tamaño de página");
  m_btn_size = btn_size.get();
  col_container->add_child(std::move(btn_size));

  m_section_setup->add_widget(std::move(col_container));
}

} // namespace pluma_app

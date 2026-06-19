#include "Ribbon/ImageFormatSection.hpp"
#include <horizon/ToolbarButton.hpp>
#include <horizon/Widget.hpp>
#include <horizon/I18n.hpp>
#include <horizon/RibbonButton.hpp>

namespace pluma_app {

ImageFormatSection::ImageFormatSection(horizon::RibbonToolbar *ribbon, int tab_index) {
  m_section_format = ribbon->add_section(tab_index, horizon::i18n().tr("pluma-writer.ribbon.image_position"));

  auto wrap_container = std::make_unique<horizon::Widget>();
  wrap_container->set_layout_type(horizon::WIDGET_LAYOUT_HORIZONTAL);
  wrap_container->set_spacing(4);
  wrap_container->set_fixed_size(480); // Ensure enough width for 7x 64px buttons + spacing

  auto create_wrap_button = [this](const std::string& name, const std::string& icon_name, pluma::TextWrapMode mode) {
      auto btn = std::make_unique<horizon::RibbonButton>();
      btn->set_text(name);
      btn->set_icon(icon_name);
      btn->set_button_size(horizon::RibbonButtonSize::Large);
      
      btn->when_click.connect([this, mode](auto&) {
          pluma::TextWrapMode mode_copy = mode;
          when_wrap_mode_selected.run(mode_copy);
      });
      m_wrap_buttons[mode] = btn.get();
      return btn;
  };

  wrap_container->add_child(create_wrap_button(horizon::i18n().tr("pluma-writer.ribbon.square"), "pluma-image-square", pluma::TextWrapMode::Square));
  wrap_container->add_child(create_wrap_button(horizon::i18n().tr("pluma-writer.ribbon.tight"), "pluma-image-tight", pluma::TextWrapMode::Tight));
  wrap_container->add_child(create_wrap_button(horizon::i18n().tr("pluma-writer.ribbon.top_bottom"), "pluma-image-topbottom", pluma::TextWrapMode::TopAndBottom));
  wrap_container->add_child(create_wrap_button(horizon::i18n().tr("pluma-writer.ribbon.through"), "pluma-image-through", pluma::TextWrapMode::Through));
  wrap_container->add_child(create_wrap_button(horizon::i18n().tr("pluma-writer.ribbon.in_front"), "pluma-image-infront", pluma::TextWrapMode::InFrontOfText));
  wrap_container->add_child(create_wrap_button(horizon::i18n().tr("pluma-writer.ribbon.behind"), "pluma-image-behind", pluma::TextWrapMode::BehindText));
  wrap_container->add_child(create_wrap_button(horizon::i18n().tr("pluma-writer.ribbon.inline"), "pluma-image-inline", pluma::TextWrapMode::InLine));

  m_section_format->add_widget(std::move(wrap_container));
}

void ImageFormatSection::update_active_mode(pluma::TextWrapMode mode) {
    for (auto& [m, btn] : m_wrap_buttons) {
        btn->set_active(m == mode);
    }
}

} // namespace pluma_app

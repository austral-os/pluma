#include "Ribbon/ImageFormatSection.hpp"
#include <horizon/ToolbarButton.hpp>
#include <horizon/Widget.hpp>

namespace pluma_app {

ImageFormatSection::ImageFormatSection(horizon::RibbonToolbar *ribbon, int tab_index) {
  m_section_format = ribbon->add_section(tab_index, "Image Position");

  auto wrap_container = std::make_unique<horizon::Widget>();
  wrap_container->set_layout_type(horizon::WIDGET_LAYOUT_HORIZONTAL);
  wrap_container->set_spacing(4);
  wrap_container->set_fixed_size(480); // Ensure enough width for 7x 64px buttons + spacing

  auto create_wrap_button = [this](const std::string& name, const std::string& icon_name, pluma::TextWrapMode mode) {
      auto btn = std::make_unique<horizon::ToolbarButton>(name, icon_name);
      btn->set_size(64, 64);
      btn->set_icon_size(32);
      btn->set_fixed_size(64);
      
      btn->when_click.connect([this, mode](auto&) {
          pluma::TextWrapMode mode_copy = mode;
          when_wrap_mode_selected.run(mode_copy);
      });
      return btn;
  };

  wrap_container->add_child(create_wrap_button("Square", "pluma-image-square", pluma::TextWrapMode::Square));
  wrap_container->add_child(create_wrap_button("Tight", "pluma-image-tight", pluma::TextWrapMode::Tight));
  wrap_container->add_child(create_wrap_button("Top/Bottom", "pluma-image-topbottom", pluma::TextWrapMode::TopAndBottom));
  wrap_container->add_child(create_wrap_button("Through", "pluma-image-through", pluma::TextWrapMode::Through));
  wrap_container->add_child(create_wrap_button("In Front", "pluma-image-infront", pluma::TextWrapMode::InFrontOfText));
  wrap_container->add_child(create_wrap_button("Behind", "pluma-image-behind", pluma::TextWrapMode::BehindText));
  wrap_container->add_child(create_wrap_button("Inline", "pluma-image-inline", pluma::TextWrapMode::InLine));

  m_section_format->add_widget(std::move(wrap_container));
}

} // namespace pluma_app

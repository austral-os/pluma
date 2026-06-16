#include "Widgets/OptionButton.hpp"
#include <horizon/Label.hpp>
#include <horizon/Icon.hpp>
#include <horizon/Widget.hpp>

namespace pluma_app {

OptionButton::OptionButton(const std::string& text, const std::string& icon_name) {
  set_layout_type(horizon::WIDGET_LAYOUT_VERTICAL);
  set_position_type(horizon::WidgetPositionTypes::FILL);
  set_fixed_size(30); 

  auto inner = std::make_unique<horizon::Widget>();
  inner->set_layout_type(horizon::WIDGET_LAYOUT_HORIZONTAL);
  inner->set_position_type(horizon::WidgetPositionTypes::FILL);
  inner->set_margin(2);
  inner->set_spacing(6);

  if (!icon_name.empty()) {
      auto icon = std::make_unique<horizon::Icon>();
      icon->set_icon_name(icon_name);
      icon->set_fixed_size(20);
      inner->add_child(std::move(icon));
  }

  auto label = std::make_unique<horizon::Label>(text);
  label->set_font_size(12);
  inner->add_child(std::move(label));

  add_child(std::move(inner));
}

} // namespace pluma_app

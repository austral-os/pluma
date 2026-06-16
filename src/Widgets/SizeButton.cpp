#include "Widgets/SizeButton.hpp"

#include <horizon/Label.hpp>

namespace pluma_app {

SizeButton::SizeButton(const std::string& name, const std::string& dims, pluma::PageSize size)
    : m_size{size} {
  
  set_layout_type(horizon::WIDGET_LAYOUT_VERTICAL);
  set_position_type(horizon::WidgetPositionTypes::FILL);
  set_fixed_size(66); // 15 + 30 + 4(spacing) + 16(margins) = 65 -> 66 is safe

  auto inner = std::make_unique<horizon::Widget>();
  inner->set_layout_type(horizon::WIDGET_LAYOUT_VERTICAL);
  inner->set_position_type(horizon::WidgetPositionTypes::FILL);
  inner->set_margin(8);
  inner->set_spacing(4);

  auto name_label = std::make_unique<horizon::Label>(name);
  name_label->set_fixed_size(15);
  name_label->set_font_weight(horizon::FONT_WEIGHT_BOLD);
  
  auto dim_label = std::make_unique<horizon::Label>(dims);
  dim_label->set_text_color({0.5f, 0.5f, 0.5f, 1.0f}); // Texto clarito
  dim_label->set_fixed_size(30);
  dim_label->set_font_size(10);

  inner->add_child(std::move(name_label));
  inner->add_child(std::move(dim_label));

  add_child(std::move(inner));
}

pluma::PageSize SizeButton::get_size() const {
  return m_size;
}

} // namespace pluma_app

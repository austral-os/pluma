#include "Widgets/MarginButton.hpp"

#include <horizon/Label.hpp>

namespace pluma_app {

MarginButton::MarginButton(const std::string& name, const std::string& dims, int t, int b, int l, int r)
    : m_margins{pluma::Twips(t), pluma::Twips(b), pluma::Twips(l), pluma::Twips(r)} {
  
  set_layout_type(horizon::WIDGET_LAYOUT_VERTICAL);
  set_position_type(horizon::WidgetPositionTypes::FILL);
  set_fixed_size(66); // 15 + 30 + 4(spacing) + 16(margins) = 65 -> 66 is safe
  set_focusable(true);

  m_bg.set_corner_radius({6, 6, 6, 6});

  when_mouse_enter.connect([this](auto&) {
      m_hovered = true;
      invalidate();
  });

  when_mouse_leave.connect([this](auto&) {
      m_hovered = false;
      invalidate();
  });

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

pluma::PageMargins MarginButton::get_margins() const {
  return m_margins;
}

void MarginButton::draw(horizon::GraphicsContext& gc) {
  if (m_hovered) {
      m_bg.set_position(m_start_draw_x, m_start_draw_y);
      m_bg.set_size(m_width, m_height);
      m_bg.set_draw_state(m_draw_state);
      m_bg.set_application_recursive(application());
      m_bg.calculate_layout();
      m_bg.draw(gc);
  }
  horizon::Widget::draw(gc);
}

} // namespace pluma_app

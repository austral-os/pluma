#include "Widgets/PlumaOptionItem.hpp"

namespace pluma_app {

PlumaOptionItem::PlumaOptionItem() {
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
}

void PlumaOptionItem::draw(horizon::GraphicsContext& gc) {
  if (m_hovered) {
      m_bg.set_application_recursive(application());
      m_bg.set_position(m_start_draw_x, m_start_draw_y);
      m_bg.set_size(m_width, m_height);
      m_bg.calculate_layout();
      m_bg.draw(gc);
  }
  horizon::Widget::draw(gc);
}

} // namespace pluma_app

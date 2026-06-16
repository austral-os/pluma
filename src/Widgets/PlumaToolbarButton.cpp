#include "Widgets/PlumaToolbarButton.hpp"

namespace pluma_app {

PlumaToolbarButton::PlumaToolbarButton(const std::string& text, const std::string& icon_name)
    : horizon::ToolbarButton(text, icon_name) {
    
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

void PlumaToolbarButton::set_active(bool active) {
    if (m_active != active) {
        m_active = active;
        invalidate();
    }
}

bool PlumaToolbarButton::is_active() const {
    return m_active;
}

void PlumaToolbarButton::draw(horizon::GraphicsContext& gc) {
    if (m_hovered || m_active) {
        m_bg.set_application_recursive(application());
        m_bg.set_position(m_start_draw_x, m_start_draw_y);
        m_bg.set_size(m_width, m_height);
        m_bg.calculate_layout();
        m_bg.draw(gc);
    }
    
    // Call base class draw to render the icon and text
    horizon::ToolbarButton::draw(gc);
}

} // namespace pluma_app

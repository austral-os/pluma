#pragma once

#include <horizon/Widget.hpp>
#include <horizon/EventsManager.hpp>

namespace pluma_app {
namespace Widgets {

class TableBordersPreview : public horizon::Widget {
public:
    TableBordersPreview();
    
    // Toggles a specific border
    void set_preset(int preset_index);
    
    void set_line_style(int style_index);
    void set_line_color(horizon::Color color);
    void set_line_thickness(float thickness);

    // 0: top, 1: bottom, 2: left, 3: right, 4: inner horiz, 5: inner vert
    bool m_active_borders[6];

    // Signals when user manually changes a border by clicking
    horizon::EventsManager<horizon::EventContext> when_user_changed;

protected:
    void draw(horizon::GraphicsContext& ctx) override;

private:
    int m_style_index = 0;
    horizon::Color m_line_color{0.0f, 0.0f, 0.0f, 1.0f};
    float m_line_thickness = 1.0f;
};

} // namespace Widgets
} // namespace pluma_app

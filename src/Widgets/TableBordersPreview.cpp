#include "pluma/Widgets/TableBordersPreview.hpp"
#include <horizon/GraphicsContext.hpp>
#include <cmath>

namespace pluma_app {
namespace Widgets {

TableBordersPreview::TableBordersPreview() : horizon::Widget() {
    set_size(150, 150);
    // Default to all borders active
    for (int i = 0; i < 6; ++i) m_active_borders[i] = true;

    when_mouse_press.connect([this](horizon::MouseButtonEventContext& ctx) {
        if (ctx.button != 1) return;

        auto w = width();
        auto h = height();
        float cell_margin = 15;
        
        float top_y = 10 + cell_margin/2.0f;
        float bottom_y = h - 10 - cell_margin/2.0f;
        float left_x = 10 + cell_margin/2.0f;
        float right_x = w - 10 - cell_margin/2.0f;
        float mid_x = w / 2.0f;
        float mid_y = h / 2.0f;

        auto dist_to_segment = [](float px, float py, float x1, float y1, float x2, float y2) {
            float l2 = (x2 - x1)*(x2 - x1) + (y2 - y1)*(y2 - y1);
            if (l2 == 0) return std::sqrt((px - x1)*(px - x1) + (py - y1)*(py - y1));
            float t = std::max(0.0f, std::min(1.0f, ((px - x1)*(x2 - x1) + (py - y1)*(y2 - y1)) / l2));
            float proj_x = x1 + t * (x2 - x1);
            float proj_y = y1 + t * (y2 - y1);
            return std::sqrt((px - proj_x)*(px - proj_x) + (py - proj_y)*(py - proj_y));
        };

        float minDist = 1000.0f;
        int bestIdx = -1;

        double local_x = ctx.x - x();
        double local_y = ctx.y - y();

        float dists[6] = {
            dist_to_segment(local_x, local_y, left_x, top_y, right_x, top_y), // Top
            dist_to_segment(local_x, local_y, left_x, bottom_y, right_x, bottom_y), // Bottom
            dist_to_segment(local_x, local_y, left_x, top_y, left_x, bottom_y), // Left
            dist_to_segment(local_x, local_y, right_x, top_y, right_x, bottom_y), // Right
            dist_to_segment(local_x, local_y, left_x, mid_y, right_x, mid_y), // Inner horiz
            dist_to_segment(local_x, local_y, mid_x, top_y, mid_x, bottom_y) // Inner vert
        };

        for (int i = 0; i < 6; ++i) {
            if (dists[i] < minDist) {
                minDist = dists[i];
                bestIdx = i;
            }
        }

        if (minDist < 10.0f && bestIdx != -1) {
            m_active_borders[bestIdx] = !m_active_borders[bestIdx];
            invalidate();
            horizon::EventContext e;
            when_user_changed.run(e);
        }
    });
}

void TableBordersPreview::set_preset(int preset_index) {
    if (preset_index == 0) { // No borders
        for (int i = 0; i < 6; ++i) m_active_borders[i] = false;
    } else if (preset_index == 1) { // Outer only
        for (int i = 0; i < 4; ++i) m_active_borders[i] = true;
        m_active_borders[4] = false;
        m_active_borders[5] = false;
    } else if (preset_index == 2) { // Outer and horiz
        for (int i = 0; i < 4; ++i) m_active_borders[i] = true;
        m_active_borders[4] = true;
        m_active_borders[5] = false;
    } else if (preset_index == 3) { // All borders
        for (int i = 0; i < 6; ++i) m_active_borders[i] = true;
    } else if (preset_index == 4) { // Outer without changing inner
        for (int i = 0; i < 4; ++i) m_active_borders[i] = true;
    }
    invalidate();
}

void TableBordersPreview::draw(horizon::GraphicsContext& ctx) {
    auto w = width();
    auto h = height();
    auto dx = x();
    auto dy = y();

    // Draw background
    ctx.setColor(0.9f, 0.9f, 0.9f);
    ctx.fillRect(dx, dy, w, h);
    
    // Draw outer frame shadow
    ctx.setColor(1.0f, 1.0f, 1.0f);
    ctx.fillRect(dx + 10, dy + 10, w - 20, h - 20);

    float cell_margin = 15;
    float cell_w = (w - 20 - 3 * cell_margin) / 2.0f;
    float cell_h = (h - 20 - 3 * cell_margin) / 2.0f;

    ctx.setColor(0.8f, 0.8f, 0.8f);
    // Draw 4 cell backgrounds
    ctx.fillRect(dx + 10 + cell_margin, dy + 10 + cell_margin, cell_w, cell_h);
    ctx.fillRect(dx + 10 + cell_w + 2 * cell_margin, dy + 10 + cell_margin, cell_w, cell_h);
    ctx.fillRect(dx + 10 + cell_margin, dy + 10 + cell_h + 2 * cell_margin, cell_w, cell_h);
    ctx.fillRect(dx + 10 + cell_w + 2 * cell_margin, dy + 10 + cell_h + 2 * cell_margin, cell_w, cell_h);

    auto draw_line = [&](float x1, float y1, float x2, float y2, bool active) {
        if (active) {
            ctx.setColor(0.0f, 0.0f, 0.0f); // Black for active
        } else {
            ctx.setColor(0.85f, 0.85f, 0.85f); // Light grey for inactive
        }
        ctx.drawLine(dx + x1, dy + y1, dx + x2, dy + y2, 2.0f);
    };

    // Top
    draw_line(10 + cell_margin/2.0f, 10 + cell_margin/2.0f, w - 10 - cell_margin/2.0f, 10 + cell_margin/2.0f, m_active_borders[0]);
    // Bottom
    draw_line(10 + cell_margin/2.0f, h - 10 - cell_margin/2.0f, w - 10 - cell_margin/2.0f, h - 10 - cell_margin/2.0f, m_active_borders[1]);
    // Left
    draw_line(10 + cell_margin/2.0f, 10 + cell_margin/2.0f, 10 + cell_margin/2.0f, h - 10 - cell_margin/2.0f, m_active_borders[2]);
    // Right
    draw_line(w - 10 - cell_margin/2.0f, 10 + cell_margin/2.0f, w - 10 - cell_margin/2.0f, h - 10 - cell_margin/2.0f, m_active_borders[3]);
    
    // Inner Horiz
    draw_line(10 + cell_margin/2.0f, h / 2.0f, w - 10 - cell_margin/2.0f, h / 2.0f, m_active_borders[4]);
    // Inner Vert
    draw_line(w / 2.0f, 10 + cell_margin/2.0f, w / 2.0f, h - 10 - cell_margin/2.0f, m_active_borders[5]);
}

} // namespace Widgets
} // namespace pluma_app

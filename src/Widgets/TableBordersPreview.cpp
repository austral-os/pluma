#include "pluma/Widgets/TableBordersPreview.hpp"
#include <horizon/GraphicsContext.hpp>
#include <horizon/ThemeManager.hpp>
#include <cairo/cairo.h>
#include <cmath>

namespace pluma_app {
namespace Widgets {

TableBordersPreview::TableBordersPreview() : horizon::Widget() {
    set_size(150, 150);
    for (int i = 0; i < 6; ++i) m_active_borders[i] = true;
    
    if (auto tm = horizon::theme_manager()) {
        m_line_color = tm->get_color("textbox_fg");
    }

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

void TableBordersPreview::set_line_style(int style_index) {
    if (m_style_index != style_index) {
        m_style_index = style_index;
        invalidate();
    }
}

void TableBordersPreview::set_line_color(horizon::Color color) {
    m_line_color = color;
    invalidate();
}

void TableBordersPreview::set_line_thickness(float thickness) {
    if (m_line_thickness != thickness) {
        m_line_thickness = thickness;
        invalidate();
    }
}

void TableBordersPreview::draw(horizon::GraphicsContext& ctx) {
    auto w = width();
    auto h = height();
    auto dx = x();
    auto dy = y();

    horizon::Color page_bg(1.0f, 1.0f, 1.0f, 1.0f);
    horizon::Color widget_bg(0.9f, 0.9f, 0.9f, 1.0f);
    horizon::Color cell_bg(0.8f, 0.8f, 0.8f, 1.0f);
    horizon::Color line_active(0.0f, 0.0f, 0.0f, 1.0f);
    horizon::Color line_inactive(0.85f, 0.85f, 0.85f, 1.0f);

    horizon::Color text_color(0.0f, 0.0f, 0.0f, 1.0f);

    if (auto tm = horizon::theme_manager()) {
        page_bg = tm->get_color("textbox_bg");
        widget_bg = tm->get_color("window_bg");
        text_color = tm->get_color("textbox_fg");
        
        // Cells should be slightly distinguishable from the page
        cell_bg = page_bg.darker(5.0f); 
        
        line_inactive = page_bg.darker(15.0f);
    }

    horizon::Color effective_line_color = m_line_color;
    if (effective_line_color.r == 0.0f && effective_line_color.g == 0.0f && effective_line_color.b == 0.0f && effective_line_color.a == 1.0f) {
        effective_line_color = text_color;
    }

    // Draw widget background
    ctx.setColor(widget_bg.r, widget_bg.g, widget_bg.b);
    ctx.fillRect(dx, dy, w, h);
    
    // Draw table background (same as document page)
    ctx.setColor(page_bg.r, page_bg.g, page_bg.b);
    ctx.fillRect(dx + 10, dy + 10, w - 20, h - 20);

    float cell_margin = 15;
    float cell_w = (w - 20 - 3 * cell_margin) / 2.0f;
    float cell_h = (h - 20 - 3 * cell_margin) / 2.0f;

    ctx.setColor(cell_bg.r, cell_bg.g, cell_bg.b);
    // Draw 4 cell backgrounds
    ctx.fillRect(dx + 10 + cell_margin, dy + 10 + cell_margin, cell_w, cell_h);
    ctx.fillRect(dx + 10 + cell_w + 2 * cell_margin, dy + 10 + cell_margin, cell_w, cell_h);
    ctx.fillRect(dx + 10 + cell_margin, dy + 10 + cell_h + 2 * cell_margin, cell_w, cell_h);
    ctx.fillRect(dx + 10 + cell_w + 2 * cell_margin, dy + 10 + cell_h + 2 * cell_margin, cell_w, cell_h);

    auto* cr = static_cast<cairo_t*>(ctx.getNativeContext());

    auto draw_line = [&](float x1, float y1, float x2, float y2, bool active) {
        if (!cr) {
            // Fallback
            if (active) {
                ctx.setColor(effective_line_color.r, effective_line_color.g, effective_line_color.b);
            } else {
                ctx.setColor(line_inactive.r, line_inactive.g, line_inactive.b);
            }
            ctx.drawLine(dx + x1, dy + y1, dx + x2, dy + y2, m_line_thickness);
            return;
        }

        cairo_save(cr);
        
        if (active) {
            cairo_set_source_rgba(cr, effective_line_color.r, effective_line_color.g, effective_line_color.b, effective_line_color.a);
            cairo_set_line_width(cr, m_line_thickness);

            // style_index mapping: 
            // 0=solid, 1=dashed, 2=dotted, 3=dash_dot, 4=dash_dot_dot, 5=double
            if (m_style_index == 1) { // dashed
                const double dashes[] = {6.0, 3.0};
                cairo_set_dash(cr, dashes, 2, 0);
            } else if (m_style_index == 2) { // dotted
                const double dashes[] = {2.0, 3.0};
                cairo_set_dash(cr, dashes, 2, 0);
            } else if (m_style_index == 3) { // dash_dot
                const double dashes[] = {6.0, 3.0, 2.0, 3.0};
                cairo_set_dash(cr, dashes, 4, 0);
            } else if (m_style_index == 4) { // dash_dot_dot
                const double dashes[] = {6.0, 3.0, 2.0, 3.0, 2.0, 3.0};
                cairo_set_dash(cr, dashes, 6, 0);
            }
        } else {
            cairo_set_source_rgba(cr, line_inactive.r, line_inactive.g, line_inactive.b, line_inactive.a);
            cairo_set_line_width(cr, 1.0);
            cairo_set_dash(cr, nullptr, 0, 0); // Solid
        }

        auto draw_path = [&]() {
            cairo_move_to(cr, dx + x1, dy + y1);
            cairo_line_to(cr, dx + x2, dy + y2);
            cairo_stroke(cr);
        };

        if (active && m_style_index == 5) { // double
            float d_x = 0, d_y = 0;
            if (std::abs(x1 - x2) < 0.1f) d_x = 1.5f + m_line_thickness / 2.0f; // vertical
            else d_y = 1.5f + m_line_thickness / 2.0f; // horizontal
            
            cairo_move_to(cr, dx + x1 - d_x, dy + y1 - d_y);
            cairo_line_to(cr, dx + x2 - d_x, dy + y2 - d_y);
            cairo_stroke(cr);
            
            cairo_move_to(cr, dx + x1 + d_x, dy + y1 + d_y);
            cairo_line_to(cr, dx + x2 + d_x, dy + y2 + d_y);
            cairo_stroke(cr);
        } else {
            draw_path();
        }

        cairo_restore(cr);
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

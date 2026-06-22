#include "pluma/Widgets/ParagraphPreview.hpp"
#include <cairo/cairo.h>
#include <horizon/ThemeManager.hpp>

namespace pluma_app {
namespace widgets {

ParagraphPreview::ParagraphPreview() {
    set_fixed_size(150);
}

void ParagraphPreview::set_indent_before(float cm) {
    m_indent_before = cm;
    invalidate();
}

void ParagraphPreview::set_indent_after(float cm) {
    m_indent_after = cm;
    invalidate();
}

void ParagraphPreview::set_indent_first_line(float cm) {
    m_indent_first_line = cm;
    invalidate();
}

void ParagraphPreview::set_spacing_above(float cm) {
    m_spacing_above = cm;
    invalidate();
}

void ParagraphPreview::set_spacing_below(float cm) {
    m_spacing_below = cm;
    invalidate();
}

void ParagraphPreview::set_line_spacing(float factor) {
    m_line_spacing = factor;
    invalidate();
}

void ParagraphPreview::draw(horizon::GraphicsContext &ctx) {
    horizon::Widget::draw(ctx);
    auto* cr = static_cast<cairo_t*>(ctx.getNativeContext());
    if (!cr) return;

    auto *tm = horizon::theme_manager();
    auto bg_color = tm->get_color("textbox_bg");
    auto fg_color = tm->get_color("textbox_fg");
    auto border_color = tm->get_color("textbox_brd");

    // Draw background
    cairo_set_source_rgba(cr, bg_color.r, bg_color.g, bg_color.b, bg_color.a);
    cairo_rectangle(cr, m_x, m_y, m_width, m_height);
    cairo_fill(cr);

    // Draw border
    cairo_set_source_rgba(cr, border_color.r, border_color.g, border_color.b, border_color.a);
    cairo_set_line_width(cr, 1.0);
    cairo_rectangle(cr, m_x + 0.5, m_y + 0.5, m_width - 1.0, m_height - 1.0);
    cairo_stroke(cr);

    // Draw dummy lines
    cairo_set_source_rgba(cr, fg_color.r, fg_color.g, fg_color.b, fg_color.a * 0.5); // Dimmed fg for lines

    double margin_x = 10.0;
    double margin_y = 10.0;
    
    // Scale cm to pixels for preview (e.g. 1 cm = 10 pixels for the sake of the preview)
    double scale = 15.0; 
    
    double line_height = 8.0;
    double default_spacing = 4.0;
    double spacing = default_spacing * m_line_spacing;
    
    double current_y = m_y + margin_y + (m_spacing_above * scale);
    
    // Paragraph 1: previous paragraph (unaffected)
    for (int i = 0; i < 3; ++i) {
        cairo_rectangle(cr, m_x + margin_x, current_y, m_width - 2 * margin_x, line_height);
        cairo_fill(cr);
        current_y += line_height + default_spacing;
    }
    
    // The configured paragraph
    current_y += (m_spacing_above * scale);
    
    cairo_set_source_rgba(cr, fg_color.r, fg_color.g, fg_color.b, fg_color.a); // Full opacity for the active paragraph
    
    for (int i = 0; i < 4; ++i) {
        double indent_left = m_indent_before * scale;
        double indent_right = m_indent_after * scale;
        if (i == 0) indent_left += (m_indent_first_line * scale);
        
        double x = m_x + margin_x + indent_left;
        double w = m_width - 2 * margin_x - indent_left - indent_right;
        
        // Randomize line width a bit for realism, except if the indent makes it too small
        if (i == 3) w = w * 0.6; 
        
        if (w > 0) {
            cairo_rectangle(cr, x, current_y, w, line_height);
            cairo_fill(cr);
        }
        current_y += line_height + spacing;
    }
    
    current_y += (m_spacing_below * scale);
    
    // Paragraph 3: next paragraph (unaffected)
    cairo_set_source_rgba(cr, fg_color.r, fg_color.g, fg_color.b, fg_color.a * 0.5);
    for (int i = 0; i < 3; ++i) {
        if (current_y + line_height > m_y + m_height - margin_y) break;
        cairo_rectangle(cr, m_x + margin_x, current_y, m_width - 2 * margin_x, line_height);
        cairo_fill(cr);
        current_y += line_height + default_spacing;
    }
}

} // namespace widgets
} // namespace pluma_app

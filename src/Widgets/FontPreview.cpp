#include "pluma/Widgets/FontPreview.hpp"
#include <cairo/cairo.h>
#include <horizon/ThemeManager.hpp>

namespace pluma_app {
namespace widgets {

FontPreview::FontPreview()
    : m_text("El veloz murciélago hindú comía feliz cardillo y kiwi."),
      m_family("sans-serif"), m_size(12.0f), m_bold(false), m_italic(false) {
}

FontPreview::~FontPreview() = default;

void FontPreview::set_preview_text(const std::string& text) {
    if (m_text != text) {
        m_text = text;
        invalidate();
    }
}

void FontPreview::set_preview_font(const std::string& family, float size, bool bold, bool italic) {
    m_family = family;
    m_size = size;
    m_bold = bold;
    m_italic = italic;
    invalidate();
}

void FontPreview::draw(horizon::GraphicsContext &ctx) {
    horizon::Widget::draw(ctx);
    auto* cr = static_cast<cairo_t*>(ctx.getNativeContext());
    if (!cr) return;

    auto *tm = horizon::theme_manager();
    horizon::Color bg_color = tm->get_color("textbox_bg");
    horizon::Color border_color = tm->get_color("textbox_brd");
    horizon::Color text_color = tm->get_color("textbox_fg");

    // Draw background
    cairo_set_source_rgba(cr, bg_color.r, bg_color.g, bg_color.b, bg_color.a);
    cairo_rectangle(cr, m_x, m_y, m_width, m_height);
    cairo_fill(cr);

    // Draw border
    cairo_set_source_rgba(cr, border_color.r, border_color.g, border_color.b, border_color.a);
    cairo_rectangle(cr, m_x + 0.5, m_y + 0.5, m_width - 1, m_height - 1);
    cairo_set_line_width(cr, 1.0);
    cairo_stroke(cr);

    // Setup font
    cairo_font_slant_t slant = m_italic ? CAIRO_FONT_SLANT_ITALIC : CAIRO_FONT_SLANT_NORMAL;
    cairo_font_weight_t weight = m_bold ? CAIRO_FONT_WEIGHT_BOLD : CAIRO_FONT_WEIGHT_NORMAL;
    
    std::string family_name = m_family.empty() ? "sans-serif" : m_family;
    cairo_select_font_face(cr, family_name.c_str(), slant, weight);
    cairo_set_font_size(cr, m_size);

    cairo_set_source_rgba(cr, text_color.r, text_color.g, text_color.b, text_color.a);

    cairo_text_extents_t extents;
    cairo_text_extents(cr, m_text.c_str(), &extents);

    double x = m_x + 10.0;
    double y = m_y + (m_height / 2.0) + (extents.height / 2.0);
    
    cairo_save(cr);
    cairo_rectangle(cr, m_x + 1, m_y + 1, m_width - 2, m_height - 2);
    cairo_clip(cr);

    cairo_move_to(cr, x, y);
    cairo_show_text(cr, m_text.c_str());

    cairo_restore(cr);
}

} // namespace widgets
} // namespace pluma_app

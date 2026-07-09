#include <pluma/Widgets/ListStylePreview.hpp>
#include <horizon/ThemeManager.hpp>

namespace pluma_app {
namespace widgets {

ListStylePreview::ListStylePreview()
    : horizon::Widget()
{
    set_focusable(true);
    set_fixed_size(82);
}

void ListStylePreview::set_list_style(bool ordered, const std::string& style) {
    m_ordered = ordered;
    m_style = style;
    invalidate();
}

void ListStylePreview::set_selected(bool selected) {
    if (m_selected != selected) {
        m_selected = selected;
        invalidate();
    }
}

std::string ListStylePreview::make_marker(int index) const {
    if (m_ordered) {
        if (m_style == "a") {
            char c = 'a' + index;
            return std::string(1, c) + ".";
        }
        if (m_style == "A") {
            char c = 'A' + index;
            return std::string(1, c) + ".";
        }
        if (m_style == "1)") {
            return std::to_string(index + 1) + ")";
        }
        if (m_style == "a)") {
            char c = 'a' + index;
            return std::string(1, c) + ")";
        }
        if (m_style == "A)") {
            char c = 'A' + index;
            return std::string(1, c) + ")";
        }
        return std::to_string(index + 1) + ".";
    }
    if (m_style == "circle") return "\xe2\x97\x8b";
    if (m_style == "square") return "\xe2\x96\xa1";
    return "\xe2\x80\xa2";
}

void ListStylePreview::draw(horizon::GraphicsContext& ctx) {
    int w = width();
    int h = height();

    auto* tm = horizon::theme_manager();
    auto bg = tm->get_color("textbox_bg");
    auto fg = tm->get_color("textbox_fg");
    auto border = tm->get_color("textbox_brd");

    // Background
    if (m_selected) {
        auto sel = tm->get_color("accent_bg");
        ctx.setColor(sel.r, sel.g, sel.b, sel.a);
    } else {
        ctx.setColor(bg.r, bg.g, bg.b, bg.a);
    }
    ctx.fillRect(x(), y(), w, h);

    // Border — thicker when selected
    ctx.setColor(border.r, border.g, border.b, border.a);
    ctx.drawRect(x(), y(), w, h, 0, m_selected ? 2.0f : 1.0f);

    // Sample lines
    ctx.setColor(fg.r, fg.g, fg.b, fg.a);
    ctx.setDrawFont("sans-serif", 11, horizon::FONT_SLANT_NORMAL, horizon::FONT_WEIGHT_NORMAL);

    int line_y = y() + 18;
    for (int i = 0; i < 3; ++i) {
        std::string marker = make_marker(i);

        int marker_x = x() + 6;
        int line_start = x() + std::min(34, std::max(24, w / 4));
        int line_end = x() + w - 10;
        int short_line_end = x() + w - 28;
        if (line_end < line_start) line_end = line_start;
        if (short_line_end < line_start) short_line_end = line_start;

        ctx.drawText(marker_x, line_y, marker.c_str());
        ctx.drawLine(line_start, line_y - 4, line_end, line_y - 4, 1.0f);
        ctx.drawLine(line_start, line_y + 4, short_line_end, line_y + 4, 1.0f);
        line_y += 22;
    }
}

int ListStylePreview::preferred_width() const {
    return 160;
}

int ListStylePreview::preferred_height() const {
    return 82;
}

int ListStylePreview::preferred_height(int /*width*/) const {
    return 82;
}

} // namespace widgets
} // namespace pluma_app

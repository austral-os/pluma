#pragma once

#include <horizon/Widget.hpp>
#include <horizon/GraphicsContext.hpp>
#include <string>

namespace pluma_app {
namespace widgets {

/**
 * @brief A preview card showing sample list lines with a given marker style.
 *
 * Draws 3 sample lines with the specified marker and a selection highlight.
 * Supports UL styles: disc, circle, square
 * Supports OL styles: 1, a, A, 1), a), A)
 */
class ListStylePreview : public horizon::Widget {
public:
    ListStylePreview();

    /// Set the list type and marker style
    void set_list_style(bool ordered, const std::string& style);

    /// Returns the style id
    const std::string& style() const { return m_style; }
    bool is_ordered() const { return m_ordered; }

    void set_selected(bool selected);
    bool is_selected() const { return m_selected; }

    void draw(horizon::GraphicsContext& ctx) override;

    int preferred_width() const override;
    int preferred_height() const override;
    int preferred_height(int width) const override;

private:
    bool m_ordered = false;
    std::string m_style = "disc";
    bool m_selected = false;

    std::string make_marker(int index) const;
};

} // namespace widgets
} // namespace pluma_app

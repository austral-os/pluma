#pragma once

#include <horizon/Widget.hpp>
#include <horizon/GraphicsContext.hpp>

namespace pluma_app {
namespace widgets {

class ParagraphPreview : public horizon::Widget {
public:
    ParagraphPreview();
    ~ParagraphPreview() override = default;

    void set_indent_before(float cm);
    void set_indent_after(float cm);
    void set_indent_first_line(float cm);
    
    void set_spacing_above(float cm);
    void set_spacing_below(float cm);
    
    void set_line_spacing(float factor);

    void draw(horizon::GraphicsContext &ctx) override;

private:
    float m_indent_before = 0.0f;
    float m_indent_after = 0.0f;
    float m_indent_first_line = 0.0f;
    float m_spacing_above = 0.0f;
    float m_spacing_below = 0.0f;
    float m_line_spacing = 1.0f;
};

} // namespace widgets
} // namespace pluma_app

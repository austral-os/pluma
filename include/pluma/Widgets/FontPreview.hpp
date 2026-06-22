#pragma once

#include <horizon/Widget.hpp>
#include <horizon/GraphicsContext.hpp>
#include <string>

namespace pluma_app {
namespace widgets {

class FontPreview : public horizon::Widget {
public:
    FontPreview();
    ~FontPreview() override;

    void set_preview_text(const std::string& text);
    void set_preview_font(const std::string& family, float size, bool bold, bool italic);

    void draw(horizon::GraphicsContext &ctx) override;

private:
    std::string m_text;
    std::string m_family;
    float m_size;
    bool m_bold;
    bool m_italic;
};

} // namespace widgets
} // namespace pluma_app

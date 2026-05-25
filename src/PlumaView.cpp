#include "PlumaView.hpp"
#include <horizon/GraphicsContext.hpp>
#include <pluma/Render/CairoRenderer.hpp>
#include <pluma/Typography/DummyTypography.hpp>

#include <horizon/WaylandWindow.hpp>

namespace pluma_app {

class RealCairoShaper : public pluma::ITextShaper {
public:
  RealCairoShaper() {
    cairo_surface_t *surface =
        cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 1, 1);
    cr_ = cairo_create(surface);
    cairo_select_font_face(cr_, "sans-serif", CAIRO_FONT_SLANT_NORMAL,
                           CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr_, 16.0);
    cairo_surface_destroy(surface);
  }

  ~RealCairoShaper() { cairo_destroy(cr_); }

  pluma::ShapedTextRun shapeText(std::string_view text,
                          const std::shared_ptr<pluma::IFont> &font) override {
    float size_pt = font->getDescriptor().size_pt;
    cairo_set_font_size(cr_, size_pt);
    
    pluma::ShapedTextRun run;
    run.max_ascent = pluma::Twips(size_pt * 15.0 * 0.8);
    run.max_descent = pluma::Twips(size_pt * 15.0 * 0.2);
    run.total_width = pluma::Twips(0);
    
    for (size_t i = 0; i < text.length(); ) {
        unsigned char c = text[i];
        size_t char_len = 1;
        if ((c & 0xE0) == 0xC0) char_len = 2;
        else if ((c & 0xF0) == 0xE0) char_len = 3;
        else if ((c & 0xF8) == 0xF0) char_len = 4;
        
        if (i + char_len > text.length()) char_len = text.length() - i;

        std::string char_str(text.substr(i, char_len));
        cairo_text_extents_t extents;
        cairo_text_extents(cr_, char_str.c_str(), &extents);
        
        pluma::Twips advance(extents.x_advance * 15.0);
        
        pluma::Glyph g;
        g.x_advance = advance;
        run.glyphs.push_back(g);
        run.total_width = run.total_width + advance;

        for (size_t j = 1; j < char_len; ++j) {
            pluma::Glyph g_empty;
            g_empty.x_advance = pluma::Twips(0);
            run.glyphs.push_back(g_empty);
        }

        i += char_len;
    }
    
    return run;
  }

private:
  cairo_t *cr_;
};

PlumaView::PlumaView() : horizon::Widget() {
    auto shaper = std::make_shared<RealCairoShaper>();
    auto font = std::make_shared<pluma::DummyFontManager>()->getFont({"Inter", 12.0f});
    m_editor = std::make_shared<pluma::PlumaEditor>(shaper, font);
    
    m_editor->setPageSize(pluma::PageSizes::A4);
    m_editor->setMargins(pluma::PageMargins(pluma::Twips(1134), pluma::Twips(1134), pluma::Twips(1134), pluma::Twips(1134)));
    m_editor->setMarginColor(pluma::Color(0xFF00AA00));
    m_editor->showMargins();

    m_editor->loadText("Bienvenido a Pluma!\nEste es el procesador de texto basado en libhorizon y libpluma.");

    set_background_color(horizon::Color(0.8f, 0.8f, 0.8f, 1.0f));
    set_focusable(true);

    when_mouse_press.connect([this](horizon::MouseButtonEventContext& ctx) {
        if (application()) {
            application()->set_focused_widget(this);
        }
        if (m_editor) {
            double local_x = ctx.x;
            double local_y = ctx.y;
            pluma::MouseButton pbtn = pluma::MouseButton::None;
            if (ctx.button == 272 || ctx.button == 1) pbtn = pluma::MouseButton::Left;
            else if (ctx.button == 273 || ctx.button == 2) pbtn = pluma::MouseButton::Right;
            else if (ctx.button == 274 || ctx.button == 3) pbtn = pluma::MouseButton::Middle;
            
            m_editor->onMouseDown(local_x, local_y, pbtn, static_cast<pluma::ModifierFlags>(ctx.modifiers));
            invalidate();
        }
    });

    when_mouse_release.connect([this](horizon::MouseButtonEventContext& ctx) {
        if (m_editor) {
            double local_x = ctx.x;
            double local_y = ctx.y;
            pluma::MouseButton pbtn = pluma::MouseButton::None;
            if (ctx.button == 272 || ctx.button == 1) pbtn = pluma::MouseButton::Left;
            else if (ctx.button == 273 || ctx.button == 2) pbtn = pluma::MouseButton::Right;
            else if (ctx.button == 274 || ctx.button == 3) pbtn = pluma::MouseButton::Middle;
            
            m_editor->onMouseUp(local_x, local_y, pbtn, static_cast<pluma::ModifierFlags>(ctx.modifiers));
            invalidate();
        }
    });

    when_mouse_drag.connect([this](horizon::MouseMoveEventContext& ctx) {
        if (m_editor) {
            double local_x = ctx.x;
            double local_y = ctx.y;
            m_editor->onMouseMove(local_x, local_y, static_cast<pluma::ModifierFlags>(ctx.modifiers));
            invalidate();
        }
    });

    when_key_press.connect([this](horizon::KeyEventContext& ctx) {
        if (!m_editor) return;
        
        bool handled = m_editor->onKeyPress(ctx.keysym, static_cast<pluma::ModifierFlags>(ctx.modifiers));
        if (!handled) {
            if (ctx.keysym == 0xff0d || ctx.keysym == 0xff8d) { // Return or KP_Enter
                m_editor->onTextInput("\n");
                handled = true;
            } else if (!ctx.text.empty()) {
                std::string text = ctx.text;
                if (text == "\r") text = "\n";
                m_editor->onTextInput(text);
                handled = true;
            }
        }

        if (handled) {
            ctx.stop_propagation = true;
            invalidate();
        }
    });

    when_key_release.connect([this](horizon::KeyEventContext& ctx) {
        if (!m_editor) return;
        bool handled = m_editor->onKeyRelease(ctx.keysym, static_cast<pluma::ModifierFlags>(ctx.modifiers));
        if (handled) {
            ctx.stop_propagation = true;
            invalidate();
        }
    });
}

void PlumaView::draw(horizon::GraphicsContext& ctx) {
    printf("PlumaView::draw called! Editor is %p\n", m_editor.get());
    if (!m_editor) return;

    cairo_t* cr = static_cast<cairo_t*>(ctx.getNativeContext());
    if (!cr) return;

    pluma::CairoRenderer renderer(cr);
    m_editor->render(renderer);
}

void PlumaView::calculate_layout() {
    horizon::Widget::calculate_layout();
    
    int doc_w = 800;
    int doc_h = 1120;
    
    if (m_editor) {
        auto bounds = m_editor->getDocumentBounds();
        doc_w = static_cast<int>(bounds.width.getValue() / 15.0);
        doc_h = static_cast<int>(bounds.height.getValue() / 15.0);
    }
    
    int target_w = doc_w;
    int target_h = doc_h;
    
    if (parent()) {
        if (parent()->width() > target_w) {
            target_w = parent()->width();
        }
    }
    
    if (width() != target_w || height() != target_h) {
        set_size(target_w, target_h);
    }

    if (m_editor) {
        m_editor->setViewport(pluma::Twips(target_w * 15), pluma::Twips(target_h * 15));
    }
}


int PlumaView::preferred_width() const {
    return 800; // default A4 width in pixels
}

int PlumaView::preferred_height() const {
    return 1120; // default A4 height in pixels
}

int PlumaView::preferred_height(int /*width*/) const {
    return 1120;
}


} // namespace pluma_app

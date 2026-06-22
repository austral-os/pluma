#pragma once

#include <horizon/WaylandWindow.hpp>
#include <horizon/EventsManager.hpp>
#include <horizon/Notebook.hpp>
#include <horizon/TextBox.hpp>
#include <horizon/TextBoxPolicies.hpp>
#include <horizon/Combo.hpp>
#include <string>

namespace pluma_app {
namespace widgets {
class ParagraphPreview;
}

namespace dialogs {

struct ParagraphSelectedEvent : public horizon::EventContext {
    void* sender = nullptr;
    float indent_before = 0.0f;
    float indent_after = 0.0f;
    float indent_first_line = 0.0f;
    float spacing_above = 0.0f;
    float spacing_below = 0.0f;
    float line_spacing = 1.0f;
};

class ParagraphDialog : public horizon::WaylandWindow {
public:
    ParagraphDialog();

    void set_initial_paragraph(float indent_before, float indent_after, float indent_first, float spacing_above, float spacing_below, float line_spacing);

    horizon::EventsManager<ParagraphSelectedEvent> when_accepted;

private:
    horizon::Notebook* m_notebook;
    
    horizon::TextBox<horizon::DoublePolicy>* m_indent_before_box;
    horizon::TextBox<horizon::DoublePolicy>* m_indent_after_box;
    horizon::TextBox<horizon::DoublePolicy>* m_indent_first_box;
    
    horizon::TextBox<horizon::DoublePolicy>* m_spacing_above_box;
    horizon::TextBox<horizon::DoublePolicy>* m_spacing_below_box;
    
    horizon::Combo* m_line_spacing_combo;
    horizon::TextBox<horizon::DoublePolicy>* m_line_spacing_value_box;
    
    widgets::ParagraphPreview* m_preview;

    void update_preview();
    void on_close();
    void populate_combos();
};

} // namespace dialogs
} // namespace pluma_app

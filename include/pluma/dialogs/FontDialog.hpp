#pragma once

#include <horizon/WaylandWindow.hpp>
#include <horizon/Window.hpp>
#include <horizon/Notebook.hpp>
#include <horizon/TextBox.hpp>
#include <horizon/TableView.hpp>
#include <horizon/Combo.hpp>
#include "pluma/widgets/FontPreview.hpp"
#include <string>

namespace pluma_app {
namespace dialogs {

struct FontSelectedEvent : public horizon::EventContext {
    std::string family;
    float size;
    bool bold;
    bool italic;
    std::string language;
};

class FontDialog : public horizon::WaylandWindow {
public:
    FontDialog();

    void set_initial_language(const std::string& lang_id);
    void set_initial_font(const std::string& family, float size, bool bold, bool italic);

    horizon::EventsManager<FontSelectedEvent> when_accepted;

private:
    horizon::Notebook* m_notebook;
    horizon::TextBox<horizon::TextPolicy>* m_search_box;
    horizon::TableView<std::string>* m_font_table;
    horizon::Combo* m_style_combo;
    horizon::Combo* m_size_combo;
    horizon::Combo* m_lang_combo;
    widgets::FontPreview* m_preview;

    void populate_fonts();
    void populate_sizes();
    void populate_styles();
    void populate_languages();
};

} // namespace dialogs
} // namespace pluma_app

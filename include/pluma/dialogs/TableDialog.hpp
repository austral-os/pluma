#pragma once

#include <horizon/WaylandWindow.hpp>
#include <horizon/EventsManager.hpp>
#include <horizon/Notebook.hpp>
#include <horizon/Combo.hpp>
#include <horizon/ColorSelector.hpp>
#include <string>

namespace pluma_app {
namespace Widgets {
class TableBordersPreview;
}

namespace dialogs {

struct TableBordersEvent : public horizon::EventContext {
    void* sender = nullptr;
    bool active_borders[6] = {false};
    int style_index = 0;
    horizon::Color line_color;
    float line_thickness = 1.0f;
};

class TableDialog : public horizon::WaylandWindow {
public:
    TableDialog();

    horizon::EventsManager<TableBordersEvent> when_accepted;

private:
    horizon::Notebook* m_notebook;
    
    // Group button for presets
    horizon::Widget* m_presets_group;
    
    Widgets::TableBordersPreview* m_preview;
    
    horizon::Combo* m_style_combo;
    horizon::ColorSelector* m_color_selector;
    horizon::Combo* m_thickness_combo;
    
    void on_close();
    void populate_combos();
};

} // namespace dialogs
} // namespace pluma_app

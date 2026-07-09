#pragma once

#include <horizon/WaylandWindow.hpp>
#include <horizon/EventsManager.hpp>
#include <horizon/Notebook.hpp>
#include <string>
#include <vector>

namespace pluma_app {
namespace widgets {
class ListStylePreview;
}

namespace dialogs {

struct ListSelectedEvent : public horizon::EventContext {
    void* sender = nullptr;
    enum Action {
        ApplyStyle,
        Remove,
        Reset
    };
    Action action = ApplyStyle;
    bool is_ordered = false;      ///< UL or OL (meaningful for ApplyStyle)
    std::string style;            ///< "disc", "circle", "square", "1", "a", "A", "1)", "a)", "A)"
};

class ListDialog : public horizon::WaylandWindow {
public:
    ListDialog();

    /// Set which list type is currently active so the right tab is shown
    void set_initial_list(bool is_ordered, const std::string& style);

    horizon::EventsManager<ListSelectedEvent> when_accepted;

private:
    horizon::Notebook* m_notebook;

    // Preview pointers for unordered tab
    std::vector<widgets::ListStylePreview*> m_ul_previews;
    // Preview pointers for ordered tab
    std::vector<widgets::ListStylePreview*> m_ol_previews;

    // Currently selected style per tab
    widgets::ListStylePreview* m_selected_ul_preview = nullptr;
    widgets::ListStylePreview* m_selected_ol_preview = nullptr;
    bool m_selected_ordered = false;

    void build_unordered_tab(horizon::Notebook* notebook);
    void build_ordered_tab(horizon::Notebook* notebook);

    void on_preview_click(widgets::ListStylePreview* preview,
                          std::vector<widgets::ListStylePreview*>& group,
                          widgets::ListStylePreview*& selected);

    void on_remove();
    void on_reset();
    void on_ok();
    void on_close();
};

} // namespace dialogs
} // namespace pluma_app

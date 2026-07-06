#pragma once

#include <horizon/WaylandWindow.hpp>
#include <horizon/EventsManager.hpp>
#include <horizon/ScrollArea.hpp>
#include <horizon/TextBox.hpp>
#include <pluma/Editor/CrossRefManager.hpp>
#include <string>
#include <vector>

namespace pluma_app {
namespace dialogs {

struct CrossRefDialogAcceptedContext : public horizon::EventContext {
    // No specific data needed — the dialog writes directly to CrossRefManager
};

class CrossRefDialog : public horizon::WaylandWindow {
public:
    CrossRefDialog();

    void set_cross_ref_manager(pluma::CrossRefManager* mgr);

    horizon::EventsManager<CrossRefDialogAcceptedContext> when_accepted;

private:
    void on_close();
    bool save_and_accept();
    void populate_list();

    pluma::CrossRefManager* m_mgr{nullptr};
    horizon::ScrollArea* m_scroll_area{nullptr};

    /// Pointers to name TextBox widgets (indexed in same order as m_entries)
    std::vector<horizon::TextBox<horizon::TextPolicy>*> m_name_boxes;

    /// Snapshot of entries when the dialog opened, for change detection
    std::vector<pluma::CrossRefTarget> m_entries;
};

} // namespace dialogs
} // namespace pluma_app

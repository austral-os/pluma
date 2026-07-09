#pragma once

#include <horizon/WaylandWindow.hpp>
#include <horizon/EventsManager.hpp>
#include <horizon/Button.hpp>
#include <horizon/Label.hpp>
#include <horizon/SearchBox.hpp>
#include <horizon/TableView.hpp>
#include <pluma/Editor/CrossRefManager.hpp>
#include <string>
#include <vector>

namespace pluma_app {
namespace dialogs {

struct CrossRefDialogAcceptedContext : public horizon::EventContext {
    std::string uuid;
    pluma::CrossRefElementType type{pluma::CrossRefElementType::Image};
    std::string display_name;
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
    void apply_filter();
    void update_selection(const pluma::CrossRefTarget& target);
    void clear_selection();

    pluma::CrossRefManager* m_mgr{nullptr};
    horizon::SearchBox* m_search_box{nullptr};
    horizon::TableView<pluma::CrossRefTarget>* m_table_view{nullptr};
    horizon::Label* m_empty_label{nullptr};
    horizon::Button<horizon::AquaObject>* m_ok_button{nullptr};
    std::string m_selected_uuid;

    /// Snapshot of entries when the dialog opened.
    std::vector<pluma::CrossRefTarget> m_entries;
    std::vector<pluma::CrossRefTarget> m_filtered_entries;
};

} // namespace dialogs
} // namespace pluma_app

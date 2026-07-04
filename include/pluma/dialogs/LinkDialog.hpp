#pragma once

#include <horizon/WaylandWindow.hpp>
#include <horizon/EventsManager.hpp>
#include <horizon/Combo.hpp>
#include <horizon/SearchBox.hpp>
#include <horizon/TableView.hpp>
#include <horizon/TextBox.hpp>
#include <horizon/Label.hpp>
#include <pluma/Editor/BookmarkManager.hpp>
#include <string>
#include <vector>

namespace pluma_app {
namespace dialogs {

struct LinkDialogAcceptedContext : public horizon::EventContext {
    uint32_t start;
    uint32_t length;
    std::string type;
    std::string target;
};

class LinkDialog : public horizon::WaylandWindow {
public:
    LinkDialog();

    void set_bookmarks(const std::vector<pluma::Bookmark>& bookmarks);

    /// Pre-fill for edit mode (or empty for new hyperlink)
    void set_initial_data(uint32_t start, uint32_t length,
                          const std::string& existing_type,
                          const std::string& existing_target);

    horizon::EventsManager<LinkDialogAcceptedContext> when_accepted;

private:
    void build_internal_panel();
    void build_url_panel();
    void build_mail_panel();
    void on_type_changed(const std::string& type_id);
    bool validate_and_accept();
    void on_close();
    void apply_filter();

    horizon::Combo* m_type_combo;
    horizon::Widget* m_stacked_area;

    // Internal mode widgets
    horizon::SearchBox* m_search_box{nullptr};
    horizon::TableView<pluma::Bookmark>* m_table_view{nullptr};
    horizon::Label* m_internal_error{nullptr};
    std::vector<pluma::Bookmark> m_all_bookmarks;

    // URL mode widgets
    horizon::TextBox<horizon::TextPolicy>* m_url_box{nullptr};

    // Mail mode widgets
    horizon::TextBox<horizon::TextPolicy>* m_mail_box{nullptr};

    // Shared validation label
    horizon::Label* m_validation_label{nullptr};

    uint32_t m_start{0};
    uint32_t m_length{0};
};

} // namespace dialogs
} // namespace pluma_app

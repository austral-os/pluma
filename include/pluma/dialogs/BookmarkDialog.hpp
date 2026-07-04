#pragma once

#include <horizon/WaylandWindow.hpp>
#include <horizon/EventsManager.hpp>
#include <horizon/TextBox.hpp>
#include <horizon/Label.hpp>
#include <string>
#include <vector>

namespace pluma_app {
namespace dialogs {

struct BookmarkDialogAcceptedContext : public horizon::EventContext {
    std::string name;
};

class BookmarkDialog : public horizon::WaylandWindow {
public:
    BookmarkDialog();

    void set_existing_bookmarks(const std::vector<std::string>& names);
    void set_cursor_offset(uint32_t offset);

    horizon::EventsManager<BookmarkDialogAcceptedContext> when_accepted;

private:
    bool validate_and_accept();
    void on_close();
    void clear_error();

    horizon::TextBox<horizon::TextPolicy>* m_name_box{nullptr};
    horizon::Label* m_error_label{nullptr};

    std::vector<std::string> m_existing_names;
    uint32_t m_cursor_offset{0};
};

} // namespace dialogs
} // namespace pluma_app

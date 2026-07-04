#include "pluma/dialogs/BookmarkDialog.hpp"
#include <horizon/Button.hpp>
#include <horizon/I18n.hpp>
#include <horizon/Label.hpp>
#include <horizon/Spacer.hpp>
#include <horizon/TextBoxPolicies.hpp>
#include <horizon/Widget.hpp>
#include <horizon/Window.hpp>
#include <algorithm>
#include <cctype>

namespace pluma_app {
namespace dialogs {

BookmarkDialog::BookmarkDialog()
    : horizon::WaylandWindow("pluma.dialog.bookmark", 350, 180, false, false) {
    auto window_widget = std::make_unique<horizon::Window>(
        horizon::i18n().tr("pluma-writer.bookmark_dialog.title"));

    auto content = std::make_unique<horizon::Widget>();
    content->set_layout_type(horizon::WIDGET_LAYOUT_VERTICAL);
    content->set_position_type(horizon::FILL);
    content->set_margin(20);
    content->set_spacing(10);

    // ── Name label ──────────────────────────────────────────────────────
    auto name_label = std::make_unique<horizon::Label>(
        horizon::i18n().tr("pluma-writer.bookmark_dialog.name"));
    name_label->set_fixed_size(24);
    content->add_child(std::move(name_label));

    // ── Name TextBox ─────────────────────────────────────────────────────
    auto name_box = std::make_unique<horizon::TextBox<horizon::TextPolicy>>();
    name_box->set_fixed_size(30);
    m_name_box = name_box.get();
    content->add_child(std::move(name_box));

    // ── Error label ──────────────────────────────────────────────────────
    auto err = std::make_unique<horizon::Label>("");
    err->set_fixed_size(20);
    err->set_text_color(horizon::Color(1.0f, 0.2f, 0.2f));
    err->set_visible(false);
    m_error_label = err.get();
    content->add_child(std::move(err));

    content->add_child(horizon::Spacer());

    // ── Button row ───────────────────────────────────────────────────────
    auto btn_row = std::make_unique<horizon::Widget>();
    btn_row->set_layout_type(horizon::WIDGET_LAYOUT_HORIZONTAL);
    btn_row->set_fixed_size(33);
    btn_row->set_spacing(10);

    btn_row->add_child(horizon::Spacer());

    auto btn_cancel = std::make_unique<horizon::Button<horizon::AquaObject>>();
    btn_cancel->set_text(
        horizon::i18n().tr("pluma-writer.bookmark_dialog.cancel"));
    btn_cancel->set_fixed_size(120);
    btn_cancel->when_click.connect(
        [this](horizon::EventContext&) { this->on_close(); });

    auto btn_ok = std::make_unique<horizon::Button<horizon::AquaObject>>();
    btn_ok->set_text(horizon::i18n().tr("pluma-writer.bookmark_dialog.ok"));
    btn_ok->set_fixed_size(120);
    btn_ok->set_accent_color(horizon::WidgetAccentColor::Primary);
    btn_ok->when_click.connect([this](horizon::EventContext&) {
        this->validate_and_accept();
    });

    btn_row->add_child(std::move(btn_cancel));
    btn_row->add_child(std::move(btn_ok));
    content->add_child(std::move(btn_row));

    window_widget->add_child(std::move(content));
    set_root(std::move(window_widget));
}

void BookmarkDialog::set_existing_bookmarks(
    const std::vector<std::string>& names) {
    m_existing_names = names;
}

void BookmarkDialog::set_cursor_offset(uint32_t offset) {
    m_cursor_offset = offset;
}

bool BookmarkDialog::validate_and_accept() {
    std::string name = m_name_box->text();

    // Trim whitespace
    auto trim_start = name.find_first_not_of(" \t\r\n");
    if (trim_start == std::string::npos) {
        m_error_label->set_text(
            horizon::i18n().tr("pluma-writer.bookmark_dialog.err_empty"));
        m_error_label->set_visible(true);
        m_error_label->invalidate();
        return false;
    }
    name = name.substr(trim_start);
    auto trim_end = name.find_last_not_of(" \t\r\n");
    if (trim_end != std::string::npos) {
        name = name.substr(0, trim_end + 1);
    }

    // Check for spaces
    if (name.find(' ') != std::string::npos) {
        m_error_label->set_text(
            horizon::i18n().tr("pluma-writer.bookmark_dialog.err_spaces"));
        m_error_label->set_visible(true);
        m_error_label->invalidate();
        return false;
    }

    // Check starts with letter or number
    if (!std::isalnum(static_cast<unsigned char>(name[0]))) {
        m_error_label->set_text(
            horizon::i18n().tr("pluma-writer.bookmark_dialog.err_invalid_start"));
        m_error_label->set_visible(true);
        m_error_label->invalidate();
        return false;
    }

    // Check only valid characters: [a-zA-Z0-9_-]
    for (char c : name) {
        if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_' && c != '-') {
            m_error_label->set_text(
                horizon::i18n().tr("pluma-writer.bookmark_dialog.err_invalid_chars"));
            m_error_label->set_visible(true);
            m_error_label->invalidate();
            return false;
        }
    }

    // Check case-insensitive uniqueness
    std::string lower_name = name;
    std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    for (const auto& existing : m_existing_names) {
        std::string lower_existing = existing;
        std::transform(lower_existing.begin(), lower_existing.end(),
                       lower_existing.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        if (lower_name == lower_existing) {
            m_error_label->set_text(
                horizon::i18n().tr("pluma-writer.bookmark_dialog.err_duplicate"));
            m_error_label->set_visible(true);
            m_error_label->invalidate();
            return false;
        }
    }

    m_error_label->set_visible(false);

    BookmarkDialogAcceptedContext ctx;
    ctx.name = name;
    when_accepted.run(ctx);

    this->quit();
    return true;
}

void BookmarkDialog::clear_error() {
    m_error_label->set_visible(false);
}

void BookmarkDialog::on_close() {
    this->quit();
}

} // namespace dialogs
} // namespace pluma_app

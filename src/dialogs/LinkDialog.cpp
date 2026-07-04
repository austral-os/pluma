#include "pluma/dialogs/LinkDialog.hpp"
#include <horizon/Button.hpp>
#include <horizon/I18n.hpp>
#include <horizon/Label.hpp>
#include <horizon/Spacer.hpp>
#include <horizon/TableColumn.hpp>
#include <horizon/TextBoxPolicies.hpp>
#include <horizon/Widget.hpp>
#include <horizon/Window.hpp>
#include <algorithm>
#include <cctype>

namespace pluma_app {
namespace dialogs {

LinkDialog::LinkDialog()
    : horizon::WaylandWindow("pluma.dialog.link", 480, 350, false, false) {
    auto window_widget = std::make_unique<horizon::Window>(
        horizon::i18n().tr("pluma-writer.link_dialog.title"));

    auto content = std::make_unique<horizon::Widget>();
    content->set_layout_type(horizon::WIDGET_LAYOUT_VERTICAL);
    content->set_position_type(horizon::FILL);
    content->set_margin(20);
    content->set_spacing(10);

    // ── Type combo row ───────────────────────────────────────────────────
    auto type_row = std::make_unique<horizon::Widget>();
    type_row->set_layout_type(horizon::WIDGET_LAYOUT_HORIZONTAL);
    type_row->set_fixed_size(30);

    auto type_label = std::make_unique<horizon::Label>(
        horizon::i18n().tr("pluma-writer.link_dialog.type"));
    type_label->set_fixed_size(60);
    type_row->add_child(std::move(type_label));

    auto type_combo = std::make_unique<horizon::Combo>();
    type_combo->set_fixed_size(250);
    type_combo->add_item("internal",
        horizon::i18n().tr("pluma-writer.link_dialog.internal"));
    type_combo->add_item("url",
        horizon::i18n().tr("pluma-writer.link_dialog.url"));
    type_combo->add_item("mail",
        horizon::i18n().tr("pluma-writer.link_dialog.mail"));
    m_type_combo = type_combo.get();
    type_row->add_child(std::move(type_combo));
    type_row->add_child(horizon::Spacer());
    content->add_child(std::move(type_row));

    // ── Stacked widget area ──────────────────────────────────────────────
    auto stacked = std::make_unique<horizon::Widget>();
    stacked->set_layout_type(horizon::WIDGET_LAYOUT_VERTICAL);
    stacked->set_position_type(horizon::FILL);
    stacked->set_spacing(8);
    m_stacked_area = stacked.get();

    // Build all three mode panels; only internal visible initially
    build_internal_panel();
    build_url_panel();
    build_mail_panel();

    content->add_child(std::move(stacked));

    // ── Validation error label ───────────────────────────────────────────
    auto val_label = std::make_unique<horizon::Label>("");
    val_label->set_fixed_size(20);
    val_label->set_text_color(horizon::Color(1.0f, 0.2f, 0.2f));
    m_validation_label = val_label.get();
    content->add_child(std::move(val_label));

    // ── Button row ───────────────────────────────────────────────────────
    auto btn_row = std::make_unique<horizon::Widget>();
    btn_row->set_layout_type(horizon::WIDGET_LAYOUT_HORIZONTAL);
    btn_row->set_fixed_size(33);
    btn_row->set_spacing(10);

    btn_row->add_child(horizon::Spacer());

    auto btn_cancel = std::make_unique<horizon::Button<horizon::AquaObject>>();
    btn_cancel->set_text(
        horizon::i18n().tr("pluma-writer.link_dialog.cancel"));
    btn_cancel->set_fixed_size(120);
    btn_cancel->when_click.connect(
        [this](horizon::EventContext&) { this->on_close(); });

    auto btn_ok = std::make_unique<horizon::Button<horizon::AquaObject>>();
    btn_ok->set_text(horizon::i18n().tr("pluma-writer.link_dialog.ok"));
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

    // ── Connect type combo ───────────────────────────────────────────────
    m_type_combo->when_item_selected.connect(
        [this](horizon::ComboItemSelectedContext& ctx) {
            this->on_type_changed(ctx.item.id);
        });
}

void LinkDialog::build_internal_panel() {
    auto panel = std::make_unique<horizon::Widget>();
    panel->set_layout_type(horizon::WIDGET_LAYOUT_VERTICAL);
    panel->set_position_type(horizon::FILL);
    panel->set_spacing(6);

    // SearchBox
    auto search = std::make_unique<horizon::SearchBox>();
    search->set_placeholder(
        horizon::i18n().tr("pluma-writer.link_dialog.search_placeholder"));
    m_search_box = search.get();
    panel->add_child(std::move(search));

    // TableView for bookmarks
    auto table = std::make_unique<horizon::TableView<pluma::Bookmark>>();
    table->set_width_mode(horizon::TableViewWidthMode::Fill);
    table->set_position_type(horizon::FILL);

    horizon::TableColumn<pluma::Bookmark> col;
    col.id = "name";
    col.title = "";
    col.width = -1;
    col.cell_factory = [](const pluma::Bookmark& bm) {
        return std::make_unique<horizon::Label>(bm.name);
    };
    table->add_column(std::move(col));
    table->set_header_visible(false);
    m_table_view = table.get();
    panel->add_child(std::move(table));

    // Error label for no-bookmarks / no-matches
    auto err = std::make_unique<horizon::Label>("");
    err->set_fixed_size(20);
    err->set_text_color(horizon::Color(1.0f, 0.2f, 0.2f));
    err->set_visible(false);
    m_internal_error = err.get();
    panel->add_child(std::move(err));

    // Search text changed → filter
    m_search_box->when_text_changed.connect([this](horizon::EventContext&) {
        apply_filter();
    });

    panel->set_visible(true); // internal is default
    m_stacked_area->add_child(std::move(panel));
}

void LinkDialog::build_url_panel() {
    auto panel = std::make_unique<horizon::Widget>();
    panel->set_layout_type(horizon::WIDGET_LAYOUT_VERTICAL);
    panel->set_position_type(horizon::FILL);
    panel->set_spacing(8);

    auto url_label = std::make_unique<horizon::Label>(
        horizon::i18n().tr("pluma-writer.link_dialog.url_label"));
    url_label->set_fixed_size(24);
    panel->add_child(std::move(url_label));

    auto url_box = std::make_unique<horizon::TextBox<horizon::TextPolicy>>();
    url_box->set_placeholder(
        horizon::i18n().tr("pluma-writer.link_dialog.url_placeholder"));
    m_url_box = url_box.get();
    panel->add_child(std::move(url_box));

    panel->set_visible(false);
    m_stacked_area->add_child(std::move(panel));
}

void LinkDialog::build_mail_panel() {
    auto panel = std::make_unique<horizon::Widget>();
    panel->set_layout_type(horizon::WIDGET_LAYOUT_VERTICAL);
    panel->set_position_type(horizon::FILL);
    panel->set_spacing(8);

    auto mail_label = std::make_unique<horizon::Label>(
        horizon::i18n().tr("pluma-writer.link_dialog.mail_label"));
    mail_label->set_fixed_size(24);
    panel->add_child(std::move(mail_label));

    auto mail_box = std::make_unique<horizon::TextBox<horizon::TextPolicy>>();
    mail_box->set_placeholder(
        horizon::i18n().tr("pluma-writer.link_dialog.mail_placeholder"));
    m_mail_box = mail_box.get();
    panel->add_child(std::move(mail_box));

    panel->set_visible(false);
    m_stacked_area->add_child(std::move(panel));
}

void LinkDialog::set_bookmarks(const std::vector<pluma::Bookmark>& bookmarks) {
    m_all_bookmarks = bookmarks;

    if (bookmarks.empty()) {
        m_table_view->set_visible(false);
        m_internal_error->set_text(
            horizon::i18n().tr("pluma-writer.link_dialog.no_bookmarks"));
        m_internal_error->set_visible(true);
    } else {
        m_table_view->set_data(bookmarks);
        m_table_view->set_visible(true);
        if (!bookmarks.empty()) {
            m_table_view->set_selected_index(0);
        }
        m_internal_error->set_visible(false);
    }
}

void LinkDialog::set_initial_data(uint32_t start, uint32_t length,
                                   const std::string& existing_type,
                                   const std::string& existing_target) {
    m_start = start;
    m_length = length;

    if (!existing_type.empty()) {
        // Edit mode: select the correct type and pre-fill
        m_type_combo->set_selected_item_by_id(existing_type);
        on_type_changed(existing_type);

        if (existing_type == "url" && m_url_box) {
            m_url_box->set_text(existing_target);
        } else if (existing_type == "mail" && m_mail_box) {
            m_mail_box->set_text(existing_target);
        }
        // For internal, the user re-selects from the list
    }
}

void LinkDialog::on_type_changed(const std::string& type_id) {
    // Hide all panels
    for (auto& child : m_stacked_area->children()) {
        child->set_visible(false);
    }

    // Show the selected panel
    // children order: [0]=internal, [1]=url, [2]=mail
    if (type_id == "internal" && m_stacked_area->children().size() > 0) {
        m_stacked_area->children()[0]->set_visible(true);
    } else if (type_id == "url" && m_stacked_area->children().size() > 1) {
        m_stacked_area->children()[1]->set_visible(true);
    } else if (type_id == "mail" && m_stacked_area->children().size() > 2) {
        m_stacked_area->children()[2]->set_visible(true);
    }

    m_validation_label->set_text("");
    m_stacked_area->calculate_layout();
    m_stacked_area->invalidate();
}

void LinkDialog::apply_filter() {
    if (m_all_bookmarks.empty()) return;

    std::string query = m_search_box->text();
    std::transform(query.begin(), query.end(), query.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    if (query.empty()) {
        m_table_view->set_data(m_all_bookmarks);
        m_table_view->set_visible(true);
        m_internal_error->set_visible(false);
        return;
    }

    std::vector<pluma::Bookmark> filtered;
    for (const auto& bm : m_all_bookmarks) {
        std::string name = bm.name;
        std::transform(name.begin(), name.end(), name.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        if (name.find(query) != std::string::npos) {
            filtered.push_back(bm);
        }
    }

    if (filtered.empty()) {
        m_internal_error->set_text(
            horizon::i18n().tr("pluma-writer.link_dialog.no_matches"));
        m_internal_error->set_visible(true);
        m_table_view->set_visible(false);
    } else {
        m_table_view->set_data(filtered);
        m_table_view->set_selected_index(0);
        m_table_view->set_visible(true);
        m_internal_error->set_visible(false);
    }
}

bool LinkDialog::validate_and_accept() {
    std::string type = m_type_combo->selected_item()
                           ? m_type_combo->selected_item()->id
                           : "internal";
    std::string target;

    if (type == "internal") {
        int sel = m_table_view->selected_index();
        if (sel < 0 || sel >= (int)m_table_view->data().size()) {
            m_validation_label->set_text(horizon::i18n().tr(
                "pluma-writer.link_dialog.err_no_bookmark_selected"));
            m_validation_label->invalidate();
            return false;
        }
        target = m_table_view->data()[sel].name;
    } else if (type == "url") {
        target = m_url_box->text();
        if (target.compare(0, 7, "http://") != 0 &&
            target.compare(0, 8, "https://") != 0) {
            m_validation_label->set_text(horizon::i18n().tr(
                "pluma-writer.link_dialog.err_invalid_url"));
            m_validation_label->invalidate();
            return false;
        }
    } else if (type == "mail") {
        target = m_mail_box->text();
        // Basic email validation: contains @ and has domain part
        auto at = target.find('@');
        if (at == std::string::npos || at == 0 ||
            at == target.size() - 1) {
            m_validation_label->set_text(horizon::i18n().tr(
                "pluma-writer.link_dialog.err_invalid_email"));
            m_validation_label->invalidate();
            return false;
        }
    }

    m_validation_label->set_text("");

    LinkDialogAcceptedContext ctx;
    ctx.start = m_start;
    ctx.length = m_length;
    ctx.type = type;
    ctx.target = target;
    when_accepted.run(ctx);

    this->quit();
    return true;
}

void LinkDialog::on_close() {
    this->quit();
}

} // namespace dialogs
} // namespace pluma_app

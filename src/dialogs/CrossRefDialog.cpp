#include "pluma/dialogs/CrossRefDialog.hpp"
#include <horizon/Button.hpp>
#include <horizon/I18n.hpp>
#include <horizon/Label.hpp>
#include <horizon/SearchBox.hpp>
#include <horizon/Spacer.hpp>
#include <horizon/TableColumn.hpp>
#include <horizon/TextBoxPolicies.hpp>
#include <horizon/Widget.hpp>
#include <horizon/Window.hpp>
#include <algorithm>
#include <cctype>
#include <string>

namespace pluma_app {
namespace dialogs {

CrossRefDialog::CrossRefDialog()
    : horizon::WaylandWindow("pluma.dialog.crossref", 520, 380, false, false) {
    auto window_widget = std::make_unique<horizon::Window>(
        horizon::i18n().tr("pluma-writer.crossref_dialog.title"));

    auto content = std::make_unique<horizon::Widget>();
    content->set_layout_type(horizon::WIDGET_LAYOUT_VERTICAL);
    content->set_position_type(horizon::FILL);
    content->set_margin(15);
    content->set_spacing(8);

    // ── Header label ──────────────────────────────────────────────────────
    auto header = std::make_unique<horizon::Label>(
        horizon::i18n().tr("pluma-writer.crossref_dialog.header"));
    header->set_fixed_size(24);
    content->add_child(std::move(header));

    auto search = std::make_unique<horizon::SearchBox>();
    search->set_placeholder(horizon::i18n().tr("pluma-writer.crossref_dialog.search_placeholder"));
    search->set_fixed_size(34);
    m_search_box = search.get();
    content->add_child(std::move(search));

    auto table = std::make_unique<horizon::TableView<pluma::CrossRefTarget>>();
    table->set_width_mode(horizon::TableViewWidthMode::Fill);
    table->set_position_type(horizon::FILL);

    horizon::TableColumn<pluma::CrossRefTarget> type_col;
    type_col.id = "type";
    type_col.title = horizon::i18n().tr("pluma-writer.crossref_dialog.type");
    type_col.width = 90;
    type_col.cell_factory = [](const pluma::CrossRefTarget& target) {
        const std::string type_text =
            (target.type == pluma::CrossRefElementType::Image)
                ? horizon::i18n().tr("pluma-writer.crossref_dialog.type_image")
                : horizon::i18n().tr("pluma-writer.crossref_dialog.type_table");
        return std::make_unique<horizon::Label>(type_text);
    };
    table->add_column(std::move(type_col));

    horizon::TableColumn<pluma::CrossRefTarget> name_col;
    name_col.id = "display_name";
    name_col.title = horizon::i18n().tr("pluma-writer.crossref_dialog.name");
    name_col.width = -1;
    name_col.cell_factory = [](const pluma::CrossRefTarget& target) {
        return std::make_unique<horizon::Label>(target.display_name);
    };
    table->add_column(std::move(name_col));

    horizon::TableColumn<pluma::CrossRefTarget> uuid_col;
    uuid_col.id = "uuid";
    uuid_col.title = horizon::i18n().tr("pluma-writer.crossref_dialog.uuid");
    uuid_col.width = 180;
    uuid_col.cell_factory = [](const pluma::CrossRefTarget& target) {
        auto label = std::make_unique<horizon::Label>(target.uuid);
        label->set_text_color(horizon::Color(0.5f, 0.5f, 0.5f));
        return label;
    };
    table->add_column(std::move(uuid_col));
    m_table_view = table.get();
    content->add_child(std::move(table));

    auto empty_label = std::make_unique<horizon::Label>("");
    empty_label->set_fixed_size(22);
    empty_label->set_text_color(horizon::Color(1.0f, 0.2f, 0.2f));
    empty_label->set_visible(false);
    m_empty_label = empty_label.get();
    content->add_child(std::move(empty_label));

    // ── Button row ────────────────────────────────────────────────────────
    auto btn_row = std::make_unique<horizon::Widget>();
    btn_row->set_layout_type(horizon::WIDGET_LAYOUT_HORIZONTAL);
    btn_row->set_fixed_size(33);
    btn_row->set_spacing(10);

    btn_row->add_child(horizon::Spacer());

    auto btn_cancel = std::make_unique<horizon::Button<horizon::AquaObject>>();
    btn_cancel->set_text(
        horizon::i18n().tr("pluma-writer.crossref_dialog.cancel"));
    btn_cancel->set_fixed_size(120);
    btn_cancel->when_click.connect(
        [this](horizon::EventContext&) { this->on_close(); });

    auto btn_ok = std::make_unique<horizon::Button<horizon::AquaObject>>();
    btn_ok->set_text(horizon::i18n().tr("pluma-writer.crossref_dialog.ok"));
    btn_ok->set_fixed_size(120);
    btn_ok->set_accent_color(horizon::WidgetAccentColor::Primary);
    btn_ok->set_enabled(false);
    m_ok_button = btn_ok.get();
    btn_ok->when_click.connect([this](horizon::EventContext&) {
        this->save_and_accept();
    });

    btn_row->add_child(std::move(btn_cancel));
    btn_row->add_child(std::move(btn_ok));
    content->add_child(std::move(btn_row));

    window_widget->add_child(std::move(content));
    set_root(std::move(window_widget));

    m_search_box->when_text_changed.connect([this](horizon::EventContext&) {
        this->apply_filter();
    });

    m_table_view->when_row_click.connect([this](horizon::EventContext& ctx) {
        auto& ev = static_cast<horizon::TableViewRowMouseClickContext<pluma::CrossRefTarget>&>(ctx);
        this->update_selection(ev.row_data);
    });
}

void CrossRefDialog::set_cross_ref_manager(pluma::CrossRefManager* mgr) {
    m_mgr = mgr;
    populate_list();
}

void CrossRefDialog::populate_list() {
    if (!m_mgr) return;

    m_entries = m_mgr->getAll();
    m_filtered_entries = m_entries;
    m_selected_uuid.clear();
    if (m_ok_button) m_ok_button->set_enabled(false);

    apply_filter();
}

void CrossRefDialog::apply_filter() {
    if (!m_mgr || !m_table_view || !m_empty_label) return;

    std::string query = m_search_box ? m_search_box->text() : "";
    std::transform(query.begin(), query.end(), query.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    m_filtered_entries.clear();
    for (const auto& entry : m_entries) {
        std::string type_str = (entry.type == pluma::CrossRefElementType::Image)
            ? horizon::i18n().tr("pluma-writer.crossref_dialog.type_image")
            : horizon::i18n().tr("pluma-writer.crossref_dialog.type_table");
        std::string haystack = type_str + " " + entry.display_name + " " + entry.uuid;
        std::transform(haystack.begin(), haystack.end(), haystack.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        if (query.empty() || haystack.find(query) != std::string::npos) {
            m_filtered_entries.push_back(entry);
        }
    }

    if (std::none_of(m_filtered_entries.begin(), m_filtered_entries.end(),
                     [this](const auto& entry) { return entry.uuid == m_selected_uuid; })) {
        clear_selection();
    }

    if (m_entries.empty()) {
        m_table_view->set_data({});
        m_table_view->set_visible(false);
        m_empty_label->set_text(horizon::i18n().tr("pluma-writer.crossref_dialog.no_entries"));
        m_empty_label->set_visible(true);
    } else if (m_filtered_entries.empty()) {
        m_table_view->set_data({});
        m_table_view->set_visible(false);
        m_empty_label->set_text(horizon::i18n().tr("pluma-writer.crossref_dialog.no_matches"));
        m_empty_label->set_visible(true);
    } else {
        m_table_view->set_data(m_filtered_entries);
        m_table_view->set_visible(true);
        m_empty_label->set_visible(false);

        auto selected_it = std::find_if(
            m_filtered_entries.begin(), m_filtered_entries.end(),
            [this](const auto& entry) { return entry.uuid == m_selected_uuid; });
        if (selected_it != m_filtered_entries.end()) {
            m_table_view->set_selected_index(
                static_cast<int>(std::distance(m_filtered_entries.begin(), selected_it)));
        }
    }
}

void CrossRefDialog::update_selection(const pluma::CrossRefTarget& target) {
    m_selected_uuid = target.uuid;
    if (m_ok_button) m_ok_button->set_enabled(true);
}

void CrossRefDialog::clear_selection() {
    m_selected_uuid.clear();
    if (m_ok_button) m_ok_button->set_enabled(false);
}

bool CrossRefDialog::save_and_accept() {
    if (!m_mgr) {
        this->quit();
        return false;
    }

    std::optional<pluma::CrossRefTarget> selected;
    if (!m_selected_uuid.empty()) {
        selected = m_mgr->findByUUID(m_selected_uuid);
    }

    if (!selected.has_value() && m_table_view) {
        int selected_index = m_table_view->selected_index();
        const auto& rows = m_table_view->data();
        if (selected_index >= 0 && selected_index < static_cast<int>(rows.size())) {
            selected = rows[static_cast<size_t>(selected_index)];
        }
    }

    if (!selected.has_value()) return false;

    CrossRefDialogAcceptedContext ctx;
    ctx.uuid = selected->uuid;
    ctx.type = selected->type;
    ctx.display_name = selected->display_name;
    when_accepted.run(ctx);

    this->quit();
    return true;
}

void CrossRefDialog::on_close() {
    this->quit();
}

} // namespace dialogs
} // namespace pluma_app

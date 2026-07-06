#include "pluma/dialogs/CrossRefDialog.hpp"
#include <horizon/Button.hpp>
#include <horizon/I18n.hpp>
#include <horizon/Label.hpp>
#include <horizon/Spacer.hpp>
#include <horizon/TextBoxPolicies.hpp>
#include <horizon/Widget.hpp>
#include <horizon/Window.hpp>
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

    // ── Scroll area for the element list ──────────────────────────────────
    auto scroll = std::make_unique<horizon::ScrollArea>();
    scroll->set_position_type(horizon::FILL);
    m_scroll_area = scroll.get();
    content->add_child(std::move(scroll));

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
    btn_ok->when_click.connect([this](horizon::EventContext&) {
        this->save_and_accept();
    });

    btn_row->add_child(std::move(btn_cancel));
    btn_row->add_child(std::move(btn_ok));
    content->add_child(std::move(btn_row));

    window_widget->add_child(std::move(content));
    set_root(std::move(window_widget));
}

void CrossRefDialog::set_cross_ref_manager(pluma::CrossRefManager* mgr) {
    m_mgr = mgr;
    populate_list();
}

void CrossRefDialog::populate_list() {
    if (!m_mgr) return;

    m_name_boxes.clear();
    m_entries = m_mgr->getAll();

    auto container = std::make_unique<horizon::Widget>();
    container->set_layout_type(horizon::WIDGET_LAYOUT_VERTICAL);
    container->set_spacing(4);
    container->set_margin(4);

    if (m_entries.empty()) {
        auto empty_label = std::make_unique<horizon::Label>(
            horizon::i18n().tr("pluma-writer.crossref_dialog.no_entries"));
        empty_label->set_fixed_size(24);
        container->add_child(std::move(empty_label));
    } else {
        for (const auto& entry : m_entries) {
            auto row = std::make_unique<horizon::Widget>();
            row->set_layout_type(horizon::WIDGET_LAYOUT_HORIZONTAL);
            row->set_fixed_size(30);
            row->set_spacing(6);

            // ── Type label ────────────────────────────────────────────────
            std::string type_str =
                (entry.type == pluma::CrossRefElementType::Image)
                    ? horizon::i18n().tr(
                          "pluma-writer.crossref_dialog.type_image")
                    : horizon::i18n().tr(
                          "pluma-writer.crossref_dialog.type_table");
            auto type_label = std::make_unique<horizon::Label>(type_str);
            type_label->set_fixed_size(70);
            row->add_child(std::move(type_label));

            // ── Editable display name ─────────────────────────────────────
            auto name_box =
                std::make_unique<horizon::TextBox<horizon::TextPolicy>>();
            name_box->set_text(entry.display_name);
            name_box->set_position_type(horizon::FILL);
            m_name_boxes.push_back(name_box.get());
            row->add_child(std::move(name_box));

            // ── UUID label (read-only, darker text) ───────────────────────
            auto uuid_label = std::make_unique<horizon::Label>(entry.uuid);
            uuid_label->set_fixed_size(180);
            uuid_label->set_text_color(horizon::Color(0.5f, 0.5f, 0.5f));
            row->add_child(std::move(uuid_label));

            container->add_child(std::move(row));
        }
    }

    m_scroll_area->set_content(std::move(container));
}

bool CrossRefDialog::save_and_accept() {
    if (!m_mgr) {
        this->quit();
        return false;
    }

    // Save changed display names back to CrossRefManager
    for (size_t i = 0; i < m_entries.size() && i < m_name_boxes.size(); ++i) {
        std::string new_name = m_name_boxes[i]->text();

        // Trim whitespace
        auto trim_start = new_name.find_first_not_of(" \t\r\n");
        if (trim_start != std::string::npos) {
            new_name = new_name.substr(trim_start);
        }
        auto trim_end = new_name.find_last_not_of(" \t\r\n");
        if (trim_end != std::string::npos) {
            new_name = new_name.substr(0, trim_end + 1);
        }

        // Only save if the name actually changed
        if (!new_name.empty() && new_name != m_entries[i].display_name) {
            m_mgr->setDisplayName(m_entries[i].uuid, new_name);
        }
    }

    CrossRefDialogAcceptedContext ctx;
    when_accepted.run(ctx);

    this->quit();
    return true;
}

void CrossRefDialog::on_close() {
    this->quit();
}

} // namespace dialogs
} // namespace pluma_app

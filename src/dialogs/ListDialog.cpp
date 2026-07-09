#include "pluma/dialogs/ListDialog.hpp"
#include <horizon/Button.hpp>
#include <horizon/I18n.hpp>
#include <horizon/Label.hpp>
#include <horizon/Spacer.hpp>
#include <horizon/Widget.hpp>
#include <horizon/Window.hpp>
#include <pluma/Widgets/ListStylePreview.hpp>

namespace pluma_app {
namespace dialogs {

ListDialog::ListDialog()
    : horizon::WaylandWindow("pluma.dialog.list", 720, 420, false, false)
{
    auto window_widget = std::make_unique<horizon::Window>(
        horizon::i18n().tr("pluma-writer.list_dialog.title"));

    auto content = std::make_unique<horizon::Widget>();
    content->set_layout_type(horizon::WIDGET_LAYOUT_VERTICAL);
    content->set_position_type(horizon::FILL);
    content->set_margin(10);
    content->set_spacing(10);

    auto notebook = std::make_unique<horizon::Notebook>();
    m_notebook = notebook.get();

    build_unordered_tab(m_notebook);
    build_ordered_tab(m_notebook);

    content->add_child(std::move(notebook));

    // --- Buttons ---
    auto btn_row = std::make_unique<horizon::Widget>();
    btn_row->set_layout_type(horizon::WIDGET_LAYOUT_HORIZONTAL);
    btn_row->set_fixed_size(33);
    btn_row->set_spacing(10);
    btn_row->add_child(horizon::Spacer());

    // Remove button
    auto btn_remove = std::make_unique<horizon::Button<horizon::AquaObject>>();
    btn_remove->set_text(horizon::i18n().tr("pluma-writer.list_dialog.remove"));
    btn_remove->set_fixed_size(120);
    btn_remove->when_click.connect(
        [this](horizon::EventContext&) { this->on_remove(); });
    btn_row->add_child(std::move(btn_remove));

    // Reset button
    auto btn_reset = std::make_unique<horizon::Button<horizon::AquaObject>>();
    btn_reset->set_text(horizon::i18n().tr("pluma-writer.list_dialog.reset"));
    btn_reset->set_fixed_size(120);
    btn_reset->when_click.connect(
        [this](horizon::EventContext&) { this->on_reset(); });
    btn_row->add_child(std::move(btn_reset));

    // Cancel button
    auto btn_cancel = std::make_unique<horizon::Button<horizon::AquaObject>>();
    btn_cancel->set_text(horizon::i18n().tr("pluma-writer.font_dialog.cancel"));
    btn_cancel->set_fixed_size(120);
    btn_cancel->when_click.connect(
        [this](horizon::EventContext&) { this->on_close(); });
    btn_row->add_child(std::move(btn_cancel));

    // OK button
    auto btn_ok = std::make_unique<horizon::Button<horizon::AquaObject>>();
    btn_ok->set_text(horizon::i18n().tr("pluma-writer.font_dialog.ok"));
    btn_ok->set_fixed_size(120);
    btn_ok->set_accent_color(horizon::WidgetAccentColor::Primary);
    btn_ok->when_click.connect(
        [this](horizon::EventContext&) { this->on_ok(); });
    btn_row->add_child(std::move(btn_ok));

    content->add_child(std::move(btn_row));

    window_widget->add_child(std::move(content));
    set_root(std::move(window_widget));
}

void ListDialog::build_unordered_tab(horizon::Notebook* notebook) {
    auto tab = std::make_unique<horizon::Widget>();
    tab->set_layout_type(horizon::WIDGET_LAYOUT_VERTICAL);
    tab->set_position_type(horizon::FILL);
    tab->set_margin(10);
    tab->set_spacing(10);

    auto header = std::make_unique<horizon::Label>(
        horizon::i18n().tr("pluma-writer.list_dialog.select_bullet"));
    header->set_fixed_size(25);
    tab->add_child(std::move(header));

    // UL style cards in a horizontal row
    auto card_row = std::make_unique<horizon::Widget>();
    card_row->set_layout_type(horizon::WIDGET_LAYOUT_HORIZONTAL);
    card_row->set_fixed_size(92);
    card_row->set_spacing(10);

    const char* ul_styles[] = {"disc", "circle", "square"};
    for (const char* s : ul_styles) {
        auto preview = std::make_unique<widgets::ListStylePreview>();
        preview->set_list_style(false, s);
        widgets::ListStylePreview* raw = preview.get();
        m_ul_previews.push_back(raw);

        preview->when_click.connect(
            [this, raw](horizon::EventContext&) {
                this->on_preview_click(raw, m_ul_previews, m_selected_ul_preview);
            });

        card_row->add_child(std::move(preview));
    }

    tab->add_child(std::move(card_row));
    tab->add_child(horizon::Spacer());

    notebook->add_tab(horizon::NotebookPage(
        horizon::i18n().tr("pluma-writer.list_dialog.unordered"),
        std::move(tab)));
}

void ListDialog::build_ordered_tab(horizon::Notebook* notebook) {
    auto tab = std::make_unique<horizon::Widget>();
    tab->set_layout_type(horizon::WIDGET_LAYOUT_VERTICAL);
    tab->set_position_type(horizon::FILL);
    tab->set_margin(10);
    tab->set_spacing(10);

    auto header = std::make_unique<horizon::Label>(
        horizon::i18n().tr("pluma-writer.list_dialog.select_numbering"));
    header->set_fixed_size(25);
    tab->add_child(std::move(header));

    auto card_row = std::make_unique<horizon::Widget>();
    card_row->set_layout_type(horizon::WIDGET_LAYOUT_HORIZONTAL);
    card_row->set_fixed_size(92);
    card_row->set_spacing(10);

    const char* ol_styles[] = {"1", "a", "A", "1)", "a)", "A)"};
    for (const char* s : ol_styles) {
        auto preview = std::make_unique<widgets::ListStylePreview>();
        preview->set_list_style(true, s);
        widgets::ListStylePreview* raw = preview.get();
        m_ol_previews.push_back(raw);

        preview->when_click.connect(
            [this, raw](horizon::EventContext&) {
                this->on_preview_click(raw, m_ol_previews, m_selected_ol_preview);
            });

        card_row->add_child(std::move(preview));
    }

    tab->add_child(std::move(card_row));
    tab->add_child(horizon::Spacer());

    notebook->add_tab(horizon::NotebookPage(
        horizon::i18n().tr("pluma-writer.list_dialog.ordered"),
        std::move(tab)));
}

void ListDialog::on_preview_click(widgets::ListStylePreview* preview,
                                   std::vector<widgets::ListStylePreview*>& group,
                                   widgets::ListStylePreview*& selected) {
    // Deselect previous
    if (selected) {
        selected->set_selected(false);
    }
    // Select new
    selected = preview;
    selected->set_selected(true);
    m_selected_ordered = (preview->is_ordered());
}

void ListDialog::set_initial_list(bool is_ordered, const std::string& style) {
    if (is_ordered) {
        m_notebook->set_current_tab(1);
        for (auto* p : m_ol_previews) {
            if (p->style() == style) {
                on_preview_click(p, m_ol_previews, m_selected_ol_preview);
                break;
            }
        }
        if (!m_selected_ol_preview && !m_ol_previews.empty()) {
            on_preview_click(m_ol_previews[0], m_ol_previews, m_selected_ol_preview);
        }
    } else {
        m_notebook->set_current_tab(0);
        for (auto* p : m_ul_previews) {
            if (p->style() == style) {
                on_preview_click(p, m_ul_previews, m_selected_ul_preview);
                break;
            }
        }
        if (!m_selected_ul_preview && !m_ul_previews.empty()) {
            on_preview_click(m_ul_previews[0], m_ul_previews, m_selected_ul_preview);
        }
    }
}

void ListDialog::on_remove() {
    ListSelectedEvent ev;
    ev.sender = this;
    ev.action = ListSelectedEvent::Remove;
    when_accepted.run(ev);
    on_close();
}

void ListDialog::on_reset() {
    ListSelectedEvent ev;
    ev.sender = this;
    ev.action = ListSelectedEvent::Reset;
    when_accepted.run(ev);
    on_close();
}

void ListDialog::on_ok() {
    ListSelectedEvent ev;
    ev.sender = this;

    if (!m_selected_ordered && m_selected_ul_preview) {
        ev.action = ListSelectedEvent::ApplyStyle;
        ev.is_ordered = false;
        ev.style = m_selected_ul_preview->style();
    } else if (m_selected_ordered && m_selected_ol_preview) {
        ev.action = ListSelectedEvent::ApplyStyle;
        ev.is_ordered = true;
        ev.style = m_selected_ol_preview->style();
    } else {
        // No selection — just close
        on_close();
        return;
    }

    when_accepted.run(ev);
    on_close();
}

void ListDialog::on_close() { this->quit(); }

} // namespace dialogs
} // namespace pluma_app
